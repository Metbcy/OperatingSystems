#define _GNU_SOURCE
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>

#include "util.h"
#include "list.c"

void parse_file(FILE *f, int input[][2], int *n, int *PARTITION_SIZE) {
    if (f == NULL) {
        fprintf(stderr, "Error: File pointer is NULL\n");
        return;
    }

    if (fscanf(f, "%d\n", PARTITION_SIZE) != 1) {
        fprintf(stderr, "Error reading partition size\n");
        return;
    }
    printf("PARTITION_SIZE = %d\n", *PARTITION_SIZE);


    while (1) {
        int read = fscanf(f, "%d %d\n", &input[*n][0], &input[*n][1]);
        if (read != 2) {
            if (feof(f)) {
          
                break;
            } else {
               
                fprintf(stderr, "Error reading file\n");
                break;
            }
        }
        *n += 1;
    }
}