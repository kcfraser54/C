// Kyle Fraser
// wzip.c
// 1/15/2021

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const int ERROR_BUFFER = 24;
int main(int argc, char *argv[]) {

    char *line = NULL;
    FILE *userFile = NULL;
    char error[ERROR_BUFFER] = "wzip: cannot open file\n";
    size_t n = 0;   
    int letterCount = 1;
    int errLetterCount = 1; 
    int fCount = 1;
    char begin;
         
    if(argv[fCount] == NULL){
        printf("wzip: file1[file2...]\n");
        exit(1);
    }

    while(argv[fCount] != NULL){
      userFile = fopen(argv[fCount], "r");
      if(!userFile){
        
        for(int err = 0; err < ERROR_BUFFER; err++){

          fwrite(&errLetterCount, sizeof(int), 1, stdout);
          fwrite(&error[err], sizeof(char), 1, stdout);
                
        }
        exit(1);
      }
           
      while(getline(&line, &n, userFile) != -1){
               
        begin = line[0];

        for(int j = 0; j < int(strlen(line)); j++){
                  
          if(line[j] == line[j + 1]){

            letterCount++;

          }

          else{
     
            fwrite(&letterCount, sizeof(int), 1, stdout);
            fwrite(&begin, sizeof(char), 1, stdout);
            letterCount = 1;
            begin = line[j + 1];
                      
          }
                
        } 

      }
            
      fCount++;
      fclose(userFile); 
           
    }

    return 0;
}

