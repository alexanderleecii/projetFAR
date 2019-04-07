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
#define TAILLE_MAX 50*sizeof(char)

int connexionServeur(int port, char ip[]){
	int dS=socket(PF_INET, SOCK_STREAM,0);//Creation socket

	if(dS==-1){
		printf("Erreur de création du socket client.\n");
		return 1;
	}

	struct sockaddr_in adServ; //définit un type générique d'adresses
	adServ.sin_family=AF_INET;
	adServ.sin_port=htons(port);

	int res=inet_pton(AF_INET,IP,&(adServ.sin_addr)); //crée une structure d'adresse
	if(res==0 || res==-1){//renvoie 0 si IP ne contient pas une adresse valide pour la famille AF_INET, -1 si af ne contient pas de famille d'adresse valide
		printf("Erreur de création de la structure d'adresse.\n");
		return 2;
	}

	socklen_t lgA=sizeof(struct sockaddr_in);//longueur de l'adresse
	int resCo=connect(dS,(struct sockaddr *)&adServ,lgA);//effectue une demande de connexion à l'adresse IP
	if(resCo==-1){
		printf("Erreur de connexion.\n");
		return 3;
	}

	return dS;
}

int echange(){
	int id1,id2,idActuel;
	int client1=connexionServeur(PORT,IP); //On crée un premier socket + connexion au serveur
	int res=recv(client1,&id1,sizeof(int),0); //On réceptionne l'id du premier socket qui servira à organiser l'échange
	if(res==-1){
		printf("Reception non reussie.\n");
		return 1;
	}
	int client2=connexionServeur(PORT,IP); //On crée un second socket + connexion
	res=recv(client2,&id2,sizeof(int),0); //On receptionne l'id du second socket
	if(res==-1){
		printf("Reception non reussie.\n");
		return 2;
	}
	
	char *message;
	char recupMessage[TAILLE_MAX];
	char messageRecu[TAILLE_MAX];

	idActuel=id1;
	while(idActuel!=-1){
		if(idActuel==id1){
			printf("Client 1, entrez un message : ");
			fgets(recupMessage,TAILLE_MAX,stdin);
			message=recupMessage;
			int s = send(client1,message,TAILLE_MAX,0);
			if(s==-1){
				printf("Erreur envoi client 1\n");
				return 3;
			}
			int rec = recv(client2,messageRecu,TAILLE_MAX,0);
			
			if(rec==-1){
				printf("Erreur reception client 2\n");
				return 4;
			}
			printf("Client 1 : %s\n",messageRecu);
		}
		else if(idActuel==id2){
			printf("Client 2, entrez un message : ");
			fgets(recupMessage,TAILLE_MAX,stdin);
			message=recupMessage;
			int s = send(client2,message,TAILLE_MAX,0);
			if(s==-1){
				printf("Erreur envoi client 2\n");
				return 5;
			}
			int rec = recv(client1,messageRecu,TAILLE_MAX,0);
			if(rec==-1){
				printf("Erreur reception client 1\n");
				return 6;
			}
			printf("Client 2 : %s\n",messageRecu);
		}
		if(strcmp(messageRecu,"fin")==0){
			idActuel=-1;
		}
		else if(idActuel==id1){
			idActuel=id2;
		}
		else{
			idActuel=id1;
		}
	}
	close(client1);
	close(client2);
	return 0;
}

int main(int argc, char* argv[])//Client
{
	echange();
	return 0;
}