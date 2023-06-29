#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


void
find(char *path, char *filename)
{
  char pathbuf[128]={0}, *p, namebuf[128]={0}, *p1;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot fstat %s\n", path);
    close(fd);
    return;
  }

  if(strlen(path) + 1 + DIRSIZ + 1 > sizeof pathbuf){
    printf("find: path too long\n");
    exit(1);
  }

  strcpy(pathbuf, path);
  p = pathbuf+strlen(pathbuf);
  *p++ = '/';
  strcpy(namebuf, pathbuf);
  p1 = namebuf+strlen(namebuf);
  strcpy(p1, filename);

  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if((de.inum == 0) || (strcmp(de.name, ".") == 0) || (strcmp(de.name, "..") == 0))
      continue;

    strcpy(p, de.name);   

    if(stat(pathbuf, &st) < 0){
      printf("find: cannot stat %s\n", pathbuf);
      continue;
    }

    switch (st.type){
    case T_FILE:
      if(strcmp(namebuf, pathbuf) == 0)
        printf("%s\n", pathbuf);
      break;
    
    case T_DIR:
        find(pathbuf, filename);
      break;
    
    }
  }
  close(fd);
  return;
}

int
main(int argc, char *argv[])
{
    if(argc < 3){
        fprintf(2, "find: Arguments error.");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}