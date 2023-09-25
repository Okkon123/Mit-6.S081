#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void sieve(int pleft[2]) {
    int primeNum;
    read(pleft[0], &primeNum, sizeof(primeNum));
    if (primeNum != -1) {
        printf("prime %d\n", primeNum);
    } else {
        exit(0);
    }
    int pright[2];
    int buf;
    pipe(pright);
    if(fork() != 0) {
        close(pright[0]);
        while(read(pleft[0], &buf, sizeof(buf)) != 0 && buf != -1) {
            if (buf % primeNum != 0) {
                write(pright[1], &buf, sizeof(buf));
            }
        }
        buf = -1;
        write(pright[1], &buf, sizeof(buf));
        wait(0);
        exit(0);
    } else {
        close(pleft[0]);
        close(pright[1]);
        sieve(pright);
        exit(0);
    }
}

int
main(int argc, char *argv[])
{
    int init_p[2];
    pipe(init_p);
    if(fork() != 0) {
        close(init_p[0]);
        int i;
        for(i = 2; i <= 35; i++) {
            write(init_p[1], &i, sizeof(i));
        }
        i = -1;
        write(init_p[1], &i, sizeof(i));
    } else {
        close(init_p[1]);
        sieve(init_p);
        exit(0);
    }
    wait(0);
    exit(0);
}
