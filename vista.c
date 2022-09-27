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

void getParams(char * pipeName);

void getParams(char * pipeName){
    if( fgets(pipeName, RBUFF, stdin) == NULL){perror("fgets error");
    exit(-3);}
    pipeName[strlen(pipeName)-1]='\0';              //fgets agrega el '\n' que lee, se lo remueve
}


int main(int argc, char * argv[]){     //Primer parametro es el nombre del shm y el segundo es el tamanio

    char pipeName[RBUFF] = {'\0'};
    int fifo_vista;


    if(argc > 1){        //Proceso vista se corrio en comandos distintos, se pasa la info por parametro
        //Orden de parametros: nombre de fifo
        fifo_vista = open(argv[1], O_RDONLY);

    }else{      //Proceso vista se pipeo de la forma: ./hm5 files/* | ./vista
        //se lee la entrada estandar (salida estandar de hm5) y se obtienen el nombre del fifo
        getParams(pipeName);

        fifo_vista = open(pipeName, O_RDONLY);
    }

    int finished = 0;
    printf("Hash\t\t\t\t   Name\t\t Pid\n");
    while (finished == 0) {
        char buffer[BUFFSIZE] = {'\0'};
        read(fifo_vista, buffer, BUFFSIZE);
        int strl = strlen(buffer);
        if (strl == 0) {
            finished = 1;
        } else {
            printf("%s", buffer);
        }
    }
    
    close(fifo_vista);
    return 0;
}

