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
#define BUFFSIZE 150


void printInfo(void * address, sem_t* mySem);
void * abrir_shm_vista(int * fd_view, char * shmName, char* shmSize);

void printInfo(void * address, sem_t* mySem) {
    int finished = 0;
    char *x_vista = (char *) address;
    printf("Hash\t\t\t\t   Name\t\t Pid\n");
    while (finished == 0) {

        //vista espera hasta que aplicacion mande señal de que hay algo para leer
        sem_wait(mySem);

        int strl = strlen(x_vista);
        if (strl == 0) {
            finished = 1;
        } else {
            printf("%s", x_vista);
            x_vista = x_vista + strlen(x_vista) + 1;
        }
    };
}

void * abrir_shm_vista(int * fd_view, char * shmName, char* shmSize){
    *fd_view = shm_open(shmName, O_RDWR, 0);
    if (*fd_view == -1) perror("shm_open vista");

    void * addr_vista = mmap(NULL, atoi(shmSize), PROT_READ|PROT_WRITE, MAP_SHARED, *fd_view, 0);
    if( addr_vista == MAP_FAILED) perror("mmap vista");

    return addr_vista;
}

sem_t * abrir_sem_vista(char * semName){
    sem_t * mySem = sem_open(semName, O_RDONLY, S_IRUSR,0);
    if (mySem == SEM_FAILED){
        perror("sem_open");
        exit(-2);
    }
    return mySem;
}

void getParams(char * name, char * sizeString, char * semName){
    if( fgets(name, RBUFF, stdin) == NULL) perror("fgets error");
    name[strlen(name)-1]='\0';              //fgets agrega el '\n' que lee, se lo remueve

    if( fgets(sizeString, RBUFF, stdin) == NULL) perror("fgets error");

    if( fgets(semName, RBUFF, stdin) == NULL) perror("fgets error");
    semName[strlen(semName)-1] = '\0';     //fgets agrega el '\n' que lee, se lo remueve
}


int main(int argc, char * argv[]){     //Primer parametro es el nombre del shm y el segundo es el tamanio

    void * addr_vista;
    sem_t * mySem;
    char semName[RBUFF] = {'\0'};
    char name[RBUFF] = {'\0'};
    int fd_view;
    int size;

    if(argc > 1){        //Proceso vista se corrio en comandos distintos, se pasa la info por parametro
        //Orden de parametros: nombre de shm, tamaño de shm, nombre de semaforo
        addr_vista = abrir_shm_vista(&fd_view,argv[1],argv[2]);
        mySem = abrir_sem_vista(argv[3]);

    }else{      //Proceso vista se pipeo de la forma: ./hm5 files/* | ./vista
                    //se lee la entrada estandar (salida estandar de hm5) y se obtienen el nombre, el tamanio del shm y el nombre del semaforo
        char sizeString[RBUFF] = {'\0'};
        getParams(name, sizeString, semName);

        addr_vista = abrir_shm_vista(&fd_view,name,sizeString);
        size = atoi(sizeString);

        mySem = abrir_sem_vista(semName);

    }

         printInfo(addr_vista, mySem);


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

