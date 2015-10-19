#include "../headers/jSon.h"
#include "../headers/logger.h"

#include <stdlib.h>
#include <string.h>


int initializeJSonDocument(jSonDocument* doc){    
    memset(doc, 0, sizeof(jSonDocument));
    
    doc->root = json_object();
    doc->stackIdx = 0;
    doc->stack[doc->stackIdx] = doc->root;
}

void pushJSonNode(jSonDocument* doc, json_t * n){
    if(doc->stackIdx < JSON_STACK_SIZE)
        doc->stack[++doc->stackIdx] = n; // Controlar maximo
}

void popJSonNode(jSonDocument* doc){
    if(doc->stackIdx > 0){
        doc->stack[doc->stackIdx] = NULL;
        doc->stackIdx--;
    }
}

void freeJSonDocument(jSonDocument* doc){
    if(doc->root != NULL) 
        json_decref(doc->root);
}

int parseJSon(jSonDocument* doc, const char* text){
    if(doc->root != NULL){
        freeJSonDocument(doc);
    } 
    
    initializeJSonDocument(doc);
    
    doc->root = json_loads(text, 0, &doc->error);   
    
    if(doc->root == NULL){
        blog(LOG_ERROR, "Error building JSon for text %s on line %d: %s", text, doc->error.line, doc->error.text);
        fflush(stdout);
        return -1;
    }
    
    return 1;
}

const char* getJSonString(jSonDocument* doc){
    return json_dumps(doc->root, 0);
}


void addJSonStringField(jSonDocument* doc, char* fieldName, char* fieldValue){
    json_object_set_new(doc->stack[doc->stackIdx], fieldName, json_string(fieldValue));
}

void addJSonIntegerField(jSonDocument* doc, char* fieldName, int fieldValue){
    json_object_set_new(doc->stack[doc->stackIdx], fieldName, json_integer(fieldValue));
}

void addJSonFloatField(jSonDocument* doc, char* fieldName, float fieldValue) {
    json_object_set_new(doc->stack[doc->stackIdx], fieldName, json_real(fieldValue));
}

void addJSonBoolField(jSonDocument* doc, char* fieldName, bool fieldValue){
    json_object_set_new(doc->stack[doc->stackIdx], fieldName, json_boolean(fieldValue));
}

void openJSonObject(jSonDocument* doc, char* objectName){
    json_t* obj = json_object();
    
    json_object_set(doc->stack[doc->stackIdx], objectName, obj);
    pushJSonNode(doc, obj);
}

void closeJSonObject(jSonDocument* doc){
    popJSonNode(doc);
}

void openJSonArray(jSonDocument* doc, char* arrayName){
    json_t* ar = json_array();
    
    json_object_set(doc->stack[doc->stackIdx], arrayName, ar);
    pushJSonNode(doc, ar);
    
}

void closeJSonArray(jSonDocument* doc){
    popJSonNode(doc);
}

void addJSonArrayString(jSonDocument* doc, char* fieldValue){
    if(!json_is_array(doc->stack[doc->stackIdx])) {
        blog(LOG_ERROR, "Error adding String to JSon Array. JSon Top Stack is not array");
        return;
    }
    json_array_append(doc->stack[doc->stackIdx], json_string(fieldValue));
}

void addJSonArrayInteger(jSonDocument* doc, int fieldValue){
    if(!json_is_array(doc->stack[doc->stackIdx])) {
        blog(LOG_ERROR, "Error adding Inteeger to JSon Array. JSon Top Stack is not array");
        return;
    }
    json_array_append(doc->stack[doc->stackIdx], json_integer(fieldValue));
}

void addJSonArrayFloat(jSonDocument* doc, float fieldValue){
    if(!json_is_array(doc->stack[doc->stackIdx])) {
        blog(LOG_ERROR, "Error adding Float to JSon Array. JSon Top Stack is not array");
        return;
    }
    json_array_append(doc->stack[doc->stackIdx], json_real(fieldValue));
    
}

void addJSonArrayBool(jSonDocument* doc, bool fieldValue){
    if(!json_is_array(doc->stack[doc->stackIdx])) {
        blog(LOG_ERROR, "Error adding Boolean to JSon Array. JSon Top Stack is not array");
        return;
    }
    json_array_append(doc->stack[doc->stackIdx], json_boolean(fieldValue));
}

json_t* getArray(jSonDocument* doc, char* arrayName){
    json_t* arr  = json_object_get(doc->root, arrayName);
    
    if(arr == NULL){
        blog(LOG_ERROR, "Array '%s' not exists in document", arrayName);
        return NULL;
    }
    
    if(!json_is_array(arr))
    {
        blog(LOG_ERROR, "Elem '%s' is not an array in document", arrayName);
        return NULL;
    }
    
    return arr;
}

int getArraySize(jSonDocument* doc, char* arrayName){
    json_t* arr = getArray(doc, arrayName);
    
    if(arr == NULL)
        return -1;
    else
        return json_array_size(arr);
}

const char* getArrayStringElem(jSonDocument* doc, char* arrayName, int nelem){
    json_t* arr;
    json_t* elem;
    
    if((arr = getArray(doc, arrayName)) == NULL || nelem > json_array_size(arr))
        return NULL;
    else{
        elem = json_array_get(arr, nelem);
        if(!json_is_string(elem)) 
            return NULL;
        else
            return json_string_value(elem);
    }
}