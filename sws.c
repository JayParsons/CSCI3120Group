/*
 * File: sws.c
 * Author: Alex Brodsky
 * Purpose: This file contains the implementation of a simple web server.
 *          It consists of two functions: main() which contains the main
 *          loop accept client connections, and serve_client(), which
 *          processes each client request.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#include "network.h"
#include "Priority_Heap.h"
#define MAX_HTTP_SIZE 32*1024                 /* size of buffer to allocate */


#define IS ==
#define ISNOT !=
#define SJF 1
#define RR 2
#define MLFB 3

#define TOP_QUEUE_QUANTUM 8*1024           // MLFB top queue
#define MID_QUEUE_QUANTUM 64*1024          // MLFB mid queue
#define RR_QUANTUM 8*1024                  // RR or MLFB
#define UNIFORM 32                         // priority

#define TOPL 1
#define MIDL 2
#define RRL 3
#define DEFAULT -1

Heap *topHeap;
Heap *midHeap;
Heap *rrHeap;

int alg_using = 0;
int seq_num_c = 0; // cumulative sequence number


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t outerMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t *semaphore;

void algorithm_init(int,char *, int);
void receive_init(int *number_of_threads);


void create_RCB_init();
RCB *create_RCB(int, FILE *, char *fileName);

void mutex_lock_enqueue(Heap *,RCB *);
RCB *mutex_lock_dequeue();

void *receive(void *);// a.k.a worker function,  the function that handles all receive jobs, parameter is the interface

void *serve_client_init(void *);
void *serve_client();


/* This function takes a file handle to a client, reads in the request,
 *    parses the request, and sends back the requested file.  If the
 *    request is improper or the file is not available, the appropriate
 *    error is sent back.
 * Parameters:
 *             fd : the file descriptor to the client connection
 * Returns: None
 */



void *serve_client_init( void *fdp ) {
  
  //sem_wait(semaphore); // wait until the there is a job assigned
  int fd = *(int *)fdp;
  char *buffer = NULL;                              /* request buffer */
  char *req = NULL;                                 /* ptr to req file */
  char *brk;                                        /* state used by strtok */
  char *tmp;                                        /* error checking ptr */
  FILE *fin;                                        /* input file handle */
  int len;                                          /* length of data read */
  
  
  char *fileName = NULL;
  fileName = malloc(MAX_HTTP_SIZE);
  memset(fileName,0,MAX_HTTP_SIZE);
  int length;
  
  
  if( !buffer ) {                                   /* 1st time, alloc buffer */
    buffer = malloc( MAX_HTTP_SIZE );
    memset( buffer, 0, MAX_HTTP_SIZE );
    if( !buffer ) {                                 /* error check */
      perror( "Error while allocating memory" );
      abort();
    }
  }
  
  
  if( read( fd, buffer, MAX_HTTP_SIZE ) <= 0 ) {    /* read req from client */
    perror( "Error while reading request" );
    abort();
  }
  
  /* standard requests are of the form
   *   GET /foo/bar/qux.html HTTP/1.1
   * We want the second token (the file path).
   */
  tmp = strtok_r( buffer, " ", &brk );              /* parse request */
  if( tmp && !strcmp( "GET", tmp ) ) {
    req = strtok_r( NULL, " ", &brk );
  }
  
  if( !req ) {                                      /* is req valid? */
    length = sprintf( buffer, "HTTP/1.1 400 Bad request\n\n" );
    write( fd, buffer, len );                       /* if not, send err */
    close(fd);
  } else {                                          /* if so, open file */
    req++;                                          /* skip leading / */
    
    printf("Request for file '%s' admitted.\n", req); // required T2 3)
   
    
    length = sprintf(fileName,"%s",req);
    //write(popped_rcb->rcb_fd,fileName,length);
    strcpy(fileName,req);
    
    fin = fopen( req, "r" );                        /* open file */
    
    if( !fin ) {                                    /* check if successful */
      len = sprintf( buffer, "HTTP/1.1 404 File not found\n\n" );
      write( fd, buffer, len );                     /* if not, send err */
      close(fd);
    } else {                                        /* if so, send file */
     
      len = sprintf( buffer, "HTTP/1.1 200 OK\n\n" );/* send success code */
      
      write( fd, buffer, len );
      //fflush(fdp);
      
      RCB *new_RCB = create_RCB(fd,fin, fileName);
      
      if (alg_using IS RR)
        mutex_lock_enqueue(rrHeap, new_RCB);
      else
        mutex_lock_enqueue(topHeap, new_RCB);
    }
  }
  free(fileName);
  free(buffer);
  return 0;
}

