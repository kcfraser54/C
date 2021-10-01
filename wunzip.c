// Kyle Fraser
// wunzip.c
// 1/15/2021

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  
  FILE *binFile = NULL;
  int status = 1;
  int data = 0;
  char data2;
  int fCount = 1; 
     
  if(argv[1] == NULL){
    printf("wunzip: file1[file2...]\n");
    exit(1);
   }

  while(argv[fCount] != NULL){

    binFile = fopen(argv[fCount], "rb");
    if(binFile == NULL){
      
      printf("wunzip:cannot open file\n");
      exit(1);
      
    }
 
    status = 1; 
    while(status != 0){
          
      status = fread(&data, sizeof(int), 1, binFile);
      status = fread(&data2, sizeof(char), 1, binFile);
      
      if(status != 0){    
        for(int count = 0; count < data; count++){
          printf("%c", data2);
        }
      }
          
    }
                       
    fclose(binFile);
    fCount++;

  }
  return 0;
}

    


