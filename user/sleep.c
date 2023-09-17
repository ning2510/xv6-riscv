#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
    if(argc <= 1) {
        fprintf(2, "usage: sleep NUMBER\n");
        exit(1);
    }

    int seconds = atoi(argv[1]);
    printf("seconds = %d\n", seconds);
    sleep(seconds);

    exit(0);
}