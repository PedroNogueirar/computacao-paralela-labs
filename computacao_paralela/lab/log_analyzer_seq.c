#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

// Pedro Nogueira Ribeiro 10324924

int main(int argc, char *argv[]) {
    long erros = 0;
    long bytes = 0;

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

    for (long i = 0; i < numeroDeLinhas; i++) {
        char *quote_ptr = strstr(lines[i], "\" ");
        if (quote_ptr) {
            int status_code;
            long long bytes_sent;
            if (sscanf(quote_ptr + 2, "%d %lld", &status_code, &bytes_sent) == 2) {
                if (status_code == 404)
                    erros++;
                else if (status_code == 200)
                    bytes += bytes_sent;
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Tempo de execução: %.4f segundos\n", time_spent);
    printf("Erros: %ld\n", erros);
    printf("Bytes: %ld\n", bytes);

    return 0;
}