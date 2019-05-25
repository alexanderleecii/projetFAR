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
int PORT;
int stockdS[5][10];
pthread_t threads[NBCLIENT];

int dSServeurPrinc;
int dSCli;
int dispoNum[NBCLIENT][1];
char channel_name[5][20] = {"1 : channel1", "2 : channel2", "3 : channel3", "4 : channel4", "5 : channel5"};
int tab_channels[5];
int nbCliActuel = 0;

pthread_t Cli1,Cli2;

struct Client
{
	int dSClient;
	char pseudo[30];
	int choixChannel;
};

struct Client clients[NBCLIENT];

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

//fonction pour creer le serveur qui acceuillera les deux clients.
int serveur(){
	int dS = socket(PF_INET, SOCK_STREAM, 0); //creation socket serveur
	struct sockaddr_in adServeur; //création de la structure contenant l'adresse du serveur
	adServeur.sin_family=AF_INET; //famille d'adresse
	adServeur.sin_addr.s_addr=INADDR_ANY;
	adServeur.sin_port=htons(PORT); //définition du port
	int sockAd = bind(dS,(struct sockaddr*)&adServeur, sizeof(adServeur)); //lie la socket a une adresse
	
	if (sockAd < 0){
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
	printf("Un nouveau client se connecte\n");
	return dSClient;
}

void* thread_communication(void *arg){
	int dSCli = clients[*(int*)arg].dSClient;
	int channel = clients[*(int*)arg].choixChannel;
	
	char mess[TAILLE_MAX];
	char messForme[TAILLE_MAX];

	while(1){
		int recep = recep_mess(mess,dSCli,nbCliActuel);
		strcat(messForme, clients[*(int*)arg].pseudo);
		strcat(messForme, " : ");
		strcat(messForme, mess);

		int nbCliChannel;
		int nbClients=0;
		while(stockdS[channel - 1][nbClients] != -1){
			nbClients++;
		}
	
		for(int i=0;i<nbClients;i++){
			if(strcmp(mess, "fin") == 0){
				if(stockdS[channel - 1][i] != dSCli){
					int res = envoie_mess(mess,stockdS[channel - 1][i],nbCliActuel + i);
				}
			}
			else{
				if(stockdS[channel - 1][i] != dSCli){
					int res = envoie_mess(messForme,stockdS[channel - 1][i],nbCliActuel + i);
				}
			}	
		}
		if(strcmp(mess, "fin") == 0){
			pthread_exit(NULL);
		}
		messForme[0]='\0';
	}
}

void* thread_connexion(void *arg){
	char mess[TAILLE_MAX];
	char channels[TAILLE_MAX];
	char pseudo[30];
	for(int i = 0;i < 5; i++){
		for(int j = 0;j < 10;j++){
			stockdS[i][j]=-1;
		}
	}
	while(nbCliActuel<NBCLIENT){
		strcat(channels,"Channels disponibles : ");
		for(int j = 0;j < 5;j++){
			strcat(channels,channel_name[j]);
			strcat(channels, " ");
		}
		int i=0;
		int dispo=0;//Sert à repérer si le numéro de client est dispo ou pas
		while(dispo!=1){
			if(dispoNum[i][0]==0){
				dispo=1;
			}
			i++;
		}
		
		int numeroDispo = i-1; //Premier numero de client disponible
		dSCli = connexion_client(dSServeurPrinc,numeroDispo+1);

		int res = recep_mess(pseudo,dSCli,numeroDispo+1);

		strcat(clients[nbCliActuel].pseudo, pseudo);

		int envoi_channels = envoie_mess(channels,dSCli,numeroDispo+1);
		if(envoi_channels==-1){
			printf("Probleme envoi message\n");
		}

		strcat(mess,"Entrez le numéro correspondant au channel choisi :");
		res = envoie_mess(mess,dSCli,numeroDispo+1);//Envoi de la demande de choix de channel

		res = recep_mess(mess,dSCli,numeroDispo+1);//Réception du choix de channel

		int choix = atoi(mess);

		clients[nbCliActuel].dSClient = dSCli;
		clients[nbCliActuel].choixChannel = choix;

		int indexInser = 0;
		while(stockdS[choix - 1][indexInser] != -1 && indexInser < 10){
			indexInser++;
		}
		stockdS[choix - 1][indexInser] = clients[nbCliActuel].dSClient;
		int *arg=malloc(sizeof(*arg));
		*arg=nbCliActuel;
		if(pthread_create(&threads[nbCliActuel],NULL,thread_communication,arg)){
			pthread_exit(NULL);
		}
		nbCliActuel+=1;
		//Traiter le mess reçu pour envoyer le client dans le bon salon
		sleep(2);
		mess[0]='\0';
		channels[0]='\0';
	}
	pthread_exit(NULL);
}

int accept_client(){
	while(1){
		printf("Attente d'une connexion...\n");
		int i;
		for(i=0;i<NBCLIENT;i++){
			dispoNum[i][0]=0;
		}

		pthread_t thread;
		int *arg=malloc(sizeof(*arg));
		*arg=dSServeurPrinc;
		if(pthread_create(&thread,NULL,thread_connexion,arg)){
			printf("erreur thread co client");
			return 81;
		}

		while(nbCliActuel < 1){}
		while(1){
			for(int j = 0;j<nbCliActuel;j++){
				if(pthread_join(threads[j],NULL)){
					printf("probleme join\n");
					perror("pthread_join");
					return 82;
				}
			}
		}
	}
}

int main(int argc,char* argv[]){
	strcpy(IP,argv[1]);
	PORT = atoi(argv[2]);

	dSServeurPrinc = serveur();
	accept_client();
	
	return 0;
}