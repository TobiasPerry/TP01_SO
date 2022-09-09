#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define SHMDIR "/myshm"
#define SHMSIZE 2048
#define BUFSIZE 50

//deftype struct martin {
//    pid_t
//    lo que vuelve de los pipes
//    md5
//};

//TODO:HACER ESTE STRUCT PARA PASARLE A LOS SLAVES. UN ARRAY DE ESTE STRUCT
//TODO: SI O SI TIENE QUE HACERSE EXECV PARA EL SLAVE



int main(int argc, char* argv[]) {
    if (argc <= 1) {
        perror("No arguments");
        exit(-1);
    }

    sleep(2);
    int fd_app;
    fd_app = shm_open(SHMDIR, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_app == -1) {
        perror("shm_open app");
        exit(-2);
    }

    if (ftruncate(fd_app, SHMSIZE) == -1) {
        perror("ftruncate");
        exit(-2);
    }

    void *addr_parent = mmap(NULL, SHMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_app, 0);
    if (addr_parent == MAP_FAILED) {
        perror("mmap app");
        exit(-2);
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("%s\n%d\n", SHMDIR, SHMSIZE);
    // TODO : PROBAR CON 1 SLAVE. EL SELECT ES UN PROBLEMA PARA CUANDO HAY MAS DE 1 SLAVE


    // int counter = argc;   //FORKS DE ESCLAVOS
    // int fd;
    // while (counter > 0) {
    //     int fd_pipeIn[2];
    //     if (pipe(fd_pipeIn) == -1) {
    //         perror("pipe");
    //         exit(-3);
    //     }
    //     int fd_pipeOut[2];
    //     if (pipe(fd_pipeOut) == -1) {
    //         perror("pipe");
    //         exit(-3);
    //     }

    //     fd = fork();
    //     if (fd == 0) {           //EL PROCESO ES ESCLAVO. DEBE ESTAR DESPIERTO HASTA QUE LA VARAIBLE QUEDAN_ARCHIVOS SEA 0.
    //         counter = 0;
    //         //wait();            //ESPERO QUE ME PASEN UN ARCHIVO POR EL PIPE TODO: hacer este wait y mil mas(chequear todos juntos)
    //         char path[BUFSIZE];
    //         read(fd_pipeIn[0], path, BUFSIZE);
    //         char *command = "md5 ";
    //         strcat(command, path);
    //         FILE *md5 = popen(command, "r");

    //         char buf[BUFSIZE];
    //         fgets(buf, BUFSIZE, md5);

    //         char result[BUFSIZE];
    //         sprintf(result, "%s\n%s\n%d\n", path, buf, getpid());   //TODO: que no imprima todo el path

    //         //TODO: HACER LOS DUP2
    //         write(fd_pipeOut[1], result, BUFSIZE);
    //         //SENDPOST();
    //     } else {
    //         counter--;
    //     }
    // }




    // munmap(NULL, SHMSIZE);
    // shm_unlink(SHMDIR);
    // close(fd_app);

    printf("Pre test\n");

    int *x_app = (int *) addr_parent;
    *x_app=10;

    while(1){
        printf("llegue aca\n");
        sleep(1);
    }

    return 0;

}




//    }
//
//    int global hay_archivos=1;
//    while ( hay_archivos )
//
//int resources = 0;
//
//if( fd == 0){	getpid
//wait(resources);		//semaforo para que no este la chance de que trabajen en el mismo archivo
//execv(...
//
//if( iterar()==null){
//hay_archivos = 0;
//}
//
//signal(resources);
//
//}
//else{
//wait();
//
//}
//
//exit()
//
//


