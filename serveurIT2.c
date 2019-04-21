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

int dS,dSClient1,dSClient2;

//fonction pour creer le serveur qui accuillera les deux clients.
int serveur(){
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
	listen(dS,10); //socket en mode ecoute (10 represente le nb max de demandes de connexion pouvant etre mis en attente)

	return dS;
}
//fonction pour connecter un client au serveur
int connexion_client(int dS, int numClient){

	struct sockaddr_in adClient; //structrue de l'adresse client
	socklen_t lg = sizeof(struct sockaddr_in*);
	int dSClient = accept(dS,(struct sockaddr*)&adClient,&lg);

	if(dSClient == -1){
		printf("erreur connexion du client numero %d", numClient);
	}
	return dSClient;
}

//fonction pour envoyer un message
int envoie_mess(char *mess, int dSCli, int numClient){
	int envoie = send(dSCli,&mess,TAILLE_MAX*sizeof(char),0);
	if(envoie == -1){
		printf("echec de l'envoie du message du client %d",numClient);
		return 34;
	}
	return envoie;
}

//fonction pour recevoir un message
int recep_mess(char *mess, int dSCli, int numClient){

	int rec = recv(dSCli,&mess,TAILLE_MAX*sizeof(char),0);
	if(rec==-1){
		printf("erreur reception");
		return 66;
	}
	return rec;

}
//communcation du client 1 vers le client 2
void *CLi1_vers_cli2(void *arg){
	char mess[TAILLE_MAX]="";
	while(1){
		int recep = recep_mess(mess,dSClient1,1);

		int envoie = envoie_mess(mess,dSClient2,2);

		if(recep!=0 || envoie!=0){
			printf("erreur envoie ou recep");
		}

	}
}

//communcation du client 2 vers le client 1
void *CLi2_vers_cli1(void *arg){
	char mess[TAILLE_MAX]="";
	while(1){
		int recep = recep_mess(mess,dSClient2,2);

		int envoie = envoie_mess(mess,dSClient1,1);

		if(recep!=0 || envoie!=0){
			printf("erreur envoie ou recep");
		}

	}
}

int communcation(){
	dS=serveur();
	while(1){
		//accept client 1 sur serveur
		dSClient1 = connexion_client(dS,1);
		int envoie1 = envoie_mess("bienvenue client 1",dSClient1,1);
		if(envoie1==-1){
			printf("probleme envoie message");
			return 45;
		}

		//accept client 2 sur serveur
		dSClient2 = connexion_client(dS,2);
		int envoie2 = envoie_mess("bienvenue client 2",dSClient2,1);
		if(envoie2==-1){
			printf("probleme envoie message");
			return 46;
		}
		printf("attente d'une double connexion");
		sleep(5);
		//previens au client 1 que deux clients sont connectes
		if(envoie_mess("vous etes deux connectez sur le serveur",dSClient1,1)==-1){
			printf("probleme envoie message");
			return 47;
		//previens au client 2 que deux clients sont connectes
		}
		if(envoie_mess("vous etes deux connectez sur le serveur",dSClient2,1)==-1){
			printf("probleme envoie message");
			return 48;

		}

		pthread_t Cli1;
		pthread_t Cli2;

		int cr1 = pthread_create(&Cli1,NULL,CLi1_vers_cli2,NULL);
		if(cr1!=0){
			printf("erreur thread client 1");
			return 72;
		}

		int cr2 = pthread_create(&Cli2,NULL, CLi2_vers_cli1,NULL);
		if(cr2!=0){
			printf("erreur thread client 2");
			return 73;
		}

		int p1 = pthread_join(Cli1,NULL);
		if(p1!=0){
			printf("erreur pause client 1");
			return 74;
		}

		int p2 = pthread_join(Cli2,NULL);
		if(p2!=0){
			printf("erreur pause client 2");
			return 75;
		}

	}

	close(dS);
	return 0;
}

int main(){
	communcation();
	return 0;
}


