// File:	mypthread.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "mypthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE
#define STACKSIZE 4096
schedule_ sched;
int schedule_initialized = 0;
tcb_ *curr_tcb;   /* current tcb */


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
    add_tcb((curr_tcb = tcb_create(*thread, curr_tcb->join_th, function, arg)));
    return 0;
};

/* give CPU possession to other user-level threads voluntarily */
int mypthread_yield() {

	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// switch from thread context to scheduler context

	// YOUR CODE HERE
    if(curr_tcb->state == READY) {
        return -1;
    }
    curr_tcb->state = READY;
    add_tcb(curr_tcb);
    swapcontext(&curr_tcb->ucp, &sched.ucp);
	return 0;
};

/* terminate a thread */
void mypthread_exit(void *value_ptr) {
	// Deallocated any dynamic memory created when starting this thread

	// YOUR CODE HERE
    curr_tcb->state = JOIN;
    curr_tcb->ret_val = value_ptr;
    tcb_ *tcb = sched.head;
    while(tcb != NULL) {
        tcb_ *temp = tcb->next;
        if(tcb->th_id == curr_tcb->join_th) {
            tcb->state = READY;
            if(tcb->join_list == NULL) {
                tcb->join_list = curr_tcb;
            } else {
                tcb_ *temp2 = tcb->join_list;
                while(temp2->next != NULL) {
                    temp2 = temp2->next;
                }
                temp2->next = curr_tcb;
            }
            break;
        }
        tcb = temp;
    }
};


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {

	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread

	// YOUR CODE HERE
    if(curr_tcb->join_list == NULL) {
        curr_tcb->state = BLOCKED;
        mypthread_yield();
    }
    tcb_ *temp = curr_tcb->join_list;
    while(temp != NULL) {
        if(temp->th_id == thread) {
            value_ptr = temp->ret_val;
            break;
        }
        temp = temp->next;
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
static void sched_fifo() {

}

static void sched_priority() {

}


// YOUR CODE HERE
/* initialize schedule */
void schedule_init() {
    sched.size = 0;
    sched.policy = FIFO;
    sched.head = NULL;
    sched.rear = NULL;
    getcontext(&sched.ucp);
    sched.ucp.uc_stack.ss_sp = malloc(STACKSIZE);
    sched.ucp.uc_stack.ss_size = STACKSIZE;
    sched.ucp.uc_stack.ss_flags = 0;
    makecontext(&sched.ucp, schedule, 1);
    schedule_initialized = 1;
}

/* clean schedule */
void schedule_clean() {
    tcb_ *tcb = sched.head;
    while (tcb != NULL) {
        tcb_ *temp = tcb->next;
        tcb_destroy(tcb);
        tcb = temp;
    }
    sched.head = NULL;
    sched.rear = NULL;
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
    tcb->priority = 0;
    tcb->state = READY;
    tcb->th_id = th_id;
    tcb->join_th = join_th;
    tcb->ret_val = NULL;
    tcb->join_list = NULL;
    tcb->prev = NULL;
    tcb->next = NULL;
    return tcb;
}

/* add tcb to schedule */
void add_tcb(tcb_ *tcb) {
    // TODO implement 4 functions?
    //  Make sure to keep in mind prev node as well.
    /*
    switch(sched.policy) {
        // FIFO
        case 0: { sched_fifo(tcb_ *tcb); } return;

        // PRIORITY
        case 1: { sched_priority(tcb_ *tcb); } return;

        // STCF
        case 2: { sched_stcf(tcb_ *tcb); } return;

        // MLFQ
        case 3: { sched_mlfq(tcb_ *tcb); } return;
    }
    */
}

tcb_ * pop_tcb() {
    tcb_ *tcb = sched.rear;
    sched.rear = tcb->prev;
    if(sched.head == tcb) {
        sched.head = NULL;
    }
    return tcb;
}

/* destroy thread control block */
void * tcb_destroy(tcb_ *tcb) {
    free(tcb->ucp.uc_stack.ss_sp);
    free(tcb);
}


/* create list */
list_ * list_create() {
    list_ *list = malloc(sizeof(list_));
    return list;
}

/* destroy list */
void destroy_list(list_ *list) {
    node_ *node = list->head;
    while(node != NULL) {
        node_ *temp = node->next;
        node_destroy(node);
        node = temp;
    }
    free(list);
}

/* create node */
node_ node_create(tcb_ *tcb) {
    node_ *node = malloc (sizeof(node_));
    node->tcb = tcb;
}

/* add node to list */
void add_node(list_ *list, node_ *node) {
    if(list->head == NULL) {
        list->head = node;
        list->rear = node;
    } else {
        list->rear->next = node;
        list->rear = node;
    }
}

/* destroy node */
void node_destroy(node_ *node) {
    free(node);
}