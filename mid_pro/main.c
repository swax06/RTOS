#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<signal.h> 
#include<stdbool.h>
#include<string.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<pthread.h>
#include "server.h"


int main(int argc,char **argv){
	signal(SIGINT, handleSigint); 
	struct sockaddr_in server, client;
	pthread_t thread_ids[200];
	unsigned int sd, clientLen;
	sd = socket(AF_INET,SOCK_STREAM,0);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_port = htons(atoi(argv[2]));
	bind(sd,(struct sockaddr *)&server,sizeof(server));
	listen(sd, 200);
	printf("Started.....\n");
	while(1){
		int temp_sd;
		clientLen = sizeof(client);
		temp_sd = accept(sd, (struct sockaddr *)&client, &clientLen);
		pthread_create(&thread_ids[cli], NULL, clientHandler, &temp_sd);
	}
	close(sd);
	return 0;
}