#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "mythread.h"
#include "Queue.h"
#define MEM 8192


ucontext_t *main_con; 
ucontext_t *curr;
ucontext_t *new;
ucontext_t *temp;

tQueue *readyQueue;
tQueue *blockQueue;

myThread *currentThread = NULL;

semaphore *sem_ptr = NULL;

void MyThreadInit (void(*start_funct)(void *), void *args)
{
	main_con = (ucontext_t*) malloc(sizeof(ucontext_t));
	curr = (ucontext_t*) malloc(sizeof(ucontext_t));
	getcontext(main_con);
	getcontext(curr);

	curr->uc_link=0;
 	curr->uc_stack.ss_sp=malloc(MEM);
	curr->uc_stack.ss_size=MEM;	
	makecontext(curr, (void(*)())start_funct , 1, args);
 	
 	currentThread = (struct myThread*)malloc(sizeof(struct myThread));	
 	currentThread->ucontext = *curr;
	currentThread->parent = NULL;
	currentThread->child = (struct tQueue*)malloc(sizeof(struct tQueue));
	currentThread->child->front = NULL;
	currentThread->is_joined = NULL;
	currentThread->is_all_joined = 0;

	readyQueue = (struct tQueue*)malloc(sizeof(struct tQueue));
	readyQueue->front = NULL;
	blockQueue = (struct tQueue*)malloc(sizeof(struct tQueue));
	blockQueue->front = NULL;

	swapcontext(main_con, curr);
}


MyThread MyThreadCreate(void(*start_funct)(void *), void *args)
{
	struct myThread *newThread;
	getcontext(curr);
	curr->uc_link=0;
 	curr->uc_stack.ss_sp=malloc(MEM);
	curr->uc_stack.ss_size=MEM;	
	makecontext(curr, (void(*)())start_funct , 1, args);
 
 	newThread = (struct myThread*)malloc(sizeof(struct myThread));	
 	newThread->ucontext = *curr;
	newThread->parent = currentThread;
	newThread->child = (struct tQueue*)malloc(sizeof(struct tQueue));
	newThread->child->front = NULL;
	newThread->is_joined = NULL;
	newThread->is_all_joined = 0;

	enqueue(newThread, currentThread->child);
	enqueue(newThread, readyQueue);

	return (MyThread)newThread;

}

void MyThreadYield(void)
{
	struct myThread *temp_thread;
	temp_thread = dequeue(readyQueue);
	if (temp_thread != NULL)
	{
		enqueue(currentThread, readyQueue);
		struct myThread *prevThread = currentThread;
		currentThread = temp_thread;
		swapcontext(&(prevThread->ucontext), &(temp_thread->ucontext));
	}
}

int MyThreadJoin(MyThread child)
{
	myThread *childNew = (myThread*) child;
	if (is_present(childNew, currentThread->child))
	{
		if ((is_present(childNew, readyQueue)) || (is_present(childNew, blockQueue)))
		{
			struct myThread *child_thread = currentThread;
			currentThread->is_joined = childNew;
			enqueue(currentThread, blockQueue);
			currentThread = dequeue(readyQueue);
			swapcontext(&(child_thread->ucontext), &(currentThread->ucontext));
		}
		else
			return 0;
	}
	else
		return -1;
}

void MyThreadJoinAll(void)
{
	if(currentThread->child->front != NULL){
		currentThread->is_all_joined = 1;
		enqueue(currentThread, blockQueue);
		struct myThread *child_thread = currentThread;
		currentThread = dequeue(readyQueue);
		swapcontext(&(child_thread->ucontext), &(currentThread->ucontext));
	}
}

void MyThreadExit(void)
{
	struct myThread *parent = currentThread->parent;
	if(parent != NULL) {
		find(currentThread, parent->child);
		currentThread->parent = NULL;
	}

	struct tQueue *child = currentThread->child;
	struct queue *front = child->front;
	while(front != NULL) {
		front->data->parent = NULL;
		front = front->next;
	}

	if(parent != NULL && is_present(parent, blockQueue)) {
		if(parent->is_joined == currentThread)
		{
			parent->is_joined = NULL;
			enqueue(parent, readyQueue);
			find(parent, blockQueue);
			currentThread = dequeue(readyQueue);
			if(currentThread != NULL)
			{
				setcontext(&(currentThread->ucontext));
			}
			return;
		}
		else if (parent->is_all_joined == 1)
		{
			if (parent->child->front == NULL)
			{
				parent->is_all_joined = 0;
				enqueue(parent, readyQueue);
				find(parent, blockQueue);
				currentThread = dequeue(readyQueue);
				if(currentThread != NULL)
				{
					setcontext(&(currentThread->ucontext));
				}
				return;
			}
		}
	}
	free((currentThread->ucontext).uc_stack.ss_sp);
	free(currentThread);
	currentThread = dequeue(readyQueue);

	if(currentThread != NULL)
	{
		setcontext(&(currentThread->ucontext));
	}
	else{
		setcontext(main_con);
	}
}

MySemaphore MySemaphoreInit(int initialValue)
{
	sem_ptr = (semaphore*)malloc(sizeof(semaphore));
	if(initialValue < 0){
	return NULL;
	}
	
	sem_ptr->semaphore_count = initialValue;
	tQueue *semblockthread = (tQueue*) malloc(sizeof(tQueue));
	semblockthread->front = NULL;
	sem_ptr->blocked_semaphores = semblockthread;
	return (MySemaphore) sem_ptr;

}

void MySemaphoreSignal(MySemaphore sem) 
{
	sem_ptr = (semaphore*) sem;
	if(sem_ptr == NULL){
		return;
	}
	if (sem_ptr->semaphore_count <= 0) {
		struct myThread *temp = dequeue(sem_ptr->blocked_semaphores);
		if (temp != NULL) {
			enqueue(temp, readyQueue);
		}
	}
	(sem_ptr->semaphore_count)++;
}

void MySemaphoreWait(MySemaphore sem)
{
	sem_ptr = (semaphore*) sem;
	if(sem_ptr == NULL){
		return;
	}
	
	if (sem_ptr->semaphore_count <= 0) 
	{
		enqueue(currentThread, sem_ptr->blocked_semaphores);
		struct myThread *temp = currentThread;
		currentThread = dequeue(readyQueue);
		if(currentThread != NULL)
		{
			swapcontext(&(temp->ucontext), &(currentThread->ucontext));
		}
		else
		{
			setcontext(main_con);
		}
	}
	(sem_ptr->semaphore_count)--;
}

int MySemaphoreDestroy(MySemaphore sem)
{
	sem_ptr = (semaphore*) sem;
	if(sem_ptr != NULL)
	{
		if (sem_ptr->semaphore_count > 0)
		{
			free(sem_ptr->blocked_semaphores);
			free(sem_ptr);
			return 0;
		}
		
	}
	return -1;
}

 
