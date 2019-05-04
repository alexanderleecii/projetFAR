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

int get_last_tty() {
  FILE *fp;
  char path[1035];
  fp = popen("/bin/ls /dev/pts", "r");
  if (fp == NULL) {
    printf("Impossible d'exécuter la commande\n" );
    exit(1);
  }
  int i = INT_MIN;
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    if(strcmp(path,"ptmx")!=0){
      int tty = atoi(path);
      if(tty > i) i = tty;
    }
  }

  pclose(fp);
  return i;
}

FILE* new_tty() {
  pthread_mutex_t the_mutex;  
  pthread_mutex_init(&the_mutex,0);
  pthread_mutex_lock(&the_mutex);
  system("gnome-terminal"); sleep(1);
  char *tty_name = ttyname(STDIN_FILENO);
  int ltty = get_last_tty();
  char str[2];
  sprintf(str,"%d",ltty);
  int i;
  for(i = strlen(tty_name)-1; i >= 0; i--) {
    if(tty_name[i] == '/') break;
  }
  tty_name[i+1] = '\0';  
  strcat(tty_name,str);  
  FILE *fp = fopen(tty_name,"wb+");
  pthread_mutex_unlock(&the_mutex);
  pthread_mutex_destroy(&the_mutex);
  return fp;
}

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

void *thread_envoi_fic(void *arg){
	FILE* fp1 = new_tty();
	fprintf(fp1,"%s\n","Ce terminal sera utilisé uniquement pour l'affichage");

	// Demander à l'utilisateur quel fichier afficher
	DIR *dp;
	struct dirent *ep;     
	dp = opendir ("./Envois");
	if (dp != NULL) {
		fprintf(fp1,"Voilà la liste de fichiers :\n");
		while ( (ep = readdir(dp)) ) {
		if(strcmp(ep->d_name,".")!=0 && strcmp(ep->d_name,"..")!=0) 
			fprintf(fp1,"%s\n",ep->d_name);
		}    
		(void) closedir (dp);
	}
	else {
		perror ("Ne peux pas ouvrir le répertoire");
	}
	printf("Indiquer le nom du fichier : ");
	char fileName[1023];
	char dossier[1023]="./Envois/";
	fgets(fileName,sizeof(fileName),stdin);
	fileName[strlen(fileName)-1]='\0';
	strcat(dossier,fileName);
	FILE *fichier = fopen(dossier, "r");
	if (fichier == NULL){
		printf("Ne peux pas ouvrir le fichier suivant : %s\n",fileName);
	}
	else {
		int taille=(strlen(fileName)+1)*sizeof(char);			  //Envoi du
		int res=send(dS,&taille,sizeof(int),0);      			  //Nom du
		res=send(dS,fileName,taille,0);//Fichier à envoyer

		char str[TAILLE_MAX];
		char end_char[1];
		end_char[0]=EOF;  
		// Lire et afficher le contenu du fichier
		while (fgets(str, TAILLE_MAX, fichier) != NULL) {
			taille=(strlen(str)+1)*sizeof(char);
			res=send(dS,&taille,sizeof(int),0);
			res=send(dS,str,taille,0);
			if(res<0){
				printf("Erreur envoi fichier\n");
			}
		}
		taille=(strlen(end_char)+1)*sizeof(char);
		res=send(dS,&taille,sizeof(int),0);
		res=send(dS,end_char,taille,0);
		if(res<0){
			printf("Erreur envoi fichier\n");
		}
	}
	fclose(fichier);
	pthread_exit(NULL);
}

void *thread_recep_fic(void *arg){
	int octMsg;
	int rec = recv(dS,&octMsg,sizeof(int),0);//nb octets nom fichier
	if(rec<0){
		printf("Erreur reception taille nom fichier\n");
		pthread_exit(NULL);
	}
	char fileName[octMsg*sizeof(char)];
	int nbOctRecu=0;
	while(nbOctRecu<octMsg){
		rec = recv(dS,fileName,octMsg*sizeof(char),0);
		if(rec<0){
			printf("Erreur reception nom fichier\n");
			pthread_exit(NULL);
		}
		nbOctRecu+=rec;
	}
	//Création du chemin de destination du fichier
	char* extension="./Recep/";
	char* buffer=malloc(strlen(extension)+octMsg*sizeof(char));
	strcpy(buffer,extension);
	strcat(buffer,fileName);
	printf("Fichier reçu : %s\n",buffer);

	FILE* fichier = fopen(buffer,"w");
	char str[TAILLE_MAX];
	while(1){
		rec=recep_mess(str,dS);
		if(str[0]==EOF){//On sort de la boucle de réception du fichier lorsqu'on rencontre le message contenant uniquement ce caractère
			break;
		}
		fputs(str,fichier);
	}
	fclose(fichier);
	free(buffer);
	pthread_exit(NULL);
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
		if(strcmp(message,"file")==0){
			int taille=(strlen(message)+1)*sizeof(char);
			int res=send(dS,&taille,sizeof(int),0);  
			if(res<0){
				printf("erreur envoi taille\n");
			}
			res=send(dS,message,(strlen(message)+1)*sizeof(char),0);
			if(res<0){
				printf("Erreur envoi message.\n");
			}
			pthread_t envoiFic;
			if(pthread_create(&envoiFic,NULL,thread_envoi_fic,NULL)!=0){
				printf("Erreur création thread fichier\n");
			}

			if(pthread_join(envoiFic,NULL)!=0){
				printf("Probleme join fichier\n");
			}
		}
		else{
			int taille=(strlen(message)+1)*sizeof(char);
			int res=send(dS,&taille,sizeof(int),0);  
			if(res<0){
				printf("erreur envoi taille\n");
			}
			res=send(dS,message,(strlen(message)+1)*sizeof(char),0);
			if(res<0){
				printf("Erreur envoi message.\n");
			}
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
		
		if(strcmp(mess,"file")==0){//Si on reçoit le message "file" on se prépare à recevoir un fichier
			pthread_t recepFic;
			if(pthread_create(&recepFic,NULL,thread_recep_fic,NULL)!=0){
				printf("Erreur création thread fichier\n");
			}

			if(pthread_join(recepFic,NULL)!=0){
				printf("Probleme join fichier\n");
			}
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



