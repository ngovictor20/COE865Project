/* A simple echo server using TCP */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <pthread.h>
//#include "configreaderv2.h"
#define SERVER_TCP_PORT 3000 /* well-known port */
#define BUFLEN 256			 /* buffer length */
#define MAX_ADJ 10

 struct asninfo {
	int asn;
	int linkcapacity;
	int linkcost;
 } asnlist[MAX_ADJ];
 struct rcinfo {
	int rcid;
	int asn;
	char ipa[15];
 } myrc, rclist[MAX_ADJ];
 struct rcu{
	int rcid;
	int asnsrc;
	int asndest;
	int linkcapacity;
	int linkcost;
 }rcuv;
 struct route{
	 int rcsrc;
	 int asn;
	 int src;
	 int linkcapacity;
	 int linkcost;
 } routingT[MAX_ADJ];

pthread_mutex_t lock;

int echod(int);
void reaper(int);
void readConfig(struct asninfo *asnlistt,struct rcinfo *myrcc, struct rcinfo *rclistt,struct route *table, char *configPath);

int main(int argc, char **argv)
{
	int sd, new_sd, client_len, port;
	char *config;
	struct sockaddr_in server, client;
	
	switch (argc)
	{
	case 3:
		config = argv[1];
		port = atoi(argv[2]);
		printf("Server stuff: %s, %d \n",config, port);
		break;
	default:
		fprintf(stderr, "Usage: %d [port]\n", argv[0]);
		exit(1);
	}
	printf("Read Config Call\n");
	readConfig(asnlist,&myrc,rclist,routingT,config);
	printf("%d %d %s \n", myrc.rcid, myrc.asn, myrc.ipa);
	printf("%d %d %s \n", rclist[0].rcid, rclist[0].asn, rclist[0].ipa);
	printf("%d %d %d \n", asnlist[0].asn, asnlist[0].linkcapacity, asnlist[0].linkcost);


	/* Create a stream socket	*/
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	/* Bind an address to the socket	*/
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	//server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	/* queue up to 5 connect requests  */
	listen(sd, 5);

	(void)signal(SIGCHLD, reaper);

	while (1)
	{
		client_len = sizeof(client);
		new_sd = accept(sd, (struct sockaddr *)&client, &client_len);
		if (new_sd < 0)
		{
			fprintf(stderr, "Can't accept client \n");
			exit(1);
		}
		switch (fork())
		{
		case 0: /* child */
			(void)close(sd);
			exit(echod(new_sd));
		default: /* parent */
			(void)close(new_sd);
			break;
		case -1:
			fprintf(stderr, "fork: error\n");
		}
	}
}

