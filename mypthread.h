#ifndef MYTHREAD_T_H
#define MYTHREAD_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_MYTHREAD macro */
#define USE_MYTHREAD 1

#include <unistd.h>
#include <ucontext.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <valgrind/valgrind.h>

typedef uint mypthread_t;
typedef struct queue queue_t;

/*************************************************************************/
/* Definitions for pthread and pthread mutex structs                     */
/*************************************************************************/
/* thread state definition */
typedef enum state {
    READY, RUNNING, BLOCKED, DEAD
} state_t;

typedef struct schedule {
    unsigned int num_threads;               /* number of allocated threads */
    int valgrind_id;                        /* stack id for valgrind use */
    ucontext_t ucp;                         /* scheduler's context */
    queue_t *run_q;                         /* thread run queue */
    queue_t *all_threads;                   /* list of all allocated threads */
} schedule_t;

/* thread control block definition */
typedef struct threadControlBlock {
    int elapsed;                            /* amount of time quanta the thread has elapsed */
    state_t state;                          /* thread state */
    mypthread_t th_id;                      /* thread id */
    int valgrind_id;                        /* stack id for valgrind use */
    mypthread_t join_th;                    /* id of the parent thread */
    ucontext_t ucp;                         /* thread context */
    void *retval;                           /* return value */
    queue_t *join_q;                        /* thread join queue */
} tcb_t;

/* mutex struct definition */
typedef struct mypthread_mutex_t {
	unsigned char flag;
    queue_t * wait_q;
} mypthread_mutex_t;

/*************************************************************************/
/* Definitions for queues and nodes structs                              */
/*************************************************************************/

/* node definition */
typedef struct node {
    tcb_t *tcb;                         /* pointer to thread control block */
    struct node *next;                  /* pointer to next node */
} node_t;

/* queue definition */
typedef struct queue {
    unsigned int size;                  /* size of queue */
    node_t *head;                       /* start of queue */
    node_t *tail;                       /* end of queue */
} queue_t;


/*************************************************************************/
/* Function Declarations                                                 */
/*************************************************************************/
/* initialize the scheduler */
int schedule_create();

/* destroy the scheduler */
void schedule_destroy();

/* create a new thread control block */
tcb_t * tcb_create(mypthread_t th_id, mypthread_t join_th, void *(*function)(void*), void *arg);

/* destroy a thread control block */
void tcb_destroy(tcb_t *tcb);

/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU possession to other user level threads voluntarily */
int mypthread_yield();

/* terminate a thread */
void mypthread_exit(void *value_ptr);

/* wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr);

/* initialize the mutex lock */
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

/* create a new node */
node_t * node_create(tcb_t *tcb);

/* destroy a node */
void node_destroy(node_t *node);

/* create a new queue */
queue_t *queue_create();

/* destroy a queue and if specified, also destroys tcbs */
void queue_destroy(queue_t *queue, int destroy_tcbs);

/* add a node to a queue */
void enqueue(queue_t *queue, tcb_t *tcb);

/* if the given tcb is not NULL, removes node with the specified tcb from queue; else, remove first node from queue */
node_t * dequeue(queue_t *queue, tcb_t *tcb);

#endif
