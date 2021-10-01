// Kyle Fraser
// wgrep.c
// 1/15/2021

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const int BUFFER_LENGTH = 100;
const int SEARCH_TERM = 1; 
int main(int argc, char* argv[])
{

  size_t n = 0; 
  char *line = NULL;
  char *check = NULL;
  char oneLine[BUFFER_LENGTH];
  FILE* userFile = NULL;
  int fileCount = 2;
  int fileOnlyCount = 1; 
 
  if(argv[SEARCH_TERM] == NULL){
    printf("wgrep: searchterm[file...]\n");
    exit(1);
  }

  userFile = fopen(argv[SEARCH_TERM], "r");  
  if(userFile != NULL){
     fclose(userFile);

    while(argv[fileOnlyCount] != NULL){
      userFile = fopen(argv[fileOnlyCount], "r"); 
      while(getline(&line, &n, userFile) != -1){

        printf(line);
        
      }

      fclose(userFile);
      fileOnlyCount++; 
    }

  }
   
  else if(userFile == NULL && !argv[fileCount]){

    fgets(oneLine, BUFFER_LENGTH, stdin);

    check = strstr(oneLine, argv[SEARCH_TERM]);
    if(check != NULL){
      printf(oneLine);
    }
    
  }

  else{

    while(argv[fileCount] != NULL){

      userFile = fopen(argv[fileCount],"r");

      if(userFile == NULL){
        printf("wgrep: cannot open file\n");
        exit(1);
      }

      while(getline(&line, &n, userFile) != -1){

        check = strstr(line, argv[SEARCH_TERM]);

        if(check != NULL){

          printf(line);

        }

      }
        
      fclose(userFile);
      fileCount++;

    }

  }

  return 0;
   
}
