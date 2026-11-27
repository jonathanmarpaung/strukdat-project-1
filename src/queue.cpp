#include "queue.h"

void initializeQueue(Queue& queue) {
  queue.head = nullptr;
  queue.tail = nullptr;
}

bool isQueueEmpty(Queue queue) {
  return queue.head == nullptr && queue.tail == nullptr;
}

QueueAddress frontQueue(Queue queue) {
  return queue.head;
}

int sizeQueue(Queue queue) {
  QueueAddress current;
  int counter;

  counter = 0;
  current = queue.head;

  while(current != nullptr) {
    counter = counter + 1;
    current = current->next;
  }

  return counter;
}

void enqueue(Queue& queue, QueueAddress new_queue) {
  new_queue->next = nullptr;
  new_queue->prev = nullptr;

  if(isQueueEmpty(queue)) {
    queue.head = new_queue;
    queue.tail = new_queue;
  } else {
    queue.tail->next = new_queue;
    new_queue->prev = queue.tail;
    queue.tail = new_queue;
  }
}

QueueAddress dequeue(Queue& queue) {
  QueueAddress front;

  front = frontQueue(queue);

  if(!isQueueEmpty(queue)) {
    queue.head = queue.head->next;

    if(queue.head == nullptr) {
      queue.tail = nullptr;
    } else {
      queue.head->prev = nullptr;
    }

    front->next = nullptr;
    front->prev = nullptr;
  }

  return front;
}
