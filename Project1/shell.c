#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

int timer_set = 0;
bool background = false;
char *custom_builtins[] = {"cd", "pwd", "echo", "export", NULL};

void set_timer(int seconds) {
    timer_set = 1;
    alarm(seconds);
}

void cancel_timer() {
    timer_set = 0;
    alarm(0);
}

void handle_SIGINT(int signum) {
    if (!background) {
        printf("\n");
        char current_dir[MAX_COMMAND_LINE_LEN];
        if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
            printf("%s> ", current_dir);
        } else {
            perror("getcwd");
        }
        fflush(stdout);
    }
}

void handle_SIGALARM(int signum) {
    if (timer_set) {
        printf("\nTimer has expired. Terminating process.\n");
        kill(0, SIGINT);
    }
}

void execute_command(char **args) {
    pid_t pid = fork();
    if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        if (!background) {
            set_timer(10);
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
            cancel_timer();
        }
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }
}

bool handle_builtins(char **args) {
    for (int i = 0; custom_builtins[i] != NULL; i++) {
        if (strcmp(args[0], custom_builtins[i]) == 0) {
            if (strcmp(args[0], "cd") == 0) {
                if (args[1] == NULL) {
                    chdir(getenv("HOME"));
                } else {
                    if (chdir(args[1]) == -1) {
                        perror("chdir");
                    }
                }
            } else if (strcmp(args[0], "pwd") == 0) {
                char current_dir[MAX_COMMAND_LINE_LEN];
                if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
                    printf("%s\n", current_dir);
                } else {
                    perror("getcwd");
                }
            } else if (strcmp(args[0], "echo") == 0) {
                for (int j = 1; args[j] != NULL; j++) {
                    printf("%s ", args[j]);
                }
                printf("\n");
            } else if (strcmp(args[0], "export") == 0) {
                if (args[1] != NULL) {
                    if (putenv(args[1]) != 0) {
                        perror("putenv");
                    }
                }
            }
            return true;
        }
    }
    return false;
}

void execute_commands_with_pipes(char **args1, char **args2) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid1 = fork();

    if (pid1 == 0){
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        execvp(args1[0], args1);
        perror("execvp (command 1)");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();

    if(pid2 == 0){
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        execvp(args2[0], args2);
        perror("execvp (command 2)");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

bool handle_pipe(char **args){
    char *token;
    char *args1[MAX_COMMAND_LINE_ARGS];
    char *args2[MAX_COMMAND_LINE_ARGS];
    int i = 0;
    int mode = 0;

    for (token = strtok(args, "|"); token != NULL; token = strtok(NULL, "|")) {
        if (mode == 0) {
            args1[i] = strdup(token);
            mode = 1;
        } else {
            args2[i] = strdup(token);
            mode = 0;
        }
        i++;
    }
    args1[i] = NULL;
    args2[i] = NULL;

    if (args1[0] != NULL && args2[0] != NULL) {
        return true;
    }

    return false;
}

int main() {
    char command_line[MAX_COMMAND_LINE_LEN];
    char *args[MAX_COMMAND_LINE_ARGS];
    signal(SIGINT, handle_SIGINT);
    signal(SIGALRM, handle_SIGALARM);

    while (true) {
        size_t line_size = 0;
        ssize_t read_size = getline(&command_line, &line_size, stdin);
        if (read_size == -1) {
            perror("getline");
            break;
        }
        if (read_size <= 1) {
            continue;
        }

        char *token = strtok(command_line, " \t\r\n");
        int i = 0;
        while (token != NULL) {
            args[i] = strdup(token);
            token = strtok(NULL, " \t\r\n");
            i++;
        }
        args[i] = NULL;

        if (strcmp(args[0], "exit") == 0) {
            break;
        } else if (handle_pipe(args)) {
        } else if (handle_builtins(args)) {
        } else {
            execute_command(args);
        }

        for (int j = 0; j < i; j++) {
            free(args[j]);
        }
    }

    free(command_line);
    return 0;
}
