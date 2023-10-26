/*------------------------------------------------------------------
Noms :   Alex Seguin
         Zachary G.
Date d'échéance : 10 décembre 2017

Fichier : Facteurs_de_friction_entre_conduit_et_fluide.c

Description :

    L’objectif principal est de démontrer, à l’aide d’un graphique, le changement du facteur
de friction selon le changement du diamètre d’un conduit qui transporte un fluide. Pour cette situation,
le débit de flux du fluide (m3/s) demeure constant. Ce flux est en fait le volume de fluide qui passe au
travers d’une surface quelconque, ici l’aire intérieure du conduit, par rapport au temps. Le facteur de
friction, lui, provoque une résistance qui s’oppose au mouvement du fluide dans le tuyau. Ainsi, notre
logiciel va évaluer les facteurs de frictions pour une étendue donnée de diamètres et va afficher les
résultats sous la forme d’une graphique. Le logiciel peut aussi sauvegarder les résultats pour un maximum
de cinq ensembles de données.
---------------------------------------------------------------------*/
#include <stdio.h>
#include <gng1106plplot.h>  // Donne des définitions pour utiliser la librarie PLplot
#include <math.h>

// Quelques définitions
#define VRAI 1
#define FAUX 0
#define NBR_SAUVER 5 //nombre de sauvegardes permises dans le fichier
#define TOLERANCE 1E-13 //tolerance pour la bissetion
#define PRESQUE_0 1E-10 //borne minimale de la bissection
#define HAUT 100000 //borne maximale pour la bissection
#define TAILLE_MAX 100 //taille maximale des tableaux de données
#define FICHIER_BIN "donneesFluide.bin"
#define VALEUR_MIN_REY 4000 //valeur minimale du nombre de Reynolds
#define IMPOSSIBLE -1 //valeur pour drapeau lorsque le facteur de friction impossible


typedef struct
{
    double rey; //nombre de Reynolds
    double dmax, dmin; //diamètre maximal et minimal
    double dtbl[TAILLE_MAX]; //tableau pour l’étendu du diamètre
    double ftbl[TAILLE_MAX]; //tableau des facteurs de friction
    double eps; //rugosité
    double ro; //densité
    double mu; //viscosité
    double debit; //débit du flux
    double vit; //vitesse
    int estVide; //drapeau qui indique si le fichier binaire est vide

} DONNEES;


// Prototypes des fonctions
void obtientDonnees(DONNEES *, DONNEES [], FILE *);
void choixDonnees(DONNEES [], DONNEES *, FILE *);
void entrerDonnees(DONNEES *, DONNEES [], FILE *);
void afficheDonnees(DONNEES []);
void sauverDonnees(DONNEES *, DONNEES [], FILE *);
int demandeReboot();
int verifieDiam(DONNEES *);
int getValeurPositive(double);
int calculerRey(DONNEES *);
int remplirTableaux(DONNEES *, DONNEES [], FILE *);
double calculFriction(DONNEES *, double, DONNEES [], FILE *);
double calculColebrook(double, double,  DONNEES *);
double getMin(double []);
double getMax(double []);
void plot(DONNEES *);
void lireFichier(DONNEES [], FILE *);
void ecrireFichier(DONNEES [], FILE *);
double invitation(char []);
void demandeSauver(DONNEES *, DONNEES [], FILE *);


/*---------------------------------------------------------------------
 Fonction : main

 Description : La fonction main est utilisée pour déclarer un tableau
 et une structure par défaut. Puis, fait appel à plusieurs fonctions
 pour accomplir la tâche du logiciel.
------------------------------------------------------------------------*/
void main()
{
    DONNEES donnees; //variable structure que la programme manipule principalement.
    DONNEES tblSave[NBR_SAUVER]; //tableau pour sauvegarder les donnees
    int recommence; //drapeau afin de recommecer le programme
    int ix;
    FILE *fichierPtr;

    printf("Bienvenue au projet!\n\n");
    do
    {
        for(ix = 0; ix < NBR_SAUVER; ix = ix +1) //affecte estVide à vrai pour chaque membre
            tblSave[ix].estVide = VRAI;

        lireFichier(tblSave, fichierPtr); //procure les données stockées dans le fichier
        obtientDonnees(&donnees, tblSave, fichierPtr); //obtient les données de l'utilisateur
        plot(&donnees); //affiche le graphique du facteur de friction

        recommence = demandeReboot(); //demande de recommencer le programme
    }
    while(recommence == VRAI);

    printf("\n\nProgramme termin\202! \n\n");
}

