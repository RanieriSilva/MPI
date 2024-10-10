#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_WORD_LENGTH 100
#define MAX_ARTISTS 1000
#define MAX_WORDS 1000000
#define MAX_THREADS 10

typedef struct {
    char artist[100];
    int count;
} ArtistCount;

typedef struct {
    char word[MAX_WORD_LENGTH];
    int count;
} WordCount;

ArtistCount artist_counts[MAX_ARTISTS];
WordCount word_counts[MAX_WORDS];
int artist_count = 0;
int word_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Função para contar palavras
void *count_words(void *arg) {
    char *music = (char *)arg;
    char *word;
    int local_word_counts[MAX_WORDS] = {0};  // Contagem local das palavras
    char *music_copy = strdup(music); // Fazer uma cópia da música para não modificar a original

    word = strtok(music_copy, " ,.-\n");
    while (word != NULL) {
        int i; // Declaração da variável i
        for (i = 0; i < word_count; i++) {
            if (strcmp(word_counts[i].word, word) == 0) {
                local_word_counts[i]++;
                break;
            }
        }

        // Se a palavra não estiver no array, adicioná-la
        if (i == word_count) {
            strcpy(word_counts[word_count].word, word);
            local_word_counts[word_count]++;
            word_count++;
        }

        word = strtok(NULL, " ,.-\n");
    }

    // Atualiza a contagem global
    pthread_mutex_lock(&mutex);
    for (int j = 0; j < word_count; j++) {
        word_counts[j].count += local_word_counts[j];
    }
    pthread_mutex_unlock(&mutex);
    
    free(music_copy); // Liberar a cópia da música
    return NULL;
}

// Função para contar artistas
void count_artists(char *artist) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < artist_count; i++) {
        if (strcmp(artist_counts[i].artist, artist) == 0) {
            artist_counts[i].count++;
            pthread_mutex_unlock(&mutex);
            return;
        }
    }
    strcpy(artist_counts[artist_count].artist, artist);
    artist_counts[artist_count].count = 1;
    artist_count++;
    pthread_mutex_unlock(&mutex);
}

void process_csv(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    char line[1024];
    char artist[100];
    char music[5000];

    // Ignorar o cabeçalho
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%99[^,],%*[^,],%*[^,],%499[^,]", artist, music);
        
        // Contar artistas
        count_artists(artist);

        // Contar palavras na música
        pthread_t thread;
        pthread_create(&thread, NULL, count_words, (void *)music);
        pthread_join(thread, NULL);
    }

    fclose(file);
}

void print_top_words() {
    printf("\nAs 3 palavras mais repetidas:\n");
    
    // Bubble sort para ordenar as palavras com base na contagem
    for (int i = 0; i < word_count - 1; i++) {
        for (int j = 0; j < word_count - i - 1; j++) {
            if (word_counts[j].count < word_counts[j + 1].count) {
                WordCount temp = word_counts[j];
                word_counts[j] = word_counts[j + 1];
                word_counts[j + 1] = temp;
            }
        }
    }

    for (int i = 0; i < 3 && i < word_count; i++) {
        printf("%s: %d\n", word_counts[i].word, word_counts[i].count);
    }
}

void print_top_artists() {
    printf("\nArtistas com mais músicas:\n");
    
    // Bubble sort para ordenar os artistas com base na contagem
    for (int i = 0; i < artist_count - 1; i++) {
        for (int j = 0; j < artist_count - i - 1; j++) {
            if (artist_counts[j].count < artist_counts[j + 1].count) {
                ArtistCount temp = artist_counts[j];
                artist_counts[j] = artist_counts[j + 1];
                artist_counts[j + 1] = temp;
            }
        }
    }

    for (int i = 0; i < 3 && i < artist_count; i++) {
        printf("%s: %d\n", artist_counts[i].artist, artist_counts[i].count);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo CSV>\n", argv[0]);
        return 1;
    }

    process_csv(argv[1]);
    
    print_top_artists(); // Imprimir os artistas com mais músicas
    print_top_words(); // Imprimir as 3 palavras mais repetidas

    return 0;
}