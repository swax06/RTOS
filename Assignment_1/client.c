#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>

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
int main(){
	struct sockaddr_in server;
	int sd,msgLength;
	char buff[50];
	char result;
	pthread_t thread_id;

	//connection establishment
	sd = socket(AF_INET,SOCK_STREAM,0);
	int *p = &sd;
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=inet_addr("127.0.0.1"); //same machine
	server.sin_port=htons(5555);
	connect(sd,(struct sockaddr *)&server,sizeof(server));
	printf("Enter your name:\n");
	pthread_create(&thread_id, NULL, reader, p);
	
	while(1) {
		gets(buff); 
		write(sd,buff,strlen(buff));
		write(sd,"\0",1);
		if(strcmp(buff, "-exit") == 0){
			break;
		}
	}
	
	close(sd);

	return 0;
}
