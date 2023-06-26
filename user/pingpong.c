#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main()
{
    int p[2];
    char *buf="";
    char mes = 'a';
    int pid = 0;

    pipe(p);
    if(fork() == 0){
        if(read(p[0], buf, 1) != 1){
            fprintf(2, "Receive error");
            exit(1);
        }
        pid = getpid();
        fprintf(1, "%d: received ping\n", pid);
        close(p[0]);
        write(p[1], buf, 1);
        close(p[1]);
        exit(0);
    }else{
        write(p[1], &mes, 1);
        close(p[1]);
        if(read(p[0], buf, 1) != 1){
            fprintf(2, "Receive error");
            exit(1);
        }
        pid = getpid();
        fprintf(1, "%d: received pong\n", pid);
        close(p[0]);
        exit(0);
    }
}