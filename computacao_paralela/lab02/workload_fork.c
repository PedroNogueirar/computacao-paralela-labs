#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>

#define VECTOR_SIZE 200000000
#define NUM_PROCESSES 4 // Define o numero de processos a serem criados

// Funcao que simula uma carga de trabalho pesada
void heavy_work(double* vector, int start, int end) {
    for (int i = start; i < end; ++i) {
        vector[i] = sin(vector[i]) * cos(vector[i]) + sqrt(vector[i]);
    }
}

int main() {
    double* vector = (double*)malloc(VECTOR_SIZE * sizeof(double));
    for(int i = 0; i < VECTOR_SIZE; i++) vector[i] = (double)i;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // --- Início da Lógica de Paralelização ---

    pid_t pid;
    int chunk_size = VECTOR_SIZE / NUM_PROCESSES;

    for (int i = 0; i < NUM_PROCESSES; i++) {
        pid = fork();

        if (pid < 0) { // Erro
            fprintf(stderr, "Fork falhou!\n");
            return 1;
        } else if (pid == 0) { // Codigo do Processo Filho
            // 1. Calcular a fatia de trabalho
            int start_index = i * chunk_size;
            int end_index = (i == NUM_PROCESSES - 1) ? VECTOR_SIZE : (i + 1) * chunk_size;

            // 2. Executar o trabalho na sua fatia
            heavy_work(vector, start_index, end_index);

            // 3. Terminar para nao continuar executando o codigo do pai
            exit(0);
        }
    }

    // --- Codigo do Processo Pai ---
    // O pai espera que TODOS os filhos terminem
    for (int i = 0; i < NUM_PROCESSES; i++) {
        wait(NULL);
    }

    // --- Fim da Lógica de Paralelização ---

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_spent = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Versao paralela com %d processos executou em %f segundos\n", NUM_PROCESSES, time_spent);
    printf (" Resultado de verificacao : vector [10] = %f\n", vector [10]) ;
    
    // Note que o vetor no processo pai nao foi modificado,
    // pois cada filho trabalhou em uma COPIA da memoria.

    free(vector);
    return 0;
}