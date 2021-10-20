// File:	mypthread_t.h

// List all group member's name: David Gbogi,
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

typedef uint mypthread_t;

// YOUR CODE HERE
typedef enum mypthread_state {
    RUNNING, READY, BLOCKED, JOIN, DEAD, MUTEX_WAIT
} mypthread_state;

typedef struct threadControlBlock {
	/* add important states in a thread control block */
	// thread Id
	// thread status
	// thread context
	// thread stack
	// thread priority
	// And more ...

	// YOUR CODE HERE
    ucontext_t ucp;                 /* thread context */
    int priority;                   /* priority number — the lower the number, the higher the priority */
    mypthread_state state;          /* state of the thread */
    mypthread_t th_id;              /* thread id */
    mypthread_t joinfrom_th;        /* thread to join to */
} threadControlBlock;

/* mutex struct definition */
typedef struct mypthread_mutex_t {
	/* add something here */
	// YOUR CODE HERE
} mypthread_mutex_t;

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

// YOUR CODE HERE
typedef struct node_ {
    mypthread_t th_id;              /* thread id */
    threadControlBlock *ptcb;       /* parent thread control block */
    void *ret_val;                  /* return value */
    struct node_ *next;             /* next node */
    struct node_ *prev;             /* prev node */
} node_;

typedef struct queue_ {
    unsigned int size;              /* queue size */
    node_ *head;                    /* pointer to head of queue */
    node_ *rear;                    /* pointer to rear of queue */
} queue_;

/* Function Declarations: */

/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU possession to other user level threads voluntarily */
int mypthread_yield();

/* terminate a thread */
void mypthread_exit(void *ret_val);

/* wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr);

/* initial the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* acquire the mutex lock */
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


/* scheduler */
static void schedule();

/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf();

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq();


// YOUR CODE HERE

/* initialize tcb */
void *tcb_init(mypthread_t *thread, void *(*function)(void*), void * arg);

/* cleanup tcb */
void *tcb_clean();

/* initialize tcb */
void *tcb_init(mypthread_t *thread, void *(*function)(void*), void * arg);

/* destroy tcb */
void *tcb_destroy(threadControlBlock *ftcb);


/* create node */
struct node_ *node_create(mypthread_t thread, threadControlBlock *ptcb, void *ret_val);

/* destroy node */
void *node_destroy(node_ *node);

/* create queue */
struct queue_ *queue_create();

/* add node to front of queue */
void enqueue_head(queue_ *q, node_ *node);

/* add node to back of queue */
void enqueue_rear(queue_ *q, node_ *node);

/* pop head of queue */
struct node_* pop_head(queue_ *q);

/* pop rear of queue */
struct node_* pop_rear(queue_ *q);

/* destroy queue */
void *queue_destroy(queue_ *q);


#endif
