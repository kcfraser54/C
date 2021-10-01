// Kyle Fraser
// wcat.c
// 1/15/2021
  
#include <stdio.h>
#include <stdlib.h>

const int BUFFER_SIZE = 200; 
int main(int argc, char *argv[])
{
  
  FILE *userFile = NULL;
  char str[BUFFER_SIZE];
  int fCount; 
  
  if(argv[1] == NULL){
    printf("wcat: file[file...]\n");
    exit(1);
  }
 

  fCount = 1; 
  while(argv[fCount] != NULL){
    
    userFile = fopen(argv[fCount], "r");
    if(userFile == NULL){
      printf("wcat: cannot open file\n");
      exit(1);
    }

    while(fgets(str, BUFFER_SIZE ,userFile) != NULL){
      printf(str);
    }

    fclose(userFile);
    fCount++;

  }

  return 0; 
}

  
