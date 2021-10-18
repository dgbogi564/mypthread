// File:	mypthread.c

// List all group member's name: David Gbogi,
// username of iLab: dog12,
// iLab Server: ilab.cs.rutgers.edu

#include "mypthread.h"

// INITIALIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE
#define STACK_SIZE 256

struct threadControlBlock *tcb;   /* thread control block */
int id_counter = 0;               /* counter for assigning threads their individual IDs */


/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr,
                      void *(*function)(void*), void * arg) {
   // create Thread Control Block
   // create and initialize the context of this thread
   // allocate space of stack for this thread to run
   // after everything is all set, push this thread int
   // YOUR CODE HERE
   tcb = malloc(sizeof(threadControlBlock));
   tcb->ucp->uc_stack.ss_sp = calloc(sizeof(stack_t), STACK_SIZE);
   tcb->ucp->uc_stack.ss_size = STACK_SIZE;
   tcb->ucp->uc_link = NULL;
   makecontext(tcb->ucp, (void (*)(void)) function, 1, arg);

   tcb->id = id_counter++;
   tcb->state = READY;
   tcb->priority = 0;

   return 0;
};

/* give CPU possession to other user-level threads voluntarily */
int mypthread_yield() {

	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// switch from thread context to scheduler context

	// YOUR CODE HERE
    tcb->state = RUNNING;
};

/* terminate a thread */
void mypthread_exit(void *value_ptr) {
	// Deallocated any dynamic memory created when starting this thread

	// YOUR CODE HERE
};


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {

	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread

	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex) {
        // use the built-in test-and-set atomic function to test the mutex
        // if the mutex is acquired successfully, enter the critical section
        // if acquiring mutex fails, push current thread into block list and //
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex) {
	// Release mutex and make it available again.
	// Put threads in block list to run queue
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in mypthread_mutex_init

	return 0;
};

/* scheduler */
static void schedule() {
	// Every time when timer interrup happens, your thread library
	// should be contexted switched from thread context to this
	// schedule function

	// Invoke different actual scheduling algorithms
	// according to policy (STCF or MLFQ)

	// if (sched == STCF)
	//		sched_stcf();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

// schedule policy
#ifndef MLFQ
	// Choose STCF
#else
	// Choose MLFQ
#endif

}

/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf() {
	// Your own implementation of STCF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

// Feel free to add any other functions you need

// YOUR CODE HERE
/* create node */
struct node_ *node_create() {
    node_ *node = malloc(sizeof(node_));
    if(node == NULL) {
        perror("node_create(): failed to allocate node.");
    }
    return node;
}

/* destroy node */
void *node_destroy(node_ *node) {
    free(node);
}

/* create queue */
struct queue_ *queue_create() {
    queue_ *q = malloc(sizeof(queue_));
    if(q == NULL) {
        perror("queue_create(): failed to allocate node.");
    }
    return q;
}

/* pop head of queue */
struct node_* pop_head(queue_ *q) {
    node_ *node = q->head;
    q->head = node->next;
    q->head->prev = NULL;
    return node;
}

/* pop rear of queue */
struct node_* pop_rear(queue_ *q) {
    node_ *node = q->rear;
    q->rear = node->prev;
    q->rear->next = NULL;
    return node;
}

/* destroy queue */
void *queue_destroy(queue_ *q) {
    while(q->size > 0) {
        free(pop_head(q));
    }
    free(q);
}