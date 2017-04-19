#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "Queue.h"

void enqueue(myThread *data, tQueue *queue_t)
{   
    queue *tempQueue = (queue*) malloc(sizeof(queue));
    tempQueue->data = data;
    tempQueue->next = NULL;
    if(queue_t->front == NULL)
    {
        queue_t->front = tempQueue;
    }
    else
    {
        queue *current = queue_t->front;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = tempQueue;
    }
}

myThread* dequeue(tQueue *queue_t)
{
    myThread *retval;
    queue *old_node = NULL;

    if (queue_t->front == NULL)
    {
        return NULL;
    }
    old_node = queue_t->front;
    retval = old_node->data;
    queue_t->front = old_node->next;
    free(old_node);
    return retval;
}

void find(myThread *data, tQueue *queue_t)
{
    if(queue_t->front == NULL) {
        return;
    }
    queue *old_node = queue_t->front;
    if(old_node->data == data) {
        queue_t->front = old_node->next;
        free(old_node);
        return;
    }
    while(old_node->next != NULL) {
        if(old_node->next->data == data) {
            queue *freeNode = old_node->next;
            old_node->next = freeNode->next;
            free(freeNode);
            return;
        }
        old_node = old_node->next;
    }
}

int is_present(myThread *data, tQueue *queue_t)
{   queue *front = queue_t->front;
	while(front != NULL)
    {
    	if(front->data == data)
    	{
    		return 1;
    	}
        front = front->next;
    }
    return 0;
}