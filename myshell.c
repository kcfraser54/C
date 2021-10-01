// Kyle Fraser
// myshell.c
// 1/31/2021

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

struct pipes{

  int fd[2];

};

const int COMMAND_LENGTH = 10;
const int ARGUMENT_LENGTH = 500;
const int ARGUMENT_COUNT = 20;
const int FULL_COMMAND_MAX = 5000;

int main(int argc, char* argv[]){
  
  char *buf[COMMAND_LENGTH][ARGUMENT_LENGTH];  
  char singleCommand[FULL_COMMAND_MAX];
  char *command[ARGUMENT_COUNT];
  char *arguments[ARGUMENT_COUNT];
  char *cmdToken = NULL;
  char *argToken = NULL;
  char *userInput = NULL;
  char *args = NULL;
  char *pipeBuf = NULL;
  int argCount[COMMAND_LENGTH];  
  int position = 0;
  int commandCount = 0;
  int pipeFlow = 0;
  unsigned long pipeFlowLen;
  pid_t cpid;
  int terminationCheck = -1;
   
  for(int i = 0; i < COMMAND_LENGTH; i++){
    
    argCount[i] = 0;
    
  }

  // Get commands from the user 
  printf("myshell$");
  fgets(singleCommand, FULL_COMMAND_MAX, stdin); 
  userInput = strdup(singleCommand);

  // Parse commands
  while((cmdToken = strsep(&userInput, "|")) != NULL){
    position = 0; 
    command[commandCount] = strdup(cmdToken);
    args = strdup(command[commandCount]);

    // parse arguments 
    while((argToken = strsep(&args, " ")) != NULL){

      if(strstr(argToken, "\n")){
        argToken[strlen(argToken) - 1] = '\0';  
      }
     
      if(argToken[0] != '\0'){
        buf[commandCount][position] = strdup(argToken);
        position += strlen(buf[commandCount][position]); 
        argCount[commandCount]++;
      }              
    }
    
    commandCount++;
  }

  // Execute commands. 
  pipes pipeList[COMMAND_LENGTH];
  for(int i = 0; i < commandCount; i++){
 
    position = 0;
    pipeFlow = 0;
    if(pipe(pipeList[i].fd) != 0){
      perror("Pipe error...\n");
      exit(1);     
    }  

    // Execute a single command.
    if(i == 0 && i == (commandCount - 1)){

      for (int j = 0; j < argCount[i]; j++) {
              
        arguments[j] = strdup(buf[i][position]);
        position += strlen(arguments[j]);
               
      }

      arguments[argCount[i]] = (char*)NULL;
      cpid = fork(); 

      if(cpid == -1){
        perror("Error in creation of child process\n");
        exit(1); 
      }

      else if(cpid == 0){
        if(execvp(arguments[0], arguments) == -1){
          perror("Child process failed to execute the command\n");
          exit(1);
          
        }  
      }

      else{
        close(pipeList[i].fd[0]);
        close(pipeList[i].fd[1]);
        terminationCheck = wait(NULL);
        
        if(terminationCheck == -1){

          perror("Child process failed to terminate\n");
          exit(1);
        }

        else{
          printf("Process %d exits with 0\n", terminationCheck);

        }

      }

    }
    // Execute a single command and pipe to the input of the next command.
    else if(i == 0 && i != (commandCount - 1)){
 
      for (int j = 0; j < argCount[i]; j++) {

        arguments[j] = strdup(buf[i][position]);
        position += strlen(arguments[j]);
        
      }

      arguments[argCount[i]] = (char*)NULL;
      cpid = fork();   
    
      if(cpid == -1){

        perror("Error in creation of child process\n");
        exit(1); 

      }

      else if(cpid == 0){

        if(dup2(pipeList[i].fd[1],1) == -1){

          perror("File descriptor error\n");
          exit(1);
        }
        
        close(pipeList[i].fd[1]);
        close(pipeList[i].fd[0]);
        if(execvp(arguments[0], arguments) == -1){
          perror("Child process failed to execute the command\n");
          exit(1);

        }
      
      }

      else{
    
        close(pipeList[i].fd[1]);  
        terminationCheck = wait(NULL);
        if(terminationCheck == -1){

          perror("Child process failed to terminate\n");
          exit(1);
        }
        else{
          printf("Process %d exits with 0\n", terminationCheck);

        }

      }
    
    }

    // Execute a command using the output of the previous command.
    // Pipe to the input of the next command. 
    else if(i != 0 && i != (commandCount - 1)){
   
      for (int j = 0; j < argCount[i]; j++) { 
        
        arguments[j] = strdup(buf[i][position]);
        position += strlen(arguments[j]);
          
      }

      cpid = fork();   

      if(cpid == -1){

        perror("Error in creation of child process\n");
        exit(1);

      }

      else if(cpid == 0){
    
        if(dup2(pipeList[i - 1].fd[0], 0) == -1){

          perror("File descriptor error\n");
          exit(1);
        }
        if(dup2(pipeList[i].fd[1], 1) == -1){

          perror("File descriptor error\n");
          exit(1);
        }
          
        close(pipeList[i].fd[1]); 
        close(pipeList[i].fd[0]);
        close(pipeList[i - 1].fd[1]);
        close(pipeList[i - 1].fd[0]);

        fseek(stdin, 0, SEEK_END);  
        pipeFlowLen = (unsigned long)ftell(stdin);  
          
        read(pipeList[i - 1].fd[0], &pipeBuf, pipeFlowLen);   
        while((argToken = strsep(&pipeBuf, " ")) != NULL){

          if(strstr(argToken, "\n")){
            argToken[strlen(argToken) - 1] = '\0';
          }

          arguments[argCount[i] + pipeFlow] = strdup(argToken);
          pipeFlow++;

        }

        arguments[argCount[i] + pipeFlow] = NULL;
        if(execvp(arguments[0], arguments) == -1){

          perror("Child process failed to execute the command\n");
          exit(1);
        }

      }


      else{
          
        close(pipeList[i - 1].fd[0]);
        close(pipeList[i].fd[1]);
        terminationCheck =  wait(NULL);
        if(terminationCheck == -1){

          perror("Child process failed to terminate\n");
          exit(1);
        }
        else{
          printf("Process %d exits with 0\n", terminationCheck);

        }

      }

    }

    // Execute a command using the output from the previous command.
    // End of pipeline.
    else{

      for (int j = 0; j < argCount[i]; j++) { 
        
        arguments[j] = strdup(buf[i][position]);
        position += strlen(arguments[j]);
        
      }

      cpid = fork(); 
  
      if(cpid == -1){

        perror("Error in creation of process\n");
        exit(1); 

      }

      else if(cpid == 0){

        if(dup2(pipeList[i - 1].fd[0], 0) == -1){

          perror("File descriptor error\n");
          exit(1);
        }
        close(pipeList[i].fd[0]);
        close(pipeList[i].fd[1]);
        close(pipeList[i - 1].fd[0]);
        close(pipeList[i - 1].fd[1]);

        fseek(stdin, 0, SEEK_END);  
        pipeFlowLen = (unsigned long)ftell(stdin);  
        read(pipeList[i - 1].fd[0], &pipeBuf, pipeFlowLen);  
           
        while((argToken = strsep(&pipeBuf, " ")) != NULL){

          if(strstr(argToken, "\n")){
            argToken[strlen(argToken) - 1] = '\0';
          }
      
          arguments[argCount[i] + pipeFlow] = strdup(argToken);
          pipeFlow++;
        }

        arguments[argCount[i] + pipeFlow] = NULL;
        if(execvp(arguments[0], arguments) == -1){

          perror("Child process failed to execute the command\n");
          exit(1);

        }

      }

      else{

        close(pipeList[i - 1].fd[1]);
        close(pipeList[i - 1].fd[0]);   
        close(pipeList[i].fd[0]);
        close(pipeList[i].fd[1]);
        terminationCheck = wait(NULL);
        if(terminationCheck == -1){

          perror("Child process failed to terminate\n");
          exit(1);
        }
        else{
          printf("Process %d exits with 0\n", terminationCheck);
          
        }

      }

    }

  }

  return 0; 

}




