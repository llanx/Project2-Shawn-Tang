#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "preempt.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

void timer_handle(int alarm);
struct itimerval timer; // struct used for timer functionality
struct sigaction sigact; // struct used to handle signal


void preempt_disable(void)
{
	sigset_t sigset;

	// Create sigset mask to block SIGVTALRM
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	// Critical Code
}

void preempt_enable(void)
{
	sigset_t sigset;

	// Create sigset mask to unblock SIGVTALRM
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &sigset, NULL);

	// Critical Code
}


void preempt_start(void)
{
	
	memset(&sigact, 0, sizeof (sigact)); // set bytes to size of struct
 	sigact.sa_handler = &timer_handle;
	sigaction (SIGVTALRM, &sigact, NULL);

 	timer.it_value.tv_usec = timer.it_interval.tv_usec = HZ*10000; // set intervals to 100 times per second for timer
	setitimer(ITIMER_VIRTUAL, &timer, NULL); // set timer to countdown against user cpu time in process
	timer_handle(SIGVTALRM); 
}

void preempt_stop(void)
{

}

void timer_handle(int alarm)
{
		uthread_yield();
}


