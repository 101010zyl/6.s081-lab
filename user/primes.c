#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void recur(int p);

int
main()
{
    int p[2];
    pipe(p);
    if(fork()==0){
        recur(p[0]);
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
    if(!read(p, &n, sizeof(n))){
        close(p);
        exit(0);
    }
    pipe(pn);
    fprintf(1, "prime %d\n", n);
    while(read(p, &i, sizeof(i))){
            if (i % n != 0){
                write(pn[1], &i, sizeof(i));
            }
        }
        close(p);
        close(pn[1]);
    if(fork()==0){
        
        recur(pn[0]);
    }else{
        
        
        wait((int *)0);
        exit(0);
    }
    
}