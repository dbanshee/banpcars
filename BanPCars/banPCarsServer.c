/* 
 * File:   BanPCarsServer.c
 * Author: banshee
 *
 * Created on 3 de junio de 2015, 20:02
 */

#include <stdio.h>
#include <stdlib.h>
#include "logger.h"
#include "pcarsApi.h"

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

/*
 * 
 */
pCarsContext ctx;

void signalHandler(int sigNum){
    
    if(sigNum == SIGTERM)
        blog(LOG_INFO, "Log Signal SIGTERM received");
    else if(sigNum == SIGINT)
        blog(LOG_INFO, "Log Signal SIGINT received");
    else if(sigNum == SIGQUIT)
        blog(LOG_INFO, "Log Signal SIGQUIT received");
    
    else
        return;
    
   // finishServer(&serverCtx);
    exit(0);
}


int main(int argc, char** argv) {
    
    signal(SIGTERM, signalHandler);
    signal(SIGINT,  signalHandler);
    signal(SIGQUIT, signalHandler);

    printf("First Example\n");
    
    printf("-----------------------------------------\n");
    printf("-- BanPCars Server                       \n");
    printf("--     Banshee 2014                      \n");
    printf("-- Start at : %s\n", getCurrentDate());
    printf("-----------------------------------------\n\n\n");
    
    while(1){
//        blog(LOG_INFO, "Bucle\n");        
//        printf("Bucle\n");
    }
    
    
//    if(initializePCarsContext(&ctx) != 0){
//        blog(LOG_ERROR, "Error initializing PCars Context. Aborting Server...");
//        return -1;            
//    }
//    
//    
//    while(1){
//        printf("RPMs %f: \n", ctx.shmMem->mRpm);
//    }
    
    /* Abriendo puerto serie
    struct termios toptions;
    int fd;
    
    char serialPort [64] = "/dev/ttyS3";
    fd = open(serialPort, O_RDWR | O_NOCTTY);
    if (fd == -1)  {
        blog(LOG_ERROR, "Open %s", serialPort);
        return -1;
    }
    */
    
    
    // Memoria compartida
/*
    int i;
    int fd;
    int *map; 

    fd = open("$pcars$", O_RDONLY);
    if (fd == -1) {
	perror("Error opening file for reading");
	exit(EXIT_FAILURE);
    }

//    map = mmap(0, FILESIZE, PROT_READ, MAP_SHARED, fd, 0);
//    if (map == MAP_FAILED) {
//	close(fd);
//	perror("Error mmapping the file");
//	exit(EXIT_FAILURE);
//    }
//    
//    for (i = 1; i <=NUMINTS; ++i) {
//	printf("%d: %d\n", i, map[i]);
//    }
//
//    if (munmap(map, FILESIZE) == -1) {
//	perror("Error un-mmapping the file");
//    }
    close(fd);
    return 0;
*/    
    
    return (EXIT_SUCCESS);
}

