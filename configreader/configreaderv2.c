#include <stdio.h>
//#include "configreaderv2.h"

#define MAX_ADJ 10
FILE *fp;

//  struct asninfo {
// 	int asn;
// 	int linkcapacity;
// 	int linkcost;
//  } asnlist[MAX_ADJ];
//  struct rcinfo {
// 	int rcid;
// 	int asn;
// 	char ipa[15];
//  } myrc, rclist[MAX_ADJ];

/* This program reads configuration file and displays the configuration information. The input is the configuration file named as "config". */ 
void readConfig(struct asninfo *asnlistt,struct rcinfo *myrcc, struct rcinfo *rclistt ) {
 int nor, noa;
 int i;
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

// int main(int argc, char **argv){
// 	readConfig(asnlist,&myrc,rclist);
// 	printf("%d %d %s \n", myrc.rcid, myrc.asn, myrc.ipa);
// 	printf("%d %d %s \n", rclist[0].rcid, rclist[0].asn, rclist[0].ipa);
// 	printf("%d %d %d \n", asnlist[0].asn, asnlist[0].linkcapacity, asnlist[0].linkcost);
// }