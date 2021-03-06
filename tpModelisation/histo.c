#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "rdjpeg.h"
#include "proc.h"
#include "cgabor.h"

// Definition ###################################
double* makeHisto(char* name, FILE *fileToWrite);
// ##############################################


/**
 * Calcul le produit scalaire entre U et V 
 * @param U
 * @param V
 * @param N taille des vecteurs U && V
 * @return 
 */
double produitScalaire(double* U, double* V, int N) {
    double ps;
    int i;
    for (ps = 0.0, i = 0; i < N; i++)
        if (U[i] - V[i] > 0)
            ps += (U[i] - V[i]);
        else
            ps -= (U[i] - V[i]);
    return ps;
}

/**
 * Fonction renvoyant la distance euclidienne entre 2 tableaux de doubles de taille 64
 * @param trainDescriptor
 * @param valDescriptor
 * @return distance euclidienne entre les deux paramètres en entrée
 */
float distanceEuclidienne(double* trainDescriptor, double* valDescriptor) {
    return produitScalaire(trainDescriptor, valDescriptor, 64);
}

/**
 * Calcul le produit scalaire entre U et V 
 * @param U
 * @param V
 * @param N taille des vecteurs U && V
 * @return 
 */
double produitScalaireFloat(float* U, float* V, int N) {
    double ps;
    int i;
    for (ps = 0.0, i = 0; i < N; i++)
        if (U[i] - V[i] > 0)
            ps += (U[i] - V[i]);
        else
            ps -= (U[i] - V[i]);
    return ps;
}

/**
 * Fonction renvoyant la distance euclidienne entre 2 tableaux de doubles de taille 56
 * @param trainDescriptor
 * @param valDescriptor
 * @return distance euclidienne entre les deux paramètres en entrée
 */
float distanceEuclidienneFloat(float* trainDescriptor, float* valDescriptor) {
    return produitScalaireFloat(trainDescriptor, valDescriptor, 56);
}

/**
 * Fonction renvoyant un tableau de key contenant les n images les plus proches 
 * @param nombreImagesProches : le nombre d'images le plus proche de l'image en entrée
 * @param vecteurEntree : histogramme de l'image rentré en paramètre 
 * @param nomFichierVecteurs : chemin du fichier contenant les histogrammes de notre base d'images
 * @param nombreImages : nombre d'images à utiliser dans la base d'image (de préférences, égale au nombre d'images)
 * @return 
 */
KEY* determinePlusProche(int nombreImagesProches, double* vecteurEntree, char* nomFichierVecteurs, int nombreImages) {

    FILE *fichierVecteurs = fopen(nomFichierVecteurs, "r");
    KEY* image = calloc(nombreImages, sizeof (KEY));
    if (fichierVecteurs == NULL) {
        printf("Erreur lors de la lecture de %s", nomFichierVecteurs);
    } else {
        int i;
        for (i = 0; i < nombreImages; i++) {
            double buffer[64];
            fread(buffer, sizeof (double), 64, fichierVecteurs);
            double res = distanceEuclidienne(buffer, vecteurEntree);
            KEY* k = (KEY*) malloc(sizeof (KEY));
	    k->k = i;
	    k->d = res;
            image[i] = *k;
        }
    }
    return image;
}
/**
 * Fonction renvoyant un tableau de key contenant les n images les plus proches 
 * @param nombreImagesProches : le nombre d'images le plus proche de l'image en entrée
 * @param vecteurEntree : histogramme de l'image rentré en paramètre 
 * @param nomFichierVecteurs : chemin du fichier contenant les histogrammes de notre base d'images
 * @param nombreImages : nombre d'images à utiliser dans la base d'image (de préférences, égale au nombre d'images)
 * @return 
 */
KEY* determinePlusProcheGabor(int nombreImagesProches, float* vecteurEntree, char* nomFichierVecteurs, int nombreImages) {

    FILE *fichierVecteurs = fopen(nomFichierVecteurs, "r");
    KEY* image = calloc(nombreImages, sizeof (KEY));
    if (fichierVecteurs == NULL) {
        printf("Erreur lors de la lecture de %s", nomFichierVecteurs);
    } else {
        int i;
        for (i = 0; i < nombreImages; i++) {
            float buffer[56];
            fread(buffer, sizeof (float), 56, fichierVecteurs);
            float res =  distanceEuclidienneFloat(buffer, vecteurEntree);
            KEY* k = (KEY*) malloc(sizeof (KEY));
	    k->k = i;
	    k->d = res;
            image[i] = *k;
        }
    }
    return image;
}

