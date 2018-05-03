#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include <sys/stat.h>

//TODO: restructTree -> refactoring, moveFiles -> comments, testing (paths)

/**
 *
 * @param client
 * @param ret
 * @param client_dir
 * @param client_cursor
 */
void moveFiles(const char *client, int ret, DIR *client_dir, struct dirent *client_cursor)
{
    char randomStuff_path[1024], isapaye_path[1024];
    regex_t year_filter;


    // On parcourt les dossiers années (2016, 2017, 2018 etc.).
    while ((client_cursor = readdir(client_dir)) != NULL) {

        ret = regcomp(&year_filter, "(20|19){1}[0-9]{2}$", REG_EXTENDED);
        if (ret != 0)
            fprintf(stderr, "Could not compile regex\n");

        ret = regexec(&year_filter, client_cursor->d_name, 0, NULL, 0);
        // Pour chaque dossier trouvé, on créé dans PAIES un dossier du même nom,
        // exemple : 260000/2016 -> 260000/PAIES/2016.
        if (ret == 0) {
            snprintf(randomStuff_path, sizeof(randomStuff_path), "%s\\PAIES\\%s", client, client_cursor->d_name);
            mkdir(randomStuff_path, 0777);

            // On parcourt les éléments dans ISAPAYE
            // (exemple : 260000/2016/ISAPAYE).
            DIR *isapaye_dir;
            snprintf(isapaye_path, sizeof(isapaye_path), "%s\\%s\\ISAPAYE", client, client_cursor->d_name);

            isapaye_dir = opendir(isapaye_path);
            if (isapaye_dir) {

                struct dirent *isapaye_cursor;
                while ((isapaye_cursor = readdir(isapaye_dir)) != NULL) {
                    snprintf(randomStuff_path, sizeof(randomStuff_path), "%s\\%s\\ISAPAYE\\%s", client, 
                             client_cursor->d_name, isapaye_cursor->d_name);
                    snprintf(isapaye_path, sizeof(isapaye_path), "%s\\PAIES\\%s\\%s", client, client_cursor->d_name,
                             isapaye_cursor->d_name);
                    rename(randomStuff_path, isapaye_path);
                }
                // Suppression du dossier ISAPAYE si posssible.
                rmdir(randomStuff_path);
                rmdir(isapaye_path);
            }
            closedir(isapaye_dir);
        }
    }
}

/**
 * Les fichiers présents dans certains dossier dans ISAPAYE sont suceptibles d'être déjà présents
 * si c'est le cas, ceux-ci ne sont pas déplacés, c'est pour ça qu'on déplace directement le dossier conteneur
 * du fichier double dans PAIES afin d'avoir un dossier ISAPAYE vide.
 * C'est dans PAIES qu'on supprime les sous-dossiers (quand ceux-ci ne sont pas importants).
 * Rappel : on souhaite conserver le minimum de sous-dossier dans 260000/2016 par exemple.
 * Les sous-dossiers contenants des fichiers doubles (par fichier double, on entend par exemple :
 * 260000/2016/DOSSIER/fichier-2016.pdf et 260000/2016/fichier-2016.pdf)
 * ne sont ni vidés et ni suppprimés. On pourrait essayer de comparer la date des deux fichiers et écraser le plus ancien
 * ou renommer le plus ancien, cette tache n'est pas prioritaire.
 *
 * Cet algorithme recherche les sous-dossiers de PAIES et déplace leur contenu à la racine de PAIES exemple :
 * 260000/PAIES/2016/DOSSIER/fichier-2016.pdf -> 260000/PAIES/2016/fichier-2016.pdf
 * "DOSSIER" est ensuite supprimé.
 * Ce traitement est fait uniquement si "fichier-2016-.pdf" n'est pas déjà présent à la racine, si tel est le cas,
 * on ignore ce dossier.
 *
 * @param client : numéro de dossier client (exemple : 260000)
 * @param ret : variable retournant 0 lorsqu'on a réussit à compiler une expression régulière.
 */