/*-----------------------------------------------------------------------
Fonction : lireFichier
Paramètres :
    tblPtr - référence au tableau de sauvegarde (type DONNEES).
    fPtr - pointeur référant à la structure FILE.
Valeur de retour :  void

Description : Cette fonction lit les données d'un fichier binaire existant et
l'affecte dans le tableau de sauvegarde.
------------------------------------------------------------------------*/
void lireFichier(DONNEES tblPtr[], FILE *fPtr)
{
    fPtr = fopen(FICHIER_BIN, "rb"); //ouvre le fichier
    if(fPtr == NULL) //si le fichier n'existe pas
    {
        fclose(fPtr);
        ecrireFichier(tblPtr, fPtr); //crée nouveau fichier
    }
    else //lit fichier existant
    {
        fread(tblPtr, sizeof(DONNEES), NBR_SAUVER, fPtr);
        fclose(fPtr);
    }
}

/*-----------------------------------------------------------------------
Fonction : ecrireFichier
Paramètres :
    tblPtr - référence au tableau de sauvegarde (type DONNEES).
    fPtr - pointeur référant à la structure FILE.
Valeur de retour :  void

Description : Écrit les valeurs du tableau de sauvegarde dans un fichier binaire.
------------------------------------------------------------------------*/
void ecrireFichier(DONNEES tblPtr[], FILE *fPtr)
{
    fPtr = fopen(FICHIER_BIN, "wb");
    fwrite(tblPtr, sizeof(DONNEES), NBR_SAUVER, fPtr); //écrit les donnees dans le fichier
    fclose(fPtr);
}

/*-----------------------------------------------------------------------
Fonction : demandeReboot
Paramètres :
    (aucun)
Valeur de retour :  reponse - VRAI ou FAUX

Description : Cette fonction demande si l'utilisateur veut recommencer le programme.
------------------------------------------------------------------------*/
int demandeReboot()
{
    char recommence; //drapeau

    do
    {
        printf("\nVoulez vous recommencer le programme? (o/n) : ");
        fflush(stdin);
        recommence = getchar();

        if(recommence != 'o' && recommence != 'n')
            printf("Caract\212re non valide! S.V.P. entrez 'o' ou 'n'.");
    }
    while(recommence != 'o' && recommence != 'n');

    if(recommence == 'o')
        return(VRAI);
    else
        return(FAUX);
}

/*-----------------------------------------------------------------------
Fonction : obtientDonnees
Paramètres :
    dPtr - pointeur référant à la structure DONNEES donnees.
    tblPtr - référence au tableau de sauvegarde (type DONNEES).
    fPtr - pointeur référant à la structure FILE.
Valeur de retour :  void

Description : Offre l'option de fournir ses propres données ou de choisir des données existantes.
------------------------------------------------------------------------*/
void obtientDonnees(DONNEES *dPtr, DONNEES tblPtr[], FILE *fPtr)
{
    int choix; //choix 1 ou 2 de l'utilisateur

    printf("\nVoulez vous : \n1) fournir vos propres donn\202es, ou\n");
    printf("2) utiliser des donn\202es d\202ja existantes?\n");

    do
    {
        printf("Entrez 1 ou 2 : ");
        fflush(stdin);
        scanf("%d", &choix);
    }
    while(choix != 1 && choix != 2);

    if(choix == 1)
        entrerDonnees(dPtr, tblPtr, fPtr);
    else if(choix == 2)
        choixDonnees(tblPtr, dPtr, fPtr);
}

