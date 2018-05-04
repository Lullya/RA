#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include <sys/stat.h>

#include "antidot.h"

#define YEAR 2020

void restructTree(const char *client) {

    int ret;
    char msgbuf[100];
    regex_t regex;
    DIR *client_dir;
    struct dirent *client_cursor;


    if (!(client_dir = opendir(client))) return;


    while ((client_cursor = readdir(client_dir)) != NULL) {
        if ((strchr(client_cursor->d_name, '.')) == NULL) {
            printf("Ouverture du dossier client %s\n", client_cursor->d_name);

            ret = regcomp(&regex, "^(26|84){1}[0-9]{4}$|^(28|88){1}[0-9]{4}$", REG_EXTENDED);

            if (ret != 0)
                fprintf(stderr, "Could not compile regex\n");

            ret = regexec(&regex, client_cursor->d_name, 0, NULL, 0);

            // Pour chaque dossier client correct
            // (dossier commencant par 26 ou 84 avec 4 chiffres derrière, SANS lettre).
            if (ret == 0) {

                printf("\tLe dossier %s est un dossier valide \n", client_cursor->d_name);

                printf("\t\tVérification du dossier PAIES\n");
                checkPAIES(client, ret);
                printf("\t\tDéplacement des fichiers de ISAPAYE vers PAIES\n");
                moveFiles(client, ret, client_dir, client_cursor);
                printf("\t\tCorrection des dossiers de PAIES\n");
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


void checkPAIES(const char *client, int ret) {
    DIR *paies_dir;
    struct dirent *paies_cursor;
    char paies_path[1024], yearPath[1024], old_paies_files_path[1024], new_paies_files_path[1024];

    regex_t date;

    // Dans le cas où un dossier PAIES est déjà existant (avec potentiellement des fichiers).

    snprintf(paies_path, sizeof(paies_path), "%s\\PAIES", client);

    if ((paies_dir = opendir(paies_path))) {

        printf("\t\t\tDossier PAIES ouvert\n");

        // On parcourt tous les fichiers cotenus dans PAIES.
        while ((paies_cursor = readdir(paies_dir)) != NULL) {

            printf("\t\t\t\tparcourt des fichiers du dossiers PAIES : %s\n", paies_cursor->d_name);

            // Et si ce sont des fichiers (avec une extension) et non des dossiers.
            if ((strchr(paies_cursor->d_name, '.')) == NULL) {

                printf("\t\t\t\tparcourt des DOSSIERS du dossiers PAIES : %s\n", paies_cursor->d_name);

                // Note : il n'y a pas de dossier ISAPAYE avant 2006.
                // Si on trouve dans le dossier PAIES des fichiers formatés par année,
                // On crée le dossier année (exemple : 2018) et on y place tous les fichiers
                // comportant cette date dans leur nom.
                for (int year = 2005; year < YEAR; ++year) {
                    char datec[5];
                    datec[0]= (char) year;

                    ret = regcomp(&date, (const char *) datec, REG_EXTENDED);

                    if (ret != 0)
                        fprintf(stderr, "Could not compile regex\n");

                    ret = regexec(&date, paies_cursor->d_name, 0, NULL, 0);
                    printf("%d\n", ret);
                    // Déplacement des fichiers.
                    if (ret == 0) {

                        snprintf(yearPath, sizeof(yearPath), "%s\\PAIES\\%d", client, year);
                        mkdir(yearPath, 0777);

                        snprintf(old_paies_files_path, sizeof(old_paies_files_path), "%s\\PAIES\\%s",
                                 client, paies_cursor->d_name);
                        snprintf(new_paies_files_path, sizeof(new_paies_files_path), "%s\\PAIES\\%d\\%s", client, year,
                                 paies_cursor->d_name);
                        rename(old_paies_files_path, new_paies_files_path);
                    } else{
                        fprintf(stderr, "Could not exec regex\n");

                    }
                }
            }
        }
        closedir(paies_dir);
        regfree(&date);
    }
        // Dans le cas où il n'y a pas de dossier PAIES existant, il n'y a pas de traitement
        // à effectuer (étant donné que s'il n'existe pas, il n'a pas de fichiers),
        // on se contente de le créer.
    else {
        mkdir(paies_path, 0777);
        printf("Dossier PAIES créé\n");
    }

}

void moveFiles(const char *client, int ret, DIR *client_dir, struct dirent *client_cursor) {
    char randomStuff_path[1024], isapaye_path[1024];
    regex_t year_filter;


    // On parcourt les dossiers années (2016, 2017, 2018 etc.).
    while ((client_cursor = readdir(client_dir)) != NULL) {
        if ((strchr(client_cursor->d_name, '.')) == NULL) {


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
                        if(isapaye_cursor->d_name != ".." && isapaye_cursor->d_name != ".") {
                            snprintf(randomStuff_path, sizeof(randomStuff_path), "%s\\%s\\ISAPAYE\\%s", client,
                                     client_cursor->d_name, isapaye_cursor->d_name);
                            snprintf(isapaye_path, sizeof(isapaye_path), "%s\\PAIES\\%s\\%s", client,
                                     client_cursor->d_name,
                                     isapaye_cursor->d_name);
                            rename(randomStuff_path, isapaye_path);
                        }
                    }
                    // Suppression du dossier ISAPAYE si posssible.
                    rmdir(randomStuff_path);
                    rmdir(isapaye_path);
                } else {
                    fprintf(stderr, "Could not open ISAPAYE \n");

                }
                closedir(isapaye_dir);
            } else {
                fprintf(stderr, "Could not exec regex MF\n");

            }
        }
    }
}

void postCorrectionPAIES(const char *client, int ret) {

    regex_t month;
    DIR *paies_dir, *year_dir, *random_dir;
    struct dirent *paies_cursor, *year_cursor, *randomDir_cursor;

    char paies_path[1024], year_path[1024], randomDIR_path[1024],
            oldRandomFileinRandomDirPath[1024], newRandomFilePath[1024];

    snprintf(paies_path, sizeof(paies_path), "%s\\PAIES", client);

    // Lecture des éléments composant le dossier PAIES.
    paies_dir = opendir(paies_path);
    if (paies_dir) {
        while ((paies_cursor = readdir(paies_dir)) != NULL) {

            snprintf(year_path, sizeof(paies_path), "%s\\PAIES\\%s", client, paies_cursor->d_name);

            if (((strchr(paies_cursor->d_name, '.')) != NULL) && (year_dir = opendir(year_path))) {
                while ((year_cursor = readdir(year_dir)) != NULL) {

                    ret = regcomp(&month, "janvier|fevrier|février|mars|avril|mai|juin|juillet|août|aout"
                                          "|septembre|octobre|novembre|décembre|decembre|JANVIER|FEVRIER"
                                          "|MARS|AVRIL|MAI|JUIN|JUILLET|AOUT|SEPTEMBRE|OCTOBRE|NOVEMBRE|"
                                          "DECEMBRE", 0);
                    if (ret != 0)
                        fprintf(stderr, "Could not compile regex\n");

                    ret = regexec(&month, paies_cursor->d_name, 0, NULL, 0);

                    snprintf(randomDIR_path, sizeof(paies_path), "%s\\PAIES\\%s\\%s", client, paies_cursor->d_name,
                             year_cursor->d_name);

                    // Recherche des dossiers ayant pour nom un mois.
                    if (ret && ((strchr(year_cursor->d_name, '.')) == NULL) &&
                        (random_dir = opendir(randomDIR_path))) {
                        while ((randomDir_cursor = readdir(random_dir)) != NULL) {

                            if (randomDir_cursor->d_name != ".." && randomDir_cursor->d_name != ".") {
                                snprintf(oldRandomFileinRandomDirPath, sizeof(oldRandomFileinRandomDirPath),
                                         "%s\\PAIES\\%s\\%s\\%s", client,
                                         paies_cursor->d_name, year_cursor->d_name, randomDir_cursor->d_name);

                                snprintf(newRandomFilePath, sizeof(newRandomFilePath), "%s\\PAIES\\%s\\%s", client,
                                         paies_cursor->d_name, randomDir_cursor->d_name);

                                rename(oldRandomFileinRandomDirPath, newRandomFilePath);
                            }

                        }
                        rmdir(newRandomFilePath);
                    } else {
                        fprintf(stderr, "Could not exec regex PCP\n");

                    }
                    closedir(random_dir);
                }
            }
            closedir(year_dir);
        }
        closedir(paies_dir);
    } else
        fprintf(stderr, "Could not read PAIES\n");
}