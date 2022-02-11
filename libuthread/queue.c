#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "queue.h"



struct queue
{
	int length;
	struct node *front;
	struct node *rear;
};

struct node
{
	void *data;
	struct node *next;
};

struct node* newNode(void *data)
{
	struct node *temp = (struct node*)malloc(sizeof(struct node));
	if (temp == NULL)
		return temp;
	temp->data = data;
	temp->next = NULL;
	return temp;
};


queue_t queue_create(void)
{
	queue_t queue = (queue_t)malloc(sizeof(struct queue));
	queue->length = 0;
	queue->front = NULL;
	queue->rear = NULL;

	return queue;
}

int queue_destroy(queue_t queue)
{	
	if (queue == NULL || queue->front != NULL)
		return -1;
	else 
	{
		free(queue);
		return 0;
	}
}

int queue_enqueue(queue_t queue, void *data)
{
	struct node *tmp = newNode(data);
	tmp = malloc(sizeof(struct node));

	if (tmp == NULL || data == NULL || queue == NULL)
		return -1;
	else if (queue->rear == NULL)
		queue->front = queue->rear = tmp;
	else
	{
		queue->rear->next = tmp;
		queue->rear = tmp;
	}
	queue->length++;
	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	if (data == NULL || queue == NULL || queue->front == NULL)
		return -1;
	data = &queue->front->data;
	struct node* temp = queue->front;
	queue->front = queue->front->next;
	free(temp);
	if (queue->front == NULL)
		queue->rear = NULL;
	queue->length--;
	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	if (data == NULL || queue == NULL)
		return -1;
	struct node* temp = queue->front;
	struct node* prev;
	if (temp != NULL && temp->data == data)
	{
		queue->front = temp->next;
		free(temp);
		if (queue->front == NULL)
		        queue->rear = NULL;
	}
	else
	{
		while (temp != NULL && temp->data != data)
		{
			prev = temp;
			temp = temp->next;
		}
		if (temp == NULL)
			return -1;
		prev->next = temp->next; 
		free(temp);
	}
	queue->length--; 
	return 0;
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	if (queue == NULL || func == NULL || data == NULL)
		return -1;
	
	int iterator;
	struct node* node = queue->front;
	void * curData;

	while(node != NULL)
	{
		curData = node->data;
		iterator = (*func)(queue, node, arg);
		
		if(iterator == 1)
		{
			if (data != NULL)
			{
				*data = curData;
			}
			return 0;
		}
		else if(iterator != 0) 
			return -1; 
	
		node = node->next;		
	}
	
	return -1;
}

int queue_length(queue_t queue)
{
	return queue->length;
}