/*-----------------------------------------------------------------------
Fonction : choixDonnees
Paramètres :
    dPtr - pointeur référant à la structure DONNEES donnees.
    tblPtr - référence au tableau de sauvegarde (type DONNEES).
    fPtr - pointeur référant à la structure FILE.
Valeur de retour :  void

Description : Affiche le contenu du fichier (tableau de sauvegarde) et demande à
l'utilisateur quelles données à choisir.
------------------------------------------------------------------------*/
void choixDonnees(DONNEES tblPtr[], DONNEES *dPtr, FILE *fPtr)
{
    int choix; //choix de l'utilisateur
    if(tblPtr[0].estVide == VRAI)
    {
        printf("\nTout est vide. Vous devez entrer vos propres donn\202es.");
        entrerDonnees(dPtr, tblPtr, fPtr);
    }
    else
    {
        afficheDonnees(tblPtr);
        printf("\nVeuillez s\202lectionner un ensemble de donn\202es (1 \205 5) : ");

        do //s'assure que le choix n'est pas vide
        {
            do //s'assure que l'entrée est valide
            {
                fflush(stdin);
                scanf("%d", &choix);

                if(choix < 1 || choix > 5)
                    printf("Entrez un chiffre de 1 \205 5 : ");
            }
            while( choix < 1 || choix > 5);  //choix invalide

            if(tblPtr[choix - 1].estVide == VRAI) //si le choix est vide
                printf("La s\202lection ne contient pas de donn\202es!\n");
        }
        while(tblPtr[choix - 1].estVide == VRAI);  //choix est vide

        *dPtr = tblPtr[choix - 1];
        printf("\nLes donn\202es ont \202t\202 import\202es.\n");
    }
}

/*-----------------------------------------------------------------------
Fonction : afficheDonnees
Paramètres :
    tblPtr - référence au tableau de sauvegarde (type DONNEES).
Valeur de retour :  void

Description : Affiche les données dans le tableau de sauvegarde.
------------------------------------------------------------------------*/
void afficheDonnees(DONNEES tblPtr[])
{
    int ix; //index de la structure à afficher

    for(ix = 0; ix < NBR_SAUVER; ix = ix +1)
    {
        printf("\n %d) \n\n", ix+1);

        if(tblPtr[ix].estVide == VRAI) //si membre est vide
            printf("%20cVIDE\n", ' ');
        else if(tblPtr[ix].estVide == FAUX) //si membre contient des données
        {
            printf("%5cDiam\212tre minimal (m)  : %.3g \n", ' ', tblPtr[ix].dmin);
            printf("%5cDiam\212tre maximal (m)  : %g \n", ' ', tblPtr[ix].dmax);
            printf("%5cRugosit\202 (m)          : %g \n", ' ', tblPtr[ix].eps);
            printf("%5cDensit\202 (kg/m^3)      : %g \n", ' ', tblPtr[ix].ro);
            printf("%5cViscosit\202 (N*s/m^2)   : %g \n", ' ', tblPtr[ix].mu);
            printf("%5cD\202bit de flux (m^3/s) : %g \n\n", ' ', tblPtr[ix].debit);
            printf("%5cNombre de Reynolds calcul\202       : %.3f \n", ' ', tblPtr[ix].rey);
            printf("%5cVitesse du fluide calcul\202e (m/s) : %.3f \n", ' ', tblPtr[ix].vit);
        }
        printf("-----------------------------------------------\n");
    }
}

