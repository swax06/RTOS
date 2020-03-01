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
#include<time.h>

int cli = 0;
struct args{
	int sid;
	char name[10];
	int gp[10],g;
} clients[200];
struct group{
	char name[10];
	int mem[10];
	int count;
} groups[10];
int grp = 0;

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
	while(j < cli) {
		clients[j].sid = clients[j + 1].sid;
		strcpy(clients[j].name,clients[j + 1].name);
		j++;
	}

	cli--;
}
void send_names(int sid){
	int j = 0;
	char buff[50];
	write(sid, "online users:\n\0",15);
	while(j < cli){
		sprintf(buff,"%d: %s\n%c", j + 1, clients[j].name, '\0');
		write(sid,buff,strlen(buff) + 1);
		j++;
	}

}
void send_groups(int sid){
	int j = 0, t, i;
	char buff[50];
	while(j < cli){
		if(clients[j].sid == sid){
			t = j;
		}
		j++;
	}
	write(sid, "your groups:\n\0",14);
	j = 0;
	while(j < clients[t].g){
		sprintf(buff,"%d: %s%c", j + 1, groups[clients[t].gp[i]].name, '\0');
		write(sid,buff,strlen(buff) + 1);
		j++;
		i++;
	}
}
void* client_handler(void* input) {
	struct args ip = *(struct args*)input;
	strcpy(ip.name,((struct args*)input)->name);
	char buff[200],buff1[200];
	int j, cl = 1, i;
	printf("%s connected\n", ip.name);
	
	while(1){
		myRead(ip.sid, buff);
		if(strcmp(buff, "-list users") == 0){
			send_names(ip.sid);
		}
		else if(strcmp(buff, "-list grps") == 0){
			send_groups(ip.sid);
		}
		else if(strcmp(buff, "-send msg") == 0){
			send_names(ip.sid);
			write(ip.sid, "server: select a user\0", 22);
			myRead(ip.sid, buff);
			if(strcmp(buff, "-exit") == 0){
				close(ip.sid);
				del_entry(ip.sid);
				break;
			}
			if(buff[0] > '9' || buff[0] <= '0'){
				continue;
			}
			cl = buff[0] - '0' - 1; 
			cl = clients[cl].sid;
			write(ip.sid,"connected\0",10);
		}
		else if(strcmp(buff, "-send grp msg") == 0){
			send_groups(ip.sid);
			write(ip.sid, "server: select a group\0", 23);
			myRead(ip.sid,buff);
			if(strcmp(buff, "-exit") == 0){
				close(ip.sid);
				del_entry(ip.sid);
				break;
			}
			if(buff[0] > '9' || buff[0] <= '0'){
				continue;
			}
			cl = buff[0] - '0' - 1; 
			write(ip.sid, "connected to group\ntype -end to end conversation\0",49);
			while (1){
				myRead(ip.sid,buff);
				if (strcmp(buff, "-end") == 0){
					break;
				}
				i = 0;
				while (i < groups[cl].count){
					sprintf(buff1,"%s - %s : %s%c",groups[cl].name,ip.name,buff,'\0');
					write(groups[cl].mem[i],buff1,strlen(buff1) + 1);
					i++;
				}
			}

		}
		else if(strcmp(buff, "-end") == 0) {
			cl = 1;
		}
		else if(strcmp(buff, "-exit") == 0){
			close(ip.sid);
			del_entry(ip.sid);
			printf("%s disconnected\n", ip.name);
			break;
		}
		else if(strcmp(buff, "-make grp") == 0){
			write(ip.sid, "server: name your group", 23);
			write(ip.sid, "\0", 1);
			myRead(ip.sid, buff);
			strcpy(groups[grp].name, buff);
			send_names(ip.sid);
			write(ip.sid, "server: select the users for group\nserver: type -end to exit selection", 70);
			write(ip.sid, "\0", 1);
			groups[grp].count = 0;
			while(strcmp(buff, "-end") != 0){
				myRead(ip.sid, buff);
				if(strcmp(buff, "-exit") == 0){
					close(ip.sid);
					del_entry(ip.sid);
					break;
				}
				if(buff[0] > '9' || buff[0] <= '0'){
					continue;
				}
				cl = buff[0] - '0' - 1; 
				clients[cl].gp[clients[cl].g] = grp;
				clients[cl].g++;
				cl = clients[cl].sid;
				groups[grp].mem[groups[grp].count] = cl;
				groups[grp].count++;
			}
			grp++;
			write(ip.sid,"server: group created\0",22);
		}
		else{
			write(cl,ip.name, strlen(ip.name));
			write(cl,": ",2);
			write(cl, buff, strlen(buff));
			write(cl,"\n\0",2);
		}
	}
	
	return NULL;
}


int main(int argc,char **argv){
	struct sockaddr_in server, client;
	pthread_t thread_ids[200];
	int sd,clientLen;
	char buff[10];
	sd=socket(AF_INET,SOCK_STREAM,0);
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=inet_addr(argv[1]);
	server.sin_port=htons(atoi(argv[2]));
	bind(sd,(struct sockaddr *)&server,sizeof(server));
	listen(sd,200);
	write(1,"Waiting for the client.....\n", sizeof("Waiting for the client.....\n"));
	while(1){
		int temp_a;
		clientLen=sizeof(client);
		temp_a = accept(sd, (struct sockaddr *)&client, &clientLen);
		clients[cli].sid=temp_a;
		myRead(clients[cli].sid, clients[cli].name);
		pthread_create(&thread_ids[cli], NULL, client_handler, (void *)&clients[cli]);
		clients[cli].g = 0;
		cli++;
	}
	close(sd);
	return 0;
}

