/* 
 * File:   BanPCarsServer.c
 * Author: banshee
 *
 * Created on 3 de junio de 2015, 20:02
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>
#include "ext/SharedMemory.h"

#define PCARS_SHAREDMEM_FILE "$pcars$"

/*
 * 
 */
int main(int argc, char** argv) {

    printf("First Example\n");
    
    HANDLE hFileHandle       = OpenFileMapping(PAGE_READONLY, FALSE, PCARS_SHAREDMEM_FILE);
    SharedMemory *sharedData = (SharedMemory* )MapViewOfFile( hFileHandle, PAGE_READONLY, 0, 0, sizeof(SharedMemory));
    
    if(sharedData == NULL){
        CloseHandle(hFileHandle);
        perror("Error opening pcars shared memory file\n");
        return -1;
    }
    
    
    while(1)
        printf("RPMs : %f \n", sharedData->mRpm);
    
    return (EXIT_SUCCESS);
}

