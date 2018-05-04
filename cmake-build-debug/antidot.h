#ifndef REALANTIDOT_ANTIDOT_H
#define REALANTIDOT_ANTIDOT_H

#endif //REALANTIDOT_ANTIDOT_H

/**
 * Algorithme principal de restructuration de la GED.
 * Globalement, cet algorithme va rechercher dans chaque dossier client (260001, 260002, 840001, 840002, etc.)
 * les fichiers contenus dans ISAPAYE (exemple : 260001/2016/ISAPAYE/fichier-2016.pdf)
 * et le déplacer dans un dossier PAIES (exemple : 260001/PAIES/2016/fichier-2016.pdf).
 * Enfin, puisque TOUT le contenu du dossier ISAPAYE est déplacé, celui-ci est vide et donc supprimé.
 * Si le dossier parent (2016) ne contenait uniquement le dossier ISAPAYE, celui-ci aussi devient vide et supprimé aussi.
 *
 * L'algorithme se compose en 3 étapes :
 * checkPAIES, moveFiles, postCorrectionPAIES.
 *
 * @param client : numéro de dossier client (exemple : 260000)
 * on passe à ce paramètre le dossier dans lequel on veut effectuer notre restructuration.
 */
void restructTree(const char *client);

/**
 * On vérifie l'existance d'un dossier PAIES. S'il existe et qu'il contient des fichiers, on les déplace dans un
 * sous-dossier année (exemple : 260001/PAIES/fichier-2016.pdf -> 260001/PAIES/2016/fichier-2016.pdf).
 * S'il n'existe pas on le crée et il n'y a pas de traitement à faire puisqu'il est vide.
 * @param client : numéro de dossier client (exemple : 260000)
 * @param ret : variable retournant 0 lorsqu'on a réussit à compiler une expression régulière.
 */
void checkPAIES(const char *client, int ret);

/**
 * A ce stade il existe un dossier PAIES (vide ou avec des dossiers années).
 * On déplace les fichiers présents dans ISAPAYE vers PAIES
 * (exemple : 260001/2016/ISAPAYE/fichier-2016/pdf -> 260001/PAIES/2016/fichier-2016.pdf).
 * @param client : numéro de dossier client (exemple : 260000)
 * @param ret : variable retournant 0 lorsqu'on a réussit à compiler une expression régulière.
 * @param client_dir : répértoire client
 * @param client_cursor : curseur qui parcourt les éléments du répértoire client
 */
void moveFiles(const char *client, int ret, DIR *client_dir, struct dirent *client_cursor);

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
void postCorrectionPAIES(const char *client, int ret);