void *serve_client() {
  char *buffer = malloc(sizeof(char) * MAX_HTTP_SIZE);
  memset(buffer, 0, sizeof(char) * MAX_HTTP_SIZE);
  RCB *popped_rcb;
  Heap *putBackHeap;
  int length;
  int level;
  int minus;
  if (!buffer) {
    perror("Error while allocating memory");
    abort();
  }
  for(;;){
  //  printf("1. semaphore num: %d\n",(int)semaphore);
    sem_wait(semaphore); // wait until the there is a job assigned
  //  printf("2. semaphore num: %d\n",(int)semaphore);
    //pthread_mutex_lock(&outerMutex);
    
   // printf("CONTIGUOUS 1\n");
    if(alg_using IS RR) {
      putBackHeap = rrHeap;
    } else if (alg_using IS MLFB) {
      if(topHeap->length ISNOT 0){
        level = MIDL;
        putBackHeap = midHeap;
      } else {
        //level = (midHeap->length IS 0)?RRL:MIDL;
        level = RRL;
        putBackHeap = rrHeap;
      }
    } else if (alg_using IS SJF) {
      putBackHeap = topHeap;
    } else {
      printf("If you can see this message, that means");
      printf("there is something wrong on your machin\n");
      printf("Because this is the least place that can cause an error\n");
      
    }
    
    popped_rcb = mutex_lock_dequeue();
    
    //length = fread(buffer,1,popped_rcb->quantum,popped_rcb->rcb_file);
    //if (length < 0) printf("Buffer error\n");
    //else printf("Buffer no error");
    //printf("%s\n",buffer);
    
    minus = (popped_rcb->rcb_data_remain>popped_rcb->quantum)?popped_rcb->quantum:popped_rcb->rcb_data_remain;
    popped_rcb->rcb_data_remain -= minus;
    printf("Sent %d bytes of file %s.\n",minus,popped_rcb->file_name);
    // pthread_mutex_unlock(&mutex);
    if(popped_rcb->rcb_data_remain > 0){
      if(alg_using IS MLFB)
         popped_rcb->quantum = (level IS MIDL)? MID_QUEUE_QUANTUM : RR_QUANTUM;
      mutex_lock_enqueue(putBackHeap,popped_rcb);
    }else{
      printf("File %s transfer complete\n",popped_rcb->file_name);
      memset(buffer,0,MAX_HTTP_SIZE);
      length = sprintf(buffer,"%s finished\n",popped_rcb->file_name);
      
      write(popped_rcb->rcb_fd,buffer,length);
      free(popped_rcb);
      
    }
   // printf("CONTIGUOUS 2\n");
    //pthread_mutex_unlock(&outerMutex);
  }
}


void receive_init(int *num_threads) {
  pthread_t CCT[*num_threads]; // client-connect thread
  for (int i = 0; i < *num_threads; i++)
    pthread_create(&CCT[i], NULL, serve_client, NULL);
  
  create_RCB_init();
  
  for (int i = 0; i < *num_threads; i++)
    pthread_join(CCT[i], NULL); // for a successful join back of all threads
}


void create_RCB_init(){
  
  int fd = 0;
  
  for(;;) {
   // pthread_mutex_lock(&mutex);
    //printf("Arrive Fault 1\n");
    network_wait();
    for (fd = network_open(); fd >= 0; fd = network_open()) {
      pthread_t t ;
      
      int *fdp = (int *) malloc(sizeof(int));
      *fdp = fd;
      pthread_create(&t, NULL, serve_client_init, (void *)fdp);
      pthread_join(t,NULL);
    }
  //  pthread_mutex_unlock(&mutex);
  }
}


RCB *create_RCB(int fd,FILE *inputFile,char *fileName) {
  
  RCB *new_RCB = (RCB *)malloc(sizeof(RCB));
  new_RCB->file_name = malloc(MAX_HTTP_SIZE);
  new_RCB->file_name = strcpy(new_RCB->file_name, fileName);
  new_RCB->rcb_fd = fd;
  new_RCB->rcb_file = inputFile;
  new_RCB->rcb_seq_num = seq_num_c++;
  
  fseek(new_RCB->rcb_file, 0, SEEK_END);
  int length = (int)ftell(new_RCB->rcb_file); //get file size
  rewind(new_RCB->rcb_file);
  
  new_RCB->rcb_data_remain = length;
  
  if(alg_using IS SJF){
    new_RCB->quantum = length; // may remove
    new_RCB->priority = length;
    
  } else if (alg_using IS RR) {
    new_RCB->quantum = RR_QUANTUM; // may remove
    new_RCB->priority = UNIFORM;
  } else if (alg_using IS MLFB) {
    new_RCB->quantum = TOP_QUEUE_QUANTUM; // may remove
    new_RCB->priority = UNIFORM;
  }
  return new_RCB;
  
}

