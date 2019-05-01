#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>

#define TAILLE_MAX 500

char IP[100];
int PORT,dS,dSClient1,dSClient2;
int echange=1;
pthread_t Cli1;
pthread_t Cli2;

//fonction pour creer le serveur qui acceuillera les deux clients.
int serveur(){
	int dS = socket(PF_INET, SOCK_STREAM, 0); //creation socket serveur
	struct sockaddr_in adServeur; //création de la structure contenant l'adresse du serveur
	adServeur.sin_family=AF_INET; //famille d'adresse
	adServeur.sin_addr.s_addr=INADDR_ANY;
	adServeur.sin_port=htons(PORT); //définition du port
	int sockAd = bind(dS,(struct sockaddr*)&adServeur, sizeof(adServeur)); //lie la socket a une adresse
	if (sockAd==-1){
		printf("Erreur association\n");
		return -1;
	}
	listen(dS,10); //socket en mode ecoute (10 represente le nb max de demandes de connexion pouvant etre mis en attente)

	return dS;
}
//fonction pour connecter un client au serveur
int connexion_client(int dS, int numClient){

	struct sockaddr_in adClient; //structrue de l'adresse client
	socklen_t lg = sizeof(struct sockaddr_in*);
	int dSClient = accept(dS,(struct sockaddr*)&adClient,&lg);//accepte une connexion client

	if(dSClient == -1){
		printf("Erreur connexion du client numero %d\n", numClient);
	}
	printf("Connexion client %d\n",numClient);
	return dSClient;
}

//fonction pour envoyer un message
int envoie_mess(char *mess, int dSCli, int numClient){
	int taille=(strlen(mess)+1)*sizeof(char);
	int res=send(dSCli,&taille,sizeof(int),0);  
	if(res<0){
		printf("erreur envoi taille\n");
		return 5;
	}
	int envoie = send(dSCli,mess,(strlen(mess)+1)*sizeof(char),0);
	if(envoie == -1){
		printf("Echec de l'envoie du message du client %d\n",numClient);
		return 34;
	}
	return envoie;
}

//fonction pour recevoir un message
int recep_mess(char *mess, int dSCli, int numClient){
	int octMsg;
	int rec = recv(dSCli,&octMsg,sizeof(int),0);//nb octets
	if(rec<0){
		printf("erreur reception taille\n");
	}
	int nbOctRecu=0;
	while(nbOctRecu<octMsg){
		rec = recv(dSCli,mess,octMsg*sizeof(char),0);
		if(rec<0){
			printf("erreur reception thread\n");
			return 66;
		}
		nbOctRecu+=rec;
	}
	if(strcmp(mess, "fin") == 0){
		printf("Fin de la reception.\n");
	}
	return rec;

}
//communcation du client 1 vers le client 2
void *CLi1_vers_cli2(void *arg){
	char mess[TAILLE_MAX];
	while(1){
		int recep = recep_mess(mess,dSClient1,1);
		
		int envoie = envoie_mess(mess,dSClient2,2);
		if(strcmp(mess, "fin") == 0){
			if(pthread_cancel(Cli2)!=0){//On ferme le thread d'échange du client2
				printf("Je n'arrive pas à supprimer le thread 2\n");
			}
			pthread_exit(NULL);
		}
		if(recep<0 || envoie<0){
			printf("Erreur envoi ou recep\n");
		}

	}
}

//communcation du client 2 vers le client 1
void *CLi2_vers_cli1(void *arg){
	char mess[TAILLE_MAX];
	while(1){
		int recep = recep_mess(mess,dSClient2,2);
		
		int envoie = envoie_mess(mess,dSClient1,1);
		if(strcmp(mess, "fin") == 0){
			if(pthread_cancel(Cli1)!=0){//On ferme le thread d'échange du client 1
				printf("Je n'arrive pas à fermer le thread1\n");
			}
			pthread_exit(NULL);
		}
		if(recep<0 || envoie<0){
			printf("Erreur envoi ou recep\n");
		}

	}
}

int communcation(){
	dS=serveur();
	while(1){
		printf("Attente d'une double connexion...\n");
		char bienv[TAILLE_MAX]="bienvenue client 1";
		char bienv2[TAILLE_MAX]="bienvenue client 2";
		//accept client 1 sur serveur
		dSClient1 = connexion_client(dS,1);
		int envoie1 = envoie_mess(bienv,dSClient1,1);
		
		if(envoie1==-1){
			printf("Probleme envoi message\n");
			return 45;
		}
		
		//accept client 2 sur serveur
		dSClient2 = connexion_client(dS,2);
		int envoie2 = envoie_mess(bienv2,dSClient2,1);
		if(envoie2==-1){
			printf("Probleme envoi message\n");
			return 46;
		}
		
		sleep(2);
		//previens au client 1 que deux clients sont connectes
		if(envoie_mess("Vous etes deux connectés sur le serveur\n",dSClient1,1)==-1){
			printf("Probleme envoi message\n");
			return 47;
		//previens au client 2 que deux clients sont connectes
		}
		if(envoie_mess("Vous etes deux connectés sur le serveur\n",dSClient2,1)==-1){
			printf("Probleme envoi message\n");
			return 48;

		}
		
		
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
		echange=0;
		close(dSClient1);
		close(dSClient2);
	}
	close(dS);

	return 0;
}

int main(int argc,char* argv[]){
	strcpy(IP,argv[1]);
	PORT=atoi(argv[2]);
	communcation();
	
	return 0;
}


