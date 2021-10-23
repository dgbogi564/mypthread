// File:	mypthread_t.h

// queue all group member's name: David Gbogi,
// username of iLab: dog12,
// iLab Server: ilab.cs.rutgers.edu

#ifndef MYTHREAD_T_H
#define MYTHREAD_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_MYTHREAD macro */
#define USE_MYTHREAD 1

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <bits/semaphore.h>

typedef uint mypthread_t;
typedef struct queue queue_;

// Feel free to add any other variables you need

// YOUR CODE HERE
typedef enum state {
    READY, RUNNING, BLOCKED, DEAD
} state_;

typedef struct threadControlBlock {
	/* add important states in a thread control block */
	// thread Id
	// thread status
	// thread context
	// thread stack
	// thread priority
	// And more ...

	// YOUR CODE HERE
    int quantum;                            /* time quantum of thread */
    ucontext_t ucp;                         /* thread context */
    state_ state;                           /* thread state */
    mypthread_t th_id;                      /* thread id */
    mypthread_t join_th;                    /* id of thread to join to */
    void *ret_val;                          /* return value */
    queue_ *join_queue;                     /* thread join queue */
} tcb_;

/* mutex struct definition */
typedef struct mypthread_mutex_t {
	/* add something here */
	// YOUR CODE HERE
    volatile unsigned char *flag;
    queue_ * wait_q;
} mypthread_mutex_t;

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked queue or queue etc...)

// YOUR CODE HERE
/* policy enum definition */
typedef enum policy {
    FIFO, PRIORITY, STCF, MLFQ
} policy_;

/* schedule struct definition */
typedef struct schedule {
    unsigned int size;                      /* schedule size */
    policy_ policy;                         /* schedule policy */
    ucontext_t ucp;                         /* scheduler context */
    queue_ *q;                              /* thread queue */
    queue_ *all_tcbs;                       /* list of all tcbs */
} schedule_;

/* node struct definition */
typedef struct node {
    tcb_ *tcb;                              /* thread control block pointer */
    struct node *next;                      /* next node */
    struct node *prev;                      /* prev node */
} node_;

/* queue struct definition */
struct queue {
    unsigned int size;                      /* queue size */
    node_ *head;                            /* first node of queue */
    node_ *rear;                            /* last node of queue */
};

/* Function Declarations: */

/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int mypthread_yield();

/* terminate a thread */
void mypthread_exit(void *value_ptr);

/* wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr);

/* initial the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex);

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex);

/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex);

#ifdef USE_MYTHREAD
#define pthread_t mypthread_t
#define pthread_mutex_t mypthread_mutex_t
#define pthread_create mypthread_create
#define pthread_exit mypthread_exit
#define pthread_join mypthread_join
#define pthread_mutex_init mypthread_mutex_init
#define pthread_mutex_lock mypthread_mutex_lock
#define pthread_mutex_unlock mypthread_mutex_unlock
#define pthread_mutex_destroy mypthread_mutex_destroy
#endif

// Feel free to add any other functions you need

// YOUR CODE HERE
/* initialize schedule */
void schedule_init();

/* clean schedule */
void schedule_clean();

/* create thread control block */
tcb_ * tcb_create(mypthread_t th_id, mypthread_t join_th, void *(*function)(void*), void *arg);

/* add tcb to schedule */
void enqueue(queue_ *queue, tcb_ *tcb, int use_quantum);

/* pop tcb from schedule */
node_ * dequeue();

/* destroy thread control block */
void * tcb_destroy(tcb_ *tcb);


/* create queue */
queue_ * queue_create();

/* destroy queue */
void queue_destroy(queue_ *queue);

/* create node */
node_ * node_create();

/* add node to queue */
void add_node(queue_ *queue, node_ *node);

/* destroy node */
void node_destroy(node_ *node);

#endif
