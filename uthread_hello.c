/*
 * Simple hello world test
 *
 * Tests the creation of a single thread and its successful return.
 */

#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int hello(void* arg)
{
	printf("Hello world!\n");
	//uthread_exit(4);
	return 0;
}

int main(void)
{
	uthread_t tid;
	printf("going to create\n");
	tid = uthread_create(hello, NULL);
	uthread_join(tid, NULL);

	return 0;
}
