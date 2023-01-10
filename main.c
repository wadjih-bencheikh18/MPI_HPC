#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MASTER 0
#define DATASIZE 15
#define CHUNKSIZE 3

// tags
#define INIT_REQUEST 0
#define DATA_REQUEST 1
#define END 2

int main(int argc, char **argv)
{
    int rang, size, valeur[CHUNKSIZE];
    int *end_msg = NULL;

    if ((DATASIZE % CHUNKSIZE) != 0)
    {
        printf("DATASIZE n'est pas divisible par CHUNKSIZE\n");
        MPI_Finalize();
        exit(-1);
    }

    // MPI variables
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rang);

    if (size < 2)
    {
        printf("size doit etre superieur a 1\n");
        exit(-1);
    }

    if (rang == MASTER)
    {
        int index = 0, end_proc = 0;
        int array[DATASIZE];
        int indexes[size];

        // initialiser le tableau

        printf("\n\nInitialisation **********************************\n\n");

        for (int i = 0; i < DATASIZE; i++)
            array[i] = rand() % 9 + 1;

        for (int i = 0; i < DATASIZE; i++)
        {
            printf("| %d \t  ", array[i]);
            if ((i + 1) % CHUNKSIZE == 0)
                printf("\n");
        }
        printf("\n\n**************************************************\n\n");

        while (end_proc < size - 1)
        {
            MPI_Recv(&valeur, CHUNKSIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == INIT_REQUEST)
            {
                if (index == DATASIZE)
                {
                    printf("[MAITRE] Arret de processus %d\n", status.MPI_SOURCE);
                    MPI_Send(end_msg, 0, MPI_INT, status.MPI_SOURCE, END, MPI_COMM_WORLD);
                    end_proc++;
                }
                else
                {
                    for (int j = 0; j < CHUNKSIZE; j++)
                    {
                        printf("[MAITRE] Envoi de valeur : %d au processus %d\n", array[index + j], status.MPI_SOURCE);
                    }
                    printf("--------------------\n");
                    for (int k = 0; k < CHUNKSIZE; k++)
                    {
                        valeur[k] = array[index + k];
                    }
                    MPI_Send(&valeur, CHUNKSIZE, MPI_INT, status.MPI_SOURCE, INIT_REQUEST, MPI_COMM_WORLD);
                    indexes[status.MPI_SOURCE] = index;
                    index += CHUNKSIZE;
                }
            }
            else if (status.MPI_TAG == DATA_REQUEST)
            {
                for (int i = 0; i < CHUNKSIZE; i++)
                {
                    array[indexes[status.MPI_SOURCE] + i] = valeur[i];
                    printf("[MAITRE] Reception de Value : %d de processus %d\n", valeur[i], status.MPI_SOURCE);
                }
                printf("--------------------\n");
                if (index == DATASIZE)
                {
                    printf("[MAITRE] Arret de processus %d\n", status.MPI_SOURCE);
                    MPI_Send(end_msg, 0, MPI_INT, status.MPI_SOURCE, END, MPI_COMM_WORLD);
                    end_proc++;
                }
                else
                {
                    for (int j = 0; j < CHUNKSIZE; j++)
                    {
                        printf("[MAITRE] Envoi de valeur : %d au processus %d\n", array[index + j], status.MPI_SOURCE);
                    }
                    printf("--------------------\n");
                    for (int k = 0; k < CHUNKSIZE; k++)
                    {
                        valeur[k] = array[index + k];
                    }

                    MPI_Send(&valeur, CHUNKSIZE, MPI_INT, status.MPI_SOURCE, INIT_REQUEST, MPI_COMM_WORLD);
                    indexes[status.MPI_SOURCE] = index;
                    index += CHUNKSIZE;
                }
            }
            // wait all salves to finish
            if (end_proc == size - 1)
            {
                for (int i = 1; i < size; i++)
                {
                    MPI_Send(end_msg, 0, MPI_INT, i, END, MPI_COMM_WORLD);
                }
            }
            // Affichage du tableau
            printf("\n\nResult **********************************************\n\n");
            for (int i = 0; i < DATASIZE; i++)
            {
                printf("| %d \t  ", array[i]);
                if ((i + 1) % CHUNKSIZE == 0)
                    printf("\n");
            }
            printf("\n\n**************************************************\n\n");
        }
    }
    else
    {
        printf("[ESCLAVE] Processus %d : Envoi de premier requete \n", rang);
        MPI_Send(&valeur, CHUNKSIZE, MPI_INT, MASTER, INIT_REQUEST, MPI_COMM_WORLD);
        MPI_Recv(&valeur, CHUNKSIZE, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        while (status.MPI_TAG != END)
        {
            for (int j = 0; j < CHUNKSIZE; j++)
            {
                printf("[ESCLAVE] Processus %d : Reception de valeur %d\n", rang, valeur[j]);
            }
            printf("--------------------\n");
            for (int j = 0; j < CHUNKSIZE; j++)
            {
                valeur[j] = valeur[j] * valeur[j];
            }
            MPI_Send(&valeur, CHUNKSIZE, MPI_INT, MASTER, DATA_REQUEST, MPI_COMM_WORLD);
            for (int j = 0; j < CHUNKSIZE; j++)
            {
                printf("[ESCLAVE] Processus %d : L'envoi de valeur %d\n", rang, valeur[j]);
            }
            printf("--------------------\n");
            MPI_Recv(&valeur, CHUNKSIZE, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG != END)
            {
                for (int j = 0; j < CHUNKSIZE; j++)
                {
                    printf("[ESCLAVE] Processus %d : Reception de valeur %d\n", rang, valeur[j]);
                }
                printf("--------------------\n");
            }
        };
        printf("[ESCLAVE] Processus %d : Terminer.\n", rang);
        fflush(stdout);
    }
    MPI_Finalize();
    return 0;
}