/*------------
Fonction :  entrerDonnees

Paramètres :    dPtr - pointeur référant à la structure DONNEES
                tblPtr - pointeur référant le tableau de type DONNEES
                fPtr - pointeur référant à la structure FILE

Valeur de retour :  void

Description :   Cette fonction demande les entrées de l’utilisateur à l'aide de invitation().
Pour les entrées de dmax et dmin, vérifie si les valeurs sont acceptables en faisant appel à
la fonction verifieDiam(). Pour la viscosité, la densité, la rugosité et le débit, la
fonction doit vérifier si ces valeurs sont plus grandes que 0 en faisant appel à la fonction
getValeurPositive(). Pour le nombre de Reynolds, on calcule sa valeur en appelant la fonction
calculerRey(). Ensuite, fait appel à demandeSauver() pour offrir de sauvgarder les données.
------------*/
void entrerDonnees(DONNEES *dPtr, DONNEES *tblPtr, FILE *fPtr)
{
    do
    {
        do
        {
            //demande des valeurs des diamètres et vérification de ces valeurs
            dPtr->dmax = invitation("le diam\212tre maximal en m");
            dPtr->dmin = invitation("le diam\212tre minimal en m");
        }
        while(!verifieDiam(dPtr));

        //demande pour la valeur de la rugosité et vérification de cette valeur
        dPtr->eps = invitation("la rugosit\202 du conduit en m");
        //demande pour la valeur de la densité et vérification de cette valeur
        dPtr->ro = invitation("la densit\202 du fluide en kg/m^3");
        //demande pour la valeur de la viscosité et vérification de cette valeur
        dPtr->mu = invitation("la viscosit\202 du fluide en N*s/m^2");
        //demande pour la valeur du débit et vérification de cette valeur
        dPtr->debit = invitation("le d\202bit de flux du fluide en m^3/s");
    }
    while(!calculerRey(dPtr)); //vérifie que # de Reynolds > 4000
    printf("Test r\202ussi : le nombre de Reynolds obtenu est %f.", dPtr->rey);

    dPtr->estVide = FAUX; //la variable struct n'est plus vide

    //vérifie que des facteurs de friction existent pour toute l'étendue et remplit tableaux si les valeurs sont valides
    if(remplirTableaux(dPtr, tblPtr, fPtr) != IMPOSSIBLE)
        demandeSauver(dPtr, tblPtr, fPtr);
}

/*-----------------------------------------------------------------------
Fonction : invitation
Paramètres :
    texte - chaîne de caractère qui corresond au nom de la variable que l'on demande de fournir.
Valeur de retour :  valeur - valeur entrée par l'utilisateur

Description : Cette fonction demande à l'utilisisateur d'entrer une valeur. Retourne cette valeur à entrerDonnees()
pour qu'elle soit affecté à la variable appropriée.
------------------------------------------------------------------------*/
double invitation(char texte[])
{
    double valeur;

    do
    {
        printf("\nVeuillez entrer la valeur pour %s : ", texte);
        fflush(stdin);
        scanf("%lf", &valeur);
    }
    while(!getValeurPositive(valeur)); //répète si valeur négative

    return(valeur);
}

/*-----------------------------------------------------------------------
Fonction : demandeSauver
Paramètres :
    dPtr - pointeur référant à la structure DONNEES
    tblPtr - pointeur référant le tableau de type DONNEES
    fPtr - pointeur référant à la structure FILE
Valeur de retour :  void

Description : Cette fonction demande à l'utilisateur s'il veut sauvegarder ses données. Si oui,
fait appel à sauverDonnees; si non, répète la demande.
------------------------------------------------------------------------*/
void demandeSauver(DONNEES *dPtr, DONNEES *tblPtr, FILE *fPtr)
{
    char reponse; //réponse de l'utilisateur
    printf("\n\nVoulez-vous sauver vos donn\202es?");
    printf("\nR\202pondre 'o' pour oui et 'n' pour non : ");
    fflush(stdin);
    scanf("%c", &reponse);
    while(reponse != 'o' && reponse != 'n')//s'assure que l'utilisateur entre un caractère valide
    {
        printf(" - ERREUR! Le caract\212re n'est pas valide. S.V.P r\202pondre 'o' ou 'n' : ");
        fflush(stdin);
        scanf("%c", &reponse);
    }
    if(reponse == 'o')
        sauverDonnees(dPtr, tblPtr, fPtr); //sauve les données
}

