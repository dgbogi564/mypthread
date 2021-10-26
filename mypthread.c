// File:	mypthread.c

// List all group member's name:
// username of iLab: dog12
// iLab Server: ilab.cs.rutgers.edu

#include "mypthread.h"

#define STACKSIZE 4096                      /* size of stack */
#define QUANTUM 5                           /* time of a single quantum */
#define DEBUG 0                             /* debug toggle */

/* GLOBALS */
schedule_t sched;                           /* scheduler information (contains tcb queue) */
tcb_t *ctcb;                                /* current tcb */
int sched_mtcb_initialized = 0;             /* if the scheduler and main tcb are initialized */

/* Routines for scheduler */
static void preempt();
static void schedule();
static void sched_stcf();
static void sched_mlfq();

/*************************************************************************/
/* Definitions for pthread and pthread mutex                             */
/*************************************************************************/
/* initialize the scheduler */
int schedule_create() {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    sched.num_threads = 0;
    sched.run_q = queue_create();
    sched.all_threads = queue_create();
    // initialize context
    getcontext(&sched.ucp);
    sched.ucp.uc_stack.ss_sp = malloc(STACKSIZE);
    sched.ucp.uc_stack.ss_size = STACKSIZE;
    sched.ucp.uc_stack.ss_flags = 0;
    makecontext(&sched.ucp, schedule, 1);
}

/* destroy the scheduler */
void schedule_destroy() {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    queue_destroy(sched.run_q, 0);
    queue_destroy(sched.all_threads, 1);
    free(sched.ucp.uc_stack.ss_sp);
    sched = (schedule_t){0};
    __atomic_clear(&sched_mtcb_initialized, __ATOMIC_SEQ_CST);
}

/* create a new thread control block */
tcb_t * tcb_create(mypthread_t th_id, mypthread_t join_th, void *(*function)(void*), void *arg) {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    tcb_t *tcb = malloc(sizeof(tcb_t));
    tcb->elapsed = 0;
    tcb->state = READY;
    tcb->th_id = th_id;
    tcb->join_th = join_th;
    tcb->retval = NULL;
    tcb->join_q = queue_create();
    // make context
    getcontext(&tcb->ucp);
    tcb->ucp.uc_stack.ss_sp = malloc(STACKSIZE);
    tcb->ucp.uc_stack.ss_size = STACKSIZE;
    tcb->ucp.uc_stack.ss_flags = 0;
    makecontext(&tcb->ucp, (void (*)(void)) function, 2, arg);
    return tcb;
}

/* destroy a thread control block */
void tcb_destroy(tcb_t *tcb) {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    // checks to see if this is not the tcb of the main thread
    if(tcb->th_id) {
        free(tcb->ucp.uc_stack.ss_sp);
    }
    queue_destroy(tcb->join_q, 0);
    free(tcb);
}

/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr,
                      void *(*function)(void*), void * arg) {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    // atomically checks if the main thread control block has been
    // initialized, and does so if not; adds main tcb into the run
    // queue and all threads list
    if(!__atomic_test_and_set(&sched_mtcb_initialized, __ATOMIC_SEQ_CST)) {
        schedule_create();
        /*
        ctcb = malloc(sizeof(tcb_t));
        ctcb->elapsed = 0;
        ctcb->state = READY;
        ctcb->th_id = 0;
        ctcb->join_th = 0;
        ctcb->retval = NULL;
        ctcb->join_q = queue_create();
        getcontext(&ctcb->ucp);
        enqueue(sched.all_threads, ctcb);
        enqueue(sched.run_q, ctcb);
        sched.num_threads++;
         */
    }
    // creates, initializes, enqueues the thread control block of
    // the new thread
    // TODO tcb_t *tcb = tcb_create(*thread, ctcb->th_id, function, arg);
    tcb_t *tcb = tcb_create(*thread, 1, function, arg); // TODO remove
    enqueue(sched.all_threads, tcb);
    enqueue(sched.run_q, tcb);
    sched.num_threads++;
    // save current context and switch to the scheduler
    //TODO swapcontext(&ctcb->ucp, &sched.ucp);
    return 0;
}

