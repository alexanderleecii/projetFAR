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
#define IP "172.26.125.102"
#define NBCLIENT 50

int nbCliActuel;
int dSCli[NBCLIENT];
char tabPseudo[NBCLIENT][30];

pthread_t connecte[NBCLIENT];


void recep_pseudo(char *pseudo, int num){
	if(recv(dSCli[num],pseudo,(strlen(pseudo)+1)*sizeof(char),0)!=0){ //&pseudo ?
		printf("erreur reception pseudo\n");
	}
	strcpy(tabPseudo[num],pseudo); //&pseudo ? //stock les pseudo
}

void* thread_recep_envoie(void *arg){
	char mess[TAILLE_MAX];
	int num = *((int *) arg);//Revoir ça
	if(recv(dSCli[num],mess,(strlen(mess)+1)*sizeof(char),0)!=0){
		printf("erreur reception thread\n");
	}



	if(strcmp(mess,"fin")==0){
		for(int i=0;i<nbCliActuel;i++){
			close(dSCli[i]);
		}
	}
	char affichage[TAILLE_MAX];
	strcpy(affichage,tabPseudo[num]);
	strcat(affichage," : ");
	strcat(affichage,mess);

	for(int parcoursCli=0;parcoursCli<nbCliActuel;parcoursCli++){
		if(parcoursCli!=num){
			if(send(dSCli[parcoursCli],affichage,(strlen(affichage)+1)*sizeof(char),0)!=0){
				printf("erreur global send\n");
			}
		}
	}
	pthread_exit(NULL);

}

void* thread_connexion(void *arg){
	nbCliActuel=0;
	int dS = *((int *) arg);
	struct sockaddr_in aClient;
	char pseudo[30];
	int nb=0;
	socklen_t lgA= sizeof(struct sockaddr_in);
	while(nb<NBCLIENT){
		dSCli[nb] = accept(dS, (struct sockaddr *)&aClient,&lgA);
		if(dSCli[nb]!=0){
			printf("erreur connexion client thread");
		}
		recep_pseudo(pseudo,nb);
		printf("CLIENT %d pseudo %s \n",nb+1,tabPseudo[nb]);
		int *arg=malloc(sizeof(*arg)); //Revoir ça
		*arg=nb;//Revoir ça
		int pcreate = pthread_create(&connecte[nb],NULL,thread_recep_envoie,arg);
		if(pcreate!=0){
			printf("erreur thread connect");
		}
		nbCliActuel++;
		nb++;
	}

	pthread_exit(NULL);

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

	while(1){
		nbCliActuel=0;
		printf("le serveur sera operationnel lorsque deux clients seront connectes\n");

		pthread_t thread;
		int *arg=malloc(sizeof(*arg)); //Revoir ça
		*arg=dS;//Revoir ça
		if(pthread_create(&thread,NULL,thread_connexion,arg)){
			printf("erreur thread co client");
			return 81;
		}

		//le systeme se lance lorsque deux clients sont connectes 
		while(nbCliActuel<2){

		}

		if(pthread_join(connecte[0],NULL)){
			printf("probleme join\n");
			return 82;
		}

		


	}

	return 0;
}



