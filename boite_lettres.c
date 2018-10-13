#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LISTE
#include "boite_lettres.h"
#endif

#define TAILLE_MESSAGE 30

lettre* Creer_lettre(char* message){
  lettre* lettre;

  lettre = malloc(sizeof(lettre));
  lettre->message = malloc(TAILLE_MESSAGE*sizeof(char));
  strcpy(lettre->message, message);
  lettre->suiv = NULL;
  return lettre;
}

///////////////////////////////////////////////////////////////////////////

void Ajouter_lettre_fin(dest* dest, lettre* lettre_ajout){
  int compteur=2;
  lettre* aux = dest->lettre;

  if (dest->lettre == NULL){ 
    dest->lettre = lettre_ajout;
    dest->lettre->num_message = 1;
  }
  else{
    while(aux->suiv != NULL){
      aux = aux->suiv;
      compteur ++;
    }
    aux->suiv = lettre_ajout;
    aux->suiv->num_message = compteur;
  }
}

///////////////////////////////////////////////////////////////////////////

void Afficher_lettre(lettre* lettre){
  printf("Message n° %d :", lettre->num_message);
  printf("%s\n", lettre->message);
}

///////////////////////////////////////////////////////////////////////////

void Afficher_liste_lettre(dest* dest){
  lettre* aux = dest->lettre;

  if(dest->lettre == NULL){
    printf("Il n'y a pas de message.\n");
  }
  else{
    while(aux != NULL){
      Afficher_lettre(aux);
      aux = aux->suiv;
    }
  }
}

//////////////////////////////////////////////////////////////////////////

liste* Creer_liste_dest(){
  liste* liste_dest = malloc(sizeof(liste));

  liste_dest->premier = NULL;
  liste_dest->courant = NULL;
  liste_dest->dernier = NULL;

  return liste_dest;
}

/////////////////////////////////////////////////////////////////////////

void Ajouter_dest_fin(liste* liste){
  dest* nouveau_dest = malloc(sizeof(dest));
  nouveau_dest->suiv = NULL;
 

  if (liste->premier == NULL){ 
    liste->premier = nouveau_dest;
    liste->courant = nouveau_dest;
    liste->dernier = nouveau_dest;
    nouveau_dest->num_dest = 1;
  }
  else{
    nouveau_dest->num_dest = liste->dernier->num_dest +1;
    liste->dernier->suiv = nouveau_dest;
    liste->dernier = nouveau_dest;
  }
}

/////////////////////////////////////////////////////////////////////////

void Aller_debut(liste* liste){
  liste->courant = liste->premier;
}

/////////////////////////////////////////////////////////////////////////

void Avancer(liste* liste){
  liste->courant = liste->courant->suiv;
}

/////////////////////////////////////////////////////////////////////////

void Aller_fin(liste* liste){
  liste->courant = liste->dernier;
}

/////////////////////////////////////////////////////////////////////////

lettre* Retourner_courant(liste* liste){
  return (liste->courant->lettre);
}

/////////////////////////////////////////////////////////////////////////

void Afficher_liste(liste* liste){
  Aller_debut(liste);
  if(liste->premier == NULL){
    printf("La liste est vide.\n");
  }
  else{
    while(liste->courant != NULL){
      printf("Destinataire n° %d :\n", liste->courant->num_dest);
      Afficher_liste_lettre(liste->courant);
      Avancer(liste);
    }
  }
}

/////////////////////////////////////////////////////////////////////////

dest* Trouver_dest(liste* liste, int numero){
  int trouve = 0; 
  Aller_debut(liste);
  while(trouve == 0 && liste->courant != liste->dernier){
    if(numero == liste->courant->num_dest){
      trouve = 1;
    }
    else{
      Avancer(liste);
     }
  }
  if(trouve == 1){
    return liste->courant;
  }
  else{
    printf("Le destinataire n° %d n'a pas ete trouve.\n", numero);
    exit(1);
  }
}

/////////////////////////////////////////////////////////////////////////
/*
int main(){
  lettre* lettre1 = Creer_lettre("Salut");
  lettre* lettre2 = Creer_lettre("Coucou");
  lettre* lettre3 = Creer_lettre("Coucouu");
  lettre* lettre4 = Creer_lettre("Coucouuu");
  lettre* lettre5 = Creer_lettre("Coucouuuu");
  lettre* lettre6 = Creer_lettre("Coucouuuuu");
  liste* liste1 = Creer_liste_dest();
  Ajouter_dest_fin(liste1);
  Ajouter_lettre_fin(liste1->premier, lettre1);
  Ajouter_lettre_fin(liste1->premier, lettre2);
  Ajouter_lettre_fin(liste1->premier, lettre3);
  Ajouter_lettre_fin(liste1->premier, lettre4);
  Ajouter_lettre_fin(liste1->premier, lettre5);
  Ajouter_lettre_fin(liste1->premier, lettre6);
  Ajouter_dest_fin(liste1);
  Ajouter_dest_fin(liste1);
  Ajouter_dest_fin(liste1);
  Ajouter_dest_fin(liste1);
  Afficher_liste(liste1);
  return 0;

}
*/
