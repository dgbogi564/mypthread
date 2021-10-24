// File:	mypthread.c

// queue all group member's name:
// username of iLab:
// iLab Server:

#include "mypthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE
#define STACKSIZE 4096                      /* size of stack */
#define QUANTUM 5                           /* time of a single quantum */
schedule_ sched;                            /* scheduler information (contains tcb queue) */
volatile int first_run = 1;                 /* variable used to check if this is the first run of the library  */
tcb_ *ctcb;                                 /* current tcb */

static void sched_stcf();
static void schedule();
static void preempt();


/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr,
                      void *(*function)(void*), void * arg) {
    // create Thread Control Block
    // create and initialize the context of this thread
    // allocate space of stack for this thread to run
    // after everything is all set, push this thread int
    // YOUR CODE HERE
    schedule_ *sched_ptr = &sched;
    tcb_ **ctcb_ptr = &ctcb;
    if(first_run) {
        schedule_init();
        ctcb = tcb_create(0, 0, NULL, NULL);
        enqueue(sched.all_tcbs, ctcb);
        sched.num_threads++;
        first_run = 0;
    }
    tcb_ *tcb = tcb_create(*thread, ctcb->th_id, function, arg);
    enqueue(sched.all_tcbs, tcb);
    enqueue(sched.q, tcb);
    sched.num_threads++;
    swapcontext(&ctcb->ucp, &sched.ucp);
    return 0;
}

/* give CPU possession to other user-level threads voluntarily */
int mypthread_yield() {

	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// switch from thread context to scheduler context

	// YOUR CODE HERE
    if(ctcb->state == RUNNING) {
        ctcb->state = READY;
    }
    swapcontext(&ctcb->ucp, &sched.ucp);
	return 0;
}

/* terminate a thread */
void mypthread_exit(void *value_ptr) {
	// Deallocated any dynamic memory created when starting this thread

	// YOUR CODE HERE
    schedule_ *sched_ptr = &sched;
    tcb_ **ctcb_ptr = &ctcb;
    ctcb->ret_val = value_ptr;
    node_ *node = sched.all_tcbs->head;
    while(node != NULL) {
        tcb_ *tcb = node->tcb;
        if(tcb->th_id == ctcb->join_th) {
            enqueue(tcb->join_queue, ctcb);
            node_destroy(dequeue(sched.all_tcbs, ctcb));
            tcb->state = READY;
            break;
        }
        node = node->next;
    }
    sched.num_threads--;
    ctcb->state = DEAD;
    setcontext(&sched.ucp);
}


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {

	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread

	// YOUR CODE HERE
    schedule_ *sched_ptr = &sched;
    tcb_ **ctcb_ptr = &ctcb;
    state_ *state = &ctcb->state;
    ctcb->state = BLOCKED;
    int waiting = 1;
    do {
        node_ *node = ctcb->join_queue->head;
        while(node != NULL) {
            tcb_ *tcb = node->tcb;
            if(tcb->th_id == thread && tcb->state == DEAD) {
                *value_ptr = tcb->ret_val;
                dequeue(ctcb->join_queue, tcb);
                node_destroy(node);
                waiting = 0;
                break;
            }
            node = node->next;
        }
        if(waiting) {
            ctcb->state = BLOCKED;
            mypthread_yield();
        }
    } while(waiting);

    if(sched.num_threads < 2) {
        clean_global();
    }

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
        enqueue(mutex->wait_q, ctcb);
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
        enqueue(sched.q, node->tcb);
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
    alarm(0);
    if(ctcb->state != DEAD) {
        enqueue(sched.q, ctcb);
    }
// schedule policy
#ifndef MLFQ
	// Choose STCF
    sched_stcf();
#else
	// Choose MLFQ
#endif
}

