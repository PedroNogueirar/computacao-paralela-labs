#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define VECTOR_SIZE 1000
#define NUM_PROCESSES 4

// Funcao que calcula a soma de uma fatia de um vetor
long long sum_chunk(int* vector, int start, int end) {
    long long sum = 0;
    for (int i = start; i < end; i++) {
        sum += vector[i];
    }
    return sum;
}

int main() {
    int vector[VECTOR_SIZE];
    long long total_sum_expected = 0;

    // Inicializa o vetor e calcula a soma esperada para verificacao
    for (int i = 0; i < VECTOR_SIZE; i++) {
        vector[i] = i + 1;
        total_sum_expected += vector[i];
    }

    int pipes[NUM_PROCESSES][2]; // Array de pipes, um para cada filho
    pid_t pid;
    int chunk_size = VECTOR_SIZE / NUM_PROCESSES;

    // --- Laco de Criacao de Processos e Pipes ---
    for (int i = 0; i < NUM_PROCESSES; i++) {
        // 1. Cria um pipe para o filho 'i'
        if (pipe(pipes[i]) == -1) {
            fprintf(stderr, "Falha ao criar o pipe para o filho %d!\n", i);
            return 1;
        }

        pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Fork falhou!\n");
            return 1;
        }

        if (pid == 0) { // --- Codigo do Processo Filho 'i' ---
            // O filho nao vai ler do seu pipe, entao fecha a ponta de leitura.
            close(pipes[i][0]);
            
            // O filho tambem fecha as pontas dos pipes dos outros irmaos que herdou
            for (int j = 0; j < NUM_PROCESSES; j++) {
                if (i != j) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }

            int start = i * chunk_size;
            int end = (i == NUM_PROCESSES - 1) ? VECTOR_SIZE : (i + 1) * chunk_size;

            long long partial_sum = sum_chunk(vector, start, end);
            printf("[FILHO %d] Soma parcial calculada: %lld. Enviando...\n", getpid(), partial_sum);

            // Escreve o resultado no seu pipe
            write(pipes[i][1], &partial_sum, sizeof(long long));

            // Fecha a ponta de escrita e termina.
            close(pipes[i][1]);
            exit(0);
        }
    }

    // --- Codigo do Processo Pai ---
    long long final_sum = 0;

    for (int i = 0; i < NUM_PROCESSES; i++) {
        // O pai nao vai escrever em nenhum pipe, entao fecha todas as pontas de escrita.
        close(pipes[i][1]);

        long long partial_sum_from_child;
        // Le o resultado do pipe do filho 'i'
        read(pipes[i][0], &partial_sum_from_child, sizeof(long long));
        
        printf("[PAI] Recebido %lld do filho do pipe %d.\n", partial_sum_from_child, i);
        final_sum += partial_sum_from_child;

        // Fecha a ponta de leitura apos o uso.
        close(pipes[i][0]);
    }

    // Espera todos os filhos terminarem
    for (int i = 0; i < NUM_PROCESSES; i++) {
        wait(NULL);
    }
    
    printf("\n----------------------------------------\n");
    printf("Soma final calculada pelo pai: %lld\n", final_sum);
    printf("Soma esperada: %lld\n", total_sum_expected);
    printf("----------------------------------------\n");

    return 0;
}