#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>

#include "context.h"
#include "preempt.h"
#include "queue.h"
#include "uthread.h"

#define INIT 0
#define READY 1
#define RUNNING 2
#define FINISHED 3
#define WAITING 4



struct TCB {
	int blockState; 
	int retValue;
	int* retCon;
	uthread_ctx_t *ctx;  
	uthread_t TID;
	uthread_t collected;
	uthread_t collecting;
};

queue_t library;

int returned;
struct TCB *currentBlock;
uthread_t TID;


int uthread_start(int preempt)
{
	returned = uthread_create(preempt);
	return 0;
}

int uthread_stop(void)
{
	uthread_exit(returned);
	return 0;
}

/*
* find currentBlock
* return 1 if successful
* return 0 to continue iterating
*/
int get_running_block(queue_t lib, void* block, void* arg)
{   	
	struct TCB *curr = (struct TCB *) block;
	
	if(curr->blockState == RUNNING)
	{
		currentBlock = (struct TCB *) block;
		return 1;
	}
	else {
		return 0;		
	}
	
}


/*
* find block via TID
*/

int find_TID(queue_t lib, void* block, void* arg)
{

	struct TCB *curBlock = (struct TCB *) block;

	if(curBlock->TID == TID)
		return 1;
	else {
		return 0;
	}
	

}

/*
* search lib for node, retrieve TCB/pointer for next node
*/


int  find_next(queue_t lib, void* tBlock, void*arg)
{

	int* flag = (int *) arg;
	struct TCB* block = (struct TCB *) tBlock;

	if(block->TID == currentBlock->TID)
	{
		*flag = 1;
		return 0;
	}
	
	if(*flag == 1)
	{
		if(block->blockState == READY)
			return 1;
	}

	
	return 0;
}

/*
* yields current active thread for other threads to execute
*/

void uthread_yield(void)
{
	int flag = 0;
	queue_func_t findNext = &find_next;	
	struct TCB* next;

	int iterator = queue_iterate(library, findNext, (void*)&flag, (void**)&next);
	
	if(iterator != 0)
		queue_iterate(library, findNext, (void*)&flag, (void**)&next);
	
	uthread_ctx_switch(currentBlock->ctx, next->ctx);
	if(currentBlock->blockState == RUNNING)
		currentBlock->blockState = READY;
	next->blockState = RUNNING;
	currentBlock = next;
	
	if(currentBlock->collecting != -1)
		uthread_join(currentBlock->collecting, currentBlock->retCon);
		
}

uthread_t uthread_self(void)
{
	queue_func_t func = &get_running_block;
  
	int iter = queue_iterate(library, func, NULL, NULL);
	
	if(iter == 0)
		return currentBlock->TID;

	return 0; 
		

}

int uthread_create(uthread_func_t func)
{
	int create;
	struct TCB *newBlock = (struct TCB*)malloc(sizeof(struct TCB));
	if(library == NULL)
	{
		struct TCB *mainBlock = (struct TCB*) malloc(sizeof(struct TCB));
		library = queue_create();	
		queue_enqueue(library, mainBlock);
		mainBlock->TID = queue_length(library) - 1;
		mainBlock->collecting = -1;
		mainBlock->collected = -1;
		mainBlock->blockState = RUNNING;
		mainBlock->ctx = malloc(sizeof(uthread_ctx_t));
		currentBlock = mainBlock;
		
	}
	
	void *stack = uthread_ctx_alloc_stack();
	newBlock->ctx = malloc(sizeof(uthread_ctx_t));

	create = uthread_ctx_init(newBlock->ctx, stack, func);
	
	if (create == 0)
	{
		queue_enqueue(library, newBlock);
		newBlock->TID = queue_length(library) - 1;
		newBlock->blockState = READY;
		newBlock->collecting = newBlock->collected = -1;
		return newBlock->TID;
	} 
	

	return -1;

}

void uthread_exit(int retval)
{
		
		if(currentBlock->blockState != RUNNING) 
		{
			uthread_self();
		}

		currentBlock->blockState = FINISHED;
		currentBlock->retValue = retval;  
		
		
		if(currentBlock->collected != -1)
		{
			queue_func_t finding = &find_TID;
			struct TCB* collectingBlock;
			
			queue_iterate(library, finding, (void*)&currentBlock->collected, 
				(void**)&collectingBlock);
			collectingBlock->blockState = READY;

		}
	
		uthread_yield();
		
}

int uthread_join(uthread_t tid, int *retval)
{
	queue_func_t func = &find_TID;
	struct TCB * zombieTCB;
	TID = tid;
	int iter = queue_iterate(library, func, NULL ,(void**) &zombieTCB);
	
	if( tid <= 0 || tid == currentBlock->TID || zombieTCB->collected <= queue_length(library) || iter != 0)
		return -1;
	if(zombieTCB->blockState == FINISHED)
	{
		if(retval)
			*retval = zombieTCB->retValue;

		uthread_ctx_destroy_stack(zombieTCB->ctx->uc_stack.ss_sp);
		currentBlock->collecting = -1;
		zombieTCB->collected = currentBlock->TID;
	}	
	else if(zombieTCB->blockState != FINISHED)
	{
		zombieTCB->collected = currentBlock->TID;
		currentBlock->blockState = WAITING;
		currentBlock->retCon = retval;
		uthread_yield();
	}
	return 0;
}

