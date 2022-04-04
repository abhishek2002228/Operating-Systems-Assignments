#include "common_structs.h"
#include <stddef.h>
// max 10 processes can enter the queue to be scheduled
#define SIZE 10 
process items[SIZE];
int front = -1;
int rear = -1;

// Check if the queue is full
int isFull() {
  if ((front == rear + 1) || (front == 0 && rear == SIZE - 1)) return 1;
  return 0;
}

int isEmpty() {
  if (front == -1) return 1;
  return 0;
}

void enqueue(process p) {
  if (rear == SIZE - 1)
    return;
  else {
    if (front == -1)
      front = 0;
    rear++;
    items[rear] = p;
    
  }
}

process dequeue() {
  process proc;
  if (isEmpty()) {
    return proc;
  } else {
    proc = items[front];
    if (front == rear) {
      front = -1;
      rear = -1;
    } else {
      front = (front + 1) % SIZE;
    }
    return proc;
  }
}

