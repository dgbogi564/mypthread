#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../mypthread.h"

/* A scratch program template on which to call and
 * test mypthread library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */

void test(void *arg) {
    printf("I am working!\n");
    int *ret_val = malloc(sizeof(int));
    *ret_val = 42;
    mypthread_exit(ret_val);
}

int main(int argc, char **argv) {

    /* Implement HERE */

    // test pthread create and join
    pthread_t th[0];
    pthread_create(&th[0], NULL, test, NULL);
    int *ret_val;
    pthread_join(th[0], &ret_val);
    printf("return value: %d", *ret_val);
    free(ret_val);

    /*
    // test queue, nodes, and tcbs
    tcb_t *tcb[5];
    queue_t * q = queue_create();
    queue_t * q1 = queue_create();
    for(int x = 0; x < 5; x++) {
        tcb[x] = tcb_create(1, 0, test, NULL);
        enqueue(q, tcb[x]);
        enqueue(q1, tcb[x]);
    }
    for(int x = 0; x < 0; x++) {
        node_t *node = dequeue(q, tcb[x]);
        //tcb_destroy(node->tcb);
        free(node);
    }
    queue_destroy(q1, 1);
    queue_destroy(q, 0);
    */

    /*
    // test schedule struct
    pthread_t th[0];
    pthread_create(&th[0], NULL, test, NULL);
    schedule_destroy();
     */

    return 0;
}
