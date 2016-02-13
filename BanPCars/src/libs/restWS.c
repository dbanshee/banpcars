#include "../headers/restWS.h"
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "../headers/logger.h"
#include "../headers/jSon.h"

#define DEFAULT_MSG_LEN         1024

// HTTP REQUESTS
#define GET_STRING    "GET"
#define POST_STRING   "POST"
#define GET           0
#define POST          1


// RESTWS SERVICES
#define GET_FIELDS_SERVICE_URL  "/getfields"
#define GET_DATA_SERVICE_URL    "/getdata"

#define JSON_ERROR_RESPONSE     "{ \"error\" }"

#define DEFAULT_CON_INFO_BUFF_LEN  1024

typedef struct conInfo
{
  char* buff;
  int buffSize;
  int buffIdx;

  //struct MHD_PostProcessor *postsprocessor;
} conInfo;


struct MHD_Response * getFieldsHandler(restWSContext* ctx);
struct MHD_Response * getDataHandler(restWSContext* ctx, const char* upload_data, ssize_t* upload_data_size);


void initializeConInfo(conInfo* c){
    memset(c, 0, sizeof(conInfo));
    
    c->buffSize = sizeof(char)*DEFAULT_CON_INFO_BUFF_LEN;
    c->buffIdx  = 0;
    
    c->buff = malloc(c->buffSize);
    memset(c->buff, 0, c->buffSize);
}

void freeConInfo(conInfo* c) {
    if(c->buff != NULL)
        free(c->buff);
}

void addChunk(conInfo* c, const char* chunk, int chunkSize){
    int newSize;
    
    if((c->buffSize-c->buffIdx) <= chunkSize){
        newSize     = c->buffSize+chunkSize+DEFAULT_CON_INFO_BUFF_LEN;
        c->buff     = realloc(c->buff, newSize);
        c->buffSize = newSize;
    }
    
    memcpy(&c->buff[c->buffIdx], chunk, chunkSize);
    c->buffIdx += chunkSize;
    c->buff[c->buffIdx] = '\0';
}

static int sendPage(struct MHD_Connection *connection, const char *page) {
    int ret;
    struct MHD_Response *response;

    response = MHD_create_response_from_buffer (strlen (page), (void *) page, MHD_RESPMEM_PERSISTENT);
    
    MHD_add_response_header (response, "Content-Type", "application/json");
    
    if (!response)
        return MHD_NO;

    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);

    return ret;
}



static int requestHandler  (void * ctx,
                            struct MHD_Connection * connection,
                            const char * url,
                            const char * method,
                            const char * version,
                            const char * upload_data,
                            size_t * upload_data_size,
                            void ** con_cls) {
    
    jSonDocument jDocOut, jDocIn;
    int response;
    restWSContext* restWS = ctx;

    if (*con_cls == NULL){
        
        // Inicializacion de estructura para recivir el cuerpo de la peticion
        conInfo * cInfo;
        
        if((cInfo = malloc (sizeof (struct conInfo))) == NULL)
            return MHD_NO;
        
        initializeConInfo(cInfo);
        
        /* 
         * Manera correcta de procesar los chunks de una peticion POST
         * Falla al crear un processor con datos JSON
         * 
            if (0 == strcmp (method, POST_STRING)){
                //MHD_set_connection_value ???
                con_info->postsprocessor =
                  MHD_create_post_processor (connection, POSTBUFFERSIZE,
                                             iterate_post, (void *) con_info);

                if (NULL == con_info->postsprocessor){
                    free (con_info);
                    return MHD_NO;
                }
            }
        */

        *con_cls = (void *) cInfo;
        return MHD_YES;
    }
    
    
    if (strcmp (method, GET_STRING) == 0 && strcmp(url, GET_FIELDS_SERVICE_URL) == 0){
        
        initializeJSonDocument(&jDocOut);
        if(getFieldsResponse(restWS, &jDocOut) == -1){
            blog(LOG_ERROR, "Error procesing RestWS getfields Response");
            freeJSonDocument(&jDocOut);
            return MHD_NO;
        }
        
        response = sendPage(connection, getJSonString(&jDocOut));
        freeJSonDocument(&jDocOut);
        
    } else if(strcmp (method, POST_STRING) == 0 && strcmp(url, GET_DATA_SERVICE_URL) == 0){

        conInfo* con_info = *con_cls;

        if (*upload_data_size != 0){
            
            addChunk(con_info, upload_data, *upload_data_size);
            
            /* HARD DEBUG
                printf(">>>>>>  Received chunk(%d), >%s<\n", *upload_data_size, upload_data);
                printf(">>>>>>> buff(%d, %d) : >%s<\n", con_info->buffIdx, con_info->buffSize, con_info->buff);
            */
            *upload_data_size = 0;
            return MHD_YES;    
        }
        else{
            
            /* HARD DEBUG
                printf(">>>>>>  Received final chunk(%d), >%s<\n", *upload_data_size, upload_data);
                printf(">>>>>>> buff(%d, %d) : >'%s'<\n", con_info->buffIdx, con_info->buffSize, con_info->buff);
            */
            
            // Request completa, generar salida
            initializeJSonDocument(&jDocIn);
            parseJSon(&jDocIn, con_info->buff);
            freeConInfo(con_info);
         
            initializeJSonDocument(&jDocOut);
            if(getDataResponse(restWS, &jDocIn, &jDocOut) == -1){
                blog(LOG_ERROR, "Error procesing RestWS getdata Response");
                freeJSonDocument(&jDocOut);
                return MHD_NO;
            }
            
            //response = send_page(connection, errorpage);
            response = sendPage(connection, getJSonString(&jDocOut));
            freeJSonDocument(&jDocOut);
        }   
    }
    else{
        response = sendPage(connection, JSON_ERROR_RESPONSE);
    }
    
    *con_cls = NULL;
    return response;
}


