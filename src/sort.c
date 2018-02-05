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

int *shared_array;


int main() {
    scanf("%d", &n);
    len = 2 * n;

    fd_mem = shm_open(SHM_BUFFER, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_mem == -1) syserr("shm_open");
    if (ftruncate(fd_mem, len) == -1) syserr("ftruncate");

    return 0;
}
