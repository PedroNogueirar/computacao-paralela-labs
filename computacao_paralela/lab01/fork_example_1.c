# include <stdio.h>
# include <unistd.h>
# include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if(pid < 0) {
        fprintf(stderr , " Fork falhou !\n");
        return 1;
    }
    else if (pid == 0) {
        printf("Eu sou o filho!\n");
    } 
    else {
        printf("Eu sou o pai!\n");
        wait(NULL);
        printf("Meu filho terminou , agora eu posso terminar.\n");
    }
    return 0;
}