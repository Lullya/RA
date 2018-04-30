#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

void listdir(const char *name, int indent)
{
    regex_t regex;
    int reti;
    char msgbuf[100];

    reti = regcomp(&regex, "26", 0);
    if( reti ){ fprintf(stderr, "Could not compile regex\n"); exit(1); }


    DIR *de;
    struct dirent *entry;

    if (!(de = opendir(name)))
        return;

    while ((entry = readdir(de)) != NULL) {
        if ((strchr(entry->d_name, '.')) == NULL) {
            char path[1024];
            reti = regexec(&regex, entry->d_name, 0, NULL, 0);
            if(!reti){
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;
                snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
                printf("%*s[%s]\n", indent, "", entry->d_name);
                listdir(path, indent + 2);
            }
            else if( reti != REG_NOMATCH ){
                regerror(reti, &regex, msgbuf, sizeof(msgbuf));
                fprintf(stderr, "Regex match failed: %s\n", msgbuf);
                exit(1);
            }
        }
        else
            printf("%*s- %s\n", indent, "", entry->d_name);
    }
    closedir(de);

    regfree(&regex);
}

int main(void) {
    listdir(".", 0);
    return 0;
}
