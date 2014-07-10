//
//  FLServerHTTP.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLServer wraps another httpPage (typically a httpdInterface) in a droppable page that can send a new source to FaustLive

#include <string>

#include "FLServerHttp.h"
#include "FLSettings.h"

#define kFile "HtmlCompiler.html"

#include "utilities.h"
#define kTmpFile "TmpFile.dsp"

using namespace std;

//--------------------------FLINTERMEDIATESERVER--------------------------//

FLServerHttp* FLServerHttp::_serverInstance = NULL;

//--------------------------FLSERVER-------------------------------------//
int FLServerHttp::fNr_of_uploading_clients = 0;

FLServerHttp::FLServerHttp(){
    fError = "";
    fUrl = "";
    fPosted = false;
    fCompiled = false;
    fHtml = "";
    fJson ="";
}

FLServerHttp::~FLServerHttp(){}

void FLServerHttp::createInstance(){
    FLServerHttp::_serverInstance = new FLServerHttp;
}

void FLServerHttp::deleteInstance(){
    delete FLServerHttp::_serverInstance;
}

FLServerHttp* FLServerHttp::getInstance(){
    return FLServerHttp::_serverInstance;
}

//Start Server Listening
bool FLServerHttp::start(){
    
    unsigned short port = FLSettings::getInstance()->value("General/Network/HttpDropPort", 7777).toInt();
   
    fServerAddress = "http://" + searchLocalIP().toStdString() + ":" + QString::number(port).toStdString() + "/";
    
    fDaemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
                               port, 
                               NULL, 
                               NULL, 
                               &answer_to_connection, 
                               this, MHD_OPTION_NOTIFY_COMPLETED, 
                               request_completed, NULL, MHD_OPTION_END);
    
    if(fDaemon!=NULL){
        printf("Server started = %p \n", fDaemon);
        return true;
    }
    else{
        MHD_stop_daemon(fDaemon);
        return false;
    }
}

//Stop Server Listening
void FLServerHttp::stop()
{
    if (fDaemon) {
        MHD_stop_daemon(fDaemon);
    }
    
    fDaemon = 0;
}

int FLServerHttp::handleGet(MHD_Connection *connection, const char* url){
    
    stringstream ss;
    
    string responseHead = readFile("/Users/denoux/FLReconstruct/Resources/Html/ServerHead.txt").toStdString();
    
    string responseTail = readFile("/Users/denoux/FLReconstruct/Resources/Html/ServerTail.txt").toStdString();
    
    if(strcmp(url,"/availableInterfaces") == 0){
        return send_page(connection, fHtml.c_str (), fHtml.size(), MHD_HTTP_OK, "text/html");
    }
    else if(strcmp(url,"/availableInterfaces/Json") == 0){
        return send_page(connection, fJson.c_str (), fJson.size(), MHD_HTTP_OK, "application/json");
    }
    else if(strcmp(url,"/") != 0 && strcmp(url, "/favicon.ico")){
        
        string portNumber(url);
        portNumber = portNumber.substr(1, portNumber.size()-1);
        
        ss << responseHead << "http://"<< searchLocalIP().toStdString().c_str() <<":"<<portNumber.c_str()<< responseTail;
        
        return send_page(connection, ss.str().c_str (), ss.str().size(), MHD_HTTP_OK, "text/html");
    }
    else{
        ss<<responseHead<<responseTail; 
        return send_page(connection, ss.str().c_str (), ss.str().size(), MHD_HTTP_OK, "text/html");
    }
}

int FLServerHttp::handlePost(MHD_Connection *connection, const char* url, void *info){
    
    struct connection_info_struct *con_info = (connection_info_struct*)info;
    
    int port = 0;
    
    if(con_info->winUrl.compare("") != 0 && con_info->winUrl.compare(fServerAddress) != 0){
        
        size_t pos = con_info->winUrl.rfind(":");
        
        string portNumber = con_info->winUrl.substr(pos+1, con_info->winUrl.size()-pos-2);
        port = atoi(portNumber.c_str()); 
    }
    
    emit compile(con_info->data.c_str(), port);
    
    while(fPosted != true){}
        
    fPosted = false;
    
    if(fCompiled){
        
        con_info->answerstring = fUrl;
            
        return send_page(connection, con_info->answerstring.c_str(), con_info->answerstring.size(), MHD_HTTP_OK, "text/plain");
        
    }
    else{
        
        return MHD_YES;
        //                string errorCompilePage = kErrorCompile1;
        //                errorCompilePage += fError;
        //                errorCompilePage += kErrorCompile2;
        //                printf("COMPILE ERROR PAGE = %s\n", errorCompilePage.c_str());
        //                
        //                return send_page(connection, errorCompilePage.c_str(),
        //                                         con_info->answerstring.size(), MHD_HTTP_OK, "text/html");  
    }
}

