struct client{
	int sid;
	bool inCall, online;
	char name[10];
	int gp[10], g;
} *clients[200];
struct group{
	char name[10];
	//int mem[10];
	struct client *mem[10];
	int count;
} groups[20];

//extern struct client *clients;
extern int cli, temp_ind, grp;
extern void handleSigint(int sig);
extern void* clientHandler(void* input);