#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int n = 4;
    cria_n_filhos(n);
    return 0;
}

int cria_n_filhos(int n) {
    pid_t pid;
    printf("Processo Pai principal com PID %d", getpid());

    for (int i = 0; i < n; i++) {
        
        pid_t pid = fork();
        
        if (pid < 0) { 
            // Erro
            fprintf(stderr, "Fork falhou!\n");
            return 1;
        } 
        
        else if (pid == 0) { 
            // Processo filho
            printf("Eu sou o filho %d! Meu PID é %d, meu pai é %d.\n", i + 1, getpid(), getppid());
            exit(0); 
        }
    }

    for (int z = 0; z < n; z++){
        wait(NULL);
    }
    return 0;
}