void export2HTML (char* url, KEY* image, int nbResult, char ** urlList){
    FILE* fichier = NULL;
    int i;

    fichier = fopen("search.html", "w");

    fprintf(fichier, "<h1>Image Source: </h1> <IMG height=\"100px\" width=\"100px\" src=\"%s\"/> <br/> <h1> Resultat(%d resultats les plus proches) </h1>", url, nbResult);
    for (i=0; i < nbResult; i++){
	fprintf(fichier, "<IMG height=\"100px\" width=\"100px\" src=\"%s\"/> Distance: %f Numero %d <br/>", urlList[image[i].k], image[i].d, image[i].k);	
    }


    fclose(fichier);
    printf("Exporté vers search.html\n");
}

/**
 * Main 
 * Utilisation : 
 * EXECUTABLE init //Permet de parser toutes les images du fichier urls.txt et d'en déduire les descripteurs qui sont écrit dans le fichier result.bin
 * EXECUTABLE cheminImage //Permet de renvoyer les images les plus proches (selon la méthode euclidienne) de celle en paramètre
 * @param argc
 * @param argv
 * @return 
 */
int main(int argc, char *argv[]) {

    char ** urlList;
    int nbAns;
    char *name;
    FILE* fileToWrite = NULL;
    int i;
    int plafond = 9637;
    char* url = argv[1];

    urlList = readList("urls.txt", &nbAns);

    //#####################" INIT ./histo init ############
    if (strcmp(argv[1], "init") == 0) {
        printf("init\n");
        fileToWrite = fopen("result.bin", "w");
        for (i = 0; i < plafond; i++) {
            printf("nom fichier lu %s, nbAns : %d\n", urlList[i], nbAns);
            name = urlList[i];
            makeHisto(name, fileToWrite);
        }
        fclose(fileToWrite);
//##################" SEARCH: ./histo gabor [url] ############
    }else if (strcmp(argv[1], "gabor") == 0) {
        float* histoImgRequest;
	float *eg;
	CIMAGE cim;

	url = argv[2];
	read_cimage(url,&cim);
	eg = egabor(cim,NDIR,NSCA,SIGMA0,LAMBDA0,SCALE);
	histoImgRequest = eg;
	plafond = 1000;
        KEY* image = determinePlusProcheGabor(10, histoImgRequest, "gabor.bin", plafond);
        qsort(image, plafond, sizeof (KEY), keyCompare);
	export2HTML(url, image, 10, urlList);	    
	}else {
    //##################" SEARCH: ./histo [url] ############
        printf("search   -%s- \n  ", url);
        double* histoImgRequest = makeHisto(url, NULL);
        KEY* image = determinePlusProche(10, histoImgRequest, "result.bin", plafond);
        qsort(image, plafond, sizeof (KEY), keyCompare);
	export2HTML(url, image, 10, urlList);
    }
//cc rdjpeg.c cgabor.c proc.c histo.c -o histo -lm
//cc -o test_gabor rdjpeg.c cgabor.c test_gabor.c -lm

    /*------------------------------------------------*/
    exit(0);
}

/**
 * Fonction qui renvoie l'histogramme(=descripteur) de l'image correspondant au chemin passé en paramètre. Cet histogramme 
 * est inscrit dans le fichier fileToWrite
 * @param name : chemin de l'image dont l'historamme est déduit
 * @param fileToWrite : fichier dans lequel est écrit l'histogramme de l'image en paramètre 
 * @return : l'histogramme de l'image
 */
double* makeHisto(char* name, FILE * fileToWrite) {
    int i, j, n, nx, ny, nb; /* j en vertical, i en horizontal */
    CIMAGE cim;
    unsigned char **r;
    unsigned char **g;
    unsigned char **b;
    int rr, bb, gg;
    int nbIntervalle = 64;
    double* h = calloc(nbIntervalle, sizeof (double));
    int k;

    for (i = 0; i < nbIntervalle; i++) {
        h[i] = 0;
    }

    /*------------------------------------------------*/
    /* lecture d'une image requête                    */
    /*------------------------------------------------*/
    read_cimage(name, &cim);
    r = cim.r;
    g = cim.g;
    b = cim.b;
    /*------------------------------------------------*/
    /* affichage des valeurs pour le premier bloc 8x8 */
    /* comme exemple de traitement                    */
    /*------------------------------------------------*/
    //printf("Largeur de l'image : %d\n", cim.nx);
    //  printf("Heuteur de l'image : %d\n", cim.ny);

    for (j = 0; j < cim.ny; j++) { /* ligne par ligne */
        for (i = 0; i < cim.nx; i++) { /* pixel par pixel */
            rr = r[i][j] / 64;
            gg = g[i][j] / 64;
            bb = b[i][j] / 64;
            k = rr + 4 * bb + 16 * gg;
            // 	printf("k : %d\n", k);
            h[k]++;
        }
    }

    double somme = 0;
    //Normalisation
    for (i = 0; i < nbIntervalle; i++) {
        h[i] = h[i] / (cim.nx * cim.ny);
        somme += h[i];
    }
    if (fileToWrite != NULL)
        fwrite(h, sizeof (double), 64, fileToWrite);
    return h;
}