/*	echod program	*/
int echod(int sd)
{
	char *bp;
	int n, bytes_to_read, fd;
	FILE * fp = NULL;
	struct rcinfo connectedRC;
	//fd = creat("serverlog.txt",O_RDWR);
	//send info about itself
	printf("A client has connected\n");
	write(sd,&myrc,sizeof(myrc));
	// printf("Client is sending its info\n");
	// read(sd,&connectedRC,sizeof(connectedRC));

	while(n = read(sd,&rcuv,20)){
		int rcount = 0,srccost=0,count,asnnum;
		printf("Received\n");
		printf("RCID SRC:%d  ASN:%d DEST ASN:%d CAP:%d COST:%d\n", rcuv.rcid,rcuv.asnsrc,rcuv.asndest,rcuv.linkcapacity,rcuv.linkcost);
		//write(1, &rcuv, n);
		//check routes
		for(rcount = 0; rcount < MAX_ADJ;rcount++){
			if(rcuv.asnsrc == routingT[rcount].asn){
				printf("Found a route to compare to:\n");
				//find the cost to the source
				printf("Finding the cost of source\n");
				printf("Getting ASN Info\n");
				for(count = 0;count < MAX_ADJ;count++){
					if(rcuv.rcid == rclist[count].rcid){
						asnnum = rclist[count].asn;
						int x;
						for(x=0;x<MAX_ADJ;x++){
							if(rclist[count].asn == asnlist[x].asn){
								srccost = asnlist[x].linkcost;
								printf("Found cost of %d, total: %d\n",srccost,srccost+rcuv.linkcost);
								break;
							}
						}
						break;
					}
				}

				if((rcuv.linkcost+srccost) < routingT[rcount].linkcost){
					printf("Advertised linkcost is shorter\n");
					routingT[rcount].rcsrc = rcuv.rcid;
					routingT[rcount].asn = rcuv.asnsrc;
					routingT[rcount].src = rcuv.rcid;
					routingT[rcount].linkcapacity = rcuv.linkcapacity;
					routingT[rcount].linkcost = rcuv.linkcost+srccost;
				}
				break;
			}
			if(routingT[rcount].rcsrc == 0){
				//this means that we went through th list, and we gotta add this entry into the table
				printf("NEW ROUTE!\n");
				routingT[rcount].rcsrc = rcuv.rcid;
				routingT[rcount].asn = rcuv.asnsrc;
				routingT[rcount].src = rcuv.rcid;
				routingT[rcount].linkcapacity = rcuv.linkcapacity;
				routingT[rcount].linkcost = rcuv.linkcost+srccost;
				break;
			}
		}
		printf("-------ROUTING TABLE------\n");
		for(rcount = 0;rcount < MAX_ADJ;rcount++){
			if(routingT[rcount].rcsrc != 0){
			printf("Route: RCSRC: %d ASN:%d SRC:%d CAP:%d COST:%d \n",routingT[rcount].rcsrc,routingT[rcount].asn,routingT[rcount].src,routingT[rcount].linkcapacity,routingT[rcount].linkcost);
			}
		}
		memset(&rcuv, 0, sizeof(rcuv));
	    //printf("Cleared: %d %d %d %d %d\n", rcuv.rcid,rcuv.asnsrc,rcuv.asndest,rcuv.linkcapacity,rcuv.linkcost);
		//write(fd, &rcuv, n);
	}
	//write(1, &rcurecv, n);
	close(sd);
	close(fp);
	return (0);
}

/*	reaper		*/
void reaper(int sig){
	int status;
	while (wait3(&status, WNOHANG, (struct rusage *)0) >= 0)
		;
}

void readConfig(struct asninfo *asnlistt,struct rcinfo *myrcc, struct rcinfo *rclistt,struct route *table,char *configPath ) {
	int nor, noa;
	int i;
	FILE *fp,*fd;
	//"./configs/configrc1.txt"
	printf("Opening filepath %s\n",configPath);
	fp= fopen(configPath, "r");
	//fd = fopen("./","w");
	//gets info for this RC
	fscanf(fp, "%d %d %s", &myrcc->rcid, &myrcc->asn, myrcc->ipa);
	//printf("%d %d %s \n", myrc.rcid, myrc.asn, myrc.ipa);
	fscanf(fp, "%d", &nor);
	//gets info to the RCs directly connected
	for (i=0; i<nor; i++) {
	fscanf(fp, "%d %d %s", &rclistt[i].rcid, &rclistt[i].asn, rclistt[i].ipa);
	//printf("%d %d %s \n", rclist[i].rcid, rclist[i].asn, rclist[i].ipa);
	}
	//gets info for the ASs connected
	fscanf(fp, "%d", &noa);
	for (i=0; i<noa; i++) {
	fscanf(fp, "%d %d %d", &asnlistt[i].asn, &asnlistt[i].linkcapacity, &asnlistt[i].linkcost);
	table[i].asn = asnlistt[i].asn;
	table[i].rcsrc = myrcc->rcid;
	table[i].src = myrcc->asn;
	table[i].linkcapacity = asnlistt[i].linkcapacity;
	table[i].linkcost = asnlistt[i].linkcost;
	//printf("%d %d %d \n", asnlistt[i].asn, asnlistt[i].linkcapacity, asnlistt[i].linkcost);
	printf("Route: RCSRC: %d ASN:%d SRC:%d CAP:%d COST:%d \n",table[i].rcsrc, table[i].asn, table[i].src,table[i].linkcapacity, table[i].linkcost);
	}
	fclose(fp);
}