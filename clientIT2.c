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


int dS;

int recep_mess(char *mess, int dScli){
	

	int rec = recv(dScli,&mess,TAILLE_MAX*sizeof(char),0);//Reception à partir du serveur dans le client

	if(rec==-1){
		printf("Erreur reception client \n");
			return 4;
	}
	//verifie si le message envoye est fin, si oui exit 
	if(strcmp(mess, "fin") == 0){
		printf("deconnection du serveur.\n");
		sleep(3);
		exit(1);//pas sur de cet exit()
	}
	if(strcmp("bienvenue client 1",mess)==0){
		printf("bienvenue client 1");
	}
	if(strcmp("bienvenue client 2",mess)==0){
		printf("bienvenue client 2");
	}
	puts(mess);

	return rec;

}

void *thread_saisie(void *arg){
	char *message=(char *)malloc((TAILLE_MAX+1)*sizeof(char));//buffer
	char recupMessage[TAILLE_MAX];
	while(1){
		//recupere l'entree clavier dans recupMessage
		fgets(recupMessage,TAILLE_MAX,stdin);
		char *pos=strchr(recupMessage,'\n');//repere et remplace le \n ajouté automatiquement à la fin de la chaine de caractere par un \0
		*pos='\0';

		message=recupMessage;

		if(strcmp(message,"fin")==0){
			exit(1);
		}

		send(dS,message,TAILLE_MAX*sizeof(char),0);
	}
	

}

void *thread_reception(void *arg){
	char mess[TAILLE_MAX]="";
	while(1){
		recep_mess(mess,dS);
		if(strcmp(mess,"fin")==0){
			exit(1);
		}
	puts(mess);

	}



}







int communication(){
	//mise en place du client
	int dS=socket(PF_INET, SOCK_STREAM,0);//Creation socket

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
	if(resCo==-1){
		printf("Erreur de connexion.\n");
		return 3;
	}

	char mess[TAILLE_MAX]="";
	while(1){
		pthread_t threadRecep;
		pthread_t threadSaisie;

		int trecep = pthread_create(&threadRecep,NULL,thread_reception,NULL);
		if(trecep!=0){
			printf("erreur thread recepetion");
			return 80;
		}

		int tsaisie = pthread_create(&threadSaisie,NULL,thread_saisie,NULL);
		if(tsaisie!=0){
			printf("erreur thread saisie");
			return 81;
		}

		if(pthread_join(thread_reception,NULL)){
			printf("probleme join reception");
			return 82;
		}

		if(pthread_join(thread_saisie,NULL)){
			printf("probleme join saisie");
			return 83;
		}

		return 0;
	}
}




int main(){
	communication();
	return 0;
}



