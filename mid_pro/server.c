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
	int j = 0, cl = 1, i = 0, ind, sd = 0;
	bool f = false;
	char buff[200],buff1[200],buff12[256];
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
	printf("%s connected\n", clients[findInd(sd)] -> name);
	bool *pt;
	
	while(1){
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
				while (1){
					myRead(sd,buff);
					if (strcmp(buff, "-end") == 0){
						break;
					}
					i = 0;
					while (i < groups[cl].count){
						sprintf(buff1,"%s - %s : %s%c",groups[cl].name,clients[findInd(sd)] -> name,buff,'\0');
						write(groups[cl].mem[i] -> sid,buff1,strlen(buff1) + 1);
						i++;
					}
				}
			}
			else if(strcmp(buff,"-no") == 0 && clients[findInd(sd)] -> inCall) {
				write(clients[temp_ind] -> sid,"-call ended\0",12);
				clients[findInd(sd)] -> inCall = false;
			}
			else if(strcmp(buff,"-yes") == 0 && clients[findInd(sd)] -> inCall) {
				cl = clients[temp_ind] -> sid;
				write(sd,"-call connected\0",16);
				write(cl,"-call connected\0",16);
				temp_ind = findInd(sd);
				// clients[temp_ind].inCall = true;
				pt = &clients[temp_ind] -> inCall;
				while(*pt) {
					read(sd, buff12, sizeof(buff12));
					write(cl, buff12, sizeof(buff12));
					if(strcmp(buff12, "-call ended") == 0) {
						break;
					}
				}
				cl = 1;
			}
			else if(strcmp(buff, "-call") == 0) {
				temp_ind = findInd(sd);
				clients[temp_ind] -> inCall = true;
				send_names(sd);
				write(sd, "server: select a user\0", 22);
				myRead(sd, buff);
				if(buff[0] > '9' || buff[0] <= '0'){
					write(sd,"invalid input\0",14);
					continue;
				}
				cl = buff[0] - '0' - 1; 
				if(clients[cl] -> inCall || !clients[cl] -> online){
					write(sd, "server: person is busy\0", 23);
					write(sd, "-call ended\0",11);
					continue;
				}
				clients[cl] -> inCall = true;
				cl = clients[cl] -> sid;
				write(cl,"-incoming call\0", strlen("-incoming call") + 1);
				pt = &clients[temp_ind] -> inCall;
				while(*pt) {
					read(sd, buff12, sizeof(buff12));
					write(cl, buff12, sizeof(buff12));
					if(strcmp(buff12, "-call ended") == 0) {
						break;
					}
				}
				cl = 1;
			}
			// else if(strcmp(buff, "-y") == 0) {
			// 	cl = clients[temp_ind].gp[cl];
			// 	j = 0;
			// 	while(j < groups[cl].count){
			// 		write(clients[groups[cl].mem[j]].sid,"-call connected\0",16);	
			// 	}
			// 	temp_ind = findInd(sd);
			// 	clients[temp_ind].inCall = true;
			// 	//pt = &clients[temp_ind].inCall;
			// 	while(1) {
			// 		read(sd, buff12, sizeof(buff12));
			// 		write(cl, buff12, sizeof(buff12));
			// 		if(strcmp(buff12, "-call ended") == 0) {
			// 			break;
			// 		}
			// 	}
			// 	cl = 1;

			// }
			// else if(strcmp(buff, "-grp call") == 0) {
			// 	temp_ind = findInd(sd);
			// 	clients[temp_ind].inCall = true;
			// 	send_groups(sd)
			// 	write(sd, "server: select a group\0", 23);
			// 	myRead(sd, buff);
			// 	if(strcmp(buff, "-exit") == 0){
			// 		close(sd);
			// 		del_entry(sd);
			// 		break;
			// 	}
			// 	if(buff[0] > '9' || buff[0] <= '0'){
			// 		write(sd,"invalid input\0",14);
			// 		continue;
			// 	}
			// 	cl = buff[0] - '0' - 1; 

			// 	// if(clients[cl].inCall){
			// 	// 	write(sd, "server: person is busy\0", 23);
			// 	// 	write(sd, "-call ended\0",11);
			// 	// 	continue;
			// 	// }
			// 	cl = clients[temp_ind].gp[cl];
			// 	j = 0;
			// 	while(j < groups[cl].count){
			// 		if(clients[groups[cl].mem[j]].sid != sd)
			// 			write(clients[groups[cl].mem[j]].sid,"-incoming call\0", strlen("-incoming call") + 1);	
			// 	}
			// 	pt = &clients[temp_ind].inCall;
			// 	while(1) {
			// 		read(sd, buff12, sizeof(buff12));
			// 		while(j < groups[cl].count){
			// 			if(clients[groups[cl].mem[j]].sid != sd)
			// 				write(clients[groups[cl].mem[j]].sid, buff12, sizeof(buff12));
			// 		}
			// 		if(strcmp(buff12, "-call ended") == 0) {
			// 			break;
			// 		}
			// 	}
			// 	cl = 1;
			// }
			else if(strcmp(buff, "-end") == 0) {
				cl = 1;
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
		else{
			write(cl,clients[findInd(sd)] -> name, strlen(clients[findInd(sd)] -> name));
			write(cl,": ",2);
			write(cl, buff, strlen(buff));
			write(cl,"\n\0",2);
		}
	}
	printf("%s disconnected\n", clients[findInd(sd)] -> name);
	clients[findInd(sd)] -> online = false;
	clients[findInd(sd)] -> sid = -1;
	return NULL;
}




