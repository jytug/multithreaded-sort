#include "err.h"

#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#define SHM_BUFFER "/pw_332069_buffer"

int n;
int len;
int fd_mem;
int flags, prot;

int pid;

int *shared_array;

int main() {

    scanf("%d", &n);
    len = 2 * n;

    fd_mem = shm_open(SHM_BUFFER, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_mem == -1) syserr("shm_open");
    if (ftruncate(fd_mem, len * sizeof(int)) == -1) syserr("ftruncate");

    shared_array = (int *) mmap(NULL, len * sizeof(int),
                                PROT_READ | PROT_WRITE, MAP_SHARED,
                                fd_mem, 0);

    // the file will still be available for use?
    close(fd_mem);
    shm_unlink(SHM_BUFFER);
    if ((pid = fork()) == -1) {
        syserr("fork");
    } else if (pid == 0) {
        printf("I am a child! The first three numbers are: %d, %d, %d\n", shared_array[0], shared_array[1], shared_array[2]);
        shared_array[0] = 6;
        shared_array[1] = 6;
        shared_array[2] = 6;
        printf("I am a child! Now, first three numbers are: %d, %d, %d\n", shared_array[0], shared_array[1], shared_array[2]);
        return 0;
    }
    wait(NULL);
    printf("I am a parent! Now, first three numbers are: %d, %d, %d\n", shared_array[0], shared_array[1], shared_array[2]);
    return 0;
}
