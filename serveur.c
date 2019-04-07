#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PORT 2633
#define IP "172.20.10.4"
#define TAILLE_MAX 50

int serveur(){
	int dS = socket(PF_INET, SOCK_STREAM, 0); //creation socket serveur
	struct sockaddr_in adServeur;
	adServeur.sin_family=AF_INET;
	adServeur.sin_addr.s_addr=INADDR_ANY;
	adServeur.sin_port=htons(PORT);
	int sockAd = bind(dS,(struct sockaddr*)&adServeur, sizeof(adServeur)); //lie la socket a une adresse
	if (sockAd==-1){
		printf("erreur association");
		return 11;
	}
	listen(dS,10); //socket en mode ecoute (10 represente le nb max de demandes de connexion pouvant etre mis en attente)
	struct sockaddr_in adClient1; //structure de l'adresse client 1
	struct sockaddr_in adClient2; //strucutre de l'adresse client 2
	socklen_t lg = sizeof(struct sockaddr_in*);
	
	int n=1;
	int n2=2;
	int *id1=&n;
	int *id2=&n2;

	//accept client 1 sur serveur
	int dSClient1 = accept(dS,(struct sockaddr*)&adClient1,&lg); //connexion socket
	if (dSClient1 == -1){
		printf("erreur connexion socket client1");
		return 12;
	}
	int s=send(dSClient1,id1,sizeof(int),0); //Envoi de l'id correspondant au premier client
	if(s==-1){
		printf("Erreur envoi id1.");
		return 13;
	}

	//accept client 2 sur serveur /!\Besoin que d'un seul client
	int dSClient2 = accept(dS,(struct sockaddr*)&adClient2,&lg); //connexion socket
	if (dSClient2 == -1){
		printf("erreur connexion socket client2");
		return 14;
	}
	s=send(dSClient2,id2,sizeof(int),0); //Envoi de l'id correspondant au second client
	if(s==-1){
		printf("Erreur envoi id2.");
		return 15;
	}

	
	
	int echange=1;

	printf("Preparation a la reception, veuillez patienter 2s ...\n");
	sleep(2);
	int idActuel=1;
	//tant que les clients sont connectés
	while(echange==1){
		char* message=(char*)malloc((TAILLE_MAX+1)*sizeof(char)); //stockera les messages recus par le serveur
		char messageSend[TAILLE_MAX]; //contiendra le message a envoyer aux clients
		if(idActuel==1){
			//reception message venant de 1 et envoi au client 2
			int rec1 = recv(dSClient1, message,TAILLE_MAX*sizeof(char),0);
			if(rec1==-1){
				printf("erreur reception message du client 1");
				return 16;
			}
			printf("recu serveur : %s\n", message);
			if(strcmp(message,"fin")==0){
				int i=0;
				while(message[i]!='\0'){
					messageSend[i]=message[i];
					i++;
				}
				messageSend[i]='\0';
				int su=send(dSClient2,&messageSend,TAILLE_MAX*sizeof(char),0);
				if(su==-1){
					printf("Erreur envoi client2.\n");
					return 17;
				}
				
				free(message);
				close(dSClient1);
				close(dSClient2);
				echange=0;
			}
			else{
				int i=0;
				while(message[i]!='\0'){
					messageSend[i]=message[i];
					i++;
				}
				messageSend[i]='\0';
				int su=send(dSClient2,&messageSend,TAILLE_MAX*sizeof(char),0);
				if(su==-1){
					printf("Erreur envoi client2.\n");
					return 17;
				}
				free(message);			
			}	
		}
		else if(idActuel==2){
			//reception message venant de 2 et envoi au client 1
			int rec2 = recv(dSClient2, message,TAILLE_MAX*sizeof(char),0);
			if(rec2==-1){
				printf("erreur reception message du client 2");
				return 18;
			}
			printf("recu serveur : %s\n", message);
			if(strcmp(message,"fin")==0){
				int i=0;
				while(message[i]!='\0'){
					messageSend[i]=message[i];
					i++;
				}
				messageSend[i]='\0';
				int su=send(dSClient1,&messageSend,TAILLE_MAX*sizeof(char),0);
				if(su==-1){
					printf("Erreur envoi client2.\n");
					return 17;
				}
				free(message);
				close(dSClient1);
				close(dSClient2);
				echange=0;
			}
			else{
				int i=0;
				while(message[i]!='\0'){
					messageSend[i]=message[i];
					i++;
				}
				messageSend[i]='\0';
				int su=send(dSClient1,&messageSend,TAILLE_MAX*sizeof(char),0);
				if(su==-1){
					printf("Erreur envoi client2.\n");
					return 17;
				}
				free(message);
			}
		}
		if(idActuel==1){
			idActuel=2;
		}
		else if(idActuel==2){
			idActuel=1;
		}
	}
	printf("Fin de la messagerie.\n");
	
	return 0;
}

int main(){
	serveur();
	return 0;
}