// File:	mypthread.c

// List all group member's name: David Gbogi,
// username of iLab: dog12,
// iLab Server: ilab.cs.rutgers.edu

#include "mypthread.old.h"

// INITIALIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE
#define STACKSIZE 4096

struct threadControlBlock *tcb;   /* thread control block */
ucontext_t sched_ucp;             /* scheduler context */
int sucp_init = 0;                /* variable to check if sched_ucp is initialized */
queue_ join_queue;                /* thread join queue */


/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr,
                      void *(*function)(void*), void * arg) {
   // create Thread Control Block
   // create and initialize the context of this thread
   // allocate space of stack for this thread to run
   // after everything is all set, push this thread int

   // YOUR CODE HERE
   tcb_init(thread, function, arg);
   if(sucp_init == 0) {
       getcontext(&sched_ucp);
       sched_ucp.uc_stack.ss_sp = malloc(STACKSIZE);
       sched_ucp.uc_stack.ss_size = STACKSIZE;
       sched_ucp.uc_stack.ss_flags = 0;
       makecontext(&sched_ucp, schedule, 1);
       sucp_init = 1;
   }
   return 0;
};

/* give CPU possession to other user-level threads voluntarily */

int mypthread_yield() {

	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// switch from thread context to scheduler context

	// YOUR CODE HERE
    if(tcb->state != READY) {
        return -1;
    }
    tcb->state = RUNNING;
    swapcontext(&tcb->ucp, &sched_ucp);

    return 0;
};

/* terminate a thread */
void mypthread_exit(void *ret_val) {
	// Deallocated any dynamic memory created when starting this thread

	// YOUR CODE HERE
    tcb->state = JOIN;
    tcb_clean();

    // TODO find a way to handle if thread exits before mypthread_join is called.
    node_ *node = join_queue.head;
    while(node != NULL) {
        if(node->th_id == tcb->th_id) {
            node->ret_val = ret_val;
            node->ptcb->state = READY;
            tcb->state = DEAD;
            break;
        }
        node = node->next;
    }
};


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {

	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread

	// YOUR CODE HERE
    enqueue_head(&join_queue, node_create(thread, tcb, value_ptr));
    tcb->state = BLOCKED;
    while(tcb->state == BLOCKED) {
        /* wait until scheduler confirms join and changes state */
        mypthread_yield();
    }

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
	// Every time when timer interrupt happens, your thread library
	// should be context switched from thread context to this
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

/* initialize tcb */
void *tcb_init(mypthread_t *thread, void *(*function)(void*), void * arg) {
    getcontext(&tcb->ucp);
    tcb->th_id = *thread;
    tcb->ucp.uc_stack.ss_sp = malloc(STACKSIZE);
    tcb->ucp.uc_stack.ss_size = STACKSIZE;
    tcb->ucp.uc_flags = 0;
    makecontext(&tcb->ucp, (void (*)(void)) function, 2, arg);
    tcb->state = READY;
    tcb->priority = 0;
}

/* cleanup tcb */
void *tcb_clean() {
    free(tcb->ucp.uc_stack.ss_sp);
}

/* create node */
struct node_ *node_create(mypthread_t thread, threadControlBlock *ptcb, void *ret_val) {
    node_ *node = malloc(sizeof(node_));
    if(node == NULL) {
        perror("node_create(): failed to allocate node.");
    }
    node->th_id = thread;
    node->ptcb = ptcb;
    node->ret_val = ret_val;
    node->next = NULL;
    node->prev = NULL;
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

/* add node to front of queue */
void enqueue_head(queue_ *q, node_ *node) {
    node->next = q->head;
    q->head->prev = node;
    q->head = node;
}

/* add node to back of queue */
void enqueue_rear(queue_ *q, node_ *node) {
    node->prev = q->rear;
    q->rear->next = node;
    q->rear = node;
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