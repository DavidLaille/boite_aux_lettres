#define LISTE

struct lettre{
  char* message;
  int num_message;
  struct lettre* suiv;
};

typedef struct lettre lettre;

struct destinataire{
  struct lettre* lettre;
  int num_dest;
  struct destinataire* suiv;
};

typedef struct destinataire dest;

struct liste_destinataire{
  dest* premier;
  dest* courant;
  dest* dernier;
};

typedef struct liste_destinataire liste;

lettre* Creer_lettre(char* message);
void Ajouter_lettre_fin(dest* dest, lettre* lettre);
void Afficher_lettre(lettre* lettre);
void Afficher_liste_lettre(dest* dest);
liste* Creer_liste_dest();
void Ajouter_dest_fin(liste* liste);
void Aller_debut(liste* liste);
void Avancer(liste* liste);
void Aller_fin(liste* liste);
lettre* Retourner_courant(liste* liste);
dest* Trouver_dest(liste* liste, int numero);

