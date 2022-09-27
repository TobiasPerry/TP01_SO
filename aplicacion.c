// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <semaphore.h>

/* According to POSIX.1-2001 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>

#define BUFSIZE 150


//Struct para guardar los datos necesarios de los esclavos
typedef struct payload {
    int pipeIn[2];
    int pipeOut[2];
    int pid;
}payload;

void crear_pipes(payload slaves[], int slavecount);
int crear_esclavos(payload slaves[], int slavecount, int * pid);
int abrir_result();
void cerrar_esclavos(payload slaves[], int slavecount);
int is_regular_file(const char *path);


int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void crear_pipes(payload slaves[], int slavecount){
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
}

int crear_esclavos(payload slaves[], int slavecount, int * pid){
    int i;
    for(i=0;i<slavecount && *pid!=0;i++){
        *pid=fork();
        if(*pid==-1){
            perror("forkerror");
            exit(-4);
        }
        else if(*pid==0){
            slaves[i].pid=getpid();
        }
        
    }
    i--;
    return i;
}

int abrir_result(){
    int resultado_fd = open("Resultado.txt", O_CREAT | O_WRONLY | O_TRUNC, 00666);
    if( resultado_fd == -1){
        perror("Resultado Open");
        exit(-5);
    }
    char title[]={"Hash\t\t\t\t\t\t\t Name\t\t\tPID\n"};
    write(resultado_fd,title, strlen(title));
    return resultado_fd;
}

void cerrar_esclavos(payload slaves[], int slavecount){
    int j;
    for(j=0;j<slavecount;j++){
        write(slaves[j].pipeIn[1],"0",2);
        close(slaves[j].pipeIn[1]);
        close(slaves[j].pipeIn[0]);
        close(slaves[j].pipeOut[1]);
        close(slaves[j].pipeOut[0]);
    }
}

int main(int argc, char* argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    int fifo;

    if (argc <= 1) {
        perror("No arguments");
        exit(-1);
    }

    //Consideramos que en promedio un esclavo procese 5 archivos
    const int slavecount = ceil((argc-1)*0.2);
    int pid =1;
    payload slaves[slavecount];

    crear_pipes(slaves, slavecount);

    int i = crear_esclavos(slaves, slavecount, &pid);
    //Por como esta formateado el for, en el valor de i queda una posicion
    //unica del arreglo de structs "payload" para cada esclavo

    if (pid == 0) {
        //Proceso Esclavo
        int finished = 0;

        close(slaves[i].pipeIn[1]);     
        close(slaves[i].pipeOut[0]);
        int k;
        for(k=0;k<slavecount;k++){
            if(k!=i){
                close(slaves[k].pipeIn[0]);
                close(slaves[k].pipeIn[1]);
                close(slaves[k].pipeOut[0]);
                close(slaves[k].pipeOut[1]);
            }
        }

        while(finished==0){
            char path[BUFSIZE] = {'\0'};
            read(slaves[i].pipeIn[0], path, BUFSIZE);       //aplicacion pasa nombre del archivo a procesar

            if(strcmp(path,"0")!=0){                        //La aplicacion manda un 0 si no hay mas archivos para procesar
                char command[BUFSIZE] = "md5sum ";
                char buf[BUFSIZE]={'\0'};
                if (is_regular_file(path)){
                    strcat(command, path);
                    FILE * md5 = popen(command, "r");
                    fgets(buf, BUFSIZE, md5);
                    buf[strlen(buf)-1] = '\0';
                    pclose(md5);
                    char pidbuf[10] = {'\0'};
                    snprintf(pidbuf,10,"  %d\n", slaves[i].pid);        //se concatena el pid
                    strcat(buf,pidbuf);
                }
                else{
                    sprintf(buf, "Is a directory or does not exist  %s\t%d\n", path,slaves[i].pid);
                }
                write(slaves[i].pipeOut[1], buf, strlen(buf));     //se le devuelve la aplicacion lo pedido

            }else{
                finished=1;
            }
        }
        //Una vez que termina de trabajar el esclavo, cierra los pipes
        close(slaves[i].pipeIn[0]);     
        close(slaves[i].pipeOut[1]);
        
    }else{
        //Proceso aplicacion
        int p;
        for(p=0;p<slavecount;p++){
            close(slaves[p].pipeIn[0]);
            close(slaves[p].pipeOut[1]);
            
        }
        char * pipeName = "/tmp/namedPipe";
        if (mkfifo(pipeName, S_IRUSR | S_IWUSR | S_IWOTH | S_IROTH) == -1){
            perror("mkfifo");
            exit(-2);
        }
        
        fifo = open(pipeName, O_RDWR);
        if (fifo == -1){
            perror("open");
            exit(-2);
        }
        
        //Impresion para cuando se ejecuta como ./md5 files/* | ./vista
        printf("%s\n", pipeName);
        sleep(2);

        int fileIndex= 0;
        int j;

        //se le pasa un archivo a cada esclavo por pipe, obligatoriamente hay mas archivos que esclavos
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

        int resultado_fd = abrir_result();

        while(archivos_a_leer > 0){
            ready_fds = current_fds;

            if( select(FD_SETSIZE,&ready_fds,NULL, NULL, NULL) == -1) { perror("select");}
            for(j=0 ; j < slavecount; j++){
                //se busca que pipe esta disponible
                if( FD_ISSET(slaves[j].pipeOut[0], &ready_fds)){
                    char resultado[BUFSIZE] = {'\0'};

                    read(slaves[j].pipeOut[0], resultado , BUFSIZE);
                    archivos_a_leer--;
                    //se escribe resultado de archivo procesado en resultado.txt
                    write(resultado_fd, resultado, strlen(resultado));
                    //se escribe resultado en fifo
                    write(fifo, resultado, strlen(resultado));

                    //se le pasa otro archivo al esclavo para que procese
                    if(fileIndex < argc-1){
                        write(slaves[j].pipeIn[1],argv[fileIndex+1],BUFSIZE);
                        fileIndex++;
                    }
                }
            }
        }

        close(resultado_fd);

        cerrar_esclavos(slaves, slavecount);

        //Con este valor se le avisa al proceso vista que no hay nada mas para imprimir
        write(fifo, "\0",1);
        close(fifo);
        remove(pipeName);
    }    
    
    return 0;

}


