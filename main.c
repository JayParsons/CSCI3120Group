//
//  main.c
//  OSP
//
//  Created by ZhangZhiyuan on 18/07/2017.
//  Copyright © 2017 Zhang. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "Priority_Heap.h"

int main() {
  // insert code here...
  
  
  Heap *a = (Heap *)malloc(sizeof(Heap));
  //a = malloc(sizeof(Heap));
  init_heap(a);
  RCB *here;
  here = (RCB *)malloc(sizeof(RCB)*15);
  
  for(int i = 0;i<15;i++){
    printf("%d \n",i);
    here[i].priority = i;
    addRCB(a, i, here+i);
    //for(int j = 0;j<=i;j++) printf("%d ", ((a->head)+j)->priority);
    
    enumerate(a);
    printf("\n\n");
  }
  
  printf("\nSize: %d\n",a->length);
  enumerate(a);
  printf("\n\n");
  RCB *ha;
  for(int i = 0; i< 18;i++) {
    ha = pop(a);
    if(ha!=NULL)
    printf("%d \n", ha->priority);
  }
  printf("\n");
  
  
  
  RCB *ret = pop(a);
  
  return 0;
}
