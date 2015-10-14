#include "../headers/jSon.h"
#include "../headers/logger.h"

#include <stdlib.h>
#include <string.h>


#define DEFAULT_BUFF_LEN 1024

int initializeJSonDocument(jSonDocument* doc){    
    memset(doc, 0, sizeof(jSonDocument));
    
    doc->closedDoc      = false;
    doc->nOpenObjects   = doc->nOpenArrays = 0;
    
    doc->buffLen = 0;
    doc->buffBuffTotalSize = DEFAULT_BUFF_LEN;
    if((doc->buff = malloc(sizeof(char)*DEFAULT_BUFF_LEN)) == NULL) {
        blog(LOG_ERROR, "Error allocating memory for Rest message");
        return -1;
    }
    
    // Init Document
    doc->buffLen = sprintf(doc->buff, "{ ");
}

int extendDocument(jSonDocument* doc) {
    if(doc->buffLen >=  doc->buffBuffTotalSize / 2){ 
        if((doc->buff = realloc(doc->buff, doc->buffBuffTotalSize+DEFAULT_BUFF_LEN)) == NULL){
            blog(LOG_ERROR, "Error reallocating memory for Rest message");
            return -1;
        }
        doc->buffBuffTotalSize += DEFAULT_BUFF_LEN;
    }
}

void freeJSonDocument(jSonDocument* doc){
    free(doc->buff);
}

int parseJSon(jSonDocument* doc, char* src){
    
}

char* getJSonString(jSonDocument* doc){
    return doc->buff;
}


void addJSonStringField(jSonDocument* doc, char* fieldName, char* fieldValue){
    int len;
    
    extendDocument(doc);
    len = sprintf(&doc->buff[doc->buffLen], "%s: \"%s\", ", fieldName, fieldValue);
    doc->buffLen += len;
}

void addJSonIntegerField(jSonDocument* doc, char* fieldName, int fieldValue){
    int len;
    
    extendDocument(doc);
    len = sprintf(&doc->buff[doc->buffLen], "%s: %d, ", fieldName, fieldValue);
    doc->buffLen += len;
    
}

void addJSonFloatField(jSonDocument* doc, char* fieldName, float fieldValue) {
    int len;
    
    extendDocument(doc);
    len = sprintf(&doc->buff[doc->buffLen], "%s: %f, ", fieldName, fieldValue);
    doc->buffLen += len;
}

void addJSonBoolField(jSonDocument* doc, char* fieldName, bool fieldValue){
    int len;
    
    extendDocument(doc);
    if(fieldValue)
        len = sprintf(&doc->buff[doc->buffLen], "%s: true, ", fieldName);
    else
        len = sprintf(&doc->buff[doc->buffLen], "%s: false, ", fieldName);
    
    doc->buffLen += len;
}

void openJSonObject(jSonDocument* doc, char* objectName){
    int len;
    
    extendDocument(doc);
    
    if(objectName == NULL)
        len = sprintf(&doc->buff[doc->buffLen], " - { ");
    else
        len = sprintf(&doc->buff[doc->buffLen], " - %s: { ", objectName);
    
    doc->buffLen += len;
    doc->nOpenObjects++;
}

void closeJSonObject(jSonDocument* doc){
    int len;
    
    extendDocument(doc);
    
    if(doc->buffLen >= 2 && memcmp(&doc->buff[doc->buffLen-2], ",", 1) == 0){
        doc->buff[doc->buffLen-2] = '\0';
        doc->buffLen = doc->buffLen - 2;
    }
    
    len = sprintf(&doc->buff[doc->buffLen], " } ");
    doc->buffLen += len;
    doc->nOpenObjects--;
}

void openJSonArray(jSonDocument* doc, char* arrayName){
    int len;
    
    extendDocument(doc);
    
    if(arrayName == NULL)
        len = sprintf(&doc->buff[doc->buffLen], " - [ ");
    else
        len = sprintf(&doc->buff[doc->buffLen], " - %s: [ ", arrayName);
    
    doc->buffLen += len;
    doc->nOpenArrays++;
}

void closeJSonArray(jSonDocument* doc){
    int len;
    
    extendDocument(doc);
    
    if(doc->buffLen >= 2 && memcmp(&doc->buff[doc->buffLen-2], ",", 1) == 0){
        doc->buff[doc->buffLen-2] = '\0';
        doc->buffLen = doc->buffLen - 2;
    }
    
    len = sprintf(&doc->buff[doc->buffLen], " ] ");
    doc->buffLen += len;
    doc->nOpenArrays--;
}

void addJSonArrayString(jSonDocument* doc, char* fieldValue){
    int len;
    
    extendDocument(doc);
    len = sprintf(&doc->buff[doc->buffLen], "%s , ", fieldValue);
    doc->buffLen += len;
}

void addJSonArrayInteger(jSonDocument* doc, int fieldValue){
    int len;
    
    extendDocument(doc);
    len = sprintf(&doc->buff[doc->buffLen], "%d, ", fieldValue);
    doc->buffLen += len;
}

void addArrayJSonFloat(jSonDocument* doc, float fieldValue){
    int len;
    
    extendDocument(doc);
    len = sprintf(&doc->buff[doc->buffLen], "%f, ", fieldValue);
    doc->buffLen += len;
    
}

void addArrayJSonBool(jSonDocument* doc, bool fieldValue){
    int len;
    
    extendDocument(doc);
    
    if(fieldValue)
        len = sprintf(&doc->buff[doc->buffLen], "true, ");
    else
        len = sprintf(&doc->buff[doc->buffLen], "false, ");
    
    doc->buffLen += len;
}

void endJSonDocument(jSonDocument* doc){
    extendDocument(doc);
    
    if(doc->buffLen >= 2 && memcmp(&doc->buff[doc->buffLen-2], ",", 1) == 0){
        doc->buff[doc->buffLen-2] = '\0';
        doc->buffLen = doc->buffLen - 2;
    }
    strcpy(&doc->buff[doc->buffLen], " }");
    doc->buffLen++;
}
