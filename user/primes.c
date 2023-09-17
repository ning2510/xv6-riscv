#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
prime_dfs(int *p)
{
    close(p[1]);
    int x;
    // if pipe is empty, will return 0
    if(read(p[0], &x, sizeof(x)) == 0) {
        close(p[0]);
        return ;
    }

    int pp[2];
    if(pipe(pp) < 0) {
        fprintf(2, "primes: pipe failed\n");
        close(p[0]);
        return ;
    }

    int pid = fork();
    if(pid == 0) {
        prime_dfs(pp);
    } else {
        close(pp[0]);
        printf("prime %d\n", x);

        int v;
        while(read(p[0], &v, sizeof(v)) != 0) {
            if(v % x == 0) continue;
            write(pp[1], &v, sizeof(v));
        }
        close(pp[1]);
        wait(&pid);
        exit(0);
    }

    return ;
}

int
main(int argc, char **argv)
{

    int p[2];
    if(pipe(p) < 0) {
        fprintf(2, "primes: pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if(pid == 0) {
        prime_dfs(p);
    } else {
        close(p[0]);
        for(int i = 2; i <= 35; i++) {
            write(p[1], &i, sizeof(i));
        }
        close(p[1]);
        wait(&pid);
    }

    exit(0);
}