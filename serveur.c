#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PORT 2632
#define IP "162.38.111.15"
#define TAILLE_MAX 50*sizeof(char)

int serveur(){
	dS = socket(PF_INET, SOCK_STREAM, 0); //creation socket serveur
	struct sockaddr_in adServeur;
	adServeur.sin_family=AF_INET;
	adServeur.sin_addr.s_addr=INADDR_ANY
	adServeur.sin_port=htons(atoi(PORT));
	int sockAd = bind(dS,(struct sockaddr*)&adServeur, sizeof(adServeur)); //lie la socket a une adresse
	if (sockAd==-1){
		printf("erreur association");
		return 11;
	}
	listen(dS,10); //socket en mode ecoute (10 represente le nb max de demandes de connexion pouvant etre mis en attente)
	struct sockaddr_in adClient1; //structure de l'adresse client 1
	struct sockaddr_in adClient2; //strucutre de l'adresse client 2
	socklen_t lg = sizeof(struct sockaddr_in*));
	
	//accept client 1 sur serveur
	int dSClient1 = accept(dS,(struct sockaddr*)&adClient1,&lg); //connexion socket
	if (dSClient1 == -1){
		printf("erreur connexion socket client1");
		return 12;
	}

	//accept client 2 sur serveur
	int dSClient2 = accept(dS,(struct sockaddr*)&adClient2,&lg); //connexion socket
	if (dSClient2 == -1){
		printf("erreur connexion socket client2");
		return 121;
	}

	char message[TAILLE_MAX];

	//tant que les clients sont connectés
	while(1){
		//recepetion message venant de 1 et envoie au client 2
	int rec1 = recv(dSClient1, message,sizeof(message),0);
	if(rec1==-1){
		printf("erreur reception message du client 1");
		return 13;
	}
	printf("recu : %s\n", message);
	if(strcmp(message,"fin")==0){
			close(dSClient1);
			close(dSClient2);
		}
	
	send(dSClient2,message,sizeof(message),0);

	//recepetion message venant de 2 et envoie au client 1
	int rec2 = recv(dSClient2, message,sizeof(message),0);
	if(rec2==-1){
		printf("erreur reception message du client 2");
		return 13;
	}
	printf("recu : %s\n", message);
	
	send(dSClient1,message,sizeof(message),0);
	}
	
	return 0;
}