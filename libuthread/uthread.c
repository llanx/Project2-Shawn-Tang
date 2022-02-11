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


queue_t library;

struct TCB {
	uthread_ctx_t *ctx;  
	uthread_t TID;
	int state; 
	int retVal;
	int* retContain;
	uthread_t collected;
	uthread_t collecting;
};

struct TCB *currentBlock;
uthread_t TID;

/*
* find currentBlock
* return 1 if successful
* return 0 to continue iterating
*/
int get_running_block(queue_t lib, void* block, void* arg)
{   	
	struct TCB *curr = (struct TCB *) block;
	
	if(curr->state == RUNNING)
	{
		currentBlock = (struct TCB*) block;
		return 1;
	}

	return 0;
}


/*
* find block via TID
*/

int find_TID(queue_t lib, void* block, void* arg)
{

	struct TCB *curBlock = (struct TCB *) block;

	if(curBlock->TID == TID)
		return 1;

	return 0;

}

/*
* search lib for node, retrieve TCB/pointer for next node
*/


int  find_next(queue_t lib, void* tBlock, void*arg)
{

	int* flag = (int*) arg;
	struct TCB* block = (struct TCB*) tBlock;

	if(block->TID == currentBlock->TID)
	{
		*flag = 1;
		return 0;
	}
	
	if(*flag == 1)
	{
		if(block->state == READY)
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

	int iter = queue_iterate(library, findNext, (void*)&flag, (void**)&next);
	
	if(iter != 0)
		queue_iterate(library, findNext, (void*)&flag, (void**)&next);
	preempt_disable();
	uthread_ctx_switch(currentBlock->ctx, next->ctx);
	preempt_enable();

	if(currentBlock->state == RUNNING)
		currentBlock->state = READY;
	next->state = RUNNING;
	currentBlock = next;
	
	if(currentBlock->collecting != -1)
		uthread_join(currentBlock->collecting, currentBlock->retContain);
		
}

uthread_t uthread_self(void)
{
	queue_func_t func = &get_running_block;
  
	int iter = queue_iterate(library, func, NULL, NULL);
	
	if(iter == 0)
		return currentBlock->TID;

	return 0; 
		

}

int uthread_create(uthread_func_t func, void *arg)
{
	int create;
	struct TCB *tcBlock = (struct TCB*)malloc(sizeof(struct TCB));
	if(library == NULL)
	{
		struct TCB *mainBlock = (struct TCB*) malloc(sizeof(struct TCB));
		library = queue_create();	
		queue_enqueue(library, mainBlock);
		mainBlock->TID = queue_length(library) - 1;
		mainBlock->collecting = -1;
		mainBlock->collected = -1;
		mainBlock->state = RUNNING;
		mainBlock->ctx = malloc(sizeof(uthread_ctx_t));
		currentBlock = mainBlock;
		
	}
	
	void *stack = uthread_ctx_alloc_stack();
	tcBlock->ctx = malloc(sizeof(uthread_ctx_t));

	create = uthread_ctx_init(tcBlock->ctx, stack, func, arg);
	
	if (create == 0)
	{
		queue_enqueue(library, tcBlock);
		tcBlock->TID = queue_length(library) - 1;
		tcBlock->state = READY;
		tcBlock->collecting = tcBlock->collected = -1;
		return tcBlock->TID;
	} 
	

	return -1;

}

void uthread_exit(int retval)
{
		
		if(currentBlock->state != RUNNING) 
		{
			uthread_self();
		}

		currentBlock->retVal = retval;  
		currentBlock->state = FINISHED;
		
		if(currentBlock->collected != -1)
		{
			struct TCB* collectingBlock;
			queue_func_t finding = &find_TID;

			queue_iterate(library, finding, (void*)&currentBlock->collected, 
				(void**)&collectingBlock);
			collectingBlock->state = READY;

		}
	
		uthread_yield();
		
}

int uthread_join(uthread_t tid, int *retval)
{
	queue_func_t func = &find_TID;
	struct TCB * zombieTCB;
	TID = tid;
	int iter = queue_iterate(library, func, NULL ,(void**) &zombieTCB);
	
	if( tid == currentBlock->TID || tid <= 0 || iter != 0 || zombieTCB->collected <=  queue_length(library) )
		return -1;
	if(zombieTCB->state == FINISHED)
	{
		if(retval)
			*retval = zombieTCB->retVal;

		uthread_ctx_destroy_stack(zombieTCB->ctx->uc_stack.ss_sp);
		currentBlock->collecting = -1;
		zombieTCB->collected = currentBlock->TID;
	}	
	else if(zombieTCB->state != FINISHED)
	{
		zombieTCB->collected = currentBlock->TID;
		currentBlock->state = WAITING;
		currentBlock->retContain = retval;
		uthread_yield();
	}
	return 0;
}

