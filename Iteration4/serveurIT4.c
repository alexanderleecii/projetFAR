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
#define NBCLIENT 50

char IP[100];
int PORT,dS;
int dSCli[50];
int dispoNum[NBCLIENT][1];

pthread_t Cli1,Cli2;

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

void* thread_connexion(void *arg){
	char mess[TAILLE_MAX]="Entrez le numéro correspondant au channel choisi :";
	char channels[TAILLE_MAX]="Channels disponibles : ";
	for(int j = 0;j < 5;j++){
		strcat(channels,channel_name[j]);
		strcat(channels, " ");
	}
	
	while(nbCliActuel<NBCLIENT){
		int i=0;
		int dispo=0;//Sert à repérer si le numéro de client est dispo ou pas
		while(dispo!=1){
			if(dispoNum[i][0]==0){
				dispo=1;
			}
			i++;
		}
		
		int numeroDispo = i-1; //Premier numero de client disponible
		dSCli[numeroDispo] = connexion_client(dS,1);
		int envoi_channels = envoie_mess(channels,dSClient[numeroDispo],numeroDispo+1);
		if(envoi_channels==-1){
			printf("Probleme envoi message\n");
			return 45;
		}
		int res = envoie_mess(mess,dSClient[numeroDispo],numeroDispo+1);//Envoi de la demande de choix de channel

		res = recep_mess(mess,dSClient[numeroDispo],numeroDispo+1);//Réception du choix de channel
		//Traiter le mess reçu pour envoyer le client dans le bon salon
		sleep(2);
	}
}

int accept_client(){
	while(1){
		printf("Attente d'une connexion...\n");
		for(int i=0;i<NBCLIENT;i++){
			dispoNum[i][0]=0;
		}
		nbCliActuel=0;

		pthread_t thread;
		int *arg=malloc(sizeof(*arg)); //Revoir ça
		*arg=dS;//Revoir ça
		if(pthread_create(&thread,NULL,thread_connexion,arg)){
			printf("erreur thread co client");
			return 81;
		}

		if(pthread_join(connecte[0],NULL)){
			printf("probleme join\n");
			return 82;
		}
	}
}

int init_channels(){
	int tab_channels[5];
	char channel_name[5][20] = {"1 : channel1", "2 : channel2", "3 : channel3", "4 : channel4", "5 : channel5"};
	for(int i = 0;i<5;i++){
		tab_channels[i] = serveur();
	}
	return
}

int main(int argc,char* argv[]){
	strcpy(IP,argv[1]);
	PORT=atoi(argv[2]);
	dS = serveur()
	init_channels();
	accept_client();
	
	return 0;
}