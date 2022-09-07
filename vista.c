#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define RBUFF 20


int main(int argc, char * argv[]){     //Primer parametro es el nombre del shm y el segundo es el tamanio
    if(argc > 1){        //Proceso vista se corrio en comandos distintos, se pasa la info por parametro
        int fd_view = shm_open(argv[1], O_RDWR,0);
        if (fd_view == -1) perror("shm_open vista");

        void *addr_vista = mmap(NULL, atoi(argv[2]), PROT_READ|PROT_WRITE, MAP_SHARED, fd_view, 0);
        if( addr_vista == MAP_FAILED) perror("mmap vista");

        //int *x_vista = (int *) addr_vista;



    }else{               //Proceso vista se pipeo de la forma: ./hm5 files/* | ./vista
        //se lee la entrada estandar y se obtienen el nombre y el tamanio del shm
        char input[RBUFF] = {'\0'};
        if( read(STDIN_FILENO, input, RBUFF) == -1) perror("Standard Input Read");
        
        const char delim = '\n';
        char * token;

        token = strtok(input, &delim);

        char name[RBUFF] = {'\0'};
        strcpy(name, token);

        token = strtok(NULL, &delim );

        char sizeString[RBUFF] = {'\0'};
        strcpy(sizeString, token);

        int size = atoi(sizeString);

        
        int fd_view = shm_open(name, O_RDWR,0);
        if (fd_view == -1) perror("shm_open vista");

        void *addr_vista = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_view, 0);
        if( addr_vista == MAP_FAILED) perror("mmap vista");

        //int *x_vista = (int *) addr_vista;


    }
    return 0;
}