#include<stdio.h>
#include<stdlib.h>
#include<signal.h> 
#include<stdbool.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<time.h>
#include<pulse/pulseaudio.h>
#include<pulse/simple.h>
#include<pulse/error.h>

static const pa_sample_spec ss = {
	.format = PA_SAMPLE_S16LE,
	.rate = 44100,
	.channels = 2
};
int sd;
bool inCall = false, k = false;
int myRead(int sd, char *buff) {
	int i = 0;
	char c;
	while(1){
		read(sd, &c, 1);
		buff[i] = c;
		i++;
		if(c == '\0'){	
			break;
		}
	}
	return 0;
}
void handle_sigint(int sig) { 
    printf("Caught signal %d\n", sig); 
	if(sig == 2 && !inCall){
		write(sd,"-exit\0",6);
		close(sd);
		exit(0);
	}
	if(sig == 2 && inCall){
		inCall = false;
	}
} 

void inpCall() {
	pa_simple *s_read;
	char buff12[256];
	int error;
	printf("initializing mic\n");
	if (!(s_read = pa_simple_new(NULL, "VoIP" , PA_STREAM_RECORD , NULL, "record", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
    }
	while(!inCall);
	while(inCall){
		if(pa_simple_read(s_read,buff12,sizeof(buff12),&error)<0){
			fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
		}
		write(sd, buff12, sizeof(buff12));
	}
	
}

void outCall() {
	char buff12[256];
	int error;
	pa_simple *s_write;
	printf("initializing speakers\n");
	if (!(s_write = pa_simple_new(NULL, "VoIP" , PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
    }
	while(!inCall);
	while(inCall){
		read(sd, buff12, sizeof(buff12));
		if(pa_simple_write(s_write,buff12,sizeof(buff12),&error)<0){
			fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
		}
		if(strcmp(buff12, "-call ended") == 0) {
			inCall = false;
			return;
		}
	}
	char temp1[256] = "-call ended\0";
	write(sd,temp1,256);
}
void* reader(void* input) {
	char buff[1024];
	while(1){
		myRead(sd, buff);
		printf("%s\n",buff);
		if(strcmp(buff, "-call connected") == 0) {
			inCall = true;
			inpCall();
		}
		if(strcmp(buff, "-incoming call") == 0){
			printf("call incoming\nenter -yes to start\nenter -no to stop\n");
		}
		if(strcmp(buff, "-call ended") == 0){
			inCall = false;
		}
	}
}
int main(int argc, char **argv){
	signal(SIGINT, handle_sigint); 
	struct sockaddr_in server;
	int msgLength;
	char buff[1000];
	char result;
	pthread_t thread_id;

	//connection establishment
	sd = socket(AF_INET,SOCK_STREAM,0);
	int *p = &sd;
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=inet_addr(/*"127.0.0.1"*/argv[1]); //same machine
	server.sin_port=htons(atoi(argv[2])/*5555*/);
	connect(sd,(struct sockaddr *)&server,sizeof(server));
	printf("Enter your name:\n");
	pthread_create(&thread_id, NULL, reader, p);
	
	while(1) {
		gets(buff); 
		write(sd,buff,strlen(buff));
		write(sd,"\0",1);
		if(strcmp(buff, "-exit") == 0){
			close(sd);
			break;
		}
		if(strcmp(buff, "-yes") == 0){
			//inCall = true;
			outCall();
		}
		if(k){
			k = false;
			outCall();
		}
		if(strcmp(buff, "-call") == 0) {
			k = true;
		}
	}
	return 0;
}
