//un thread par client pour ecouter les messages qui proviennent de lui et les diffuser vers tous les autres clients.
#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>

#define TAILLE_MAX 50
#define PORT 2633
#define IP "192.168.1.99"
#define NBCLIENT 50;

int dSCli[NBCLIENT];
char tabPseudo[NBCLIENT][30];


int recep_pseudo(char *pseudo, int num){
	if(recv(dSCli[num],pseudo,(strlen(pseudo)+1)*sizeof(char),0)!=0){ //&pseudo ?
		printf("erreur reception pseudo\n");
		return 30;
	}
	strcpy(tabPseudo[i],pseudo); //&pseudo ? //stock les pseudo
}



int main(){
	int dS = socket(PF_INET, SOCK_STREAM, 0); //creation socket serveur
	struct sockaddr_in adServeur; //création de la structure contenant l'adresse du serveur
	adServeur.sin_family=AF_INET; //famille d'adresse
	adServeur.sin_addr.s_addr=INADDR_ANY;
	adServeur.sin_port=htons(PORT); //définition du port
	int sockAd = bind(dS,(struct sockaddr*)&adServeur, sizeof(adServeur)); //lie la socket a une adresse
	if (sockAd==-1){
		printf("erreur association");
		return -1;
	}
	if(listen(dS,10)!=0){
		printf("erreur listen");
		return 60;
	} //socket en mode ecoute (10 represente le nb max de demandes de connexion pouvant etre mis en attente)

	
}