/* give CPU possession to other user-level threads voluntarily */
int mypthread_yield() {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    // changes thread state from Running to Ready
	ctcb->state = READY;
    // saves context of this thread and switches from the
    // current thread's context to the scheduler context
    swapcontext(&ctcb->ucp, &sched.ucp);
	return 0;
}

/* terminate a thread */
void mypthread_exit(void *value_ptr) {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
	// Deallocated any dynamic memory created when starting this thread

	// YOUR CODE HERE

    ctcb->retval = value_ptr;
    node_t *temp = sched.all_threads->head;
    // finds the parent thread to return to
    while(temp != NULL) {
        if(temp->tcb->th_id == ctcb->join_th) {
            break;
        }
        temp = temp->next;
    }
    // kills current thread and sets parent's state to READY,
    // switches to the scheduler's context
    tcb_t *ptcb = temp->tcb;
    enqueue(ptcb->join_q, ctcb);
    node_destroy(dequeue(sched.all_threads, ctcb));
    ptcb->state = READY;
    sched.num_threads--;
    ctcb->state = DEAD;
    setcontext(&sched.ucp);
}


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
	int waiting = 1;
    ctcb->state = BLOCKED;
    while(waiting) {
        node_t *temp = ctcb->join_q->head;
        // searches for thread with the given thread id
        while(temp != NULL) {
            tcb_t *tcb = temp->tcb;
            // if the thread is found, passes the return value to the
            // current thread's tcb, dequeues the thread that was found
            // from the join list, destroys the found thread, and sets
            // waiting to 0
            if(tcb->th_id == thread && tcb->state == DEAD) {
                if(value_ptr != NULL) {
                    *value_ptr = tcb->retval;
                }
                node_destroy(dequeue(ctcb->join_q, tcb));
                waiting = 0;
                break;
            }
            temp = temp->next;
        }
        // if the thread is still waiting for the thread to end, yield thread
        if(waiting) {
            ctcb->state = BLOCKED;
            mypthread_yield();
        }
    }
    // if that was the last thread to join, destroy the scheduler
    if(sched.num_threads < 2) {
        alarm(0);
        schedule_destroy();
    }
	return 0;
}

/* initialize the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
	//initialize data structures for this mutex

	// YOUR CODE HERE
    mutex->flag = 0;
    mutex->wait_q = queue_create();
	return 0;
}

/* aquire the mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex) {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    // use the built-in test-and-set atomic function to test the mutex
    // if the mutex is acquired successfully, enter the critical section
    // if acquiring mutex fails, push current thread into block list and //
    // context switch to the scheduler thread

    // YOUR CODE HERE
    int waiting = 0;
    while(__atomic_test_and_set(&mutex->flag, __ATOMIC_SEQ_CST)) {
        if(!waiting) {
            enqueue(mutex->wait_q, ctcb);
            waiting = 1;
        }
        mypthread_yield();
    }
    return 0;
}

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex) {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    // Releases mutex and adds all waiting threads to the run queue
    __atomic_clear(&mutex->flag, __ATOMIC_SEQ_CST);
    queue_t *wait_q = mutex->wait_q;
    while(wait_q->size) {
        node_t * node = dequeue(wait_q, NULL);
        enqueue(sched.run_q, node->tcb);
        node_destroy(node);
    }
    wait_q->head = NULL;
    wait_q->tail = NULL;
    return 0;
}


/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex) {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
	// Deallocate dynamic memory created in mypthread_mutex_init
    queue_destroy(mutex->wait_q, 0);
	return 0;
}

/* scheduler */
static void schedule() {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
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
        enqueue(sched.run_q, ctcb);
    }
// schedule policy
#ifndef MLFQ
	sched_stcf();
#else
    sched_mlfq();
