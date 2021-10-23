// File:	mypthread.c

// queue all group member's name:
// username of iLab:
// iLab Server:

#include "mypthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE
#define STACKSIZE 4096          /* size of stack */
#define DEFAULT_QUANTUM 5;      /* default time quantum */
schedule_ sched;                /* scheduler information (contains tcb queue) */
int schedule_initialized = 0;   /* variable used to check if the scheduler has been initialized */
tcb_ *curr_tcb;                 /* current tcb */


/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr,
                      void *(*function)(void*), void * arg) {
    // create Thread Control Block
    // create and initialize the context of this thread
    // allocate space of stack for this thread to run
    // after everything is all set, push this thread int
    // YOUR CODE HERE
    if(!schedule_initialized) {
        schedule_init();
    }
    enqueue(sched.q, (curr_tcb = tcb_create(*thread, curr_tcb->join_th, function, arg)), 1);
    return 0;
}

/* give CPU possession to other user-level threads voluntarily */
int mypthread_yield() {

	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// switch from thread context to scheduler context

	// YOUR CODE HERE
    curr_tcb->state = READY;
    swapcontext(&curr_tcb->ucp, &sched.ucp);
	return 0;
}

/* terminate a thread */
void mypthread_exit(void *value_ptr) {
	// Deallocated any dynamic memory created when starting this thread

	// YOUR CODE HERE
    curr_tcb->state = DEAD;
    curr_tcb->ret_val = value_ptr;
    node_ *node = sched.all_tcbs->head;
    while(node != NULL) {
        tcb_ *tcb = node->tcb;
        if(tcb->th_id == curr_tcb->join_th) {
            enqueue(tcb->join_queue, tcb, 0);
            tcb->state = READY;
            enqueue(sched.q, tcb, 1);
            break;
        }
        node = node->next;
    }
}


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {

	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread

	// YOUR CODE HERE
    node_ *head = curr_tcb->join_queue->head;
    state_ *state = &curr_tcb->state;
    do {
        node_ *node = head;
        while(node != NULL) {
            tcb_ *tcb = node->tcb;
            if(tcb->th_id == thread) {
                value_ptr = tcb->ret_val;
                free(node);
                curr_tcb->state = RUNNING;
                break;
            }
            node = node->next;
        }
        if(curr_tcb->state == BLOCKED) {
            mypthread_yield();
            curr_tcb->state = BLOCKED;
        }
    } while(*state == BLOCKED);

	return 0;
}

/* initialize the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex

	// YOUR CODE HERE
    mutex->flag = 0;
    mutex->wait_q = queue_create();
	return 0;
}

/* acquire the mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex) {
    // use the built-in test-and-set atomic function to test the mutex
    // if the mutex is acquired successfully, enter the critical section
    // if acquiring mutex fails, push current thread into block queue and //
    // context switch to the scheduler thread

    // YOUR CODE HERE
    while(__atomic_test_and_set(mutex->flag, __ATOMIC_SEQ_CST)) {
        enqueue(mutex->wait_q, curr_tcb, 0);
        mypthread_yield();
    }
    return 0;
}

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex) {
	// Release mutex and make it available again.
	// Put threads in block queue to run queue
	// so that they could compete for mutex later.

	// YOUR CODE HERE
    queue_ *wait_q = mutex->wait_q;
    node_ *node = wait_q->head;
    while(node != NULL) {
        node_ *temp = node->next;
        node->tcb->state = READY;
        enqueue(sched.q, node->tcb, 1);
        node_destroy(node);
        node = temp;
    }
    wait_q->head = NULL;
    wait_q->rear = NULL;
    __atomic_clear(mutex->flag, __ATOMIC_SEQ_CST);
	return 0;
}


/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in mypthread_mutex_init
    queue_destroy(mutex->wait_q);
    free(mutex);
	return 0;
}

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
/* initialize schedule */
void schedule_init() {
    sched.size = 0;
    sched.policy = FIFO;
    getcontext(&sched.ucp);
    sched.ucp.uc_stack.ss_sp = malloc(STACKSIZE);
    sched.ucp.uc_stack.ss_size = STACKSIZE;
    sched.ucp.uc_stack.ss_flags = 0;
    makecontext(&sched.ucp, schedule, 1);
    sched.q = queue_create();
    schedule_initialized = 1;
}

/* clean schedule */
void schedule_clean() {
    node_ *node = sched.all_tcbs->head;
    while (node != NULL) {
        node_ *temp = node->next;
        tcb_destroy(node->tcb);
        node_destroy(node);
        node = temp;
    }
    sched.q = NULL;
    free(sched.ucp.uc_stack.ss_sp);
    sched.ucp.uc_stack.ss_sp = NULL;
}

/* create thread control block */
tcb_ * tcb_create(mypthread_t th_id, mypthread_t join_th, void *(*function)(void*), void * arg) {
    tcb_ *tcb = malloc(sizeof(tcb_));
    getcontext(&tcb->ucp);
    tcb->ucp.uc_stack.ss_sp = malloc(STACKSIZE);
    tcb->ucp.uc_stack.ss_size = STACKSIZE;
    tcb->ucp.uc_flags = 0;
    makecontext(&tcb->ucp, (void (*)(void)) function, 2, arg);
    tcb->quantum = DEFAULT_QUANTUM;
    tcb->state = READY;
    tcb->th_id = th_id;
    tcb->join_th = join_th;
    tcb->ret_val = NULL;
    queue_create(tcb->join_queue);
    return tcb;
}

/* add tcb to schedule */
void enqueue(queue_ *queue, tcb_ *tcb, int use_quantum) {
    node_ *node = node_create(tcb);
    if(!queue->size) {
        queue->head = node;
        queue->rear = node;
        return;
    }
    node_ *temp = NULL;
    if(use_quantum) {
        int quantum = tcb->quantum;
        node_ *prev = NULL;
        temp = queue->head;
        while(temp != NULL) {
            if(quantum < temp->tcb->quantum) {
                if(prev == NULL) {
                    queue->head = temp;
                }
                node->next = temp;
                break;
            }
            prev = temp;
            temp = temp->next;
        }
    }
    if(!use_quantum || temp == NULL) {
        queue->rear->next = node;
        queue->rear = node;
    }
}

tcb_ * pop_tcb(queue_ *queue) {
    node_ *node = queue->rear;
    tcb_ * tcb = node->tcb;
    queue->rear = node->prev;
    if(queue->head == queue->rear) {
        queue->head = NULL;
    }
    node_destroy(node);
    return tcb;
}

/* destroy thread control block */
void * tcb_destroy(tcb_ *tcb) {
    queue_destroy(tcb->join_queue);
    free(tcb->ucp.uc_stack.ss_sp);
    free(tcb);
}


/* create queue */
queue_ * queue_create() {
    queue_ *queue = malloc(sizeof(queue_));
    return queue;
}

/* destroy queue */
void queue_destroy(queue_ *queue) {
    node_ *node = queue->head;
    while(node != NULL) {
        node_ *temp = node->next;
        node_destroy(node);
        node = temp;
    }
    free(queue);
}

/* create node */
node_ * node_create(tcb_ *tcb) {
    node_ *node = malloc (sizeof(node_));
    node->tcb = tcb;
    return node;
}

/* add node to queue */
void add_node(queue_ *queue, node_ *node) {
    if(queue->head == NULL) {
        queue->head = node;
        queue->rear = node;
    } else {
        queue->rear->next = node;
        queue->rear = node;
    }
}

/* destroy node */
void node_destroy(node_ *node) {
    free(node);
}