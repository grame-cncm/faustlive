//
//  FLServerHTTP.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <string>
#include <QtGui>
#include "FLServerHttp.h"

#define kFile "HtmlCompiler.html"

#define kTmpFile "TmpFile.dsp"

using namespace std;

int FLServerHttp::fNr_of_uploading_clients = 0;

FLServerHttp::FLServerHttp(string localIP){
    
    fIPLocal = localIP;
    fPosted = false;
    fError = "";
    fUrl = "";
}

FLServerHttp::~FLServerHttp(){}

bool FLServerHttp::start(){
    
    unsigned short port = 7777;
    
    fDaemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
                               port, 
                               NULL, 
                               NULL, 
                               &answer_to_connection, 
                               this, MHD_OPTION_NOTIFY_COMPLETED, 
                               request_completed, NULL, MHD_OPTION_END);
    
    printf("Server started???\n");
    
    return fDaemon != NULL;
    
}

void FLServerHttp::stop()
{
    if (fDaemon) {
        MHD_stop_daemon(fDaemon);
    }
    
    fDaemon = 0;
}

int FLServerHttp::answer_to_connection	(void *cls, MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls){
    
    printf("FaustServer::answer_to_connection = %s\n", method);
    printf("CONNECTION = %p\n", connection);
    printf("URL = %s\n", url);
    printf("DATA = %s\n", upload_data);
    
    FLServerHttp *server = (FLServerHttp*)cls;
    string errorPage = kErrorPage;
    
    string param(url);
    param = param.substr(1, param.size()-1);
    
    if (NULL == *con_cls) {
        struct connection_info_struct *con_info;
    
        if (fNr_of_uploading_clients >= server->getMaxClients()) {
            
            string busyServer(kBusyPage);
            
            return server->send_page(connection, busyServer.c_str (), busyServer.size (), MHD_HTTP_SERVICE_UNAVAILABLE, "text/html");
        }
        
        con_info = new connection_info_struct();
        con_info->data = "";
        
        if (NULL == con_info) {
            return MHD_NO;
        }
        
        if (0 == strcmp(method, "POST")) {
            
            con_info->postprocessor = MHD_create_post_processor(connection, POSTBUFFERSIZE, iterate_post, (void*)con_info);
            
            if (NULL == con_info->postprocessor) {
                free(con_info);
                return MHD_NO;
            }
            
            fNr_of_uploading_clients++;
            
            con_info->connectiontype = POST;
            con_info->answercode = MHD_HTTP_OK;
            con_info->answerstring = kErrorPage;
        }
        
        else {
            con_info->connectiontype = GET;
        }
        
        *con_cls = (void*)con_info;
        
        return MHD_YES;
    }
    
    if (0 == strcmp(method, "GET")) {

        stringstream ss;
           
        ss << kResponseHead;
         
        if(url != "/"){            
            ss << "http://"<< server->fIPLocal <<":"<<param;
        }
    
        ss << kResponseTail;
        return server->send_page(connection, ss.str().c_str (), ss.str().size(), MHD_HTTP_OK, "text/html");
        
    }
    
    if (0 == strcmp(method, "POST")) {
        
        printf("POST PROCESSORING\n");
        
        struct connection_info_struct *con_info = (connection_info_struct*)*con_cls;
        
        server->parseURL(url, con_info);
        
        if (0 != *upload_data_size) {
            
            MHD_post_process(con_info->postprocessor, upload_data, *upload_data_size);
            *upload_data_size = 0;
                        
            return MHD_YES;
        } 
        else{
            
            // need to close the file before request_completed
            // so that it can be opened by the methods below
            
            printf("DATA = %s\n", con_info->data.c_str());
            emit server->compile_Data(con_info->data.c_str(), con_info->compilationOptions.c_str(), atoi(param.c_str()));
            
            while(server->fPosted != true){}
                
            server->fPosted = false;
            
            if(server->fCompiled){
                
                con_info->answerstring =server->fUrl;
                
                return server->send_page(connection, con_info->answerstring.c_str(), con_info->answerstring.size(), MHD_HTTP_OK, "text/plain");
                
            }
            else{
                
//                string errorCompilePage = kErrorCompile1;
//                errorCompilePage += server->fError;
//                errorCompilePage += kErrorCompile2;
//                printf("COMPILE ERROR PAGE = %s\n", errorCompilePage.c_str());
//                
//                return server->send_page(connection, errorCompilePage.c_str(),
//                                         con_info->answerstring.size(), MHD_HTTP_OK, "text/html");  
            }
        }
    }
    else
        return server->send_page(connection, errorPage.c_str(), errorPage.size(), MHD_HTTP_BAD_REQUEST, "text/html");
}


