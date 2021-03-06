/* A simple echo client using TCP */
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
//#include "configreaderv2.h"
#define SERVER_TCP_PORT 3000	/* well-known port */
#define BUFLEN		256	/* buffer length */
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
 } myrc,connectedrc, rclist[MAX_ADJ];
struct rcu{
		int rcid;
		int asnsrc;
		int asndest;
		int linkcapacity;
		int linkcost;
}rcus;
void readConfig(struct asninfo *asnlistt,struct rcinfo *myrcc, struct rcinfo *rclistt );

int main(int argc, char **argv)
{
	int 	n, i, bytes_to_read;
	int 	sd, port, c;
	struct	hostent		*hp;
	struct	sockaddr_in server;
	char	*host, *bp, rbuf[BUFLEN], sbuf[BUFLEN];
	time_t lastsent,tnow;
	printf("Read Config\n");
	readConfig(asnlist,&myrc,rclist);
	// printf("%d %d %s \n", myrc.rcid, myrc.asn, myrc.ipa);
	// printf("%d %d %s \n", rclist[0].rcid, rclist[0].asn, rclist[0].ipa);
	// printf("%d %d %d \n", asnlist[0].asn, asnlist[0].linkcapacity, asnlist[0].linkcost);
	switch(argc){
	case 2:
		host = argv[1];
		port = SERVER_TCP_PORT;
		break;
	case 3:
		host = argv[1];
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
		exit(1);
	}

	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (hp = gethostbyname(host)) 
	  bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	else if ( inet_aton(host, (struct in_addr *) &server.sin_addr) ){
	  fprintf(stderr, "Can't get server's address\n");
	  exit(1);
	}

	/* Connecting to the server */
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
	  fprintf(stderr, "Can't connect \n");
	  exit(1);
	}

	read(sd,&connectedrc,sizeof(myrc));
	printf("Bytes read from server");
	printf("%d %d %s\n", connectedrc.rcid, connectedrc.asn, connectedrc.ipa);

	time(&lastsent);
	printf("Timer start: %ld\n",lastsent);
	// n = sizeof(rcus);
	// write(sd,&rcus,n);
	time(&tnow);
	//look into timer function
	int size = sizeof(asnlist)/sizeof(asnlist[0]);
	printf("Size: %d",size);
	while(1){
		time(&tnow);
		if(( tnow - lastsent) == 10){ //5 second delay
			printf("Time to write RCU\n");
			// n = sizeof(rcus);
			// printf("Size: %d\n",n);
			int size = sizeof(asnlist)/sizeof(asnlist[0]);
			for(c = 0;c < 4;c++){ //we should make this dynamic
				rcus.rcid = myrc.rcid;
				rcus.asnsrc = asnlist[c].asn;
				rcus.asndest = 2;
				rcus.linkcapacity = asnlist[c].linkcapacity;
				rcus.linkcost = asnlist[c].linkcost;
				write(sd,&rcus,20);
				memset(&rcus, 0, sizeof(rcus));
			}
			
			time(&lastsent);
			fflush(stdout);
		}
		// sleep(5);
		// printf("Time to write RCU");
		// write(sd,&rcus,n);
	}
	close(sd);
	return(0);
}


void readConfig(struct asninfo *asnlistt,struct rcinfo *myrcc, struct rcinfo *rclistt) {
	int nor, noa;
	int i;
	FILE *fp;
	fp= fopen("./configs/configrc2.txt", "r");
	//gets info for this RC
	fscanf(fp, "%d %d %s", &myrcc->rcid, &myrcc->asn, myrcc->ipa);
	printf("%d %d %s \n", myrc.rcid, myrc.asn, myrc.ipa);
	fscanf(fp, "%d", &nor);
	//gets info to the RCs directly connected
	for (i=0; i<nor; i++) {
	fscanf(fp, "%d %d %s", &rclistt[i].rcid, &rclistt[i].asn, rclistt[i].ipa);
	printf("%d %d %s \n", rclist[i].rcid, rclist[i].asn, rclist[i].ipa);
	}
	//gets info for the ASs connected
	fscanf(fp, "%d", &noa);
	for (i=0; i<noa; i++) {
	fscanf(fp, "%d %d %d", &asnlistt[i].asn, &asnlistt[i].linkcapacity, &asnlistt[i].linkcost);
		printf("%d %d %d \n", asnlist[i].asn, asnlist[i].linkcapacity, asnlist[i].linkcost);
	}
	printf("END READCONFIG\n");
	fclose(fp);
}