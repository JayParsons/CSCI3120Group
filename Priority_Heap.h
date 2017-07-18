//
//  Heap.h
//  
//
//  Created by ZhangZhiyuan on 16/07/2017.
//
//

#ifndef Priority_Heap_h
#include <stdio.h>

typedef struct {
  int rcb_seq_num;
  int rcb_fd;
  FILE *rcb_file;
  int rcb_data_remain;
  int quantum;
  int priority;
  char *file_name;
  
}RCB;

typedef struct {
  RCB *head;
  int size;
  int length;
}Heap;

int size(Heap *);
void init_heap(Heap *h);
void addRCB(Heap *h,int p,RCB *rcb);
RCB *pop(Heap *h);
void enumerate(Heap *h);
RCB *cpy(RCB *);
#define Priority_Heap_h
#endif /* Heap_h */