/* This function is where the program starts running.
 *    The function first parses its command line parameters to determine port #
 *    Then, it initializes, the network and enters the main loop.
 *    The main loop waits for a client (1 or more to connect, and then processes
 *    all clients by calling the seve_client() function for each one.
 * Parameters:
 *             argc : number of command line parameters (including program name
 *             argv : array of pointers to command line parameters
 * Returns: an integer status code, 0 for success, something else for error.
 */
int main( int argc, char **argv ) {
  int port = -1;                                    /* server port # */
  //int fd;       // not needed                     /* client file descriptor */
  
  //sem_open(&semaphore, 0, 0);         // initialize a semaphore for work when network accept
  semaphore = sem_open("/semaphore", O_CREAT, 0644, 1);
  int num_threads = 0;
  char alg_in[5];
  
  
  /* check for and process parameters */
  if( ( argc != 4 ) ||
     ( sscanf( argv[1], "%d", &port ) < 1 ) ||
     ( *(strcpy(alg_in, argv[2])) < 1 )     ||    // decide the algorithm
     ( sscanf(argv[3], "%d", &num_threads) < 1)
     ) {
    printf( "usage: sws <port> <algrithm> <number of threads>\n" );
    return 0;
  }
  
  algorithm_init(port,alg_in,num_threads);
  
  network_init( port );                             /* init network module */
  printf("\nServer all Green! //initialization complete");
  printf("\nStart waiting for clients request\n\n");
  
  
  receive_init(&num_threads);
  
  return EXIT_SUCCESS; // return status
}

void mutex_lock_enqueue(Heap *h, RCB *c) {
  
  pthread_mutex_lock(&mutex);
  
  addRCB(h, c->priority, c);
  
  pthread_mutex_unlock(&mutex);
  //printf("before enqueueu: %d\n",(int)semaphore);
  sem_post(semaphore); // here is a thread can start in threads
  //printf("after enqueueu: %d\n",(int)semaphore);
}

RCB *mutex_lock_dequeue() {
  pthread_mutex_lock(&mutex);
  RCB *ret = NULL;
  
  if(alg_using IS SJF) {
    ret = pop(topHeap);
  } else if (alg_using IS RR) {
    ret = pop(rrHeap);
  } else if (alg_using IS MLFB) {
    if( topHeap->length != 0 ) {
      ret = pop(topHeap);
    } else if ( midHeap->length != 0 ){
      ret = pop(midHeap);
    } else {
      ret = pop(rrHeap);
    }
  }
  
  pthread_mutex_unlock(&mutex);
  return ret;
}

void algorithm_init(int port, char *alg_in, int num_threads) {
 // printf("%s\n",alg_in);
  
  if( strcmp(alg_in,"SJF") == 0 || strcmp(alg_in,"sjf") == 0){
    
    alg_using = SJF;
    topHeap = malloc(sizeof(Heap));
    init_heap(topHeap);
  } else if( strcmp(alg_in,"RR") == 0 || strcmp(alg_in,"rr") == 0 ){
    
    alg_using = RR;
    rrHeap = malloc(sizeof(Heap));
    init_heap(rrHeap);
  }else if( strcmp(alg_in,"MLFB") == 0 || strcmp(alg_in,"mlfb") == 0){
    
    alg_using = MLFB;
    topHeap = malloc(sizeof(Heap));
    midHeap = malloc(sizeof(Heap));
    rrHeap = malloc(sizeof(Heap));
    
    init_heap(topHeap);
    init_heap(midHeap);
    init_heap(rrHeap);
  }else {
    printf("What algorithm is that?");
    exit(0);
  }
  
  printf("Using algorithm ");
  if(alg_using IS SJF)  printf("Shortest Job First");
  if(alg_using IS MLFB)printf("Multilevel Feedback");
  if(alg_using IS RR)printf("Round Robin");
  
  printf(" on port: %d, with %d threads using\n",port,num_threads);
  
}

