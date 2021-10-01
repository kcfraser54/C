// Kyle Fraser
// pzip.c
// 2/14/2021
    
#include <stdio.h>  
#include <stdlib.h> 
#include <pthread.h>  
#include <semaphore.h>  
#include <sys/mman.h>  
#include <sys/stat.h> 
#include <fcntl.h>    
#define THREAD_NUM 64
#define TASKS 256

typedef struct Task{

  char *f;
  int order;
  int final;

}Task;

sem_t *outputOrder;
sem_t full, empty;    
pthread_mutex_t m;  
int taskCount = 0;   
int continueCount = 0;
Task q[TASKS];

void submitCompression(Task t);
void* startThread(void *arg);

int main(int argc,  char *argv[])
{

  // Check for a valid argument 
  if(argv[1] == NULL){
    printf("pzip: file1[file2...]\n");
    exit(1);
  }

  outputOrder = (sem_t*)malloc(sizeof(sem_t) * (argc - 1));

  // fill a buffer with arguments and initialize semaphores to
  // order printing. 
  Task buf[argc - 1];
  for(int i = 0; i < argc - 1; i++){

    buf[i].f = argv[i + 1]; 
    buf[i].order = i;
    buf[i].final = argc - 1;
    sem_init(&outputOrder[i], 0, 0);   
    
  }
  
  sem_init(&full, 0, 0);
  m = PTHREAD_MUTEX_INITIALIZER;

  // Create threads 
  pthread_t tidProduce[THREAD_NUM];  
  for(int i = 0; i < THREAD_NUM; i++){
    pthread_create(&tidProduce[i], NULL, &startThread, NULL); 
  }

  // submit jobs for compression 
  for(int i = 0; i < argc - 1; i++){

    submitCompression(buf[i]); 
    sem_post(&full);
  }

  // Increment semaphore to begin printing process. 
  sem_post(&outputOrder[0]);

  // Join threads 
  for(int i = 0; i < THREAD_NUM; i++){
    pthread_join(tidProduce[i], NULL); 
  }

  // destroy semaphores 
  for(int i = 0; i < argc - 1; i++){
    sem_destroy(&outputOrder[i]);
   
  }

  free(outputOrder);
  sem_destroy(&full);
  return 0;  

}

void submitCompression(Task t){

  // enqueue a task 
  pthread_mutex_lock(&m);
  q[taskCount] = t;
  taskCount++;
  pthread_mutex_unlock(&m);
}

void* startThread(void *args){  
 
  sem_wait(&full);
  sem_post(&full);
  while(continueCount < q[0].final){   
    continueCount++;
    Task t;
    t.f = NULL;

    // dequeue a task. 
    pthread_mutex_lock(&m);
    if(taskCount > 0){
          
      t = q[0];
      for(int i = 0; i < taskCount - 1; i++){
        q[i] = q[i + 1];
      }

      taskCount--;
    }
    
    pthread_mutex_unlock(&m);
    
    if(t.f != NULL){

      // Execute a task 
      int letterCount = 1;
      char begin;
      char* uncompressedBuffer;
      int writeCount = 0;
      
      // open the file 
      int fd = open(t.f, O_RDONLY, S_IRUSR | S_IWUSR);
      if(fd == -1){

        fprintf(stderr, "Error: cannot open file %d\n", (t.order + 1));
        sem_wait(&outputOrder[t.order]);
        if(t.order != t.final){
          sem_post(&outputOrder[t.order + 1]);
        }
        pthread_exit(0); 
       
      }
      struct stat sb;
      if(fstat(fd, &sb) == -1){
        fprintf(stderr, "Counldn't get file size\n");
        exit(1);
      }

      int cBufSize = sb.st_size;

      // allocate memory for compressed buffers 
      int *lCountBuffer = (int*)malloc(sizeof(int) * cBufSize);
      char *letterBuffer = (char*)malloc(sizeof(char) * cBufSize);

      // map file to uncompressed buffer 
      uncompressedBuffer = (char*)mmap(NULL, sb.st_size,
                                   PROT_READ, MAP_PRIVATE, fd, 0); 

      begin = uncompressedBuffer[0];
      
      // Do compression 
      for(int i = 0; i < sb.st_size; i++){

        if(uncompressedBuffer[i] == uncompressedBuffer[i + 1]){
          letterCount++;
        }
      
        else{
     
          lCountBuffer[writeCount] = letterCount;
          letterBuffer[writeCount] = begin;
          writeCount++;
          begin = uncompressedBuffer[i + 1];
          letterCount = 1;

        }

      }

      munmap(NULL, sb.st_size); 
      sem_wait(&outputOrder[t.order]);  

      // write compressed buffer to output  
      for(int i = 0; i < writeCount; i++){

        fwrite(&lCountBuffer[i], sizeof(int), 1, stdout);
        fwrite(&letterBuffer[i], sizeof(char), 1, stdout);
      }

      free(lCountBuffer);
      free(letterBuffer);
      if(t.order != t.final){
        sem_post(&outputOrder[t.order + 1]);  
      }
      
    }

  }

  pthread_exit(0);
   
}
