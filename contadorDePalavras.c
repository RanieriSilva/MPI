#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_LENGTH 100
#define MAX_LINE_LENGTH 1000
#define MAX_WORDS 100000

// Estrutura para armazenar as palavras e suas contagens
typedef struct {
    char word[MAX_WORD_LENGTH];
    int count;
} WordCount;

// Função para tokenizar e contar as palavras na letra
void count_words(WordCount *word_counts, int *word_count, char *lyrics) {
    char *token = strtok(lyrics, " ,.-\n");  // Divide a string em palavras
    while (token != NULL) {
        // Verifica se a palavra já existe na lista de contagens
        int found = 0;
        for (int i = 0; i < *word_count; i++) {
            if (strcmp(word_counts[i].word, token) == 0) {
                word_counts[i].count++;
                found = 1;
                break;
            }
        }
        // Se a palavra não foi encontrada, adiciona à lista
        if (!found && *word_count < MAX_WORDS) {
            strcpy(word_counts[*word_count].word, token);
            word_counts[*word_count].count = 1;
            (*word_count)++;
        }
        // Pega o próximo token (palavra)
        token = strtok(NULL, " ,.-\n");
    }
}

// Função para comparar duas palavras pela contagem
int compare_word_counts(const void *a, const void *b) {
    WordCount *wordA = (WordCount *)a;
    WordCount *wordB = (WordCount *)b;
    return wordB->count - wordA->count;  // Ordem decrescente
}

// Função principal
int main() {
    FILE *file = fopen("musicas.csv", "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    WordCount word_counts[MAX_WORDS];
    int word_count = 0;

    // Pular a primeira linha (cabeçalho)
    fgets(line, MAX_LINE_LENGTH, file);

    // Ler o arquivo linha por linha
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        char *lyrics = strtok(line, ",");  // Ignorar as três primeiras colunas
        for (int i = 0; i < 3; i++) {
            lyrics = strtok(NULL, ",");
        }
        // Agora 'lyrics' contém o conteúdo da quarta coluna (letras)
        if (lyrics != NULL) {
            count_words(word_counts, &word_count, lyrics);
        }
    }

    // Classifica as palavras pela contagem
    qsort(word_counts, word_count, sizeof(WordCount), compare_word_counts);

    // Imprime as 3 palavras mais repetidas
    printf("As 3 palavras mais repetidas são:\n");
    for (int i = 0; i < 3 && i < word_count; i++) {
        printf("%s: %d vezes\n", word_counts[i].word, word_counts[i].count);
    }

    fclose(file);
    return 0;
}
