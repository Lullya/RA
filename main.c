#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

void listdir(const char *name)
{
    printf("test\n");
    regex_t regex;
    regex_t date;
    regex_t date_annee;

    int reti;
    char msgbuf[100];
    char path0[1024];
    char path[1024];
    char path2[1024];
    char pathIsapaye[1024];

    reti = regcomp(&regex, "(26|84){1}[0-9]{4}$|^(./)?(28|88){1}[0-9]{4}$", 0);
    if( reti ){ fprintf(stderr, "Could not compile regex\n"); exit(1); }

    DIR *de;
    struct dirent *entry;

    if (!(de = opendir(name)))
        return;
    printf("test\n");

    while ((entry = readdir(de)) != NULL) {
        if ((strchr(entry->d_name, '.')) == NULL) {
            printf("test\n");


            reti = regexec(&regex, entry->d_name, 0, NULL, 0);

            // pour chaque dossier client correct (commencant uniquement par 26 ou 84)
            if(!reti){
                snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
                printf("%*s\n", entry->d_name);

                DIR* dir;
                struct dirent *annees;
                if((dir = opendir("PAIES")))
                {
                    // on parcourt tous les fichiers cotenus dans PAIES
                    while ((annees = readdir(dir)) != NULL)
                    {
                        // si ce sont des fichiers avec une extension et non un dossier
                        if ((strchr(annees->d_name, '.')) != NULL)
                        {
                            for (int i = 2009; i < 2020; ++i) {
                                char j[100];
                                j[0]=i;
                                int datec = regcomp(&date, j, 0);

                                datec = regexec(&date, annees->d_name, 0, NULL, 0);

                                if (!datec)
                                {
                                    snprintf(path0, sizeof(path), "%s/PAIES/%s/%d", name, i);
                                    snprintf(path, sizeof(path), "%s/PAIES/%s", name, annees->d_name);
                                    snprintf(path2, sizeof(path), "%s/PAIES/%d/%s", name, i, annees->d_name);
                                    mkdir(path0, 0777);
                                    rename(path, path2);
                                }
                            }
                        }
                    }
                    closedir(dir);
                }
                else
                {
                    snprintf(path, sizeof(path), "%s/PAIES", name);
                    mkdir(path, 0777);
                }

                while ((annees = readdir(dir)) != NULL)
                {
                    int reg = regcomp(&date_annee, "(20|19){1}[0-9]{2}$", 0);
                    reg = regexec(&date_annee, annees->d_name, 0, NULL, 0);

                    if(!reg)
                    {
                        snprintf(path, sizeof(path), "%s/PAIES/$s", name, annees->d_name);
                        mkdir(path, 0777);
                        DIR* dire;
                        snprintf(path2, sizeof(path2), "%s/ISAPAYE", annees->d_name);
                        if(dire = opendir(path2))
                        {
                            struct dirent *isapaye;
                            while((isapaye = readdir(dire)) != NULL)
                            {
                                snprintf(path0, sizeof(path0), "%s/PAIES/%s/%s", name, annees->d_name);
                                snprintf(path, sizeof(path), "%s/%s/ISAPAYE/%s", name, annees->d_name, isapaye->d_name);
                                snprintf(path2, sizeof(path2), "%s/PAIES/%s/%s", name, annees->d_name, isapaye->d_name);
                                mkdir(path0, 0777);
                                rename(path, path2);
                            }
                            rmdir(path);
                            snprintf(pathIsapaye, sizeof(pathIsapaye), "%s/%s/ISAPAYE", name, annees->d_name);
                            rmdir(pathIsapaye);
                        }
                        closedir(dire);
                    }

                }
                printf("test\n");

                DIR* paies;
                struct dirent* dossiersPaies;
                if((paies = opendir("PAIES")))
                {
                    while ((dossiersPaies = readdir(paies)) != NULL)
                    {
                        DIR* paiesAnnee;
                        if (paiesAnnee = opendir(dossiersPaies->d_name))
                        {

                            struct dirent* dossiersPaiesAnnee;

                            while(( dossiersPaiesAnnee = readdir(paiesAnnee)) != NULL)
                            {
                                regex_t mois;
                                int ret;
                                ret = regcomp(&mois, "janvier|fevrier|février|mars|avril|mai|juin|juillet|août|aout|septembre|octobre|novembre|décembre|decembre|JANVIER|FEVRIER|MARS|AVRIL|MAI|JUIN|JUILLET|AOUT|SEPTEMBRE|OCTOBRE|NOVEMBRE|DECEMBRE", 0);
                                ret = regexec(&mois, dossiersPaies->d_name, 0, NULL, 0);
                                if(!ret && ((strchr(dossiersPaiesAnnee->d_name, '.')) != NULL))
                                {
                                    DIR* dossierPA;
                                    struct dirent* fo;

                                    if ((dossierPA = opendir(dossiersPaiesAnnee->d_name)))
                                    {
                                        while((fo = readdir(dossierPA)) != NULL)
                                        {
                                            snprintf(path, sizeof(path), "%s/PAIES/%s/%s/%s", name, dossiersPaies->d_name, dossiersPaiesAnnee->d_name, fo->d_name);
                                            snprintf(path2, sizeof(path2), "%s/PAIES/%s/%s", name, dossiersPaies->d_name, fo->d_name);
                                            rename(path, path2);
                                        }
                                        rmdir(path2);
                                    }
                                    closedir(dossierPA);
                                }
                            }
                        }
                        closedir(paiesAnnee);

                    }
                }
                closedir(paies);
            }
            else if( reti != REG_NOMATCH ){
                regerror(reti, &regex, msgbuf, sizeof(msgbuf));
                fprintf(stderr, "Regex match failed: %s\n", msgbuf);
                exit(1);
            }
        }
        else
            printf("%*s\n", entry->d_name);
    }
    closedir(de);
    regfree(&regex);
    printf("test\n");

}

int main(void) {
    listdir(".");
    return 0;
}
