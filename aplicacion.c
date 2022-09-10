#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* According to POSIX.1-2001 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>

#define SHMDIR "/myshm"
#define SHMSIZE 2048
#define BUFSIZE 50


typedef struct payload {
    int pipeIn[2];
    int pipeOut[2];
    int pid;//se puede optimizar el size
}payload;

//TODO:HACER ESTE STRUCT PARA PASARLE A LOS SLAVES. UN ARRAY DE ESTE STRUCT
//TODO: SI O SI TIENE QUE HACERSE EXECV PARA EL SLAVE



int main(int argc, char* argv[]) {
    //setvbuf(stdout, NULL, _IONBF, 0);
    // if (argc <= 1) {
    //     perror("No arguments");
    //     exit(-1);
    // }

    // sleep(2);
    // int fd_app;
    // fd_app = shm_open(SHMDIR, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    // if (fd_app == -1) {
    //     perror("shm_open app");
    //     exit(-2);
    // }

    // if (ftruncate(fd_app, SHMSIZE) == -1) {
    //     perror("ftruncate");
    //     exit(-2);
    // }


    // void *addr_parent = mmap(NULL, SHMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_app, 0);
    // if (addr_parent == MAP_FAILED) {
    //     perror("mmap app");
    //     exit(-2);
    // }

    // printf("%s\n%d\n", SHMDIR, SHMSIZE);
    // TODO : PROBAR CON 1 SLAVE. EL SELECT ES UN PROBLEMA PARA CUANDO HAY MAS DE 1 SLAVE


    // int counter = argc;   //FORKS DE ESCLAVOS
    const int slavecount = ceil((argc-1)*0.2);
    int pid =1;
    payload slaves[slavecount];
    
    int i;
    for(i=0;i<slavecount;i++){
        if (pipe(slaves[i].pipeOut) == -1) {
                perror("pipe");
                exit(-3);
            }
        if (pipe(slaves[i].pipeIn) == -1) {
            perror("pipe");
            exit(-3);
        }
            
    }
    for(i=0;i<slavecount && pid!=0;i++){
        pid=fork();
        if(pid==-1){
            perror("forkerror");
        }
        else if(pid==0){
            slaves[i].pid=getpid();
        }
        
    }
    i--;
    
    if (pid == 0) {  
        //EL PROCESO ES ESCLAVO. DEBE ESTAR DESPIERTO HASTA QUE LA VARAIBLE QUEDAN_ARCHIVOS SEA 0.
        int finished = 0;
        
        while(finished==0){
            //ESPERO QUE ME PASEN UN ARCHIVO POR EL PIPE TODO: hacer este wait y mil mas(chequear todos juntos)

            char path[BUFSIZE];
            read(slaves[i].pipeIn[0], path, BUFSIZE);

            if(strcmp(path,"0")!=0){
                char command[BUFSIZE] = "md5sum ";
                strcat(command, path);

                FILE * md5 = popen(command, "r");

                char buf[BUFSIZE]={'\0'};
                fgets(buf, BUFSIZE, md5);
                buf[strlen(buf)-1]='\0';
                
                //TODO: que no imprima todo el path

                //TODO: HACER LOS DUP2
                char pidbuf[10] = {'\0'};
                snprintf(pidbuf,10,"  %d\n", slaves[i].pid);
                strcat(buf,pidbuf);
                //printf("%s\n", buf);
                write(slaves[i].pipeOut[1], buf, strlen(buf));
            }else{
                finished=1;
            }
        }
        close(slaves[i].pipeIn[0]);
        close(slaves[i].pipeOut[1]);
        exit(0);
        //printf("llegue\n");
        //SENDPOST();
    }else{
        /*int j;
        
        for(j=0;j<argc-1;j++){
            write(slaves[j%slavecount].pipeIn[1],argv[j+1],BUFSIZE);
        }
        for(j=0;j<slavecount;j++){
                write(slaves[j].pipeIn[1],"0",BUFSIZE);
            close(slaves[j].pipeIn[1]);
        }
        
        char resultado[10];
        read(slaves[0].pipeOut[0], resultado , BUFSIZE);
        printf("%s\n",resultado);
        
        */
        //close de todos los slaves

        //prueba con select
        //primer ciclo que se le mandan a todos los esclavos
        int fileIndex= 0;
        int j;

        for(j=0;j<slavecount;j++){
            write(slaves[j].pipeIn[1],argv[fileIndex+1],BUFSIZE);
            fileIndex++;
        }

        fd_set current_fds, ready_fds;

        FD_ZERO(&current_fds);
        for(j=0;j<slavecount;j++){
            FD_SET(slaves[j].pipeOut[0],&current_fds);
        }
        
        int archivos_a_leer = argc-1;
        char resultado[BUFSIZE] = {'\0'};

        while(archivos_a_leer > 0){
            ready_fds = current_fds;

            if( select(FD_SETSIZE,&ready_fds,NULL, NULL, NULL) == -1) { perror("select");}

            for(j=0 ; j < slavecount; j++){
                if( FD_ISSET(slaves[j].pipeOut[0], &ready_fds)){
                    read(slaves[j].pipeOut[0], resultado , BUFSIZE);
                    archivos_a_leer--;
                    printf("%s\n", resultado);
                    //le pasa por shm a vista para que imprima
                    //...

                    //le cargamos otro archivo para que procese
                    if(fileIndex < argc-1){
                        write(slaves[j].pipeIn[1],argv[fileIndex+1],BUFSIZE);
                        fileIndex++;
                    }
                }
            }
        }
        
        for(j=0;j<slavecount;j++){
            write(slaves[j].pipeIn[1],"0",BUFSIZE);
            close(slaves[j].pipeIn[1]);
        }
    }
//int * x_app = (int *) addr

// munmap(NULL, SHMSIZE);
// shm_unlink(SHMDIR);
// close(fd_app);

return 0;

}


