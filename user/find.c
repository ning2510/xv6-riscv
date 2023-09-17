#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
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

void
lookup(char *path, char *filename)
{
    int fd;
    char *p = 0;
    char buf[512] = {0};
    struct stat st;
    struct dirent de;

    if((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return ;
    }

    if(fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return ;
    }

    switch(st.type) {
    case T_DIR:
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
            printf("find: path too long\n");
            break;
        }

        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)) {
            if(de.inum == 0 || strcmp(de.name, ".") == 0 
                || strcmp(de.name, "..") == 0)
                continue;
            
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0) {
                printf("find: cannot stat %s\n", buf);
                continue;
            }

            if(strcmp(fmtname(buf), filename) == 0) {
                printf("%s\n", buf);
            }
            lookup(buf, filename);
        }
        break;
    }

    close(fd);
    return ;
}

int
main(int argc, char **argv)
{
    if(argc <= 2) {
        fprintf(2, "usage: [path] [filename]\n");
        exit(1);
    }

    lookup(argv[1], argv[2]);
    exit(0);
}