void postCorrectionPAIES(const char *client, int ret) {

    regex_t month;
    DIR *paies_dir, *year_dir, *random_dir;
    struct dirent *paies_cursor, *year_cursor, *randomDir_cursor;

    char oldRandomFileinRandomDirPath[1024], newRandomFilePath[1024];

    // Lecture des éléments composant le dossier PAIES.
    paies_dir = opendir("PAIES");
    if (paies_dir) {
        while ((paies_cursor = readdir(paies_dir)) != NULL) {


            if (((strchr(paies_cursor->d_name, '.')) != NULL) && (year_dir = opendir(paies_cursor->d_name)))
            {
                while ((year_cursor = readdir(year_dir)) != NULL) {

                    ret = regcomp(&month, "janvier|fevrier|février|mars|avril|mai|juin|juillet|août|aout"
                                          "|septembre|octobre|novembre|décembre|decembre|JANVIER|FEVRIER"
                                          "|MARS|AVRIL|MAI|JUIN|JUILLET|AOUT|SEPTEMBRE|OCTOBRE|NOVEMBRE|"
                                          "DECEMBRE", 0);
                    if (ret != 0)
                        fprintf(stderr, "Could not compile regex\n");

                    ret = regexec(&month, paies_cursor->d_name, 0, NULL, 0);

                    // Recherche des dossiers ayant pour nom un mois.
                        if (!ret && ((strchr(year_cursor->d_name, '.')) != NULL) &&
                            (random_dir = opendir(year_cursor->d_name))) {
                            while ((randomDir_cursor = readdir(random_dir)) != NULL) {

                                snprintf(oldRandomFileinRandomDirPath, sizeof(oldRandomFileinRandomDirPath),
                                         "%s\\PAIES\\%s\\%s\\%s", client,
                                         paies_cursor->d_name, year_cursor->d_name, randomDir_cursor->d_name);

                                snprintf(newRandomFilePath, sizeof(newRandomFilePath), "%s\\PAIES\\%s\\%s", client,
                                         paies_cursor->d_name, randomDir_cursor->d_name);

                                rename(oldRandomFileinRandomDirPath, newRandomFilePath);
                            }
                            rmdir(newRandomFilePath);
                        }
                        closedir(random_dir);
                }
            }
            closedir(year_dir);
        }
        closedir(paies_dir);
    }
    else
        fprintf(stderr, "Could not read PAIES\n");
}
/**
 * Algorithme principal de restructuration de la GED.
 * Globalement, cet algorithme va :
 * dans chaque dossier client (260001, 260002, 840001, 840002, etc.) rechercher les fichiers contenus dans ISAPAYE :
 * exemple : 260001/2016/ISAPAYE/fichier-2016.pdf
 * Et le déplacer dans un dossier PAIES (exemple : 260001/PAIES/2016/fichier-2016/pdf)
 * Enfin, puisque TOUT le contenu du dossier ISAPAYE est déplacé, celui-ci est vide et donc supprimé.
 * Si le dossier parent ne contenait uniquement le dossier ISAPAYE, celui-ci aussi devient vide et supprimé aussi.
 *
 * L'algorithme se compose en 3 étapes :
 *
 * @param client : numéro de dossier client (exemple : 260000)
 * on passe à ce paramètre le dossier dans lequel on veut effectuer notre restructuration.
 */
