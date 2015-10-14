#ifndef JSON_H
#define	JSON_H

#include <stdbool.h>
#include <stdio.h>


typedef struct jSonDocument {
    bool    closedDoc;
    
    int     nOpenObjects;
    int     nOpenArrays;
    
    char*   buff;
    int     buffLen;
    int     buffBuffTotalSize;
    
} jSonDocument;

int initializeJSonDocument(jSonDocument* doc);
void freeJSonDocument(jSonDocument* doc);

int   parseJSon(jSonDocument* doc, char* src);
char* getJSonString(jSonDocument* doc);


void addJSonStringField(jSonDocument* doc, char* fieldName, char* fieldValue);
void addJSonIntegerField(jSonDocument* doc, char* fieldName, int fieldValue);
void addJSonFloatField(jSonDocument* doc, char* fieldName, float fieldValue);
void addJSonBoolField(jSonDocument* doc, char* fieldName, bool fieldValue);

void openJSonObject(jSonDocument* doc, char* objectName);
void closeJSonObject(jSonDocument* doc);

void openJSonArray(jSonDocument* doc, char* arrayName);
void closJSonArray(jSonDocument* doc);
void addJSonArrayString(jSonDocument* doc, char* fieldValue);
void addJSonArrayInteger(jSonDocument* doc, int fieldValue);
void addArrayJSonFloat(jSonDocument* doc, float fieldValue);
void addArrayJSonBool(jSonDocument* doc, bool fieldValue);


void endJSonDocument(jSonDocument* doc);






#endif	/* JSON_H */

