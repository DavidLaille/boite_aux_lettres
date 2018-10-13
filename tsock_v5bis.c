#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "./boite_lettres.c"


/* Recap : Problème au niveau du serveur boite aux lettres car les messages ne sont pas enregistrés dans la liste chainée de destinataire. Quand le processus fils est fermé, les données recues au niveau du serveur sont perdues. De ce fait, le processus en réception ne recoit jamais de messages.
Solution : il aurait fallu mettre un select pour gérer les différentes connexions mais je n'ai pas eu le temps. 

De plus, les messages doivent etre de taille fixe. */

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
  int mode_BAL = 0 ; /* 0=pas de capacité boite aux lettres, 1=mode boite aux lettres*/
  int mode_envoi = 0;
  int mode_reception = 0;

  int s; int sbis;
  int num_port ;
  char * nom_dest;
  struct hostent *hp;
  struct sockaddr_in addr_distant, addr, addr_emetteur;
  socklen_t lg_addr_emetteur;

  struct sockaddr_in addr_serveur, addr_client;
  socklen_t lg_addr_client;

  int longueur_message = -1;

  int num_dest = -1; /* si num_dest pas modifié, pas option -e ni -r*/

  while ((c = getopt(argc, argv, "psbun:l:e:r:")) != -1) {
    switch (c) {
    case 'p':
      if (source == 1) {
	printf("usage: cmd [-p|-s|-b|-e ##|-r ##][-n ##][-l ##]\n");
	exit(1) ;}
      source = 0;
      num_port = atoi(argv[argc-1]);
      num_port = htons(num_port); // sous Linux (inutile sous windows)
      break;
    case 's':
      if (source == 0) {
	printf("usage: cmd [-p|-s|-b|-e ##|-r ##][-n ##][-l ##]\n");
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
    case 'b':
      source = 0;
      num_port = atoi(argv[argc-1]);
      num_port = htons(num_port); // sous Linux
      mode_BAL = 1;
      break;
    case 'e':
      source = 1;
      nom_dest = argv[argc-2];
      num_dest = atoi(optarg);
      num_port = atoi(argv[argc-1]);
      num_port = htons(num_port); // sous Linux
      mode_envoi = 1;
      break;
    case 'r':
      source = 1;
      nom_dest = argv[argc-2];
      num_dest = atoi(optarg);
      num_port = atoi(argv[argc-1]);
      num_port = htons(num_port); // sous Linux
      mode_reception = 1;
      break;
    default:
      printf("usage: cmd [-p|-s|-b|-e ##|-r ##][-n ##][-l ##]\n");
      break;
    }
  }
  if (source == -1) {
    printf("usage: cmd [-p|-s|-b|-e ##|-r ##][-n ##][-l ##]\n");
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
  // Mode Serveur Boite aux Lettres

 if(mode_BAL == 1){
   liste* liste_dest = Creer_liste_dest();
   lettre* lettre;
   dest* destinataire;
   dest* aux;

   for(i=0; i<10; i++){ // On crée une liste de 10 destinataires
     Ajouter_dest_fin(liste_dest);
   }

   s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   addr_client.sin_family = AF_INET;
   addr_client.sin_port = num_port;
   addr_client.sin_addr.s_addr = INADDR_ANY;
   lg_addr_client = sizeof(addr_client);
   if (bind(s, (struct sockaddr *)&addr_client, lg_addr_client) == -1){
     printf("erreur bind \n");
     exit(1);
   }    
   listen(s,10);

   printf("PUITS : port=%d, TP=tcp\n", ntohs(num_port));
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
       recv(sbis, &mode_envoi, sizeof(int), 0);
       if(mode_envoi == 1){
	 recv(sbis, &num_dest, sizeof(int), 0);
	 destinataire = Trouver_dest(liste_dest, num_dest);
	 while(recv(sbis, message_recu, longueur_message*sizeof(char), 0) > 0){
	   compteur ++;
	   printf("PUITS : Reception lettre destinataire n° %d : ", num_dest);
	   afficher_message(message_recu, longueur_message);
	   lettre = Creer_lettre(message_recu);
	   Ajouter_lettre_fin(destinataire, lettre);
	 }
       }
       else{
	 recv(sbis, &num_dest, sizeof(int), 0);
	 destinataire = Trouver_dest(liste_dest, num_dest);
	 aux = destinataire;
	 printf("PUITS : Lecture BAL destinataire %d\n", num_dest);
	 if(destinataire->lettre == NULL){
	   printf("PUITS : Pas de message pour le destinataire en question\n");
	 }
	 else{
	   while(aux->lettre->suiv != NULL){
	     printf("%s", aux->lettre->message);
	     send(sbis, aux->lettre->message, longueur_message*sizeof(char), 0);
	     aux->lettre = aux->lettre->suiv;
	   }
	 }
       }
       exit(0);	  
     default : // On est dans le processus père
       close(sbis); // Fermeture du socket du processus fils
     }

   }
 }

  //////////////////////////////////////////////////////////////////////////////////////////////
  // Cas Source/Puits, et Client envoi/Client lecture
 //////////////////////////////////////////////////////////////////////////////////////////////

 // Création du socket de communication, adressage, connection(si TCP) ou mise en attente de réception (si UDP)

  // Cas Source
 else{
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
      printf("SOURCE : lg_message_emis=%d, port=%d, nb_envois=%d, TP=udp, dest=%s\n", longueur_message, ntohs(num_port), nb_message, nom_dest);
      for(i=0; i<nb_message; i++){    
	construire_message(message_envoi, i, longueur_message);
	sendto(s, message_envoi, longueur_message*sizeof(char), 0, (struct sockaddr *)&addr_distant, sizeof(addr_distant));
	printf("SOURCE : Envoi n° : %d ", (i+1));
	afficher_message(message_envoi, longueur_message);
      }
    }
    else{ // Cas TCP
      if(mode_envoi == 1){
	printf("SOURCE : lg_message_emis=%d, port=%d, nb_envois=%d, TP=tcp, dest=%s\n", longueur_message, ntohs(num_port), nb_message, nom_dest);
	send(s, &mode_envoi, sizeof(int), 0);
	send(s, &num_dest, sizeof(int), 0);
	for(i=0; i<nb_message; i++){     
	  construire_message(message_envoi, i, longueur_message);
	  send(s, message_envoi, longueur_message*sizeof(char), 0);
	  printf("SOURCE : Envoi n° : %d ", (i+1));
	  printf("Destinataire n° : %d ", num_dest);
	  printf(" (%d) ", longueur_message);
	  afficher_message(message_envoi, longueur_message);
	}	
	printf("SOURCE : fin\n");
      }
      else if(mode_reception == 1){
	printf("RECEPTION : demande message destinataire %d , port=%d, TP=tcp, dest=%s\n", num_dest, ntohs(num_port), nom_dest);
	send(s, &mode_envoi, sizeof(int), 0);
	send(s, &num_dest, sizeof(int), 0);
	while(recv(s, message_recu, longueur_message*sizeof(char), 0) > 0){
	  printf("RECEPTION : destinataire %d (%d)", num_dest, longueur_message);
	  afficher_message(message_envoi, longueur_message);
	}
	printf("RECEPTION : fin\n");
      }
      else{ // Cas sans option BAL
	printf("SOURCE : lg_message_emis=%d, port=%d, nb_envois=%d, TP=tcp, dest=%s\n", longueur_message, ntohs(num_port), nb_message, nom_dest);
	for(i=0; i<nb_message; i++){     
	  construire_message(message_envoi, i, longueur_message);
	  send(s, message_envoi, longueur_message*sizeof(char), 0);
	  printf("SOURCE : Envoi n° : %d ", (i+1));
	  afficher_message(message_envoi, longueur_message);
	}
      }
    }
  }

  else{ // Puits
    if(protocole == 1){ // Cas UDP
      printf("PUITS : lg_message_recu=%d, port=%d, nb_receptions=infini, TP=udp\n", longueur_message, ntohs(num_port));
      while(1){ 
	recvfrom(s, message_recu, longueur_message*sizeof(char), 0, (struct sockaddr *)&addr_emetteur, &lg_addr_emetteur);
	compteur++;
	printf("PUITS : Reception n° : %d ", compteur); 
	afficher_message(message_recu, longueur_message);
      }
    }
    else{ // Cas TCP 
      printf("PUITS : lg_message_recu=%d, port=%d, TP=tcp\n", longueur_message, ntohs(num_port));
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
 }
 
  close(s);

  return 0;
}
 