void restructTree(const char *client) {

    int ret;
    char msgbuf[100], path[1024];
    regex_t regex, date;
    DIR *client_dir, *paies_dir;
    struct dirent *client_cursor, *paies_cursor;


    if (!(client_dir = opendir(client)))
        return;

    printf("1\n");

    while ((client_cursor = readdir(client_dir)) != NULL) {

        printf("2\n");

        if ((strchr(client_cursor->d_name, '.')) == NULL) {

            printf("2b\n");
            printf("\t%s\n", client_cursor->d_name);

            ret = regcomp(&regex, "^(26|84){1}[0-9]{4}$|^(28|88){1}[0-9]{4}$", REG_EXTENDED);

            if (ret)
                fprintf(stderr, "Could not compile regex\n");

            ret = regexec(&regex, client_cursor->d_name, 0, NULL, 0);

            // Pour chaque dossier client correct
            // (dossier commencant par 26 ou 84 avec 4 chiffres derrière, SANS lettre).
            if (ret == 0) {

                // Dans le cas où un dossier PAIES est déjà existant (avec potentiellement des fichiers.
                if ((paies_dir = opendir("PAIES"))) {

                    printf("Dossier PAIES ouvert\n");

                    // On parcourt tous les fichiers cotenus dans PAIES.
                    while ((paies_cursor = readdir(paies_dir)) != NULL) {

                        printf("\tparcourt des fichiers du dossiers PAIES : %s\n", paies_cursor->d_name);

                        // Si ce sont des fichiers (avec une extension) et non des dossiers.
                        if ((strchr(paies_cursor->d_name, '.')) != NULL) {

                            printf("\t\tparcourt des DOSSIERS du dossiers PAIES : %s\n", paies_cursor->d_name);

                            // Il n'y a pas de dossier ISAPAYE avant 2006.
                            // Si on trouve dans le dossier PAIES des fichiers formatés par année,
                            // On crée le dossier année (exemple : 2018) et on y place tous les fichiers
                            // comportant cette date dans leur nom.
                            for (int year = 2005; year < 2020; ++year) {

                                ret = regcomp(&date, (const char *) year, REG_EXTENDED);

                                if (ret != 0)
                                    fprintf(stderr, "Could not compile regex\n");

                                ret = regexec(&date, paies_cursor->d_name, 0, NULL, 0);

                                // Déplacement des fichiers.
                                if (ret == 0) {

                                    char yearPath[1024], old_paies_files_path[1024], new_paies_files_path[1024];

                                    snprintf(yearPath, sizeof(path), "%s\\PAIES\\%d", client, year);
                                    mkdir(yearPath, 0777);

                                    snprintf(old_paies_files_path, sizeof(path), "%s\\PAIES\\%s",
                                             client, paies_cursor->d_name);
                                    snprintf(new_paies_files_path, sizeof(path), "%s\\PAIES\\%d\\%s", client, year,
                                             paies_cursor->d_name);
                                    rename(old_paies_files_path, new_paies_files_path);
                                }
                            }
                        }
                    }
                    closedir(paies_dir);
                }

                // Dans le cas où il n'y a pas de dossier PAIES existant, il n'y a pas de traitement
                // à effectuer (étant donné que s'il n'existe pas, il n'a pas de fichiers),
                // on se contente de le créer.
                else {
                    snprintf(path, sizeof(path), "%s\\PAIES", client);
                    mkdir(path, 0777);
                    printf("Dossier PAIES créé\n");
                }

                moveFiles(client, ret, client_dir, client_cursor);

                // On a déplacé tout ce qui était contenu dans ISAPAYE (ex : 260000/2015/ISAPAYE) dans PAIES
                // (ex : 260000/PAIES/2015)
                // exemple : 260000/2015/ISAPAYE/fichier-2015.txt
                // exemple : 260000/2015/ISAPAYE/janvier
                // exemple : 260000/2015/ISAPAYE/DOSSIER_JEAN
                // On souhaite garder le minimum de dossier dans PAIES
                postCorrectionPAIES(client, ret);

            } else if (ret != REG_NOMATCH) {
                regerror(ret, &regex, msgbuf, sizeof(msgbuf));
                fprintf(stderr, "Regex match failed: %s\n", msgbuf);
                exit(1);
            }
        }
    }
    closedir(client_dir);
    regfree(&regex);
}

int main(void) {
    restructTree(".");
    return 0;
}


