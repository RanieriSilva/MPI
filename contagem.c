#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_WORD_LENGTH 100
#define MAX_ARTISTS 1000
#define MAX_BUFFER 1024

typedef struct {
    char artist[MAX_WORD_LENGTH];
    int count;
} ArtistCount;

// Função para contar palavras
void count_words(char *music, int *word_counts) {
    char *token;
    const char *delim = " \n\t.,;:!?'\"()";

    token = strtok(music, delim);
    while (token != NULL) {
        // Incrementar contagem da palavra
        int index = atoi(token) % 100;  // Ajuste o cálculo do índice conforme necessário
        word_counts[index]++;
        token = strtok(NULL, delim);
    }
}

// Função para contar artistas
void count_artists(ArtistCount *artist_counts, int *artist_count, char *artist) {
    for (int i = 0; i < *artist_count; i++) {
        if (strcmp(artist_counts[i].artist, artist) == 0) {
            artist_counts[i].count++;
            return;
        }
    }
    strcpy(artist_counts[*artist_count].artist, artist);
    artist_counts[*artist_count].count = 1;
    (*artist_count)++;
}

// Função principal
int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Verifica se o arquivo CSV foi fornecido como argumento
    if (argc < 2) {
        if (rank == 0) {
            printf("Uso: %s <arquivo CSV>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    FILE *file;
    if (rank == 0) {
        // O processo 0 abre o arquivo CSV
        file = fopen(argv[1], "r");
        if (file == NULL) {
            perror("Não foi possível abrir o arquivo");
            MPI_Finalize();
            return 1;
        }
    }

    // Broadcast do ponteiro do arquivo para todos os processos
    MPI_Bcast(&file, sizeof(file), MPI_BYTE, 0, MPI_COMM_WORLD);

    // Lê o arquivo linha por linha
    char line[MAX_BUFFER];
    ArtistCount artist_counts[MAX_ARTISTS];
    int artist_count = 0;
    int word_counts[100] = {0};

    // Ignora o cabeçalho
    if (rank == 0) {
        fgets(line, sizeof(line), file);
    }

    // Distribui linhas do CSV entre os processos
    while (fgets(line, sizeof(line), file)) {
        char *artist = strtok(line, ",");
        strtok(NULL, ","); // Ignorar a coluna da música
        strtok(NULL, ","); // Ignorar o link
        char *music = strtok(NULL, ","); // A letra da música

        if (music != NULL) {
            count_words(music, word_counts);
            count_artists(artist_counts, &artist_count, artist);
        }
    }

    fclose(file);

    // Reduz os resultados
    ArtistCount global_artist_counts[MAX_ARTISTS];
    int global_artist_count = 0;
    MPI_Reduce(artist_counts, global_artist_counts, MAX_ARTISTS * sizeof(ArtistCount), MPI_BYTE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&artist_count, &global_artist_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Exibe resultados no processo 0
    if (rank == 0) {
        printf("\nArtistas e suas contagens:\n");
        for (int i = 0; i < global_artist_count; i++) {
            printf("%s: %d\n", global_artist_counts[i].artist, global_artist_counts[i].count);
        }

        printf("\nContagem de palavras:\n");
        for (int i = 0; i < 100; i++) {
            if (word_counts[i] > 0) {
                printf("Palavra %d: %d\n", i, word_counts[i]); // Ajuste conforme necessário
            }
        }
    }

    MPI_Finalize();
    return 0;
}
