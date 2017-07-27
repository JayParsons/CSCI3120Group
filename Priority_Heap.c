/*
 * Date: July. 27th
 * CSCI3120 - Operating System - Group project
 *
 * Priority_Heap.c
 * -------<before start>------
 * Problem: The designation of a Priority Heap in this project is a total
 * failure and miscalculation. The purpose of designing a heap in
 * this project is for using only the data structure array in
 * c, instead of linked list, and avoiding using a normal heap that
 * may cause a complex malloc() and realloc() issue or memory leak.
 * However, in the real programming iteration. We found a heap cannot
 * perfectly do its job as a priority queue: Especially in Multilevel
 * Feedback Queue and Round Robin algorithms, the priorities are
 * identical but requires every RCB blocks to be put to the end. A
 * heap, here, will, put the RCB to either the beginning or the end
 * Yet, it is not able to pick anything in the right branch of the
 * tree.
 * Solution: In sws.c file, we manually change the priority in every
 * iteration, by adding priority itself and the sequence number.
 *     priority += seq_num_c;
 * In addition, for avoiding the overflow on integer priority, we add
 * another modulo statement after it, namely,
 *     priority %= 1024
 * Tile here, the main problem of the priority in MLFB and RR is solved
 * ---<"before start" ends>---
 * Priority_Heap is designed for acting as a priority queue in the
 * project. It supports mainly two operation addRCB() and pop(), and
 * init_heap() before every heap is used. (See function comment for detail)
 *
 */

#include "Priority_Heap.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * get method of geting the size of this heap
 *
 * @param *h: Heap    this heap
 * @return size of this heap
 */
int size(Heap *h) {
  return h->length;
}

/**
 * init_heap() function will pass in a heap struct
 * and the function will initialize it. Since heap
 * itself is a pointer, no need to return anything
 *
 * @param *h: Heap          this heap
 * @return none
 */
void init_heap(Heap *h){
  h->head = malloc(sizeof(RCB)*4);
  h->size = 4;
  h->length = 0;
}

/**
 * addRCB function will be past in a heap, an integer, a RCB
 * that function will add the RCB into the heap with its
 * priority
 *
 * @param *h: Heap          this heap
 * @param priority: int     the priority of this RCB
 * @param *new_RCB: RCB     the added RCB
 * @return none
 */
void addRCB(Heap *h,int priority,RCB *new_rcb) {
  
  if(h->length+1 > h->size) { // if heap exceed
    h->size = h->size*2;
    h->head = (RCB *) realloc(h->head, h->size * sizeof(RCB));
  }
  h->head[h->length] = *cpy(new_rcb);

  h->length += 1;
  int index = h->length - 1;
  int pindex = (index - 1) / 2; // parent index
  RCB *parent = (h->head) + pindex;
  
  while(index > 0 && new_rcb->priority < parent->priority) {
    h->head[index] = *parent;
    h->head[pindex] = *new_rcb;
    index = pindex;
    pindex = (index - 1) / 2;
    parent = (h->head) + pindex;
  }
}

/**
 * pop() function will pop the most important RCB in this
 * heap.
 *
 * @param *h: Heap          this heap
 * @return a RCB with smallest(most important) priority
 */
RCB *pop(Heap *h){

  if(h->length == 0) {
    printf("Error, empty heap, return null\n");
    fflush(stdout);
    return NULL;
  }
  
  RCB *ret_rcb = cpy( h->head);
  RCB item = h->head[h->length-1];
  (h->length)--;

  if(h->length  == 0) {
    return ret_rcb;
  }

  *h->head = item;
  RCB maxChild;
  int found, index, lIndex, rIndex,maxIndex;
  found = 0;
  index = 0;
  lIndex = index * 2 + 1;
  rIndex = index * 2 + 2;
  
  while(!found) {
    if(lIndex < h->length && rIndex < h->length) {
      if(((h->head)+lIndex)->priority <= ((h->head)+rIndex)->priority){
        maxChild = (h->head) [ lIndex];
        maxIndex = lIndex;
      } else {
        maxChild = (h->head) [ rIndex];
        maxIndex = rIndex;
      }
      
      if(item.priority >= maxChild.priority) {
        *(h->head+maxIndex) = item;
        *(h->head+index) = maxChild;
        index = maxIndex;
      } else
        found = 1;
    } else if (lIndex <= h->length) {
      if(item.priority >= (h->head+lIndex)->priority) {
        *(h->head+index) = *(h->head+lIndex);
        *(h->head+lIndex) = item;
        index = lIndex;
      } else found = 1;
    } else found = 1;
    lIndex = index * 2 + 1;
    rIndex = index * 2 + 2;
  }
  
  return ret_rcb;
}

/**
 * enumerate() function will enumerate this heap
 *
 * @param *h: Heap         this heap
 * @return none
 */
void enumerate(Heap *h) {
  if(h == NULL ) return;
  if(h->length == 0) return;
  
  for (int i = 0;i<h->length;i++) {
    printf("%d ",(h->head+i)->priority);
  }
  printf("\n");
}

/**
 * cpy() is a function take an RCB struct pointer in
 * and create a new RCB, copy every information
 * And return as a copy.
 *
 * @param *o: RCB          this RCB
 * @return the copy RCB
 */
RCB *cpy(RCB *o){
  RCB *n = malloc(sizeof(RCB));
  n->file_name=o->file_name;
  n->priority=o->priority;
  n->quantum=o->quantum;
  n->rcb_data_remain=o->rcb_data_remain;
  n->rcb_fd=o->rcb_fd;
  n->rcb_file=o->rcb_file;
  n->rcb_seq_num =o->rcb_seq_num;
  return n;
}

