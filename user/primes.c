#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void recur(int *p);

int
main()
{
    int p[2];
    pipe(p);
    if(fork()==0){
        recur(p);
    }else{
        int i;
        close(p[0]);
        for(i=2; i<=35; i++){
            write(p[1], &i, sizeof(i));
        }
        close(p[1]);
        wait((int *)0);
    }
    exit(0);

}

void
recur(int *p)
{
    int pn[2];
    int n, i;
    close(p[1]);
    if(!read(p[0], &n, sizeof(n))){
        close(p[0]);
        exit(0);
    }
    pipe(pn);
    fprintf(1, "prime %d\n", n);
    
    if(fork()==0){
        
        recur(pn);
    }else{
        while(read(p[0], &i, sizeof(i))){
            if (i % n != 0){
                write(pn[1], &i, sizeof(i));
            }
        }
        close(p[0]);
        close(pn[1]);
        
        wait((int *)0);
        exit(0);
    }
    
}