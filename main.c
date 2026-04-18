#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MEX_MAX 1024 * 1024
#define ALIGN_MAX _Alignof(max_align_t)

static _Alignas(max_align_t) char mem[MEX_MAX];
size_t mem_size = sizeof(mem) / sizeof(mem[0]);
bool premier_block = false; // premier bloc innitialisé ou non
bool premier_free = false;  // premier free effectué ou non
int nb_free = 0;
int nb_pointeurs = 0;

// char *curseur = mem;

typedef struct s_block
{
    size_t taille;
    struct s_block *suivant;
    int libre; // Libre = 1, occupe = 0
} t_block;

typedef struct s_pointeurs
{
    char nom[64];
    void *ptr;
} t_pointeur;

t_block *
init_block(size_t taille)
{
    t_block *bloc = (t_block *)mem;
    bloc->taille = taille;
    bloc->suivant = NULL;
    bloc->libre = 0;

    return bloc;
}

t_block *curseur = NULL;
t_block *free_block = NULL;

int aligner(size_t taille)
{

    return ((taille % ALIGN_MAX == 0 ? taille : taille + ALIGN_MAX - (taille % ALIGN_MAX)));
}

static void *static_malloc(size_t taille)
{
    t_block *alloue = NULL;
    t_block *bloc = NULL;
    t_block *temp = NULL;
    size_t nb_octets = 0;
    t_block *premier_block_init = (t_block *)mem;
    t_block *free_block_head = NULL;

    // Checker si on a déjà un premier bloc
    if (!(premier_block))
    {
        bloc = premier_block_init;
        // curseur = premier_block_init;
        bloc->taille = taille;
        bloc->libre = 0;
        bloc->suivant = (t_block *)((char *)bloc + aligner(sizeof(t_block)) + (aligner(taille)));
        curseur = bloc->suivant;
        alloue = (t_block *)((char *)bloc + (aligner(sizeof(t_block))));
        premier_block = true;
    }

    else
    {
        if (free_block != NULL) // Si il y'a des blocs libres dispos
        {
            free_block_head = free_block;
            if (taille <= free_block_head->taille) // Si la taille correspond et qu'on peut l'allouer
            {
                bloc = free_block;
                bloc->taille = taille;
                if (bloc == NULL)
                {
                    fprintf(stderr, "IL n'y a plus d'espace disponible \n");
                    return NULL;
                }
                alloue = (t_block *)((char *)bloc + aligner(sizeof(t_block)));
                bloc->suivant = (t_block *)((char *)bloc + aligner(sizeof(t_block)) + (aligner(taille)));
                free_block = free_block->suivant;
            }

            else // Si taille supérieure
            {
                temp = free_block;
                bloc = free_block;
                free_block_head = (t_block *)((char *)bloc - aligner(sizeof(t_block)));
                while ((bloc != NULL) && (free_block_head->taille >= taille))
                {
                    temp = free_block->suivant;
                    bloc = temp;
                    free_block_head = (t_block *)((char *)bloc - aligner(sizeof(t_block)));
                }

                if (bloc == NULL) // ON a rien trouvé
                {
                    bloc = curseur;
                    alloue = bloc;
                }

                else // On a trouvé un bloc qui correspond
                {
                    free_block = free_block->suivant;
                    alloue = (t_block *)((char *)bloc + aligner(sizeof(t_block)));
                    bloc->taille = taille;
                }

                if (bloc == NULL)
                {
                    fprintf(stderr, "IL n'y a plus d'espace disponible \n");
                    return NULL;
                }
            }
        }

        else // Si plus de blocs libres dispos on reprend depuis le curseur
        {
            bloc = curseur;
            bloc->suivant = (t_block *)((char *)bloc + aligner(sizeof(t_block)) + (aligner(taille)));
            bloc->taille = taille;
            curseur = bloc->suivant;
            alloue = (t_block *)((char *)bloc + (aligner(sizeof(t_block))));
        }

        if ((t_block *)((char *)bloc + (sizeof(t_block)) + aligner(taille)) > (t_block *)((char *)mem + MEX_MAX))
        {
            fprintf(stderr, "IL n'y a plus d'espace disponible \n");
            return NULL;
            t_block *curseur = NULL;
        }

        // bloc->suivant = (t_block *)((char *)bloc + aligner(sizeof(t_block)) + aligner(taille)); // alloue = (t_block *)((char *)bloc + (aligner(sizeof(t_block))));
    }
    // Checker si libre = 0

    return alloue;
}

void static_free(void *ptr)
{
    void *temp = NULL;

    if (!premier_free)
    {
        free_block = (t_block *)((char *)ptr - aligner(sizeof(t_block)));
        free_block->suivant = NULL;
        nb_pointeurs--;
        premier_free = true;
    }

    else
    {
        temp = free_block;
        free_block = ptr;
        free_block->suivant = temp;
        nb_pointeurs--;
    }

    if (nb_pointeurs == 0)
        premier_free = false;
}

int verifier_nombre(int nombre)
{
    while (scanf("%d", &nombre) != 1)
    {
        while (getchar() != '\n')
            ;
        printf("Veuillez rentrer un seul nombre valide : ");
    }

    return nombre;
}

char *verifier_nom(char *s)
{
    while (fgets(s, sizeof(s), stdin) != NULL)
    {
        if (sscanf(s, "%63s", s) == 1)
            break;
        printf("Veuillez rentrer une chaine valide : ");
    }

    return s;
}

int main()
{
    int octets = 0;
    int choix = 0;
    char buffer[64];
    t_pointeur pointeurs[100]; // Tableau de 100 pointeurs max

    printf("adresse de début du tableau : %p \n", &mem);

    do
    {
        printf("Choissiez 0 pour arreter, 1 pour malloc ou 2 pour free. \n");
        choix = verifier_nombre(choix);
        switch (choix)
        {
        case 0:
            return EXIT_SUCCESS;
        case 1:
            printf("Nom du pointeur que vous voulez allouer : ");
            verifier_nom(buffer);
            sscanf(buffer, "%63s", pointeurs[nb_pointeurs].nom);

            printf("Combien de mémoire voulez vous allouer ? ");
            octets = verifier_nombre(octets);
            pointeurs[nb_pointeurs].ptr = static_malloc(octets);
            printf("\n");
            printf("Allouée : %p \n", pointeurs[nb_pointeurs].ptr);
            nb_pointeurs++;

            break;

        case 2:
            printf("Quel pointeur voulez vous  libérer ? ");
            verifier_nom(buffer);
            for (int i = 0; i < nb_pointeurs; i++)
            {
                if (strcmp(pointeurs[i].nom, buffer) == 0)
                {
                    static_free(pointeurs[i].ptr);
                    break;
                }
            }
            break;
        }

    } while (choix != 0);
}