/*------------
Fonction :  sauverDonnees

Paramètres :    dPtr - pointeur référant à la structure DONNEES
                tblPtr - pointeur référant le tableau de type DONNEES
                fPtr - pointeur référant à la structure FILE
Valeur de retour :  void

Description :   Cette fonction sauvegarde les données de l'utilisateur dans le premier espace vide
à l'intérieur du fichier binaire. Si il n'y a pas d'espace vide dans le fichier, la
fonction fait appel à afficheDonnees. Ensuite, la fonction demande à l'utilisateur
quel membre qu'il veut remplacer et sauve ses données à ce membre.
------------*/
void sauverDonnees(DONNEES *dPtr, DONNEES tblPtr[], FILE *fPtr)
{
    int ix; // Index pour la boucle
    int ixVide = -1; // Pour l'index du tableau du 1er membre vide retrouvé par la boucle
    int choix; // Choix pour le numéro du membre à remplacer

    for(ix = 0; ix < NBR_SAUVER && ixVide == -1; ix = ix +1)
    {
        if(tblPtr[ix].estVide == VRAI) //si c'est vide, sauve les donnees dans le premier membre vide
        {
            tblPtr[ix] = *dPtr;
            ixVide = ix;
        }
    }
    if(ixVide == -1) //aucun membre vide trouvé
    {
        afficheDonnees(tblPtr);
        printf("Le fichier est plein. \nS.V.P. choisir le membre que vous voulez remplacer par vos donn\202es (1-5) : ");
        fflush(stdin);
        scanf("%d", &choix);
        ix = choix;
        tblPtr[ix-1] = *dPtr;
    }
    ecrireFichier(tblPtr, fPtr); //sauve dans le fichier
    printf("\nVos donn\202es ont \202t\202 sauv\202es dans le membre %d du fichier.", ix);

}

/*------------
Fonction :  verifieDiam

Paramètres :    dPtr - pointeur référant à la structure DONNEES
Valeur de retour :  bonDiam - VRAI si diamètres sont valides, FAUX autrement

Description :   Cette fonction vérifie les valeurs de dmax et de dmin
sont valides (dmax > dmin). Affiche message si les données sont invalides.
------------*/
int verifieDiam(DONNEES *dPtr)
{
    int bonDiam = VRAI; //drapeau
    if(dPtr->dmax <= dPtr->dmin) //diamètres invalides
    {
        printf(" - ERREUR! Les valeurs du diam\212tre maximal et du diam\212tre minimal sont invalides.");
        printf("\n - S.V.P. entrez des valeurs telles que le diam\212tre maximal est plus grand que le diam\212tre minimal.\n");
        bonDiam = FAUX;
    }
    return(bonDiam);
}

/*------------
Fonction :  getValeurPositive

Paramètres :    valeur - valeur envoyée par entrerDonnees
Valeur de retour :  int (VRAI ou FAUX)

Description :   Cette fonction vérifie si les valeurs sont valides ( > 0 ). La
fonction affiche un message si les valeurs sont invalides et retourne FAUX,
sinon retourne VRAI.
------------*/
int getValeurPositive(double valeur)
{
    if (valeur < 0)
    {
        printf(" - ERREUR! La valeur fournie n'est pas valide. Elle doit \210tre plus grande que z\202ro.");

        return(FAUX);
    }
    return(VRAI);
}

