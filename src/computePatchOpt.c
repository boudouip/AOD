/*! \file computePatchOpt.c
 * \brief Ceci implémente le programme computePatchOpt
 * \author Philippe Boudouin
 * \author Adrien Bouchoux
 * \date 27/11/2015
 * \warning Usage computePatchOpt sourceFile targetFile
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

 /** Facteur de multiplication de la taille des tableaux dynamiques */
#define MULTSIZE 2
/** Taille maximale d'une ligne (une taille de ligne supérieure entrainera une erreur d'exécution) */
#define TAILLELIGNE 2048 

/** Type d'opérations possibles */
enum decision_enum {
	AUCUNE, AJOUT, SUPPRESSION, RECOPIE, SUBSTITUTION
};

/** Structure stockée pour chaque combinaison de numéro de ligne contenant l'opération effectuée et le nombre de suppression */
struct decision {
	enum decision_enum type;
	uint32_t nb_suppr;
};

// Variables globales
/** Tableau contenant les lignes de chaque fichier */
char** tabligne[2]; 
/** Nombre de lignes de chaque fichier */
uint32_t nbligne[2] = {0};
/** Nombre de caractères de chaque ligne de chaque fichier*/
uint32_t* nbcar[2];
/** Tableau de décision */
struct decision **dec;

/*!
 * Fonction qui écrit le patch trouvé sur la sortie standard. (codée par Adrien Bouchoux)
 * @param n1 nombre de ligne du premier fichier + 1
 * @param n2 nombre de ligne du second fichier + 1
 * @param n2 nombre de ligne du second fichier + 1
 * @param dec tableau de decisions
 */
void writePatch(uint32_t n1, uint32_t n2, struct decision **dec) {
	uint32_t i = n1 - 1;
	uint32_t j = n2 - 1;
	char **patch = calloc(2 * (n1 + n2), sizeof(char*));
	uint32_t k = 0;
	while (i > 0 || j > 0) {
		if (!patch[k]) {
			patch[k] = calloc(TAILLELIGNE, sizeof(char));
		}
		if (!patch[k+1]) {
			patch[k+1] = calloc(TAILLELIGNE, sizeof(char));
		}
		if (i == 0) { /* cas du bod */
			dec[j][i].type = AJOUT;
		} else if (j == 0) {
			dec[j][i].type = SUPPRESSION;
			dec[j][i].nb_suppr = i;
		}
		switch (dec[j][i].type) {
		case RECOPIE:
			i--;
			j--;
			break;
		case SUBSTITUTION:
			sprintf(patch[k], "%s", tabligne[1][j-1]);
			sprintf(patch[k+1], "= %d\n", i);
			i--;
			j--;
			k += 2;
			break;
		case AJOUT:
			sprintf(patch[k], "%s", tabligne[1][j-1]);
			sprintf(patch[k+1], "+ %d\n", i);
			j--;
			k += 2;
			break;
		case SUPPRESSION:
			if (dec[j][i].nb_suppr == 1) {
				sprintf(patch[k], "d %d\n", i);
			} else {
				sprintf(patch[k], "D %d %d\n", i - dec[j][i].nb_suppr + 1, dec[j][i].nb_suppr);
			}
			k++;
			i -= dec[j][i].nb_suppr;
			break;
		case AUCUNE:
			printf("Arbre non couvrant");
			exit(EXIT_FAILURE);
			break;

		}
	}
	if (k != 0) {
		do {
			k--;
			if (patch[k]) {
				printf("%s", patch[k]);
				free(patch[k]);
			}
		} while (k > 0);
	}
	free(patch);
}


/*!
 * Fonction qui lit les fichiers et en enregistre le contenu, le nombre de caractères et le nombre de caractères par ligne dans la mémoire. (Codée par Philippe Boudouin)
 * @param argv tableau de nom de fichier de taille 2
 * \returns { EXIT_SUCCESS si succès, EXIT_FAILURE sinon}
 */
int parseur(char* argv[]) {
	uint32_t nblignemax[2] = {10,10}; // Nombre de lignes max
	for (uint8_t i = 0; i < 2; i++) {
		FILE* fichier = NULL;
		fichier = fopen(argv[i+1], "r");
		if (fichier == NULL) {
			printf("Le fichier %s n'a pas été ouvert correctement.", argv[i+1]);
			return EXIT_FAILURE;
		}

		char buff[TAILLELIGNE];
		char** temp1;
		uint32_t* temp2;
		tabligne[i] = malloc(nblignemax[i]*sizeof(char**));
		nbcar[i] = calloc(nblignemax[i],sizeof(uint32_t));
		int cour = 0;
		cour = fgetc(fichier);
		while (cour != EOF) {
			// nouvelle ligne
			// On alloue de la place supplémentaire si besoin (gestion du tableau de taille dynamique)
			if (nbligne[i] >= nblignemax[i]) { 
				temp1 = calloc(nblignemax[i]*MULTSIZE,sizeof(char**));
				temp2 = calloc(nblignemax[i]*MULTSIZE,sizeof(uint32_t));
				memcpy(temp1, tabligne[i], sizeof(char**)*nblignemax[i]);
				memcpy(temp2, nbcar[i], sizeof(uint32_t)*nblignemax[i]);
				nblignemax[i] = nblignemax[i]*MULTSIZE;
				free(tabligne[i]);
				free(nbcar[i]);
				tabligne[i] = temp1;
				nbcar[i] = temp2;
				temp1 = NULL;
				temp2 = NULL;
			}
			// On écrit la ligne en mémoire
			while (cour != '\n') {
				buff[nbcar[i][nbligne[i]]] = (unsigned char) (cour);
				nbcar[i][nbligne[i]]++;
				cour = fgetc(fichier);	
			}
			buff[nbcar[i][nbligne[i]]] = (unsigned char) (cour);
			nbcar[i][nbligne[i]]++;
			buff[nbcar[i][nbligne[i]]] = '\0';
			// On alloue la ligne
			tabligne[i][nbligne[i]] = malloc((nbcar[i][nbligne[i]]+1)*sizeof(char));
			memcpy(tabligne[i][nbligne[i]], &buff, (nbcar[i][nbligne[i]]+1)*sizeof(char));
			nbligne[i]++;
			cour = fgetc(fichier);
		}
		// Si erreur lors de la lecture
		if (ferror(fichier)) {
			printf("Erreur lors de la lecture du fichier %s.", argv[i+1]);
			return EXIT_FAILURE;
		}
		fclose(fichier);
	}
	return EXIT_SUCCESS;
}

