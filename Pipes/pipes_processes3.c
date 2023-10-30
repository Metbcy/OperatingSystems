#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <grep_argument>\n", argv[0]);
    return 1;
  }

  int pipe1[2];
  int pipe2[2];
  int pid1, pid2;

  char *cat_args[] = {"cat", "scores", NULL};
  char *grep_args[] = {"grep", argv[1], NULL};
  char *sort_args[] = {"sort", NULL};

  if (pipe(pipe1) == -1) {
    perror("pipe");
    return 1;
  }

  pid1 = fork();

  if (pid1 == 0) {
    close(pipe1[1]);
    dup2(pipe1[0], 0);
    execvp("grep", grep_args);
  } else {
    if (pipe(pipe2) == -1) {
      perror("pipe");
      return 1;
    }

    pid2 = fork();

    if (pid2 == 0) {
      close(pipe1[0]);
      close(pipe2[1]);
      dup2(pipe2[0], 0);
      execvp("sort", sort_args);
    } else {
      close(pipe1[0]);
      dup2(pipe1[1], 1);
      execvp("cat", cat_args);
    }
  }

  close(pipe1[0]);
  close(pipe1[1]);
  close(pipe2[0]);
  close(pipe2[1]);

  wait(NULL);
  wait(NULL);
  wait(NULL);

  return 0;
}
