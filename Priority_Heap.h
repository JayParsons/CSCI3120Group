/*
 * Date: July. 27th
 * CSCI3120 - Operating System - Group project
 * Priority_Heap.h
 *
 * Is the header file for Priority_Heap.c
 * Is the file declares the structure of Heap, RCB, function that can operate on heap
 * Function details see header comment in the file.
 *
 */

#ifndef Priority_Heap_h
#include <stdio.h>

typedef struct {
  int rcb_seq_num;
  int rcb_fd;
  FILE *rcb_file;
  int rcb_data_remain;
  int quantum; // may remove
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
#endif
