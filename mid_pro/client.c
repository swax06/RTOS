// #include "opencv2/opencv.hpp"
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
bool inCall = false, k = false, tryConnect = false;
int myRead(int sd, char *buff) {
	int i = 0;
	char c;
	while(i < 256){
		read(sd, &c, 1);
		buff[i] = c;
		i++;
		if(c == '\0'){	
			return 0;
		}
	}
	buff[255] = '\0';
	return 0;
}
void handle_sigint(int sig) { 
    printf("Caught signal %d\n", sig); 
	if(sig == 2 && !inCall){
		write(sd, "-exit\0", 6);
		close(sd);
		exit(0);
	}
	if(sig == 2 && inCall){
		write(sd, "-call end req\0", 256);
	}
} 

void inpCall() {
	pa_simple *s_read;
	char buff[256];
	int error;
	while(!inCall && tryConnect);
	// printf("initializing mic\n");
	if (!(s_read = pa_simple_new(NULL, "VoIP" , PA_STREAM_RECORD , NULL, "record", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
    }
	while(inCall){
		if(pa_simple_read(s_read,buff,sizeof(buff),&error)<0){
			fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
		}
		write(sd, buff, sizeof(buff));
	}
}

void outCall() {
	char buff[256];
	int error;
	pa_simple *s_write;
	while(!inCall);
	// printf("initializing speakers\n");
	if (!(s_write = pa_simple_new(NULL, "VoIP", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
    }
	while(inCall){
		read(sd, buff, sizeof(buff));
		if(pa_simple_write(s_write,buff,sizeof(buff),&error)<0){
			fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
		}
		if(strcmp(buff, "-call ended") == 0) {
			inCall = false;
		}
	}
	printf("ending call.......\n");
	while(strcmp(buff, "done\0") != 0){
		myRead(sd, buff);
	}
	printf("ended\n");
}
void* reader(void *inp) {
	char buff[257];
	while(1){
		myRead(sd, buff);
		printf("%s\n", buff);
		if(strcmp(buff, "-connecting") == 0) {
			tryConnect = false;
			inCall = true;
			outCall();
		}

		if(strcmp(buff, "-incoming call") == 0){
			printf("enter -yes to start\nenter -no to stop\n");
		}

		if(strcmp(buff, "-call ended") == 0){
			inCall = false;
		}
		
		if(strcmp(buff, "-server_c") == 0){
			close(sd);
			exit(0);
		}
	}
}
int main(int argc, char **argv){
	signal(SIGINT, handle_sigint); 
	struct sockaddr_in server;
	int msgLength;
	char buff[256];
	char result;
	pthread_t thread_id;

	//connection establishment
	sd = socket(AF_INET,SOCK_STREAM,0);
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=inet_addr(argv[1]);
	server.sin_port=htons(atoi(argv[2]));
	connect(sd,(struct sockaddr *)&server,sizeof(server));
	printf("Enter your name:\n");
	pthread_create(&thread_id, NULL, reader, &sd);
	
	while(1) {
		// fflush(stdin); 
		scanf("%255[^\n]", buff); 
		getchar();
		write(sd, buff, strlen(buff));
		write(sd, "\0", 1);
		if(strcmp(buff, "-exit") == 0){
			close(sd);
			break;
		}
		if(strcmp(buff, "-yes") == 0){
			k = true;
		}
		if(k){
			k = false;
			inpCall();
			fflush(stdout); 
		}
		if(strcmp(buff, "-call") == 0) {
			tryConnect = true;
			k = true;
		}
	}
	return 0;
}
