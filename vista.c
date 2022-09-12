// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

#define RBUFF 20
#define BUFFSIZE 100


int main(int argc, char * argv[]){     //Primer parametro es el nombre del shm y el segundo es el tamanio

    void * addr_vista;
    sem_t * mySem;
    char semName[RBUFF] = {'\0'};
    char name[RBUFF] = {'\0'};
    int size = 0;

    if(argc > 1){        //Proceso vista se corrio en comandos distintos, se pasa la info por parametro
        //Orden de parametros: nombre de shm, tamaño de shm, nombre de semaforo

        //apertura de shared memory
        int fd_view = shm_open(argv[1], O_RDWR,0);
        if (fd_view == -1) perror("shm_open vista");

        addr_vista = mmap(NULL, atoi(argv[2]), PROT_READ|PROT_WRITE, MAP_SHARED, fd_view, 0);
        if( addr_vista == MAP_FAILED) perror("mmap vista");

        //apertura de semaforo
        mySem = sem_open(argv[3], O_RDONLY, S_IRUSR,0);
        if (mySem == SEM_FAILED){
            perror("sem_open");
            exit(-2);
        }


    }else{      //Proceso vista se pipeo de la forma: ./hm5 files/* | ./vista
        //se lee la entrada estandar (salida estandar de hm5) y se obtienen el nombre, el tamanio del shm y el nombre del semaforo
        if( fgets(name, RBUFF, stdin) == NULL) perror("fgets error");
        name[strlen(name)-1]='\0';              //fgets agrega el '\n' que lee, se lo remueve

        char sizeString[RBUFF] = {'\0'};
        if( fgets(sizeString, RBUFF, stdin) == NULL) perror("fgets error");

        size = atoi(sizeString);

        //apertura de shared memory
        int fd_view = shm_open(name, O_RDWR,0);
        if (fd_view == -1) perror("shm_open vista");

        addr_vista = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_view, 0);
        if( addr_vista == MAP_FAILED) perror("mmap vista");

        
        if( fgets(semName, RBUFF, stdin) == NULL) perror("fgets error");
        semName[strlen(semName)-1] = '\0';     //fgets agrega el '\n' que lee, se lo remueve

        //apertura de semaforo
        mySem = sem_open(semName, O_RDONLY, S_IWUSR,0);
        if (mySem == SEM_FAILED){
            perror("sem_open vista");
            exit(-2);
        }
    }

    int finished = 0;
    char * x_vista = (char *) addr_vista;
    while (finished == 0){

        char message[BUFFSIZE] = {'\0'};
        int index = 0;

        //vista espera hasta que aplicacion mande señal de que hay algo para leer
        sem_wait(mySem);

        //vista lee resultado de un archivo del shm hasta un '\0' (delimitador)
        // while(*x_vista != '\0'){
        //     message[index++] = *x_vista;
        //     x_vista++;
        // }
        // //Si lo primero que manda aplicacion es un '\0', significa que no hay nada mas para imprimir
        // if (*x_vista == '\0' && index == 0){
        //     finished = 1;
        // }
        // x_vista++;               0 1 2 3 4 5 1
        // printf("%s\n", message); a b c d e
         int strl = strlen(x_vista);
         if (strl==0){
            finished=1;
         }
         else{
            printf("%s",x_vista);
            x_vista=x_vista+strlen(x_vista)+1;
         }
        
            
         
         


    }

    //se cierra shm y semaphore
    if(argc > 1){
        munmap(NULL, atoi(argv[2]));
        sem_close(mySem);
    }
    else{
        munmap(NULL, size);
        sem_close(mySem);
    }
    return 0;
}