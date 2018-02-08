#include "err.h"

#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

int i;
int n;
int len;
int fd_mem = -1;
int flags, prot;

int *shared_memory;

int *idle_count;
int *shared_array;
int *sorting_done;

sem_t *mutex_p;
sem_t (*sem_A_p)[][2],
      (*sem_B_p)[][2];

/* returns 0 if the swap has been performed, 1 otherwise */
int compare_and_swap(int *x, int *y) {
    if (*x > *y) {
        int dummy = *x;
        *x = *y;
        *y = dummy;
        return 0;
    } else {
        return 1;
    }
}

void proc_A(int index) {
    int idle;
    int *left  = shared_array + 2 * index,
        *right = shared_array + 2 * index + 1;

    for (int round = 0; round < n; round++) {
        /* swap left and right if necessary */
        idle = compare_and_swap(left, right);

        /* notify B(index) and B(index-1) */
        if (index < n - 1)
            if (sem_post(&(*sem_B_p)[index    ][0])) syserr("sem_post");

        if (index > 0)
            if (sem_post(&(*sem_B_p)[index - 1][1])) syserr("sem_post");

        /* wait for B(index) and B(index-1) */
        if (index < n - 1)
            if (sem_wait(&(*sem_A_p)[index][1])) syserr("sem_wait");

        if (index > 0)
            if (sem_wait(&(*sem_A_p)[index][0])) syserr("sem_wait");

        /*if (sem_wait(mutex_p) == -1) syserr("sem_wait");*/
        /*idle_count[round] += idle;*/
        /*if (idle_count[round] == 2 * n - 1)*/
            /**sorting_done = 1;*/
    }
}

void proc_B(int index) {
    int idle;
    int *left  = shared_array + 2 * index + 1,
        *right = shared_array + 2 * index + 2;
    for (int round = 0; round < n; round++) {
        /* wait for A(index) and A(index+1) */
        if (sem_wait(&(*sem_B_p)[index][0])) syserr("sem_wait");

        if (sem_wait(&(*sem_B_p)[index][1])) syserr("sem_wait");

        /* swap left and right if necessary */
        idle = compare_and_swap(left, right);

        /* notify A(index) and A(index+1) */
        if (sem_post(&(*sem_A_p)[index    ][1])) syserr("sem_post");

        if (sem_post(&(*sem_A_p)[index + 1][0])) syserr("sem_post");
    }
}

int main() {
    int pid;
    scanf("%d", &n);
    len = n << 1;

    prot = PROT_READ | PROT_WRITE;
    flags = MAP_SHARED | MAP_ANONYMOUS;

    shared_memory = (int *) mmap(NULL, (len + n + 1) * sizeof(int), prot, flags, fd_mem, 0);
    if (shared_array == MAP_FAILED) syserr("mmap");

    shared_array = shared_memory;
    idle_count = shared_memory + len;
    sorting_done = shared_memory + len + n;

    for (i = 0; i < len; i++)
        scanf("%d", shared_array + i);

    *sorting_done = 0;

    sem_t mutex;
    sem_t sem_A[len    ][2],      /* Process A(i) waits on sem_A[i] */
          sem_B[len - 1][2];      /* Process B(i) waits on sem_B[i] */

    mutex_p = &mutex;
    sem_A_p = &sem_A;
    sem_B_p = &sem_B;

    if (sem_init(&mutex, 1, 1)) syserr("sem_init");
    for (i = 0; i < len; i++) {
        if (sem_init(&sem_A[i][0], 1, 0)) syserr("sem_init");
        if (sem_init(&sem_A[i][1], 1, 0)) syserr("sem_init");
    }

    for (i = 0; i < len - 1; i++) {
        if (sem_init(&sem_B[i][0], 1, 0)) syserr("sem_init");
        if (sem_init(&sem_B[i][1], 1, 0)) syserr("sem_init");
    }

    /* spawn n processes of type A */
    for (i = 0; i < n; i++) {
        if ((pid = fork()) < 0) {
            syserr("fork");
        } else if (pid == 0) {
            proc_A(i);
            return 0;
        }
    }

    /* spawn n-1 processes of type B */
    for (i = 0; i < n - 1; i++) {
        if ((pid = fork()) < 0) {
            syserr("fork");
        } else if (pid == 0) {
            proc_B(i);
            return 0;
        }
    }

    /* wait for all children to end their work */
    while (wait(NULL) > 0);

    for (i = 0; i < len; i++) {
        printf("%d\n", shared_array[i]);
    }

    return 0;
}
