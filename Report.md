# ECS 150: Project #2 - User-level thread library


### Authors
- Shawn Headley
- Yehan Tang

### Works Cited

A The primary sources used to implement this project include the in-class
 slides, specifically the slides involving threads and the assignment 
ReadMe included in the source files. Outside of class material is listed below.

- To help in the creation of timers/signals: 
http://www.informit.com/articles/article.aspx?p=23618&seqNum=14 and 
https://www.gnu.org/software/libc/manual/html_node/Setting-an-Alarm.html.
- To help in the implementation and structure of the TCB: 
https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.22.2918&rep=rep1&type=pdf
- To create a singularly linked list implementation of a queue: 
https://www.geeksforgeeks.org/queue-linked-list-implementation/

### Project Implementation

#### Phase 1: Queue API Implementation
We began our implementation of a queue using a singularly linked 
list as it abides be the O(1) criteria while also being simple enough to
create that it didn't require an abdundant amount of time to create. As is 
standard, the queue object contains a pointer to the front and rear nodes, 
as well as length tracker for quick insertion and deletion. 

#### Phase 2: uthread API Implementation
This was the most difficult part of the project, we created a Thread Control 
Block(TCB) struct that contained a thread id, a return value from 
thread_exit(), a pointer that receives another return value to communicate
when a thread exists for whichever thread will now operate at, a thread
context pointer utilized for context switches, and two uthread_t values
acting as indicators for joining availablity and needing to be joined. 
Finally we have a state variable that tracks thread status. One thing to
note is there is also a currentBlock TCB initialized globally for access
in a number of function calls that tracks the current block that is
running. This specifically allowed us to create additional implied
functions outside of the header file that would find the correct TID by
iterating through our single queue, find the next block readied up,
and figure out which block is current running. Additionally, the start
function operates by creating a TCB structure with the main thread
being a scheduable thread, with initial TID, context, and a newly
initialized queue for the library to track different threads. 
However, what should have been added was a scheduler that provides
the processor with threads/tasks to complete which was an oversight.
All the exit function does is call uthread_exit() with the return
value of uthread_create after that function is finished and our global
block is set as finished until it becomes the next running block found.

There were many issues that arose during this phase, primarily with
certain (* ) void pointers that were not able to be debugged in time
of submission. As well, the start and exit functions were an afterthought
which was a mistake as we built our functionality into uthread_create
and uthread_exit() first, which was shown to be an enormous issue once
we began using the autograder to test our program. As well, there were
definitely memory leaks from this part of the application that were
not fixed in time.

#### Phase 3: uthread_join() Implementation
This was another difficult task for us, as it took a high conceptualization
of how the thread exit process is supposed to work. Realizing that a
thread must be joined and collected by the next thread before the first
thread resources are freed. We decided to use uthread_t variables within 
the TCB itself called collected and collecting to act as statuses during
the joining process while storing the return value from the call to
uthread_exit() which is utilized by the next block. The primary issue was
extracting this logic into our start and stop functions as we were
unsure if it needed to operate in the same manner.



#### Phase 4: Preemption
This phase was entirely dependent upon our sources used to figure out
how to initialize timers and signals to stop threads from keeping
resources from other threads by not yielding. Utilizing sigaction
was no easy task with testing being an even harder issue altogether.
Once initialized, deciding where to call enable and disable within
uthread.c was also difficult, as there were certain conditions where
it was possible that, during sensitive operations in uthread, 
there is a possibility of shared variables being changed in an
unwanted way. Thus if we would disable preempt before a context switch,
we realized we would need to reenable it by the other thread before it
continues the function. We ended up not using preemption often, as
it led to more errors with than without it.