void loadDefaultRestWSContext(restWSContext* ctx){
    memset(ctx, 0, sizeof(restWSContext));
}

void setRestWSPort(restWSContext* ctx, int port){
    ctx->port = port;
}

void setRestWSSource(restWSContext* ctx, simSourceContext* simSrcCtx){
    ctx->simSrcCtx = simSrcCtx;
}


int initializeRestWSContext(restWSContext* ctx){
    
    if((ctx->httpdCtx = MHD_start_daemon(   MHD_USE_THREAD_PER_CONNECTION,
                                            ctx->port,
                                            NULL,
                                            NULL,
                                            &requestHandler,
                                            ctx,
                                            MHD_OPTION_END)) == NULL){
        blog(LOG_ERROR, "Error inicializando RESTWS en puerto %d", ctx->port);
        return -1;
    }
    
    return 0;
}

void freeRestWSContext(restWSContext* ctx){
    if(ctx->httpdCtx != NULL){
        blog(LOG_INFO, "Deteniendo servidor REST.");
        MHD_stop_daemon(ctx->httpdCtx);
    }
}


int getFieldsResponse(restWSContext* ctx, jSonDocument* jdoc){
    
    if(getSimSourceFields(jdoc) == -1)
        return -1;
    
    return 0;
}

int getDataResponse(restWSContext* ctx, jSonDocument* in, jSonDocument* out){
    int i, idx, nFields;
    const char* fieldName;
    int dataFields[END_PCARS_FIELDS-1];
    
    if((nFields = getArraySize(in, "fields")) == -1){
        blog(LOG_ERROR, "Request getdata has no 'fields' array");
        return -1;
    }
    
    memset(dataFields, 0, sizeof(int)*(END_PCARS_FIELDS-1));
    for(i = 0; i < nFields; i++){
        fieldName = getArrayStringElem(in, "fields", i);
        
        if((idx = enumPCarsFieldsFromString(fieldName)) == -1){
            blog(LOG_WARN, "RestWS getdata unkown field : '%s'. Ignoring", fieldName);
        }else{
            dataFields[idx] = 1;
        }
    }
    
    if(getPCarsData(ctx->simSrcCtx, dataFields, out) == -1)
        return -1;
    
    return 0;
}

//static int
//iterate_post (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
//              const char *filename, const char *content_type,
//              const char *transfer_encoding, const char *data, uint64_t off,
//              size_t size)
//{
//  struct connection_info_struct *con_info = coninfo_cls;
//
//  if (0 == strcmp (key, "name"))
//    {
//      if ((size > 0) && (size <= MAXNAMESIZE))
//        {
//          char *answerstring;
//          answerstring = malloc (MAXANSWERSIZE);
//          if (!answerstring)
//            return MHD_NO;
//
//          snprintf (answerstring, MAXANSWERSIZE, greetingpage, data);
//          con_info->answerstring = answerstring;
//        }
//      else
//        con_info->answerstring = NULL;
//
//      return MHD_NO;
//    }
//
//  return MHD_YES;
//}
//
//static void
//request_completed (void *cls, struct MHD_Connection *connection,
//                   void **con_cls, enum MHD_RequestTerminationCode toe)
//{
//  struct connection_info_struct *con_info = *con_cls;
//
//  if (NULL == con_info)
//    return;
//
//  if (con_info->connectiontype == POST)
//    {
//      MHD_destroy_post_processor (con_info->postprocessor);
//      if (con_info->answerstring)
//        free (con_info->answerstring);
//    }
//
//  free (con_info);
//  *con_cls = NULL;
//}
