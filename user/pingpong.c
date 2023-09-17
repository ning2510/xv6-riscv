#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
    // parent: p1[0] read, p2[1] write
    // child:  p1[1] write, p2[0] read
    int p1[2], p2[2];
    if(pipe(p1) < 0 || pipe(p2) < 0) {
        fprintf(2, "pingpong: pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if(pid == 0) {
        // child process
        close(p1[0]);
        close(p2[1]);
        uint8 one;
        read(p2[0], &one, sizeof(one));
        printf("%d: received ping\n", getpid());
        write(p1[1], &one, sizeof(one));
        close(p1[1]);
        close(p2[0]);
    } else {
        // parent process
        close(p1[1]);
        close(p2[0]);
        uint8 one;
        write(p2[1], &one, sizeof(one));
        read(p1[0], &one, sizeof(one));
        printf("%d: received pong\n", getpid());
        close(p1[0]);
        close(p2[1]);
    }
    
    wait(&pid);
    exit(0);
}