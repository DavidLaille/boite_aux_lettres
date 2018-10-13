#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "boite_lettres.h"

////////////////////////////////////////////////////////////////////////////////////////////

void construire_message(char* message, int motif, int lg){
  int i;
  motif = motif%26;
  motif = (char) (motif + 97);
  for(i=0; i<lg; i++){
      message[i] = motif;
  }
  message[lg]='\0';
}

////////////////////////////////////////////////////////////////////////////////////////////

void afficher_message(char* message, int lg){
  int i;
  for(i=0; i<lg; i++){
    printf("%c", message[i]);}
  printf("\n");

}

/////////////////////////////////////////////////////////////////////////////////////////////

int main (int argc, char **argv)
{
  int c, i; int compteur=0;
  extern char *optarg;
  extern int optind;
  int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10
			  en émission, infini en réception */
  int source = -1 ; /* 0=puits, 1=source */
  int protocole = 0 ; /* 0=TCP, 1=UDP */

  int s; int sbis;
  int num_port ;
  char * nom_dest;
  struct hostent *hp;
  struct sockaddr_in addr_distant, addr, addr_emetteur;
  socklen_t lg_addr_emetteur;

  struct sockaddr_in addr_serveur, addr_client;
  socklen_t lg_addr_client;

  int longueur_message = -1;

  while ((c = getopt(argc, argv, "psun:l:")) != -1) {
    switch (c) {
    case 'p':
      if (source == 1) {
	printf("usage: cmd [-p|-s][-n ##][-l ##]\n");
	exit(1) ;}
      source = 0;
      num_port = atoi(argv[argc-1]);
      num_port = htons(num_port); // sous Linux (inutile sous windows)
      break;
    case 's':
      if (source == 0) {
	printf("usage: cmd [-p|-s][-n ##]\n");
	exit(1) ;}
      source = 1;
      nom_dest = argv[argc-2];
      num_port = atoi(argv[argc-1]);
      num_port = htons(num_port); // sous Linux
      break;
    case 'u':
      protocole = 1;
      break;
    case 'n':
      nb_message = atoi(optarg);
      break;
    case 'l':
      longueur_message = atoi(optarg);
      break;
    default:
      printf("usage: cmd [-p|-s][-n ##]\n");
      break;
    }
  }
  if (source == -1) {
    printf("usage: cmd [-p|-s][-n ##]\n");
    exit(1) ;
  }
 if (nb_message == -1) { // Cas sans option -n  
   nb_message = 10 ; // On initialise le nombre de message par défaut
  }   
 if (longueur_message == -1) { // Cas sans option -n  
   longueur_message = 30 ; // On initialise la taille des messages par défaut
  }  
 
 char message_envoi[longueur_message];
 char message_recu[longueur_message];

  //////////////////////////////////////////////////////////////////////////////////////////////
 // Création du socket de communication, adressage, connection(si TCP) ou mise en attente de réception (si UDP)

  // Cas Source

  if (source == 1){
    if (protocole == 1){ // Cas UDP
      s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      addr_distant.sin_family = AF_INET;
      addr_distant.sin_port = num_port;
      if((hp = gethostbyname(nom_dest)) == NULL){
	printf("erreur gethostbyname \n");
	exit(1);
      }
      memcpy((char*)&(addr_distant.sin_addr.s_addr), hp->h_addr, hp->h_length);
    }    
    else{ // Cas TCP
      s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      addr_serveur.sin_family = AF_INET;
      addr_serveur.sin_port = num_port;
      if((hp = gethostbyname(nom_dest)) == NULL){
	printf("erreur gethostbyname \n");
	exit(1);
      }
      memcpy((char*)&(addr_serveur.sin_addr.s_addr), hp->h_addr, hp->h_length);
      if(connect(s, (struct sockaddr *)&addr_serveur, sizeof(addr_serveur)) == -1){
	printf("erreur connect \n");
	exit(1);
      }
      
    }    
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////
 
  // Cas Puits

  else{
    if (protocole == 1){ // Cas UDP
      s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      addr.sin_family = AF_INET;
      addr.sin_port = num_port;
      addr.sin_addr.s_addr = INADDR_ANY;
      if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) == -1){
	printf("erreur bind \n");
	exit(1);
      }    
    }
    else{ // Cas TCP
      s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      addr_client.sin_family = AF_INET;
      addr_client.sin_port = num_port;
      addr_client.sin_addr.s_addr = INADDR_ANY;
      lg_addr_client = sizeof(addr_client);
      if (bind(s, (struct sockaddr *)&addr_client, lg_addr_client) == -1){
	printf("erreur bind \n");
	exit(1);
      }    
      listen(s,5);
    }
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////
  // Procédure d'envoi et de réception des messages

  if (source == 1) {
    if(protocole == 1){ // Cas UDP
      printf("SOURCE : lg_message_emis=%d, port=%d, nb_envois=%d, TP=udp, dest=%s\n", longueur_message, num_port, nb_message, nom_dest);
      for(i=0; i<nb_message; i++){    
	construire_message(message_envoi, i, longueur_message);
	sendto(s, message_envoi, longueur_message*sizeof(char), 0, (struct sockaddr *)&addr_distant, sizeof(addr_distant));
	printf("SOURCE : Envoi n° : %d ", (i+1));
	afficher_message(message_envoi, longueur_message);
      }
    }
    else{ // Cas TCP
      printf("SOURCE : lg_message_emis=%d, port=%d, nb_envois=%d, TP=tcp, dest=%s\n", longueur_message, num_port, nb_message, nom_dest);  
      for(i=0; i<nb_message; i++){     
	construire_message(message_envoi, i, longueur_message);
	send(s, message_envoi, longueur_message*sizeof(char), 0);
	printf("SOURCE : Envoi n° : %d ", (i+1));
	afficher_message(message_envoi, longueur_message);       
      }
    }
  }

  else{ // Puits
    if(protocole == 1){ // Cas UDP
      printf("PUITS : lg_message_recu=%d, port=%d, nb_receptions=infini, TP=udp\n", longueur_message, num_port);
      while(1){ 
	recvfrom(s, message_recu, longueur_message*sizeof(char), 0, (struct sockaddr *)&addr_emetteur, &lg_addr_emetteur);
	compteur++;
	printf("PUITS : Reception n° : %d ", compteur); 
	afficher_message(message_recu, longueur_message);
      }
    }
    else{ // Cas TCP
      printf("PUITS : lg_message_recu=%d, port=%d, TP=tcp\n", longueur_message, num_port);
      while(1){
	if ((sbis = accept(s, (struct sockaddr *)&addr_client, &lg_addr_client)) == -1){
	  printf("echec du  accept \n");
	  exit(1);
	}   
	switch (fork()){
	case -1 :
	  printf("erreur fork \n");
	  exit(1);
	case 0 :
	  close(s); // Fermeture du socket du processus père	  
	  while(recv(sbis, message_recu, longueur_message*sizeof(char), 0) > 0){
	    compteur ++;
	    printf("PUITS : Reception n° : %d ", compteur);
	    afficher_message(message_recu, longueur_message);    
	  }	 
	  exit(0);	  
	default : // On est dans le processus père
	  close(sbis); // Fermeture du socket du processus fils
	}
      }
    }
  }

  close(s);

  return 0;
}
 
