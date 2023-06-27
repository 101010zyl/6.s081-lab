#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void recur(int p);

int
main()
{
    // int p[2];
    // pipe(p);
    // if(fork()==0){
    //     recur(p[0]);
    // }else{
    //     int i;
    //     for(i=2; i<=35; i++){
    //         write(p[1], &i, sizeof(i));
    //     }
    //     close(p[1]);
    //     wait((int *)0);
    // }
    // exit(0);

    int p[2];
    pipe(p);
    if(fork()==0){
        int n, i, pn[2];
        if(!read(p[0], &n, sizeof(n))){
            exit(1);
        }
        fprintf(1, "prime %d\n", n);
        pipe(pn);
        while(read(p[0], &i, sizeof(i))){
            if (i % n != 0){
                write(pn[1], &i, sizeof(i));
            }
        }
        close(p[0]);
        close(pn[1]);
        exit(0);
    }else{
        int i;
        for(i=2; i<=35; i++){
            write(p[1], &i, sizeof(i));
        }
        close(p[1]);
        wait((int *)0);
    }
    exit(0);    
}

void
recur(int p)
{
    int pn[2];
    int n, i;
    pipe(pn);
    if(!read(p, &n, sizeof(n))){
        exit(1);
    }
    if(fork()==0){
        recur(pn[0]);
    }else{
        fprintf(1, "prime %d\n", n);
        while(read(p, &i, sizeof(i))){
            if (i % n != 0){
                write(pn[1], &i, sizeof(i));
            }
        }
        close(p);
        close(pn[1]);
        wait((int *)0);
    }
    
}