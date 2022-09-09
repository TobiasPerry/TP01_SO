#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define BUFSIZE 50;
typedef struct payload {
   pid_t pid;
   char path[BUFSIZE];
   char md5[BUFSIZE];//se puede optimizar el size
};

int main(){
    wait(1);
    


}