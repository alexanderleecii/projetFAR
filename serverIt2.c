//pour n client coter server un thread par client , utilisation de tableau pour stocker client.

#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define PORT 2633
#define IP "192.168.1.16"
#define TAILLE_MAX 5

//fonction qui creer le serveur
int creation_serveur(){

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

}
//tableau de taille deux avec les descripteur de socket dedans

//fonction qui relie les messages du client 1 vers le client 2

int cli1_vers_cli2(int tab[]){

    dSClient1=tab[0];
    dSClient2=tab[1];


    while (1){
        //reception message venant de 1 et envoi au client 2
        int rec1 = recv(dSClient1, message,TAILLE_MAX*sizeof(char),0);
        if(rec1==-1){
            printf("erreur reception message du client 1");
            return 16;
        }
        printf("recu serveur : %s\n", message);
        if(strcmp(message,"fin")==0){
            int i=0;
            while(message[i]!='\0'){ //on copie le message contenu dans le buffer dans une chaîne de caractères en s'assurant qu'elle se termine bien par \0
                messageSend[i]=message[i];
                i++;
            }
            messageSend[i]='\0';

            int su=send(dSClient2,&messageSend,TAILLE_MAX*sizeof(char),0);//Envoi au client2
            if(su==-1){
                printf("Erreur envoi client2.\n");
                return 17;
            }
            
            free(message); //On libère la mémoire allouée au pointeur message
            close(dSClient1);
            close(dSClient2);
            echange=0;
        }
        else{
            int i=0;
            while(message[i]!='\0'){//on copie le message contenu dans le buffer
                messageSend[i]=message[i];//dans une chaîne de caractères en s'assurant qu'elle se termine bien par \0
                i++;
            }
            messageSend[i]='\0';

            int su=send(dSClient2,&messageSend,TAILLE_MAX*sizeof(char),0);//Envoi au client2
            if(su==-1){
                printf("Erreur envoi client2.\n");
                return 17;
            }
            free(message);//On libere la mémoire allouée à message		
        }
    }
    

}

int cli2_vers_cli1(int tab[]){

    dSClient1=tab[0];
    dSClient2=tab[1];

    while(1){

        //reception message venant de 2 et envoi au client 1
        int rec2 = recv(dSClient2, message,TAILLE_MAX*sizeof(char),0);
        if(rec2==-1){
            printf("erreur reception message du client 2");
            return 18;
        }
        printf("recu serveur : %s\n", message);
        if(strcmp(message,"fin")==0){
            int i=0;
            while(message[i]!='\0'){//on copie le message contenu dans le buffer
                messageSend[i]=message[i];//dans une chaîne de caractères en s'assurant qu'elle se termine bien par \0
                i++;
            }
            messageSend[i]='\0';

            int su=send(dSClient1,&messageSend,TAILLE_MAX*sizeof(char),0);//Envoi au client1
            if(su==-1){
                printf("Erreur envoi client2.\n");
                return 17;
            }
            free(message);//On libere la mémoire allouée à message
            close(dSClient1);
            close(dSClient2);
            echange=0;
        }
        else{
            int i=0;
            while(message[i]!='\0'){//on copie le message contenu dans le buffer
                messageSend[i]=message[i];//dans une chaîne de caractères en s'assurant qu'elle se termine bien par \0
                i++;
            }
            messageSend[i]='\0';

            int su=send(dSClient1,&messageSend,TAILLE_MAX*sizeof(char),0);//Envoi au client1
            if(su==-1){
                printf("Erreur envoi client2.\n");
                return 17;
            }
            free(message);//On libere la memoire allouée à message
        }    

    }
        

}



int main (void){
    //mise en place du serveur
    creation_serveur();

    //accept client 1 sur serveur
    int dSClient1 = accept(dS,(struct sockaddr*)&adClient1,&lg); //connexion socket
    if (dSClient1 == -1){
        printf("erreur connexion socket client1");
        return 12;
    }

    //accept client 2 sur serveur
    int dSClient2 = accept(dS,(struct sockaddr*)&adClient2,&lg); //connexion socket
    if (dSClient2 == -1){
        printf("erreur connexion socket client2");
        return 14;
    }

    int tabCli[2]:
    tabCli[0]=dSClient1;
    tabCli[1]=dSClient2;
    //identifiant thread pour la communication du client 1 vers le client 2
    pthread_t threadCli1_to_Cli2;
    //identifiant thread pour la communication du client 2 vers le client 1
    pthread_t threadCli2_to_Cli1;

    //creation thread client 1 vers client 2
    int thread1 = pthread_create(&threadCli1_to_Cli2, NULL, cli1_vers_cli2, tabCli)
    if (thread1 == -1){
        printf("erreur thread client1 vers client 2");
        return 98;
    }
    //creation thread client 2 vers client 1
    int thread2 = pthread_create(&threadCli2_to_Cli1, NULL, cli1_vers_cli2, tabCli)
    if (thread2 == -1){
        printf("erreur thread client1 vers client 2");
        return 99;
    }



}