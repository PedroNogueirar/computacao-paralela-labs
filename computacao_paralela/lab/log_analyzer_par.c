#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

// Pedro Nogueira Ribeiro 10324924

typedef struct {
    long long errors404;
    long long total_bytes;
} Stats;

typedef struct {
    char **linhas;
    long quantidade;
} Parametros;

Stats stats = {0, 0};

pthread_mutex_t mutex;

void *processar(void *parametro) {
    Parametros *parametros = (Parametros *)parametro;
    long long erros = 0;
    long long bytes = 0;

    for (long i = 0; i < parametros->quantidade; i++) {
        char *quote_ptr = strstr(parametros->linhas[i], "\" ");
        if (quote_ptr) {
            int status_code;
            long long bytes_sent;
            if (sscanf(quote_ptr + 2, "%d %lld", &status_code, &bytes_sent) == 2) {
                if (status_code == 404) {
                    erros++;
                } else if (status_code == 200) {
                    bytes += bytes_sent;
                }
            }
        }
    }

    pthread_mutex_lock(&mutex);
    stats.errors404 += erros;
    stats.total_bytes += bytes;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Passe somente o numero de threads como argumento.\n");
        return 1;
    }

    int threads = atoi(argv[1]);

    FILE *fp = fopen("access_log_large.txt", "r");
    if (!fp) {
        perror("Erro ao abrir o arquivo");
        return EXIT_FAILURE;
    }

    long numeroDeLinhas = 0;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1) {
        numeroDeLinhas++;
    }

    rewind(fp);

    char **lines = malloc(numeroDeLinhas * sizeof(char *));

    for (int i = 0; (read = getline(&line, &len, fp)) != -1; i++) {
        lines[i] = malloc(strlen(line) + 1);
        strcpy(lines[i], line);
    }

    fclose(fp);
    if (line) {
        free(line);
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_mutex_init(&mutex, NULL);
    pthread_t pthreads[threads];
    Parametros *parametros = malloc(threads * sizeof(Parametros));
    long quantidadeDeLinhas = numeroDeLinhas / threads;
    for (int i = 0; i < threads; i++) {
        parametros[i].linhas = &lines[i * quantidadeDeLinhas];
        if (i == threads - 1) {
            parametros[i].quantidade = numeroDeLinhas - i * quantidadeDeLinhas;
        } else {
            parametros[i].quantidade = quantidadeDeLinhas;
        }
        pthread_create(&pthreads[i], NULL, processar, &parametros[i]);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(pthreads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Tempo de execução: %.4f segundos\n", time_spent);
    printf("Erros: %lld\n", stats.errors404);
    printf("Bytes: %lld\n", stats.total_bytes);
    printf("Threads: %d\n", threads);

    for (long i = 0; i < numeroDeLinhas; i++) {
        free(lines[i]);
    }
    free(lines);
    free(parametros);

    pthread_mutex_destroy(&mutex);

    return 0;
}