#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int parent[2];
    int child[2];
    int tmp = 1;
    char buf;
    pipe(parent);
    pipe(child);
    if(fork() != 0) {
        write(child[1], &tmp, 4);
        read(parent[0], &buf, 4);
        fprintf(2, "%d: received pong\n", getpid());
    } else {
        read(child[0], &buf, 4);
        fprintf(2, "%d: received ping\n", getpid());
        write(parent[1], &tmp, 4);
        exit(0);
    }
    
    exit(0);
}