/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf() {
    // Your own implementation of STCF
    // (feel free to modify arguments and return types)

    // YOUR CODE HERE
    schedule_ *sched_ptr = &sched;
    tcb_ **ctcb_ptr = &ctcb;
    ctcb->elapsed++;
    node_ *node = sched.q->head;
    while(node != NULL) {
        if(node->tcb->state != BLOCKED) {
            break;
        }
        node = node->next;
    }
    tcb_ *tcb = node->tcb;
    while(node != NULL) {
        if(node->tcb->state != BLOCKED && tcb->elapsed < node->tcb->elapsed) {
            tcb = node->tcb;
        }
        node = node->next;
    }

    node_destroy(dequeue(sched.q, tcb));
    /* set new sigaction and alarm */
    sigaction(SIGALRM, &(struct sigaction){preempt, 0, 0}, NULL);
    setitimer(ITIMER_REAL, &(struct itimerval){0, QUANTUM*1000}, NULL);
    ctcb = tcb;
    ctcb->state = RUNNING;
    setcontext(&ctcb->ucp);
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

// Feel free to add any other functions you need

// YOUR CODE HERE

/* handle preempts */
static void preempt() {
    ctcb->state = READY;
    schedule();
}

/* initialize schedule */
void schedule_init() {
    sched.num_threads = 0;
    getcontext(&sched.ucp);
    sched.ucp.uc_stack.ss_sp = malloc(STACKSIZE);
    sched.ucp.uc_stack.ss_size = STACKSIZE;
    sched.ucp.uc_stack.ss_flags = 0;
    makecontext(&sched.ucp, schedule, 1);
    sched.q = queue_create();
    sched.all_tcbs = queue_create();
}

/* clean schedule */
void clean_global() {
    // Free schedule's tcb list (not freed in queue_destroy())
    schedule_ *sched_ptr = &sched;
    node_ *node = sched.all_tcbs->head;
    while (node != NULL) {
        node_ *temp = node->next;
        tcb_destroy(node->tcb);
        node_destroy(node);
        node = temp;
    }
    free(sched.all_tcbs);
    /* Free rest of schedule */
    queue_destroy(sched.q);
    sched.q = NULL;
    free(sched.ucp.uc_stack.ss_sp);
    sched.ucp.uc_stack.ss_sp = NULL;
}

/* create thread control block */
tcb_ * tcb_create(mypthread_t th_id, mypthread_t join_th, void *(*function)(void*), void * arg) {
    tcb_ *tcb = malloc(sizeof(tcb_));

    tcb->elapsed = 0;
    tcb->state = READY;
    tcb->th_id = th_id;
    tcb->join_th = join_th;

    getcontext(&tcb->ucp);
    tcb->ucp.uc_stack.ss_sp = calloc(1, STACKSIZE);
    tcb->ucp.uc_stack.ss_size = STACKSIZE;
    tcb->ucp.uc_flags = 0;
    makecontext(&tcb->ucp, (void (*)(void)) function, 2, arg);

    tcb->ret_val = NULL;
    tcb->join_queue = queue_create(tcb->join_queue);

    return tcb;
}

/* add tcb to queue */
void enqueue(queue_ *queue, tcb_ *tcb) {
    node_ *node = node_create(tcb);
    if(!queue->size) {
        queue->head = node;
        queue->rear = node;
    } else {
        queue->rear->next = node;
        queue->rear = node;
    }
    queue->size++;
}

/* remove tcb from queue */
node_ * dequeue(queue_ *queue, tcb_ *tcb) {
    node_ *node = queue->head;
    node_ *prev = NULL;
    while(node != NULL) {
        if(node->tcb == tcb) {
            if(prev == NULL) {
                queue->head = node->next;
                if(queue->head == NULL) {
                    queue->rear = NULL;
                }
                queue->size--;
                return node;
            }
            if(queue->rear == node) {
                queue->rear = prev;
                prev->next = NULL;
            } else {
                prev->next = node->next;
            }
            queue->size--;
            return node;
        }
        prev = node;
        node = node->next;
    }
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
    queue->size = 0;
    queue->head = NULL;
    queue->rear = NULL;
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