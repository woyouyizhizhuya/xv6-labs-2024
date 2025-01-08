#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last '/'.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), 0, DIRSIZ-strlen(p));
  return buf;
}

int
norecurse(char* path){
    char* buf = fmtname(path);
    if (buf[0] == '.' && buf[1] ==0){
        return 1;
    }
    if (buf[0] == '.' && buf[1] == '.' && buf[2] == 0){
        return 1;
    }
    return 0;
}

void
find(char *path, char *target)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  

  if (strcmp(fmtname(path), target) == 0) {// 处理过的path == target
    printf("%s\n", path);
  }

  switch(st.type){
  case T_DEVICE:
  case T_FILE:
    // printf("%s %d %d %d\n", fmtname(path), st.type, st.ino, (int) st.size);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("find: cannot stat %s\n", buf);
        continue;
      }
    //   printf("%s\n", buf);
    //   printf("%s\n", fmtname(buf));
    //   if (strcmp(fmtname(buf), target) == 0){
    //     printf("%s\n", buf);
    //   }
      if (norecurse(buf) == 0){
        find(buf, target);
      }
      
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc == 1){
    fprintf(2, "cannot find input path\n");
    exit(1);
  }
  if(argc == 2){
    find(".",argv[1]);
    exit(0);
  }
  
  if(argc == 3){
    find(argv[1], argv[2]);
    exit(0);
  }
  exit(0);
}
