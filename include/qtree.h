#include <g2x.h>
#include <utils.h>

typedef struct _node
{
    ushort moy; /* pixel ou valeur différentielle [-255,+255] ramenée à [0,510] */
    uchar err; /* erreur par rapport à la moyenne - sur 2 bits */
    uchar uni; /* bloc uniforme ou pas - sur 1 bit */
    double var; /* variance */
    struct _node* next; /* qnode ’1° fils’ dans le niveau suivant */
} qnode;
typedef struct
{
    qnode** map; /* pointeur d’accès aux niveaux intermédiaires [0...depth] */
    int depth; /* hauteur de l’arbre - (depth+1) ’map’ [0...depth] */
} qtree;