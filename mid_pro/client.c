#include<stdio.h>
#include<stdlib.h>
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

void* reader(void* input) {
	int *pt = (int*) input;
	char buff[50];
	while(1){
		myRead(*pt, buff);
		printf("%s\n",buff);
	}
}
int main(int argc, char **argv){
	static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
    };

	pa_simple *s;
	int error;
	
	if (!(s = pa_simple_new(NULL, "VoIP" , PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        return 34;
    }

	unsigned long buff12[1024];
	int i;
	for(i=0;i<1024;i++){
		buff12[i]=i;
	}

	pa_simple_write(s,buff12,sizeof(buff12),&error);

	return 35;


	struct sockaddr_in server;
	int sd,msgLength;
	char buff[50];
	char result;
	pthread_t thread_id;

	//connection establishment
	sd = socket(AF_INET,SOCK_STREAM,0);
	int *p = &sd;
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=inet_addr(argv[1]); //same machine
	server.sin_port=htons(atoi(argv[2]));
	connect(sd,(struct sockaddr *)&server,sizeof(server));
	printf("Enter your name:\n");
	pthread_create(&thread_id, NULL, reader, p);
	
	while(1) {
		buff[0]=0;
		gets(buff); 
		write(sd,buff,strlen(buff));
		write(sd,"\0",1);
		if(strcmp(buff, "-exit") == 0){
			close(sd);
			break;
		}
	}
	return 0;
}
