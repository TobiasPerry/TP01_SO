#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

#define RBUFF 20
#define BUFFSIZE 50


int main(int argc, char * argv[]){     //Primer parametro es el nombre del shm y el segundo es el tamanio

    void * addr_vista;
    sem_t * mySem;


    if(argc > 1){        //Proceso vista se corrio en comandos distintos, se pasa la info por parametro
        int fd_view = shm_open(argv[1], O_RDWR,0);
        if (fd_view == -1) perror("shm_open vista");

        addr_vista = mmap(NULL, atoi(argv[2]), PROT_READ|PROT_WRITE, MAP_SHARED, fd_view, 0);
        if( addr_vista == MAP_FAILED) perror("mmap vista");

//        mySem = sem_open(argv[3], O_RDONLY, S_IWUSR,0);
//        if (mySem == SEM_FAILED){
//            perror("sem_open");
//            exit(-2);
//        }


    }else{               //Proceso vista se pipeo de la forma: ./hm5 files/* | ./vista
        //se lee la entrada estandar y se obtienen el nombre y el tamanio del shm

        char name[RBUFF] = {'\0'};
        if( fgets(name, RBUFF, stdin) == NULL) perror("fgets error");


        name[strlen(name)-1]='\0';

        char sizeString[RBUFF] = {'\0'};
        if( fgets(sizeString, RBUFF, stdin) == NULL) perror("fgets error");

        int size = atoi(sizeString);


        int fd_view = shm_open(name, O_RDWR,0);
        if (fd_view == -1) perror("shm_open vista");

        char semName[RBUFF] = {'\0'};
        if( fgets(semName, RBUFF, stdin) == NULL) perror("fgets error");
        printf("%s",semName);
        semName[strlen(semName-1)] = '\0';

        mySem = sem_open(semName, O_RDONLY, S_IWUSR,0);
        if (mySem == SEM_FAILED){
            perror("sem_open vista");
            exit(-2);
        }


        addr_vista = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_view, 0);
        if( addr_vista == MAP_FAILED) perror("mmap vista");

    }


    int finished = 0;
    char * x_vista = (char *) addr_vista;
    while (finished == 0){

        char message[BUFFSIZE] = {'\0'};
        int index = 0;
        sem_wait(mySem);
        while(*x_vista != '\0'){
            message[index++] = *x_vista;
            x_vista++;
        }
        if (*x_vista == '\0' && index == 0)
            finished = 1;
        x_vista++;

        printf("%s\n", message);
    }


    return 0;
}