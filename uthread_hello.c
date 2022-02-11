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
	uthread_exit(4);
	return 0;
}

int main(void)
{
	uthread_t tid;
	tid = uthread_create(hello);
	uthread_join(tid, NULL);
	return 0;
}
