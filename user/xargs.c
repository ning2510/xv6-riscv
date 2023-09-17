#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAXLEN 100

int
main(int argc, char **argv)
{
    char c;
    char *command = argv[1];
    char params[MAXARG][MAXLEN];
    char *m[MAXARG];    

    while(1) {
        memset(params, 0, MAXARG * MAXLEN);
        for(int i = 1; i < argc; i++) {
            strcpy(params[i - 1], argv[i]);
        }

        int rt;
        int idx = 0;
        int flag = 0;
        int count = argc - 1;

        while(((rt = read(0, &c, sizeof(c))) > 0) && c != '\n') {
            if(c == ' ' && flag == 1) {
                flag = 0;
                idx = 0;
                count++;
            } else if(c != ' ') {
                flag = 1;
                params[count][idx++] = c;
            }
        }

        if(rt == 0) {
            break;
        }

        for(int i = 0; i < MAXARG - 1; i++) {
            m[i] = params[i];
        }
        m[MAXARG - 1] = 0;

        int pid = fork();
        if(pid == 0) {
            exec(command, m);
        } else {
            wait(&pid);
        }
    }

    exit(0);
}