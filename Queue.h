#ifndef QUEUE_H
#define QUEUE_H

#include <ucontext.h>

typedef struct myThread 
{ 
ucontext_t ucontext; 
struct myThread *parent;
struct tQueue *child;
struct myThread *is_joined;
int is_all_joined;
}myThread; 

typedef struct queue
{
	struct myThread *data;
    struct queue *next;
}queue;

typedef struct tQueue
{
    struct queue *front;
}tQueue;

typedef struct semaphore{
	int semaphore_count;
	struct tQueue *blocked_semaphores;
	}semaphore;

void enqueue(myThread *data, tQueue *queue_t);
myThread* dequeue(tQueue *queue_t);
void find(myThread *data, tQueue *queue_t);
int is_present(myThread *data, tQueue *queue_t);

#endif /* QUEUE_H */