#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <limits.h>

#define TAILLE_MAX 500

char IP[100];
int PORT,dS;
pthread_t threadRecep;
pthread_t threadSaisie;

int recep_mess(char *mess, int dScli){
	int octMsg;
		
	int rec = recv(dScli,&octMsg,sizeof(int),0);//nb octets
	if(rec<0){
		printf("erreur reception taille\n");
		return -6;
	}
	int nbOctRecu=0;
	while(nbOctRecu<octMsg){
		rec = recv(dScli,mess,octMsg*sizeof(char),0);
		if(rec<0){
			printf("erreur reception mess\n");
			return -6;
		}
		nbOctRecu+=rec;
	}
	return rec;
}

void *thread_saisie(void *arg){
	char *message=(char *)malloc((TAILLE_MAX)*sizeof(char));
	char recupMessage[TAILLE_MAX];
	while(1){
		//recupere l'entree clavier dans recupMessage
		fgets(recupMessage,TAILLE_MAX,stdin);
		char *pos=strchr(recupMessage,'\n');//repere et remplace le \n ajouté automatiquement à la fin de la chaine de caractere par un \0
		*pos='\0';

		message=recupMessage;
		int taille=(strlen(message)+1)*sizeof(char);
		int res=send(dS,&taille,sizeof(int),0);  
		if(res<0){
			printf("erreur envoi taille\n");
		}
		res=send(dS,message,(strlen(message)+1)*sizeof(char),0);
		if(res<0){
			printf("Erreur envoi message.\n");
		}
		if(strcmp(message,"fin")==0){//Si le client a envoye "fin" on arrete ce thread
			pthread_cancel(threadRecep);
			pthread_exit(NULL);
		}
	}
}

void *thread_reception(void *arg){
	while(1){
		char mess[TAILLE_MAX];
		char messageForme[TAILLE_MAX]="\n                         > ";
		recep_mess(mess,dS);
		strcat(messageForme,mess);
		puts(messageForme);
		//verifie si le message envoye est fin, si oui exit 
		if(strcmp(mess,"fin")==0){
			printf("Déconnexion du serveur.\n");
			pthread_cancel(threadSaisie);
			pthread_exit(NULL);
		}
	}
}


int communication(){
	//mise en place du client
	dS=socket(PF_INET, SOCK_STREAM,0);//Creation socket

	if(dS==-1){
		printf("Erreur de création du socket client.\n");
		return 1;
	}
	
	struct sockaddr_in adServ; //définit un type générique d'adresses
	adServ.sin_family=AF_INET;
	adServ.sin_port=htons(PORT);

	int res=inet_pton(AF_INET,IP,&(adServ.sin_addr)); //crée une structure d'adresse
	if(res==0 || res==-1){//renvoie 0 si IP ne contient pas une adresse valide pour la famille AF_INET, -1 si af ne contient pas de famille d'adresse valide
		printf("Erreur de création de la structure d'adresse.\n");
		return 2;
	}
	socklen_t lgA=sizeof(struct sockaddr_in);//longueur de l'adresse
	int resCo=connect(dS,(struct sockaddr *)&adServ,lgA);//effectue une demande de connexion à l'adresse IP
	perror("connect");
	if(resCo==-1){
		printf("Erreur de connexion.\n");
		return 3;
	}

	int trecep = pthread_create(&threadRecep,NULL,thread_reception,NULL);
	if(trecep!=0){
		printf("Erreur thread recepetion\n");
		return 80;
	}
	
	int tsaisie = pthread_create(&threadSaisie,NULL,thread_saisie,NULL);
	if(tsaisie!=0){
		printf("Erreur thread saisie\n");
		return 81;
	}
	
	if(pthread_join(threadRecep,NULL)){
		printf("Probleme join reception\n");
		return 82;
	}
	
	if(pthread_join(threadSaisie,NULL)){
		printf("Probleme join saisie\n");
		return 83;
	}
	close(dS);
	return 0;
}




int main(int argc, char* argv[]){
	strcpy(IP,argv[1]); //On récupère l'IP dans le premier argument passé lors de l'exécution
	PORT=atoi(argv[2]); //On récupère le PORT dans le second argument
	communication();
	return 0;
}