//Callback answering to any request to the server
int FLServerHttp::answer_to_connection	(void *cls, MHD_Connection *connection, const char *url, const char *method, const char */**version*/, const char *upload_data, size_t *upload_data_size, void **con_cls){
    
    FLServerHttp *server = (FLServerHttp*)cls;
    string errorPage = kErrorPage;
    
    if (NULL == *con_cls) {
        struct connection_info_struct *con_info;
    
        if (fNr_of_uploading_clients >= server->getMaxClients()) {
            
            string busyServer(kBusyPage);
            
            return server->send_page(connection, busyServer.c_str (), busyServer.size (), MHD_HTTP_SERVICE_UNAVAILABLE, "text/html");
        }
        
        con_info = new connection_info_struct();
        con_info->data = "";
        con_info->winUrl = "";
        
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

        return server->handleGet(connection, url);
    }
    
    if (0 == strcmp(method, "POST")) {
    
        struct connection_info_struct *con_info = (connection_info_struct*)*con_cls;
        
        if (0 != *upload_data_size) {
            
            MHD_post_process(con_info->postprocessor, upload_data, *upload_data_size);
            *upload_data_size = 0;
                        
            return MHD_YES;
        } 
        else{
            return server->handlePost(connection, url, (void*)con_info);
        }
    }
    else
        return server->send_page(connection, errorPage.c_str(), errorPage.size(), MHD_HTTP_BAD_REQUEST, "text/html");
}


//Send back a HTML page to the client
int FLServerHttp::send_page(struct MHD_Connection *connection, const char *page, int length, int status_code, const char * type)
{
    
    int ret;
    struct MHD_Response *response;
    
    response = MHD_create_response_from_buffer(length, (void*)page,
                                               MHD_RESPMEM_MUST_COPY);
    if (!response) {
        return MHD_NO;
    }
    
    MHD_add_response_header(response, "Content-Type", type ? type : "text/plain");
    cout << "FLServerHttp::send_page " << status_code << endl;
    ret = MHD_queue_response(connection, status_code, response);
    
    return ret;
}


//Callback ending a client connection
void FLServerHttp::request_completed(void */*cls*/, MHD_Connection */*connection*/, void **con_cls, MHD_RequestTerminationCode /*toe*/)
{
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


//Callback that parses the content of a post request
int FLServerHttp::iterate_post(void *coninfo_cls, MHD_ValueKind /*kind*/, const char *key, const char */*filename*/, const char */*content_type*/, const char */*transfer_encoding*/, const char *data, uint64_t /*off*/, size_t size)
{
    struct connection_info_struct *con_info = (connection_info_struct*)coninfo_cls;
    
//    printf("FLServer::iterate_post DATA = %s/n", data);
    
    if (size > 0) {

        if(strcmp(key,"var") == 0)
            con_info->data += data;
        
        if(strcmp(key,"interfaceurl") == 0)
            con_info->winUrl = data;
    }
    
    con_info->answercode = MHD_HTTP_OK;
    
    return MHD_YES;
}

//Actions on Success or Fail of source compilation
void FLServerHttp::compile_Successfull(const string& url){
    
    fUrl = url;
    fCompiled = true;
    fPosted = true;
}

void FLServerHttp::compile_Failed(const string& error){
    
    fCompiled = false;
    fError = error;    
    fPosted = true;
}

void FLServerHttp::declareHttpInterface(int port,  const string& name){
    
    fDeclaredNames[port] = name;
    createJson();
    createHtml();
}

void FLServerHttp::removeHttpInterface(int port){
    fDeclaredNames.erase(port);
    createJson();
    createHtml();
}

void FLServerHttp::createJson(){

    stringstream json;
    
    json << '{';
    
    for (map<int, string>::iterator it = fDeclaredNames.begin(); it != fDeclaredNames.end(); it++) {
        
        json << std::endl << '"' << it->second << '"' << ": [" << '"' << it->first << '"' << ']';
        if(it != fDeclaredNames.end())
            json<<',';
    }
    
    json << std::endl << "}";
    
    fJson = json.str();
}

void FLServerHttp::createHtml(){
    
    stringstream html;
    
    html << readFile("/Users/denoux/FLReconstruct/Resources/Html/ServerAvailableInterfacesHead.txt").toStdString();

    html<<"<table width=\"90%\" border=\"0\" cellspacing=\"10\" cellpadding=\"10\" align=\"center\">";
    
    for(map<int, string>::iterator it = fDeclaredNames.begin(); it != fDeclaredNames.end(); it++){
        
        html<<"<tr>"<<std::endl;
        html<<"<td>"<<it->second<<"</td>";
        html<<"<td><a href=\""<<it->first<<"\">"<<fServerAddress<<it->first<<"</a></td>"<<std::endl;
        html<<"<td><iframe width=\"30%\" name=\"iframe name\" height=\"90\" src=\""<<"http://"<<searchLocalIP().toStdString()<<":"<<it->first<<"\" border=\"0\" frameborder=\"0\" scrolling=\"no\" align=\"left\" hspace=\"0\" vspace=\"0\"></iframe></td>"<<std::endl;
        html<<"</tr>"<<std::endl;
    }
    
    html<<"</table>"<<std::endl;
    
    html<<std::endl<<readFile("/Users/denoux/FLReconstruct/Resources/Html/ServerAvailableInterfacesTail.txt").toStdString();
    
    fHtml = html.str();
}

//Accessor to Max Client Number
int FLServerHttp::getMaxClients(){ 
    return fMax_clients; 
}





