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
#define MAX_HTTP_SIZE 8192                 /* size of buffer to allocate */


#define IS ==
#define SJF 1
#define RR 2
#define MLFB 3

#define TOP_QUEUE_QUANTUM 8*1024           // MLFB top queue
#define MID_QUEUE_QUANTUM 64*1024          // MLFB mid queue
#define RR_QUANTUM 8*1024                  // RR or MLFB
#define UNIFORM 32

Heap *topHeap;
Heap *midHeap;
Heap *rrHeap;

int alg_using = 0;
int seq_num_c = 0; // cumulative sequence number



pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t semaphore;

void algorithm_init(int,char *, int);
void receive_init(int *number_of_threads);


void create_RCB_init();
RCB *create_RCB(int, FILE *, char *, char *fileName);

void mutex_lock_enqueue(Heap *,RCB *);


void *receive(void *);// a.k.a worker function,  the function that handles all receive jobs, parameter is the interface




/* This function takes a file handle to a client, reads in the request,
 *    parses the request, and sends back the requested file.  If the
 *    request is improper or the file is not available, the appropriate
 *    error is sent back.
 * Parameters:
 *             fd : the file descriptor to the client connection
 * Returns: None
 */

static void *serve_client( void *fdp ) {
  int fd = *(int *)fdp;
  static char *buffer;                              /* request buffer */
  char *req = NULL;                                 /* ptr to req file */
  char *brk;                                        /* state used by strtok */
  char *tmp;                                        /* error checking ptr */
  FILE *fin;                                        /* input file handle */
  int len;                                          /* length of data read */
  
  if( !buffer ) {                                   /* 1st time, alloc buffer */
    buffer = malloc( MAX_HTTP_SIZE );
    if( !buffer ) {                                 /* error check */
      perror( "Error while allocating memory" );
      abort();
    }
  }
  
  memset( buffer, 0, MAX_HTTP_SIZE );
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
    len = sprintf( buffer, "HTTP/1.1 400 Bad request\n\n" );
    write( fd, buffer, len );                       /* if not, send err */
  } else {                                          /* if so, open file */
    req++;                                          /* skip leading / */
    
    
    printf("Request for file '%s' admitted.\n", req); // required T2 3)
    
    
    fin = fopen( req, "r" );                        /* open file */
    char fileName[20];
    memset(fileName, '\0', sizeof(fileName));
    
    if( !fin ) {                                    /* check if successful */
      len = sprintf( buffer, "HTTP/1.1 404 File not found\n\n" );
      write( fd, buffer, len );                     /* if not, send err */
      close(fd);
    } else {                                        /* if so, send file */
      len = sprintf( buffer, "HTTP/1.1 200 OK\n\n" );/* send success code */
      write( fd, buffer, len );
      
      
      
      
      
      RCB *new_RCB = create_RCB(fd,fin, buffer, fileName);
      mutex_lock_enqueue(topHeap, new_RCB);
      
      //TODO: enqueue now do create RCB
      
      do {                                          /* loop, read & send file */
        len = fread( buffer, 1, MAX_HTTP_SIZE, fin );  /* read file chunk */
        if( len < 0 ) {                             /* check for errors */
          perror( "Error while writing to client" );
        } else if( len > 0 ) {                      /* if none, send chunk */
          len = write( fd, buffer, len );
          if( len < 1 ) {                           /* check for errors */
            perror( "Error while writing to client" );
          }
        }
      } while( len == MAX_HTTP_SIZE );              /* the last chunk < 8192 */
      fclose( fin );
    }
  }
  close( fd );                                     /* close client connectuin*/
  return 0;
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
  
  sem_init(&semaphore, 0, 0);         // initialize a semaphor for work when network accept
  int num_threads = 0;
  char alg_in[4];
  
  
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
  
  // put into receive_init
  //  for( ;; ) {                                       /* main loop */
  //    network_wait();                                 /* wait for clients */
  
  //    for( fd = network_open(); fd >= 0; fd = network_open() ) { /* get clients */
  //      serve_client( fd );                           /* process each client */
  //    }
  //  }
  
  return EXIT_SUCCESS; // return status
}


