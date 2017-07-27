/*
 *
 * Date: July. 27th
 * CSCI3120 - Operating System - Group project
 *
 * sws.c
 * usage: ./sws <port> <algorithm> <number_of_threads>
 * Example:  ./sws 38080 SJF 8
 * sws.c file after execution will act as a server waiting
 * for clients. After receive the request from the user,
 * will print the current information of sending and send
 * to the clients.
 * Design and logic flow:
 *
 *                        main
 *                          |
 *                          |
 *                       receive_init()
 *          (threads)       |            (main thread)
 *                __________|______________________________
 *               |                                         |
 *       (number of threads)                         network_wait() <---------------------|
 *         |||||||||||||||                                 |                              |
 *          serve_client()                 create a thread for every income requests      |
 *               |                                   |||||||||||||                        |
 *     |--->sem_wait()                           request parsing                          |
 *     |         |                                         |                              |
 *     |    dequeue()                              identify file name                     |
 *     |         |                                         |                              |
 *     |   process quantum data                      search for file                      |
 *     |         |                                         |                              |
 *     |  determine put back or not                   create_RCB()                        |
 *     |         |                                         |                              |
 *     |_________|                                    enqueue RCB                         |
 *                                                         |                              |
 *                                                   rise a semaphore ____________________|
 *
 *
 *
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

#define TOPL 1                             // put back heap level
#define MIDL 2
#define RRL 3
#define DEFAULT -1

Heap *topHeap;         // top heap works for MLFB, SJF
Heap *midHeap;         // MLFB
Heap *rrHeap;          // MLFB, RR

int alg_using = 0; // initialize the algorithm in use
int seq_num_c = 0; // cumulative sequence number

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t *semaphore;

void algorithm_init(int,char *, int);
void receive_init(int *number_of_threads);

void create_RCB_init();
RCB *create_RCB(int, FILE *, char *fileName);

void mutex_lock_enqueue(Heap *,int,RCB *);
RCB *mutex_lock_dequeue();

void *serve_client_init(void *);
void *serve_client();// a.k.a worker function,  the function that handles all receive jobs, parameter is the interface


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
  
  semaphore = sem_open("/semaphore", O_CREAT, 0644, 1);          // initialize a semaphore for work when network accept
  int num_threads = 0;
  char alg_in[5];
  
  /* check for and process parameters */
  if( ( argc != 4 ) ||
     ( sscanf( argv[1], "%d", &port ) < 1 ) ||
     ( *(strcpy(alg_in, argv[2])) < 1 )     ||                   // decide the algorithm
     ( sscanf(argv[3], "%d", &num_threads) < 1)
     ) {
    printf( "usage: sws <port> <algrithm> <number of threads>\n" );
    return 0;
  }
  algorithm_init(port,alg_in,num_threads);                      // initialize alogrithm
  
  network_init( port );                                         /* init network module */
  printf("\nServer all Green! //initialization complete");
  printf("\nStart waiting for clients request\n\n");
  
  receive_init(&num_threads);
  
  return EXIT_SUCCESS; // return status
}



/* This function takes a file handle to a client, reads in the request,
 *    parses the request, and sends back the requested file.  If the
 *    request is improper or the file is not available, the appropriate
 *    error is sent back.
 * Parameters:
 *             fd : the file descriptor to the client connection
 * Returns: None
 */

void *serve_client_init( void *fdp ) {
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
    printf("Request for file '%s' admitted.\n", req); // get request and file name from client
    
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
      
      RCB *new_RCB = create_RCB(fd,fin, fileName);            // create a RCB
      if (alg_using IS RR)                                    // enqueue to coresponding queue
        mutex_lock_enqueue(rrHeap, new_RCB->priority ,new_RCB);
      else
        mutex_lock_enqueue(topHeap, new_RCB->priority ,new_RCB);
    }
  }
  free(fileName);
  free(buffer);
  return 0;
}

/**
 * serve_client function will is created into thread and
 * wait for a semaphore value. Once semaphore is rose,
 * function pop a RCB from a heap and start sending
 * If there are data remain not send, enqueue back to
 * a heap; or no data left, then free the RCB
 *
 * @return *num_thread: int   number of threads
 */