/*------------
Fonction :  calculerRey

Paramètres :    dPtr - pointeur référant à la structure DONNEESà
Valeur de retour :  int (VRAI ou FAUX)

Description :   Cette fonction calcule le nombre de Reynolds et vérifie si ce nombre est valide
( > VALEUR_MIN_REY ). Si le nombre de Reynolds est plus petit que 4000, la fonction
affiche un message que la valeur n'est pas valide et retourne FAUX. Sinon, retourne VRAI.
------------*/
int calculerRey(DONNEES *dPtr)
{
    //calcul de la vitesse
    dPtr->vit = pow((0.5*dPtr->dmax), 2); /*Diamètre maximal donnera la vitesse minimale,
                                            qui donnera le nombre de Reynolds minimal qu'atteint
                                            la fonction*/
    dPtr->vit = dPtr->vit * M_PI;
    dPtr->vit = dPtr->debit / dPtr->vit;

    //calcul du nombre de Reynolds
    dPtr->rey = dPtr->ro * dPtr->vit * dPtr->dmax;
    dPtr->rey = dPtr->rey / dPtr->mu;

    if(dPtr->rey < VALEUR_MIN_REY) //Reynold < 4000
    {
        printf("\nLa valeur obtenue pour le nombre de Reynolds, soit %.4f, n'est pas valide car elle est plus petite que 4000.", dPtr->rey);
        printf("\nVous devez entrer vos valeurs de nouveau afin que le nombre de Reynolds soit plus grand que 4000.");
        return(FAUX);
    }
    return(VRAI);
}

/*-----------------------------------------------------------------------
Fonction : remplirTableaux
Paramètres :
    dPtr - pointeur référant à la structure de données.
    tblPtr - pointeur référant le tableau de type DONNEES
    fPtr - pointeur référant à la structure FILE
Valeur de retour :  IMPOSSIBLE si valeurs impossibles, ou 0 si le tout est acceptable

Description : Cette fonction remplit les tableaux du diamètre et
du facteur de friction. Les valeurs sont calculées à l'aide de la
fonction calculFriction(). Ces tableaux seront utilisées pour le graphique.
------------------------------------------------------------------------*/
int remplirTableaux(DONNEES *dPtr, DONNEES tblPtr[], FILE *fPtr)
{
    double dtemp; //stocker diamètres intermédiaires
    int ix;
    double inc;//valeur pour incrémenter le diamètre
    int impossible = FAUX; //valeurs irréelles (drapeau)

    dtemp = dPtr->dmin;

    inc = (dPtr->dmax - dPtr->dmin)/(TAILLE_MAX-1);

    //calcul les valeurs de friction et remplit les tableaux
    for(ix = 0; ix < TAILLE_MAX && impossible == FAUX; ix = ix +1)
    {
        dPtr->dtbl[ix] = dtemp;
        dPtr->ftbl[ix] = calculFriction(dPtr, dtemp, tblPtr, fPtr);

        if(dPtr->ftbl[ix] < 0) //vérifie si facteur de friction existe
            impossible = VRAI;

        dtemp = dtemp + inc;
    }
    if(impossible == VRAI)//si facteurs de frictions impossibles, redemande des données
    {
        obtientDonnees(dPtr, tblPtr, fPtr);
        return(IMPOSSIBLE);
    }
    return(0);
}

/*-----------------------------------------------------------------------
Fonction : calculFriction
Paramètres :
    dPtr - pointeur référant à la structure de données.
    d - variable du diamètre à calculer afin de déterminer le facteur de friction.
    tblPtr - pointeur référant le tableau de type DONNEES
    fPtr - pointeur référant à la structure FILE
Valeur de retour :  temp - valeur de la racine trouvée

Description : Cette fonction utilise la méthode de bissection et la fonction calculColebrook()
afin de trouver la valeur du facteur de friction pour le diamètre donné.
------------------------------------------------------------------------*/
double calculFriction(DONNEES *dPtr, double d, DONNEES tblPtr[], FILE *fPtr)
{
    double haut, bas, temp;

    haut = HAUT; //borne supérieure
    bas = PRESQUE_0; //borne inférieure
    temp = (haut+bas)/2; //borne du milieu

    if(calculColebrook(haut, d, dPtr)*calculColebrook(bas, d, dPtr) < 0) //si racine existe
    {
        //tant et autant que le produit des facteurs de friction aux bornes est plus grand que la tolérance
        //si le facteur de friction à temp = 0, c'est la racine
        while(fabs(calculColebrook(haut, d, dPtr)*calculColebrook(bas, d, dPtr)) > TOLERANCE && calculColebrook(temp, d, dPtr) != 0)
        {
            if(calculColebrook(haut, d, dPtr)*calculColebrook(temp, d, dPtr) < 0) //si racine est dans demi supérieure
                bas = temp;
            else if(calculColebrook(bas, d, dPtr)*calculColebrook(temp, d, dPtr) < 0) //si racine est dans demi inférieur
                haut = temp;

            temp = (haut+bas)/2; //affecte la nouvelle valeur du mileu
        }
    }
    else //si il n'a pas de racine
    {
        printf("\n\nLes donn\202es entr\202es sont irr\202alistes! \nDes facteurs de frictions n'existent pas pour tout l'\202tendue des diam\212tres donn\202e.");
        printf("\nVous devez utiliser des donn\202es r\202alistes.\n");
        temp = IMPOSSIBLE;// valeur de -1
    }
    return(temp);
}

