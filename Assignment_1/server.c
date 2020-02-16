#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdbool.h>
#include<string.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<pthread.h>

int cli = 0;
struct args{
	int sid;
	char name[10];
} clients[200];


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
void del_entry(int sid){
	int j = 0;
	while(1) {
		if(sid == clients[j].sid) {
			break;
		}
		j++;
	}
	while(clients[j].sid != -1) {
		clients[j].sid = clients[j + 1].sid;
		strcpy(clients[j].name,clients[j + 1].name);
		j++;
	}
	cli--;
}
void send_names(int sid){
	int j = 0;
	char buff[50], c;
	while(clients[j].sid != -1){
		sprintf(buff,"%d: ",j + 1);
		write(sid,buff,strlen(buff));
		write(sid, clients[j].name, strlen(clients[j].name));
		write(sid, "\0", 1);
		j++;
	}
}
void* client_handler(void* input) {
	struct args* ip = (struct args*)input;
	char buff[50];
	int j, cl = 0;
	printf("%s connected\n", ip -> name);
	
	while(1){
		myRead(ip -> sid, buff);
		printf("%s\n",buff);
		if(strcmp(buff, "-list users") == 0){
			send_names(ip -> sid);
		}
		else if(strcmp(buff, "-send msg") == 0){
			send_names(ip -> sid);
			write(ip -> sid, "server: select a user", strlen("server: select a user"));
			write(ip -> sid, "\0", 1);
			myRead(ip -> sid, buff);
			cl = buff[0]- '0' - 1; 
			cl = clients[cl].sid;
			write(ip -> sid,"connected\0",10);
		}
		else if(strcmp(buff, "-end") == 0) {
			cl = 0;
		}
		else if(strcmp(buff, "-exit") == 0){
			close(ip -> sid);
			del_entry(ip -> sid);
			break;
		}
		
		else{
			write(cl,ip ->name, strlen(ip -> name));
			write(cl,":\0",2);
			write(cl, buff, strlen(buff));
			write(cl,"\0",1);
		}
	}
	
	return NULL;
}


int main(){
	struct sockaddr_in server, client;
	pthread_t thread_ids[200];
	int sd,clientLen;
	char buff[10];
	sd=socket(AF_INET,SOCK_STREAM,0);
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=inet_addr("127.0.0.1");
	server.sin_port=htons(5555);

	bind(sd,(struct sockaddr *)&server,sizeof(server));


	listen(sd,200);
	int k = cli;
	write(1,"Waiting for the client.....\n", sizeof("Waiting for the client.....\n"));
	//struct args clients[200];
	while(1){
		k = cli;
		clientLen=sizeof(client);
		clients[k].sid = accept(sd, (struct sockaddr *)&client, &clientLen);
		myRead(clients[k].sid, clients[k].name);
		pthread_create(&thread_ids[k], NULL, client_handler, (void *)&clients[k]);
		k++;
		clients[k].sid = -1;
		cli = k;
	}
	close(sd);
	return 0;
}