void *serve_client() {
  char *buffer = malloc(sizeof(char) * MAX_HTTP_SIZE);         // buffer just work as a multiple usage buffer
  memset(buffer, 0, sizeof(char) * MAX_HTTP_SIZE);             // always write/send immediately after assigned value
  RCB *popped_rcb;                                             // the RCB that is poped
  Heap *putBackHeap;                                           // the next heap it is going to be enqueued
  int length;
  int level;
  int minus;
  int passInPrio;
  if (!buffer) {
    perror("Error while allocating memory");
    abort();
  }
  for(;;){
    sem_wait(semaphore);                                       // wait until the there is a job assigned
  
    if(alg_using IS RR) {                                      // identify which queue going to put back next
      putBackHeap = rrHeap;
    } else if (alg_using IS MLFB) {
      if(topHeap->length ISNOT 0){
        level = MIDL;
        putBackHeap = midHeap;
      } else {
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
   
    popped_rcb = mutex_lock_dequeue();                        // pop, get rcb
    
    minus = (popped_rcb->rcb_data_remain>popped_rcb->quantum)?popped_rcb->quantum:popped_rcb->rcb_data_remain; // identify the length it is going to reduce
    popped_rcb->rcb_data_remain -= minus;
    printf("Sent %d %s.\n",minus,popped_rcb->file_name);
    length = fread(buffer, 1, minus, popped_rcb->rcb_file);   // read file
    if(length < 0) {
      printf("Error writing content to client\n");
      abort();
    }
    write(popped_rcb->rcb_fd, buffer,length);                 // write to client
    printf("%s",buffer);
    
    if(popped_rcb->rcb_data_remain > 0){                      // enqueue only if there is no data remain needs to be sent
      if(alg_using IS MLFB)
         popped_rcb->quantum = (level IS MIDL)? MID_QUEUE_QUANTUM : RR_QUANTUM;
      if(alg_using ISNOT SJF) {
        popped_rcb->priority += seq_num_c+1;
        passInPrio =popped_rcb->priority;
        popped_rcb->priority %= 1024;
      }
      mutex_lock_enqueue(putBackHeap,passInPrio,popped_rcb);
    }else{                                                    // else free the RCB
      printf("File %s transfer complete\n",popped_rcb->file_name);
      memset(buffer,0,MAX_HTTP_SIZE);
      length = sprintf(buffer,"%s finished\n",popped_rcb->file_name);
      
      write(popped_rcb->rcb_fd,buffer,length);
      close(popped_rcb->rcb_fd);
      fclose(popped_rcb->rcb_file);
      free(popped_rcb->file_name);
      free(popped_rcb);
    }
  }
}

/**
 * receive_init() function is the first initialization
 * that create many threads to semaphore wait for the
 * input rcb, and start the function that is wait for
 * sending RCBs
 *
 * @return *num_thread: int   number of threads
 */
void receive_init(int *num_threads) {
  pthread_t CCT[*num_threads]; // client-connect thread
  for (int i = 0; i < *num_threads; i++)
    pthread_create(&CCT[i], NULL, serve_client, NULL);
  
  create_RCB_init();
  
  for (int i = 0; i < *num_threads; i++)
    pthread_join(CCT[i], NULL); // for a successful join back of all threads
}

/**
 * create_RCB_init function create an infinite loop that
 * wait for network and put into another thread and receive
 *
 * @return none
 */
void create_RCB_init(){
  int fd = 0;
  for(;;) {
    network_wait();
    for (fd = network_open(); fd >= 0; fd = network_open()) {
      pthread_t t ;
      int *fdp = (int *) malloc(sizeof(int));
      *fdp = fd;
      pthread_create(&t, NULL, serve_client_init, (void *)fdp);
      pthread_join(t,NULL);
    }
  }
}

/**
 * create_RCB function takes in enough information to
 * encapsulate a RCB struct and return the object.
 *
 * @param fd: int           file descriptor
 * @param *inputFile: FILE  pointer to file
 * @param *fileName: char   file name
 * @return the recutrn rcb
 */
RCB *create_RCB(int fd, FILE *inputFile, char *fileName) {
  RCB *new_RCB = (RCB *)malloc(sizeof(RCB));
  new_RCB->file_name = malloc(MAX_HTTP_SIZE);
  new_RCB->file_name = strcpy(new_RCB->file_name, fileName);
  new_RCB->rcb_fd = fd;
  new_RCB->rcb_file = inputFile;
  new_RCB->rcb_seq_num = seq_num_c++;
  seq_num_c %= 1024;
  fseek(new_RCB->rcb_file, 0, SEEK_END);
  int length = (int)ftell(new_RCB->rcb_file); //get file size
  rewind(new_RCB->rcb_file);
  
  new_RCB->rcb_data_remain = length;
  
  if(alg_using IS SJF){
    new_RCB->quantum = length;
    new_RCB->priority = length;
  } else if (alg_using IS RR) {
    new_RCB->quantum = RR_QUANTUM;
    new_RCB->priority = UNIFORM + seq_num_c;
  } else if (alg_using IS MLFB) {
    new_RCB->quantum = TOP_QUEUE_QUANTUM;
    new_RCB->priority = UNIFORM + seq_num_c;
  }
  return new_RCB;
}

/**
 * implementation base on heap function addRCB.
 * Adding mutex lock prevent from the race condition
 *
 * @param *h: Heap         Target hep to add
 * @param p: int           Priority
 * @param *c: RCB          RCB object
 * @return none
 */
void mutex_lock_enqueue(Heap *h, int p,RCB *c) {
  pthread_mutex_lock(&mutex);  
  addRCB(h, p, c);
  pthread_mutex_unlock(&mutex);
  sem_post(semaphore);                          // rise a semaphore, start send file
}

/**
 * implementation base on heap function pop.
 * Adding mutex lock prevent from the race condition
 *
 * @return the recutrn rcb
 */
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

/**
 * algorithm_init function takes in several information and print
 * out. Information includes port number, algorithm abbreviation,
 * and the number of thread
 *
 * @param port: int        integer port number
 * @param *alg_in: char    command line argument about algorithm
 * @param num_thread: int  number of threads
 * @return none
 */
void algorithm_init(int port, char *alg_in, int num_threads) {
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