int FLServerHttp::send_page(struct MHD_Connection *connection, const char *page, int length, int status_code, const char * type)
{
    
    printf("Send page \n");
    
    int ret;
    struct MHD_Response *response;
    
    response = MHD_create_response_from_buffer(length, (void*)page,
                                               MHD_RESPMEM_PERSISTENT);
    if (!response) {
        return MHD_NO;
    }
    
    MHD_add_response_header(response, "Content-Type", type ? type : "text/plain");
    cout << "FLServerHttp::send_page " << status_code << endl;
    ret = MHD_queue_response(connection, status_code, response);
    
    return ret;
}


void FLServerHttp::request_completed(void *cls, MHD_Connection *connection, void **con_cls, MHD_RequestTerminationCode toe)
{
    printf("FaustServer::request_completed()\n");
    
    struct connection_info_struct *con_info = (connection_info_struct*)*con_cls;
    
    if (NULL == con_info) {
        return;
    }
    
    if (con_info->connectiontype == POST) {
        if (NULL != con_info->postprocessor) {
            MHD_destroy_post_processor(con_info->postprocessor);
            fNr_of_uploading_clients--;
        }
    }
    
    free(con_info);
    *con_cls = NULL;
}


int FLServerHttp::iterate_post(void *coninfo_cls, MHD_ValueKind kind, const char *key, const char *filename, const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size)
{
    struct connection_info_struct *con_info = (connection_info_struct*)coninfo_cls;
    
    printf("FLServer::iterate_post DATA = %s/n", data);
    
    if (size > 0) {

        con_info->data += data;
    }
    
    con_info->answercode = MHD_HTTP_OK;
    
    return MHD_YES;
}

void FLServerHttp::compile_Successfull(string& url){
    
    fUrl = url;
    fCompiled = true;
    fPosted = true;
}

void FLServerHttp::compile_Failed(string error){
    
    fPosted = true;
    fCompiled = false;
    fError = error;

}

const int FLServerHttp::getMaxClients(){ 
    return fMax_clients; 
}

void FLServerHttp::parseURL(const char* url, connection_info_struct* con_info){
    
    string param(url);
    param = param.substr(1, param.size()-1);

    printf("PARAM = %s\n", param.c_str());

    int pos = param.find('/');
    con_info->IP = param.substr(0, pos);
    param = param.substr(pos+1, param.size());
    printf("IP = %s\n", con_info->IP.c_str());
    
    pos = param.find('/');
    con_info->Port = param.substr(0, pos);
    param = param.substr(pos+1, param.size());
    printf("Port = %s\n", con_info->Port.c_str());
    
    pos = param.find('/');
    con_info->CV = param.substr(0, pos);
    param = param.substr(pos+1, param.size());
    printf("CV = %s\n", con_info->CV.c_str());
    
    pos = param.find('/');
    con_info->Latency = param.substr(0, pos);
    param = param.substr(pos+1, param.size());
    printf("Latency = %s\n", con_info->Latency.c_str());
    
    pos = param.find('/');
    con_info->compilationOptions = param.substr(0, pos);
    param = param.substr(pos+1, param.size());
    printf("Options = %s\n", con_info->compilationOptions.c_str());
}