/*-----------------------------------------------------------------------
Fonction : calculColebrook
Paramètres :
    f - facteur de friction dans la formule Colebrook, correspond à haut, temp, ou bas
	d - diamètre actuel du conduit pour trouver le facteur de friction
	dPtr - pointeur référant à la structure DONNEES
Valeur de retour :  g_de_f - valeur calculée par l’équation Colebrook

Description : Cette fonction calcule g en fonction de f, g(f) dans la formule,
à l'aide de l'équation Colebrook.
------------------------------------------------------------------------*/
double calculColebrook(double f, double d, DONNEES *dPtr)
{
    double g_de_f; //valeur finale

    g_de_f = dPtr->eps/(3.7*d) + 2.51/(dPtr->rey * sqrt(f));
    g_de_f = 2.0*log10(g_de_f);
    g_de_f = 1/sqrt(f) + g_de_f; //accumulation

    return(g_de_f);
}

/*-----------------------------------------------------------------------
Fonction : plot
Paramètres :
   dPtr - pointeur référant à la structure donnees
Valeur de retour :  void

Description : Cette fonction affiche le graphique du facteur de friction en fonction du diamètre.
------------------------------------------------------------------------*/
void plot(DONNEES *dPtr)
{
    double miny, maxy;

    miny = getMin(dPtr->ftbl); //min en y
    maxy = getMax(dPtr->ftbl); //max en y

    plsdev("wingcc");
    plinit(); //initialisation

    plwidth(3); //largeur de la plume

    plenv(dPtr->dmin, dPtr->dmax, miny, maxy, 0, 1); //établir échelles
    plcol0(GREEN); //couleur de la plume

    //étiquettes
    pllab("Diametre (m)", "Facteur de friction", "Facteur de friction d'un conduit selon son diametre");

    plcol0(BLUE); //couleur de la plume pour la courbe
    plline(TAILLE_MAX, dPtr->dtbl, dPtr->ftbl);

    plend(); //ferme le graphique
}

/*-----------------------------------------------------------------------
Fonction : getMin
Paramètres :
    tbl - référence au tableau de facteur de friction
Valeur de retour :  min - valeur minimale trouvée.

Description : Cette fonction trouve la valeur minimale dans le tableau et retourne cette valeur.
------------------------------------------------------------------------*/
double getMin(double tbl[])
{
    double min; //valeur minimale
    int ix; //index

    min = tbl[0]; //assume que le 1er membre est le plus petit

    for(ix = 1; ix< TAILLE_MAX; ix = ix + 1)
    {
        if(tbl[ix] < min)
            min = tbl[ix];
    }
    return(min);
}

/*-----------------------------------------------------------------------
Fonction : getMax
Paramètres :
    tbl - référence au tableau de facteur de friction
Valeur de retour :  max - valeur maximale trouvée

Description : Cette fonction trouve la valeur maximale dans le tableau et retourne cette valeur.
------------------------------------------------------------------------*/
double getMax(double tbl[])
{
    double max; //valeur maximale
    int ix; //index

    max = tbl[0]; //assume que le 1er membre est le plus grand

    for(ix = 1; ix< TAILLE_MAX; ix = ix + 1)
    {
        if(tbl[ix] > max)
            max = tbl[ix];
    }
    return(max);
}
