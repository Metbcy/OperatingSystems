#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd1[2];
    int fd2[2];

    char fixed_str[] = "howard.edu";
    char input_str[100];
    pid_t p;

    if (pipe(fd1) == -1) {
        fprintf(stderr, "Pipe 1 Failed");
        return 1;
    }
    if (pipe(fd2) == -1) {
        fprintf(stderr, "Pipe 2 Failed");
        return 1;
    }

    printf("Enter a string to concatenate: ");
    scanf("%s", input_str);

    p = fork();

    if (p < 0) {
        fprintf(stderr, "fork Failed");
        return 1;
    } else if (p > 0) {
        close(fd1[0]);
        close(fd2[1]);

        write(fd1[1], input_str, strlen(input_str) + 1);

        char concat_str[100];
        read(fd2[0], concat_str, 100);

        printf("Concatenated string: %s\n", concat_str);

        close(fd1[1]);
        close(fd2[0]);
    } else {
        close(fd1[1]);
        close(fd2[0]);

        char received_str[100];
        read(fd1[0], received_str, 100);

        int k = strlen(received_str);
        int i;
        for (i = 0; i < strlen(fixed_str); i++)
            received_str[k++] = fixed_str[i];

        received_str[k] = '\0';

        printf("Concatenated string: %s\n", received_str);

        char second_input_str[100];
        printf("Enter a second string: ");
        scanf("%s", second_input_str);

        k = strlen(received_str);
        for (i = 0; i < strlen(second_input_str); i++)
            received_str[k++] = second_input_str[i];

        received_str[k] = '\0';

        write(fd2[1], received_str, strlen(received_str) + 1);

        close(fd1[0]);
        close(fd2[1]);

        exit(0);
    }
}
