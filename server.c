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
 } myrc,anotherRC, rclist[MAX_ADJ];
 struct rcu{
	int rcid;
	int asnsrc;
	int asndest;
	int linkcapacity;
	int linkcost;
 }rcuv;
int echod(int);
void reaper(int);
void readConfig(struct asninfo *asnlistt,struct rcinfo *myrcc, struct rcinfo *rclistt );

int main(int argc, char **argv)
{
	int sd, new_sd, client_len, port;
	struct sockaddr_in server, client;
	printf("Read Config");
	readConfig(asnlist,&myrc,rclist);
	printf("%d %d %s \n", myrc.rcid, myrc.asn, myrc.ipa);
	printf("%d %d %s \n", rclist[0].rcid, rclist[0].asn, rclist[0].ipa);
	printf("%d %d %d \n", asnlist[0].asn, asnlist[0].linkcapacity, asnlist[0].linkcost);
	switch (argc)
	{
	case 1:
		port = SERVER_TCP_PORT;
		break;
	case 2:
		port = atoi(argv[1]);
		break;
	default:
		fprintf(stderr, "Usage: %d [port]\n", argv[0]);
		exit(1);
	}

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
	//fd = creat("serverlog.txt",O_RDWR);
	//upon connection, server will send info about itself to the connecting client.
	//(not an acknowledgement)
	while(n = read(sd,&rcuv,20)){
		printf("Received\n");
		printf("%d %d %d %d %d\n", rcuv.rcid,rcuv.asnsrc,rcuv.asndest,rcuv.linkcapacity,rcuv.linkcost);
		//write(1, &rcuv, n);
		memset(&rcuv, 0, sizeof(rcuv));
	    printf("Cleared: %d %d %d %d %d\n", rcuv.rcid,rcuv.asnsrc,rcuv.asndest,rcuv.linkcapacity,rcuv.linkcost);
		//write(fd, &rcuv, n);
	}
	//write(1,anotherRC,24);
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

void readConfig(struct asninfo *asnlistt,struct rcinfo *myrcc, struct rcinfo *rclistt ) {
	int nor, noa;
	int i;
	FILE *fp;
	fp= fopen("./configs/configrc1.txt", "r");
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
	//printf("%d %d %d \n", asnlist[i].asn, asnlist[i].linkcapacity, asnlist[i].linkcost);
	}
	fclose(fp);
}