#define _GNU_SOURCE
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include "util.h"
#include "list.h"
void parse_file(FILE * f, int input[][2], int *n, int *PARTITION_SIZE)
{  
  fscanf(f,"%d\n", PARTITION_SIZE);
  printf("PARTITION_SIZE = %d\n", *PARTITION_SIZE);
  while (!feof(f)) 
  {
		fscanf(f, "%d %d\n", &input[*n][0], &input[*n][1]);
    *n += 1;
	}
}