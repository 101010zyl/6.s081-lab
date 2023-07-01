#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"



int
main(int argc, char *argv[])
{
    sleep(10);
    char xargs[8][8][32]={0}, ch;
    int xargt = 0,xargc=0, col = 0, i=0, j=0, n=0, p=0;
    char *xargv[MAXARG];
    int xargcs[MAXARG]={0};
    
    if((argc<2)||(argc>32)){
        printf("xargs: augments error\n");
        exit(1);
    }
    while (read(0, &ch, 1) > 0)
    {
        if(ch=='\n'){
            xargs[xargt][xargc][col]='\0';
            xargcs[xargt]=xargc;
            xargt++;
            col=0;
            xargc=0;
            continue;
        }
        if(ch==' '){
            xargs[xargt][xargc][col]='\0';
            xargc++;
            col=0;
            continue;
        }
        xargs[xargt][xargc][col]=ch;
        col++;
    }
    for(j=0;j<argc-1;j++){
        xargv[j]=argv[j+1];
    }

    while(i<xargt){
        for(n=argc-1;n<8;n++){
            xargv[n]=0;
        }
        p=j;
        for(n=0;n<xargcs[i]+1;n++){
            xargv[p]=xargs[i][n];
            p++;
        }
        if(fork()==0){
            exec(argv[1], xargv);
        }else{
            wait(0);
        }
        i++;
    }
    exit(0);
}