#endif
}

/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf() {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    // increments elapsed counter of the current thread
	ctcb->elapsed++;
    node_t *temp = sched.run_q->head;
    tcb_t *tcb = NULL;
    // Find the shortest, unblocked thread
    while(temp != NULL) {
        if(temp->tcb->state != BLOCKED) {
            if(tcb == NULL) {
                tcb = temp->tcb;
                continue;
            }
            if(temp->tcb->elapsed < tcb->elapsed) {
                tcb = temp->tcb;
            }
        }
        temp = temp->next;
    }
    // remove node from run queue
    node_destroy(dequeue(sched.run_q, tcb));
    // set sigaction and alarm
    sigaction(SIGALRM, &(struct sigaction){preempt, 0, 0}, NULL);
    setitimer(ITIMER_REAL, &(struct itimerval){0, QUANTUM*1000}, NULL);
    ctcb = tcb;
    ctcb->state = RUNNING;
    setcontext(&ctcb->ucp);
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

/* runs when threads are preempted */
static void preempt() {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    ctcb->state = READY;
    schedule();
}

/*************************************************************************/
/* Definitions for queue and node functions                              */
/*************************************************************************/
/* create a new node */
node_t * node_create(tcb_t *tcb) {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    node_t *node = malloc(sizeof(node_t));
    node->tcb = tcb;
    node->next = NULL;
    return node;
}

/* destroy a node */
void node_destroy(node_t *node) {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    free(node);
}

/* create a new queue */
queue_t *queue_create() {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    queue_t *queue = malloc(sizeof(queue_t));
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

/* destroy a queue and, if specified, also destroys tcbs */
void queue_destroy(queue_t *queue, int destroy_tcbs) {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    node_t *temp = queue->head;
    // destroy all nodes in the queue
    while(temp != NULL) {
        node_t *next = temp->next;
        // destroy tcbs if specified
        if(destroy_tcbs) {
            tcb_destroy(temp->tcb);
        }
        node_destroy(temp);
        temp = next;
    }
    // free queue
    free(queue);
}

/* add a node to a queue */
void enqueue(queue_t *queue, tcb_t *tcb) {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    // if the queue is null or if the node is null
    if(queue == NULL || tcb == NULL) {
        return;
    }
    node_t *node = node_create(tcb), *temp = queue->head;
    // if the queue is empty
    if(temp == NULL) {
        queue->head = node;
        queue->tail = node;
        queue->size++;
        return;
    }
    // add node to the end of the queue
    queue->tail->next = node;
    queue->tail = node;
    queue->size++;
}

/* if the given tcb is not NULL, removes node with the specified tcb from queue; else, remove first node from queue */
node_t * dequeue(queue_t *queue, tcb_t *tcb) {
#ifdef DEBUG
    schedule_t *sched_ptr = &sched;
    tcb_t **ctcb_ptr = &ctcb;
    int *sched_mtcb_initialized_ptr = &sched_mtcb_initialized;
#endif
    // if the queue pointer is null
    if(queue == NULL) {
        return NULL;
    }
    // if there's nothing in the queue
    node_t *temp = queue->head;
    if(temp == NULL) {
        return NULL;
    }
    // if the tcb is not specified or if the first node matches the specified tcb
    if(tcb == NULL || temp->tcb == tcb) {
        // if there is only one node in the queue
        if(temp->next == NULL) {
            queue->tail = NULL;
        }
        queue->head = temp->next;
        queue->size--;
        return temp;
    }
    node_t *prev = temp;
    temp = temp->next;
    // search for node with specified tcb
    while(temp != NULL) {
        // if the node is found
        if(temp->tcb == tcb) {
            // if the node is the last node in the queue
            if(temp->next == NULL) {
                queue->tail = prev;
            }
            prev->next = temp->next;
            queue->size--;
            return temp;
        }
        temp = temp->next;
    }
    // if the specified tcb is not in the queue
    return NULL;
}
