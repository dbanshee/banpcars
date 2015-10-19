#ifndef JSON_H
#define	JSON_H

#include <stdbool.h>
#include <stdio.h>
#include <jansson.h>

#define JSON_STACK_SIZE 256


typedef struct jSonDocument { 
    json_t*         root;
    json_error_t    error;
    json_t*         stack[JSON_STACK_SIZE];
    int             stackIdx;
} jSonDocument;

int initializeJSonDocument(jSonDocument* doc);
void freeJSonDocument(jSonDocument* doc);

int   parseJSon(jSonDocument* doc, const char* text);
const char* getJSonString(jSonDocument* doc);

void addJSonStringField(jSonDocument* doc, char* fieldName, char* fieldValue);
void addJSonIntegerField(jSonDocument* doc, char* fieldName, int fieldValue);
void addJSonFloatField(jSonDocument* doc, char* fieldName, float fieldValue);
void addJSonBoolField(jSonDocument* doc, char* fieldName, bool fieldValue);

void openJSonObject(jSonDocument* doc, char* objectName);
void closeJSonObject(jSonDocument* doc);

void openJSonArray(jSonDocument* doc, char* arrayName);
void closeJSonArray(jSonDocument* doc);
int addJSonArrayString(jSonDocument* doc, char* fieldValue);
int addJSonArrayInteger(jSonDocument* doc, int fieldValue);
int addJSonArrayFloat(jSonDocument* doc, float fieldValue);
int addJSonArrayBool(jSonDocument* doc, bool fieldValue);


int getArraySize(jSonDocument* doc, char* arrayName);
const char* getArrayStringElem(jSonDocument* doc, char* arrayName, int nelem);


#endif	/* JSON_H */

