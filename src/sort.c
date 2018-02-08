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

int pid;

int *shared_array;

void proc_A(int *shared_array, sem_t *mutex, sem_t *sem_A[], sem_t *sem_B[], int ind) {

}

int main() {
    scanf("%d", &n);
    len = 2 * n;

    prot = PROT_READ | PROT_WRITE;
    flags = MAP_SHARED | MAP_ANONYMOUS;

    /* shared_array[len] indicates how many processes are finished */
    shared_array = (int *) mmap(NULL, (len + 1)* sizeof(int), prot, flags, fd_mem, 0);
    if (shared_array == MAP_FAILED) syserr("mmap");

    for (i = 0; i < len; i++)
        scanf("%d", shared_array + i);

    sem_t mutex;
    sem_t sem_A[len],          /* Process A(i) waits on sem_A[i] */
          sem_B[len - 1];      /* Process B(i) waits on sem_B[i] */

    if (sem_init(&mutex, 1, 1)) syserr("sem_init");
    for (i = 0; i < len; i++)
        if (sem_init(&sem_A[i], 1, 0)) syserr("sem_init");

    for (i = 0; i < len - 1; i++)
        if (sem_init(&sem_B[i], 1, 0)) syserr("sem_init");

    /* wait for all children to end their work */
    while (wait(NULL) > 0);

    return 0;
}