/*!
 * Fonction qui calcule le patch minimal. (Codée par Philippe Boudouin)
 * \returns { EXIT_SUCCESS si succès, EXIT_FAILURE sinon}
 */
int bellman() {
	// On traite les cas particuliers (pas de lignes)
	if (nbligne[0] == 0) {
		printf("Cas trivial non supporté, seulement des additions à faire.");
		return EXIT_FAILURE;
	}
	// Algo général
	// Initialisation
	// On ne stocke les K[i,j] que pour les j courants
	uint32_t* K = malloc(sizeof(uint32_t)*(nbligne[0]+1));
	K[0] = 0;
	K[1] = 10;
	for (uint32_t j = 2; j < nbligne[0]+1; j++) {
		K[j] = 15;
	}
	uint32_t i; // ligne du fichier 1
	uint32_t j = 1; // ligne du fichier 2
	uint64_t min; // valeur minimal pour le K[i,j] en train d'être calculé
	uint64_t minD; // valeur minimal sur laquelle se baser pour les opérations de multi-destruction
	uint32_t Prev; // K[i-1,j], la valeur de K[i-1,j-1] étant toujours utilisée.
	
	// On parcourt selon les lignes du fichier 2 puis 1 afin de limiter les défauts de cache
	dec = calloc(nbligne[1] + 1, sizeof(struct decision *));
	for (i = 0; i < nbligne[1] + 1; i++) {
		dec[i] = calloc(nbligne[0] + 1, sizeof(struct decision));
	}

	// Parcours
	while (j != nbligne[1]+1) {
		i = 1;
		Prev = K[0] + nbcar[1][j-1] + 10;
		minD = 4294967295; // UINT32_T MAX
		while (i <= nbligne[0]) {
			min = K[i] + nbcar[1][j-1] + 10; // a
			dec[j][i].type = AJOUT;
			dec[j][i].nb_suppr = 0;

			// On trouve le minimum
			if (Prev + 10 < min) { // d
				min = Prev + 10;
				dec[j][i].type = SUPPRESSION;
				dec[j][i].nb_suppr = 1;
			}
			if (minD + 15 < min) { // D
				min = minD + 15;
				dec[j][i].type = SUPPRESSION;
				dec[j][i].nb_suppr = dec[j][i-1].nb_suppr + 1;
			}
			if (nbcar[0][j-1] == nbcar[1][j-1]) {
				if (strcmp(tabligne[0][i-1],tabligne[1][j-1]) == 0) {
					if (K[i-1] < min) { // r
						min = K[i-1];
						dec[j][i].type = RECOPIE;
						dec[j][i].nb_suppr = 0;
					}	
				} else {
					if (K[i-1] + nbcar[1][j-1] + 10  < min) { // s
						min = K[i-1] + nbcar[1][j-1] + 10;
						dec[j][i].type = SUBSTITUTION;
						dec[j][i].nb_suppr = 0;
					}
				}
			} else {
				if (K[i-1] + nbcar[1][j-1] + 10  < min) { // s
					min = K[i-1] + nbcar[1][j-1] + 10;
					dec[j][i].type = SUBSTITUTION;
					dec[j][i].nb_suppr = 0;
				}
			}
			// 
			K[i-1] = Prev;
			if (Prev < minD) {
				minD = Prev;
			}
			Prev = min;
			i++;
		}
		K[i-1] = Prev;
		j++;
	}
#if 0
	printf("K[%u][%u] = %u\n",nbligne[0],nbligne[1],K[nbligne[0]]);
#endif
	writePatch(nbligne[0] + 1, nbligne[1] + 1, dec);
	free(K);
	for (uint32_t k = 0; k < 2; k++) {
		for (uint32_t l = 0; l < nbligne[k]; l++) {
			free(tabligne[k][l]);
		}
		free(nbcar[k]);
		free(tabligne[k]);
	}
	return EXIT_SUCCESS;
}

/**
 * \brief fonction main
 * \param argc Nombre d'arguments de la ligne de commande.
 * \param argv Tableau de charactères contenant les arguments.
 * \warning Doit être appelé avec 2 noms de fichiers sourceFile, targetFile en tant qu'argument d'une ligne de commande dans cet ordre ci.
 * \returns { EXIT_SUCESS si succès, EXIT_FAILURE sinon}
 */
int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Cet executable requiert 2 paramètres, vous en avez fourni %i.\nExemple : ComputePatchOpt F1 F2",argc-1);
		return EXIT_FAILURE;
	}
	// Parseur
	if (parseur(argv) == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}
	// Traitement
	if (bellman() == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}
	for (uint32_t k = 0; k < nbligne[1] + 1; k++) {
		free(dec[k]);
	}
	free(dec);
}

