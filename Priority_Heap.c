//
//  Heap.c
//
//
//  Created by ZhangZhiyuan on 16/07/2017.
//
//

#include "Priority_Heap.h"
#include <stdio.h>
#include <stdlib.h>

int size(Heap *h) {
  return h->length;
}

void init_heap(Heap *h){
  
  h->head = malloc(sizeof(RCB)*8);
  
  h->size = 4;
  h->length = 0;

}

void addRCB(Heap *h,int priority,RCB *new_rcb) {
  if(h->length+1 > h->size) { // if heap exceed
    
    h->size = h->size*2;
    h->head = (RCB *) realloc(h->head, h->size * sizeof(RCB));
  }
  
  *(h->head+h->length) = *new_rcb;
  h->length++;
  int index = h->length - 1;
  int pindex = (index - 1) / 2; // parent index
  RCB *parent = (h->head) + pindex;
  
  while(index > 0 && new_rcb->priority > parent->priority) {
    h->head[index] = *parent;
    h->head[pindex] = *new_rcb;
    index = pindex;
    pindex = (index - 1) / 2;
    parent = (h->head) + pindex;
  }
  //h->length++;
}

RCB *pop(Heap *h){
  if(h->length == 0) {
    printf("Error, empty heap, return null\n");
    fflush(stdout);
    //fflush();
    return NULL;
  }
  RCB *ret_rcb = cpy( h->head);
  RCB item = h->head[h->length-1];
  (h->length)--;
  if(h->length  == 0) {
    h->head = NULL;
    
    return ret_rcb;
  }
  //(h->head)+(h->length)-1);
  *h->head = item;
  RCB maxChild;
  int found, index, lIndex, rIndex,maxIndex;
  found = 0;
  index = 0;
  lIndex = index * 2 + 1;
  rIndex = index * 2 + 2;
  while(!found) {
    if(lIndex < h->length && rIndex < h->length) {
      if(((h->head)+lIndex)->priority > ((h->head)+rIndex)->priority){
        maxChild = (h->head) [ lIndex];
        maxIndex = lIndex;
      } else {
        maxChild = (h->head) [ rIndex];
        maxIndex = rIndex;
      }
      
      if(item.priority < maxChild.priority) {
        *(h->head+maxIndex) = item;
        *(h->head+index) = maxChild;
        index = maxIndex;
      } else
        found = 1;
    } else if (lIndex < h->length) {
      if(item.priority < (h->head+lIndex)->priority) {
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

void enumerate(Heap *h) {
  for (int i = 0;i<h->length;i++) {
    printf("%d ",(h->head+i)->priority);
  }
  printf("\n");
}

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