void algorithm_init(int port, char *alg_in, int num_threads) {
  if( strcmp(alg_in,"SJF") || strcmp(alg_in,"sjf") ){
    alg_using = SJF;
    init_heap(topHeap);
  } else if( strcmp(alg_in,"RR") || strcmp(alg_in,"rr") ){
    alg_using = RR;
    init_heap(rrHeap);
  }else if( strcmp(alg_in,"MLFB") || strcmp(alg_in,"mlfb")){
    alg_using = MLFB;
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

void receive_init(int *num_threads) {
  pthread_t CCT[*num_threads]; // client-connect thread
  for (int i = 0; i < *num_threads; i++)
    pthread_create(&CCT[i], NULL, receive, NULL);
  
  create_RCB_init();
  
  for (int i = 0; i < *num_threads; i++)
    pthread_join(CCT[i], NULL); // for a successful join back of all threads
}


void *receive(void *def) { // worker function
  // initializing resources before real receive files
  // including local memory checking
  char *buffer = malloc(sizeof(char) * MAX_HTTP_SIZE);
  memset(buffer, 0, sizeof(char) * MAX_HTTP_SIZE);
  //rcb *popped_rcb;
  if (!buffer) {
    perror("Error while allocating memory");
    abort();
  }
  // initialization successfully to this stage, ready
  for(;;) {
    sem_wait(&semaphore); // wait until the there is a job assigned 
  }
  
  
  
}


void create_RCB_init(){
  int fd = 0;
  for(;;) {
    network_wait();
    for (fd = network_open(); fd >= 0; fd = network_open()) {
      pthread_t init_thread;
      int *fdp = (int *) malloc(sizeof(int));
      *fdp = fd;
      pthread_create(&init_thread, NULL, serve_client, (void *)fdp);
    }
  }
}

// The nserve_client the in s_c create RCB's


RCB *create_RCB(int fd,FILE *inputFile, char *buffer,char *fileName) {
  
  RCB *new_RCB = (RCB *)malloc(sizeof(RCB));
  new_RCB->file_name = fileName;
  new_RCB->rcb_fd = fd;
  new_RCB->rcb_file = inputFile;
  new_RCB->rcb_seq_num = seq_num_c++;
  fseek(new_RCB->rcb_file, 0, SEEK_END);
  int length = (int)ftell(new_RCB->rcb_file); //get file size
  rewind(new_RCB->rcb_file);
  new_RCB->rcb_data_remain = length;
  
  if(alg_using IS SJF){
    new_RCB->quantum = length;
    new_RCB->priority = length;
  } else if (alg_using IS RR) {
    new_RCB->quantum = RR_QUANTUM;
    new_RCB->priority = UNIFORM;
  } else if (alg_using IS MLFB) {
    new_RCB->quantum = TOP_QUEUE_QUANTUM;
    new_RCB->priority = UNIFORM;
  }
  return new_RCB;
  
}




void mutex_lock_enqueue(Heap *h, RCB *c) {
  pthread_mutex_lock(&mutex);
  
  addRCB(h, c->priority, c);
  
  pthread_mutex_unlock(&mutex);
  sem_post(&semaphore); // here is a thread can start in threads
}
RCB *mutex_lock_dequeue() {
  pthread_mutex_lock(&mutex);
  
  RCB *ret ;//= pop(h);
  
  if(alg_using IS SJF) {
    ret = pop(topHeap);
  } else if (alg_using IS RR) {
    ret = pop(rrHeap);
  } else if (alg_using IS MLFB) {
    if( size(topHeap) != 0 ) {
      ret = pop(topHeap);
    } else if ( size(midHeap) != 0 ){
      ret = pop(midHeap);
    } else {
      ret = pop(rrHeap);
    }
  }
  pthread_mutex_unlock(&mutex);
  return ret;
}



