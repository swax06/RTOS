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
#include "server.h"
#include "server_ext.h"


int cli = 0, temp_ind = 0, grp = 0;

void handleSigint(int sig){ 
    printf("Exiting... %d\n", sig); 
	if(sig == 2){
		exit(0);
	}
}

void* clientHandler(void* input) {
	int j = 0, cl = 100, i = 0, mode = 1, sd = 0;
	bool f = false;
	char buff[256], buff1[256], buff2[256] = "-call ended";
	struct session temp;
	struct client *loc;
	sd = *(int*)input;
	myRead(sd, buff);
	while(i < cli){
		if(strcmp(buff, clients[i] -> name) == 0){
			f = true;
			break;
		}
		i++;
	}
	if(!f) {
		clients[cli] = (struct client *) malloc(sizeof(struct client));
		strcpy(clients[cli] -> name, buff);
		clients[cli] -> g = 0;
		clients[cli] -> inCall = false;
		clients[cli] -> online = true;
		cli++;
	}
	clients[i] -> sid = sd;
	loc = clients[i];
	printf("%s connected\n", loc -> name);
	
	while(1){
		if(mode == 3){
			read(sd, buff, sizeof(buff));
		}
		else
			myRead(sd, buff);
		if(buff[0] == '-'){
			if(strcmp(buff, "-users") == 0){
				send_names(sd);
			}
			else if(strcmp(buff, "-groups") == 0){
				send_groups(sd);
			}
			else if(strcmp(buff, "-send msg") == 0){
				send_names(sd);
				write(sd, "server: select a user\0", 22);
				myRead(sd, buff);
				if(buff[0] > '9' || buff[0] <= '0'){
					continue;
				}
				cl = buff[0] - '0' - 1; 
				cl = clients[cl] -> sid;
				write(sd,"connected\0",10);
				mode = 1;
			}
			else if(strcmp(buff, "-send grp msg") == 0){
				send_groups(sd);
				write(sd, "server: select a group\0", 23);
				myRead(sd,buff);
				if(buff[0] > '9' || buff[0] <= '0'){
					continue;
				}
				cl = buff[0] - '0' - 1; 
				write(sd, "connected to group\ntype -end to end conversation\0",49);
				mode = 2;
			}
			else if(strcmp(buff, "-yes") == 0) {
				write(loc -> sid,"-call connected\0",16);
				loc -> inCall = true;
				mode = 3;
			}
			else if(strcmp(buff,"-no") == 0 || strcmp(buff, "-call ended") == 0) {
				loc -> inCall = false;
				loc -> ptr -> count--;
				mode = 1;
			}
			else if(strcmp(buff, "-call") == 0) {
				loc -> inCall = true;
				send_names(sd);
				write(sd, "server: select a user\0", 22);
				myRead(sd, buff);
				if(buff[0] > '9' || buff[0] <= '0'){
					write(sd,"invalid input\0",14);
				}
				cl = buff[0] - '0' - 1; 
				if(clients[cl] -> inCall || !clients[cl] -> online){
					write(sd, "server: cannot place call\0", 26);
					write(sd, "-call ended\0",11);
				}
				clients[cl] -> ptr = &temp;
				loc -> ptr = &temp;
				temp.mem[0] = clients[cl];
				temp.mem[1] = loc;
				temp.count = 2;
				write(clients[cl] -> sid,"-incoming call\0", strlen("-incoming call") + 1);
				write(loc -> sid, "-call connected\0", 16);
				mode = 3;
			}
			else if(strcmp(buff, "-grp call") == 0) {
				loc -> inCall = true;
				send_groups(sd);
				write(sd, "server: select a group\0", 23);
				myRead(sd, buff);
				if(buff[0] > '9' || buff[0] <= '0'){
					write(sd,"invalid input\0",14);
					continue;
				}
				cl = buff[0] - '0' - 1;
				loc -> ptr = &temp;
				temp.count = 1;
				temp.mem[0] = loc;
				j = 1;
				i = 0;
				while(i < groups[cl].count) {
					if(!groups[cl].mem[i] -> inCall){
						groups[cl].mem[i] -> ptr = &temp;
						temp.mem[j] = groups[cl].mem[i];
						temp.count++;
						write(groups[cl].mem[i] -> sid,"-incoming call\0", strlen("-incoming call") + 1);
					}
					i++;
				}
				write(loc -> sid, "-call connected\0", 16);
				mode = 3;
			}
			
			else if(strcmp(buff, "-end") == 0) {
				cl = 100;
			}

			else if(strcmp(buff, "-exit") == 0){
				close(sd);
				break;
			}

			else if(strcmp(buff, "-make grp") == 0){
				write(sd, "server: name your group\0", 24);
				myRead(sd, buff);
				strcpy(groups[grp].name, buff);
				send_names(sd);
				write(sd, "server: select the users for group\nserver: type -end to exit selection\0", 71);
				groups[grp].count = 0;
				while(strcmp(buff, "-end") != 0){
					myRead(sd, buff);
					if(buff[0] > '9' || buff[0] <= '0'){
						continue;
					}
					cl = buff[0] - '0' - 1; 
					clients[cl] -> gp[clients[cl] -> g] = grp;
					clients[cl] -> g++;
					groups[grp].mem[groups[grp].count] = clients[cl];
					groups[grp].count++;
				}
				grp++;
				write(sd,"server: group created\0",22);
			}
			else{
				write(sd,"server: invalid input\nenter -h for help\0",40);
			}
		}
		else {
			if(mode == 1) { 
				sprintf(buff1,"%s : %s%c", loc -> name, buff, '\0');
				write(cl, buff1, strlen(buff1) + 1);
			}
			if(mode == 2) {
				i = 0;
				while (i < groups[cl].count){
					sprintf(buff1,"%s@%s : %s%c", loc -> name, groups[cl].name, buff, '\0');
					write(groups[cl].mem[i] -> sid,buff1,strlen(buff1) + 1);
					i++;
				}	
			}
			if(mode == 3) {
				i = 0;
				while(i < loc -> ptr -> count){
					if(loc -> ptr -> mem[i] -> inCall && loc -> ptr -> mem[i] -> sid != sd){
						write(loc -> ptr -> mem[i] -> sid, buff, sizeof(buff));
					}
					i++;
				}
				if(loc -> ptr -> count == 1) {
					loc -> inCall = false;
					write(loc -> sid, "-call ended\0", 256);
					loc -> ptr -> count --;
					//cl = 1;
					mode = 1;
				}
			}
		}
	}
	printf("%s disconnected\n", loc -> name);
	loc -> online = false;
	loc -> sid = -1;
	return NULL;
}




