struct client{
	int sid;
	bool inCall, online;
	char name[10];
	int gp[10], g;
	struct session *ptr;
} *clients[200];
struct group{
	char name[10];
	struct client *mem[10];
	int count;
} groups[20], *glptr;
struct session{
	struct client *mem[10];
	int count;
};

extern int cli, temp_ind, grp;
extern void handleSigint(int sig);
extern void* clientHandler(void* input);