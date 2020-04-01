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
 } myrc, rclist[MAX_ADJ];

void readConfig(struct asninfo *asnlistt,struct rcinfo *myrcc, struct rcinfo *rclistt );


int main(int argc, char **argv)
{
	int 	n, i, bytes_to_read;
	int 	sd, port;
	struct	hostent		*hp;
	struct	sockaddr_in server;
	char	*host, *bp, rbuf[BUFLEN], sbuf[BUFLEN];
	printf("Read Config\n");
	readConfig(asnlist,&myrc,rclist);
	printf("%d %d %s \n", myrc.rcid, myrc.asn, myrc.ipa);
	printf("%d %d %s \n", rclist[0].rcid, rclist[0].asn, rclist[0].ipa);
	printf("%d %d %d \n", asnlist[0].asn, asnlist[0].linkcapacity, asnlist[0].linkcost);
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
	n = sizeof(myrc);
	printf("Size: %d\n",n);
	write(sd,&myrc,n);

	// while(n=read(0, sbuf, BUFLEN)){	/* get user message */
	//   printf("Sending user message");
	//   write(sd, sbuf, n);		/* send it out */
	//   strcat(text,sbuf);
	//   //printf("Receive: \n");
	//   // bp = rbuf;
	//   // bytes_to_read = n;
	//   //while ((i = read(sd, bp, bytes_to_read)) > 0){
	//   //	bp += i;
	//   //	bytes_to_read -=i;
	//   // }
	//   //write(1, rbuf, n);
	//   //printf("Transmit: \n");
	//   int fp;
	//   while(n=read(sd,rbuf,BUFLEN)){
	//     fp = creat(text,O_RDWR);
	//     write(fp,rbuf,BUFLEN);
	//   }
	// }

	close(sd);
	return(0);
}


void readConfig(struct asninfo *asnlistt,struct rcinfo *myrcc, struct rcinfo *rclistt ) {
 int nor, noa;
 int i;
 FILE *fp;
 fp= fopen("config.txt", "r");
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