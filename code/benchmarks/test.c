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
    printf("I am working!");
    mypthread_exit(0);
}

int main(int argc, char **argv) {

	/* Implement HERE */
    pthread_t th[0];
    pthread_create(&th[0], NULL, test, NULL);
    int *ret_val;
    pthread_join(&th[0], &ret_val);
    printf("return value: %d", *ret_val);
	return 0;
}
