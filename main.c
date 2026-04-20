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
    t_block *free_block_tail = NULL;
    t_block *next_free_block = NULL;
    size_t head_taille = 0;
    bool bloc_trouve = false;

    // Checker si on a déjà un premier bloc
    if (!(premier_block))
    {
        bloc = premier_block_init;
        // curseur = premier_block_init;
        bloc->taille = taille;
        bloc->libre = 0;
        bloc->suivant = (t_block *)((char *)bloc + aligner(sizeof(t_block)) + (aligner(taille)));
        curseur = bloc->suivant;
        if ((char *)curseur > (char *)mem + MEX_MAX)
        {
            fprintf(stderr, "plus d'espace disponible\n");
            return NULL;
        }
        alloue = (t_block *)((char *)bloc + (aligner(sizeof(t_block))));
        premier_block = true;
    }

    else // SI il y'a eu une première allocation effectuée
    {
        if (free_block != NULL) // Si il y'a des blocs libres dispos
        {
            free_block_head = free_block;
            free_block_tail = free_block;
            head_taille = free_block_tail->taille;
            next_free_block = free_block_tail->suivant;

            if (taille <= free_block_head->taille) // Si la taille correspond et qu'on peut l'allouer
            {
                free_block_head->taille = taille;
                alloue = (t_block *)((char *)free_block_head + aligner(sizeof(t_block)));
                free_block = free_block->suivant;
            }

            else // Si taille supérieure
            {
                while (free_block_tail != NULL)
                {
                    if (((char *)free_block_tail + aligner(sizeof(t_block)) + aligner(head_taille)) == (char *)next_free_block)
                    {
                        if (taille >= head_taille)
                        {
                            alloue = (t_block *)((char *)free_block_head + (aligner(sizeof(t_block))));
                            free_block_head->taille = taille;
                            free_block = next_free_block;
                            break;
                        }
                    }

                    free_block_tail = next_free_block;
                    next_free_block = free_block_tail->suivant;
                    head_taille += free_block_tail->taille;
                }

                bloc = curseur;
                bloc->suivant = (t_block *)((char *)bloc + aligner(sizeof(t_block)) + (aligner(taille)));
                bloc->taille = taille;
                curseur = bloc->suivant;
                if ((char *)curseur > (char *)mem + MEX_MAX)
                {
                    fprintf(stderr, "plus d'espace disponible\n");
                    return NULL;
                }
                alloue = (t_block *)((char *)bloc + (aligner(sizeof(t_block))));
            }

            if (bloc == NULL)
            {
                fprintf(stderr, "IL n'y a plus d'espace disponible \n");
                return NULL;
            }
        }

        else // Si plus de blocs libres dispos on reprend depuis le curseur
        {
            bloc = curseur;
            bloc->suivant = (t_block *)((char *)bloc + aligner(sizeof(t_block)) + (aligner(taille)));
            bloc->taille = taille;
            curseur = bloc->suivant;
            alloue = (t_block *)((char *)bloc + (aligner(sizeof(t_block))));
            if ((char *)alloue > ((char *)mem + MEX_MAX))
            {
                fprintf(stderr, "IL n'y a plus d'espace disponible \n");
                return NULL;
            }
        }
    }

    return alloue;
}

void static_free(void *ptr)
{

    t_block *temp = NULL;
    t_block *bloc = (t_block *)((char *)ptr - aligner(sizeof(t_block)));

    if (!premier_free)
    {

        free_block = bloc;
        free_block->suivant = NULL;
        nb_pointeurs--;
        premier_free = true;
    }

    else // Si un premier free a déjà eu lieu
    {
        temp = (t_block *)free_block;
        while (temp != NULL)
        {
            if (((char *)ptr > (char *)temp) && ((char *)ptr) < (char *)temp->suivant)
            {
                temp = temp->suivant;
                temp->suivant = ptr;
                bloc->suivant = temp;
                break;
            }

            else if ((char *)ptr < (char *)free_block)
            {
                free_block = ptr;
                free_block->suivant = temp;
                break;
            }

            else
            {
                temp->suivant = ptr;
            }

            temp = temp->suivant;
        }

        free_block = ptr;
        free_block->suivant = temp;
    }

    if (nb_pointeurs == 0)
    {
        premier_free = false;
        premier_block = false;
    }
}

int verifier_nombre(void)
{
    int nombre = 0;
    char buffer[64];
    int valide = 1;

    while (1)
    {
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0; // supprime \n

        for (int i = 0; buffer[i] != '\0'; i++)
        {
            if (!isdigit(buffer[i]))
            {
                valide = 0;
                break;
            }
        }

        if ((valide) && (sscanf(buffer, "%d", &nombre) == 1))
            return nombre;

        printf("Veuillez rentrer un seul nombre valide : ");
    }
}

char *verifier_nom(char *s)
{
    char buffer[64];
    int valide = 1;

    while (1)
    {
        valide = 1;
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        for (int i = 0; buffer[i] != '\0'; i++)
        {
            if (!isalnum(buffer[i]))
            {
                valide = 0;
                break;
            }
        }
        if (valide && sscanf(buffer, "%63s", s) == 1)
            return s;
        printf("Veuillez rentrer un nom valide (lettres uniquement) : ");
    }
}

int main()
{
    int octets = 0;
    int choix = 0;
    char nom_ptr[64];
    char buffer[64];
    t_pointeur pointeurs[100]; // Tableau de 100 pointeurs max

    printf("adresse de début du tableau : %p \n", &mem);

    do
    {
        printf("Choissiez 0 pour arreter, 1 pour malloc ou 2 pour free. \n");
        choix = verifier_nombre();
        switch (choix)
        {
        case 0:
            return EXIT_SUCCESS;
        case 1:
            printf("Nom du pointeur que vous voulez allouer : ");
            verifier_nom(buffer);
            for (int i = 0; i < nb_pointeurs; i++)
            {
                if (pointeurs[i].ptr = NULL)
                {
                    sscanf(buffer, "%63s", pointeurs[nb_pointeurs].nom);
                    break;
                }
            }
            printf("Combien de mémoire voulez vous allouer ? ");
            octets = verifier_nombre();
            pointeurs[nb_pointeurs].ptr = static_malloc(octets);
            printf("\n");
            printf("Allouée : %p \n", pointeurs[nb_pointeurs].ptr);
            nb_pointeurs++;

            break;

        case 2:
            printf("Quel pointeur voulez vous  libérer ? ");
            verifier_nom(buffer);
            for (int i = 0; pointeurs[i].ptr != NULL; i++)
            {
                if (strcmp(pointeurs[i].nom, buffer) == 0)
                {
                    static_free(pointeurs[i].ptr);
                    for (int j = i; j < nb_pointeurs - 1; j++)
                        pointeurs[j] = pointeurs[j + 1];

                    memset(pointeurs[nb_pointeurs - 1].nom, 0, sizeof(pointeurs[nb_pointeurs - 1].nom));
                    pointeurs[nb_pointeurs - 1].ptr = NULL;
                    nb_pointeurs--;
                    break;
                }
            }
            break;
        }

    } while (choix != 0);
}