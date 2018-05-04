#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

#include "cmake-build-debug/antidot.h"



int main(void) {

     restructTree(".");

    // rename("TEST\\fichier.txt", "TEST\\T\\fichier.txt");

    /*
    DIR *paies_dir, *dir;
    struct dirent *paies_cursor, *curs;
    char paies_path[1024];


    snprintf(paies_path, sizeof(paies_path), "TEST");

    if ((paies_dir = opendir(paies_path))) {


        while ((paies_cursor = readdir(paies_dir)) != NULL) {

            if ((strchr(paies_cursor->d_name, '.')) == NULL) {

                printf("%s : %s\n", paies_dir->__d_dirname, paies_cursor->d_name);

                snprintf(paies_path, sizeof(paies_path), "TEST\\%s", paies_cursor->d_name);

                if ((dir = opendir(paies_path))) {
                    while ((curs = readdir(dir)) != NULL) {

                            printf("%s - %s \n", paies_cursor->d_name, curs->d_name);

                    }

                    closedir(dir);
                } else
                    printf("%s v\n", paies_path);
            }
        }



        closedir(paies_dir);
    }
    else
        printf("%s z\n", paies_path);
*/
    return 0;

}
