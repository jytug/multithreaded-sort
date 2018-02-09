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

sem_t *mutex;
sem_t *sem_A,       /* sem_A is of length  n */
      *sem_B;       /* sem_B is of length (n - 1) */

void A_waits_for_B(int index) {
    if (index < n - 1)
        if (sem_wait(sem_A + index)) syserr("sem_wait");

    if (index > 0)
        if (sem_wait(sem_A + index)) syserr("sem_wait");
}

void B_waits_for_A(int index) {
    if (sem_wait(sem_B + index)) syserr("sem_wait");
    if (sem_wait(sem_B + index)) syserr("sem_wait");
}

void A_notifies_B(int index) {
    if (index < n - 1)
        if (sem_post(sem_B + index)) syserr("sem_post");

    if (index > 0)
        if (sem_post(sem_B + index - 1)) syserr("sem_post");
}

void B_notifies_A(int index) {
    if (sem_post(sem_A + index)) syserr("sem_post");
    if (sem_post(sem_A + index + 1)) syserr("sem_post");
}

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

    int round;
    for (round = 0; round < n; round++) {
        /* swap left and right if necessary */
        idle = compare_and_swap(left, right);

        if (sem_wait(mutex)) syserr("sem_wait");
        idle_count[round] += idle;
        if (idle_count[round] == 2 * n - 1)
            *sorting_done = 1;
        if (sem_post(mutex)) syserr("sem_post");

        A_notifies_B(index);

        if (sem_wait(mutex)) syserr("sem_wait");
        if (*sorting_done) {
            if (sem_post(mutex)) syserr("sem_post");
            A_notifies_B(index);
            break;
        }
        if (sem_post(mutex)) syserr("sem_post");

        A_waits_for_B(index);
    }
}

void proc_B(int index) {
    int idle;
    int *left  = shared_array + 2 * index + 1,
        *right = shared_array + 2 * index + 2;
    int round;
    for (round = 0; round < n; round++) {
        if (sem_wait(mutex)) syserr("sem_wait");
        if (*sorting_done) {
            if (sem_post(mutex)) syserr("sem_post");
            B_notifies_A(index);
            break;
        }
        if (sem_post(mutex)) syserr("sem_post");

        B_waits_for_A(index);

        /* swap left and right if necessary */
        idle = compare_and_swap(left, right);

        if (sem_wait(mutex)) syserr("sem_wait");
        idle_count[round] += idle;
        if (idle_count[round] == 2 * n - 1)
            *sorting_done = 1;
        if (sem_post(mutex)) syserr("sem_post");

        /* notify A(index) and A(index+1) */
        B_notifies_A(index);
    }
}

int main() {
    int pid;
    scanf("%d", &n);
    len = n << 1;

    prot = PROT_READ | PROT_WRITE;
    flags = MAP_SHARED | MAP_ANONYMOUS;

    int mem_size = (len + n + 1) * sizeof(int) + 2 * len * sizeof(sem_t);
    shared_memory = (int *) mmap(NULL, mem_size, prot, flags, fd_mem, 0);
    if (shared_array == MAP_FAILED) syserr("mmap");

    shared_array = shared_memory;
    idle_count = shared_array + len;
    sorting_done = idle_count + n;

    sem_A = (sem_t *)(sorting_done + 1);
    sem_B = sem_A + len;
    mutex = sem_B + len;

    for (i = 0; i < len; i++)
        scanf("%d", shared_array + i);

    *sorting_done = 0;

    if (sem_init(mutex, 1, 1)) syserr("sem_init");
    for (i = 0; i < len; i++)
        if (sem_init(sem_A + i, 1, 0)) syserr("sem_init");

    for (i = 0; i < len - 1; i++)
        if (sem_init(sem_B + i, 1, 0)) syserr("sem_init");

    /* spawn n processes of type A */
    for (i = 0; i < n; i++) {
        if ((pid = fork()) < 0) {
            syserr("fork");
        } else if (pid == 0) {
            proc_A(i);
            munmap(shared_memory, mem_size);
            return 0;
        }
    }

    /* spawn n-1 processes of type B */
    for (i = 0; i < n - 1; i++) {
        if ((pid = fork()) < 0) {
            syserr("fork");
        } else if (pid == 0) {
            proc_B(i);
            munmap(shared_memory, mem_size);
            return 0;
        }
    }

    /* wait for all children to end their work */
    while (wait(NULL) > 0);


    for (i = 0; i < len; i++) {
        printf("%d\n", shared_array[i]);
    }

    /* free all resources */
    if (sem_destroy(mutex)) syserr("sem_destroy");
    for (i = 0; i < len; i++)
        if (sem_destroy(sem_A + i)) syserr("sem_destroy");

    for (i = 0; i < len - 1; i++)
        if (sem_destroy(sem_B + i)) syserr("sem_destroy");
    munmap(shared_memory, mem_size);
    return 0;
}
