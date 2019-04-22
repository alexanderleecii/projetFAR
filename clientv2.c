//dans le message diffuse par le serveur il faudrait prevoir une partie contenant l'identité du client. 
//stocker descripteur de sockets dans un tableau 

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
#define NBCLIENT 50

int dS;


void send_pseudo(char* pseudo){
	if(send(dS,pseudo,(strlen(pseudo)+1)*sizeof(char),0)!=0){
		printf("erreur envoie pseudo\n");
		pthread_exit(NULL);
	}
}

void recep_mess(char *mess,int dSCli){

	if(recv(dSCli,mess,(strlen(mess)+1)*sizeof(char),0)!=0){
		printf("erreur reception message.\n");
	}

}

void *thread_reception(void *arg){
	int arret = 1;
	char mess[TAILLE_MAX];
	while(arret==1){
		recep_mess(mess,dS);
		//verifie si mess==fin, si oui on coupe le thread sinon on continue
		if(strcmp(mess,"fin")==0){
			arret=0;
		}
		puts(mess);

	}
	pthread_exit(NULL);

}

void *thread_saisie(void *arg){
	
	char *message=(char *)malloc((TAILLE_MAX)*sizeof(char));//buffer
	char recupMessage[TAILLE_MAX];
	while(1){
		fgets(recupMessage,TAILLE_MAX,stdin);
		char *pos=strchr(recupMessage,'\n');//repere et remplace le \n ajouté automatiquement à la fin de la chaine de caractere par un \0
		*pos='\0';

		message=recupMessage;

		if(send(dS,message,(strlen(message)+1)*sizeof(char),0)!=0){
			printf("erreur saisie.\n");
		}
	}
	pthread_exit(NULL);

}

int communication(){
	//mise ne place du client 
	int dS = socket(PF_INET, SOCK_STREAM,0); //creation socket

	if(dS==-1){
		printf("erreur de creation de socket client.\n");
		return 21;
	}

	struct sockaddr_in adServ; //definit un type generique d'adresses
	adServ.sin_family=AF_INET;
	adServ.sin_port=htons(PORT);

	int res=inet_pton(AF_INET,IP,&(adServ.sin_addr)); //cree une structure d'adresse

	if(res == 0 || res == -1){  //renvoie 0 si IP ne contient pas une adresse valide pour la famille AF_INET, -1 si af ne contient pas de famille d'adresse valide
		printf("Erreur de creation de la structure d'adresse.\n");
		return 22;
	}

	socklen_t lgA=sizeof(struct sockaddr_in); //longueur de l'adresse
	int resCo=connect(dS,(struct sockaddr *)&adServ,lgA); //effectue une demande de connexion à l'adresse IP

	if(resCo!=0){
		printf("erreur de connexion.\n");
		return 23;
	}
	//tableau pour stocker les pseudo
	char tabPseudo[NBCLIENT];
	//stock le pseudo
	char pseudo[30];
	
	//a chaque nouvelle connexion client celui ci doit choisir un pseudo qui n'existe pas deja
	int arret = 0;
	while(arret=1){
		printf("veuillez choisir un pseudo qui ne depasse pas 30 caracteres :\n");
		fgets(pseudo,30,stdin);
		char *pos=strchr(pseudo,'\n');//repere et remplace le \n ajouté automatiquement à la fin de la chaine de caractere par un \0
		*pos='\0';
		int position=0; 
		for(int i=0;i<position;i++){
			if(tabPseudo[i]==pseudo){
				printf("pseudo deja utilise veuillez en choisir un autre\n");
				printf("\n");
			}
			else{
				tabPseudo[position]=pseudo;
				position+=1;
				arret=0;
			}
		}
	}


	pthread_t threadRecep;
	pthread_t threadSaisie;

	int trecep = pthread_create(&threadRecep,NULL,thread_reception,NULL);
	if(trecep!=0){
		printf("erreur thread recep\n");
		return 24;
	}

	int tsaisie = pthread_create(&threadSaisie,NULL,thread_saisie,NULL);
	if(tsaisie!=0){
		printf("erreur thread saisie\n");
		return 25;
	}

	if(pthread_join(threadRecep,NULL)!=0){
		printf("erreur join recep\n");
		return 26;
	}

	if(pthread_join(threadSaisie,NULL)!=0){
		printf("erreur join saisie\n");
		return 27;
	}

	return 0;

}


int main(){
	communication();
	return 0;
}


