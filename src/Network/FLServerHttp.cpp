//
//  FLServerHTTP.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLServer wraps another httpPage (typically a httpdInterface) in a droppable page that can send a new source to FaustLive

#include <string>
#include <sstream>
#include <iostream>
#include <fstream> 

#ifndef WIN32
#include <curl/curl.h>
#endif

// don't change the next includes order and always keep FLServerHttp.h on first place
// it (indirectly) solves the conflict between winsock2 and windows
#include "FLServerHttp.h"
#include "FLSettings.h"
#include "utilities.h"

#define kFile       "HtmlCompiler.html"
#define kTmpFile    "TmpFile.dsp"


//--------------------------FLINTERMEDIATESERVER--------------------------//

FLServerHttp* FLServerHttp::_serverInstance = NULL;

//--------------------------FLSERVER-------------------------------------//
int FLServerHttp::fUploadingClients = 0;

FLServerHttp::FLServerHttp(const std::string& home)
{
    fHome = home;
    fError = "";
    fUrl = "";
    fPosted = false;
    fCompiled = false;
    fHtml = "";
    fJson = "";
    fMaxCients = 20;
}

FLServerHttp::~FLServerHttp() {}

void FLServerHttp::createInstance(const std::string& home)
{
    FLServerHttp::_serverInstance = new FLServerHttp(home);
}

void FLServerHttp::deleteInstance()
{
    delete FLServerHttp::_serverInstance;
    FLServerHttp::_serverInstance = NULL;
}

FLServerHttp* FLServerHttp::_Instance()
{
    return FLServerHttp::_serverInstance;
}

//---------------------- START/STOP DAEMON ------------------------
bool FLServerHttp::start()
{
    unsigned short port = FLSettings::_Instance()->value("General/Network/HttpDropPort", 7777).toInt();
    fServerAddress = "http://" + searchLocalIP().toStdString() + ":" + QString::number(port).toStdString() + "/";

    fDaemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
                               port, 
                               NULL, 
                               NULL, 
                               (MHD_AccessHandlerCallback)answerToConnection,
                               this, MHD_OPTION_NOTIFY_COMPLETED, 
                               requestCompleted, NULL, MHD_OPTION_END);
    
    if (fDaemon) {
        printf("Server started = %p \n", fDaemon);
        return true;
    } else {
        return false;
    }
}

//Stop Server Listening
void FLServerHttp::stop()
{
    if (fDaemon) {
        MHD_stop_daemon(fDaemon);
        fDaemon = 0;
    }
}

//---------------------- HANDLE REQUESTS ------------------------
int FLServerHttp::handleGet(MHD_Connection* connection, const char* url)
{
    std::stringstream ss;
    
    std::string head = fHome + "/ServerHead.html";
    std::string tail = fHome + "/ServerTail.html";
    
    std::string responseHead = readFile(head.c_str()).toStdString();
    std::string responseTail = readFile(tail.c_str()).toStdString();
    
    // Request for the server
    if (strcmp(url,"/availableInterfaces") == 0) {
        return sendPage(connection, fHtml.c_str (), fHtml.size(), MHD_HTTP_OK, "text/html");
    
    } else if (strcmp(url,"/availableInterfaces/JSON") == 0) {
        return sendPage(connection, fJson.c_str (), fJson.size(), MHD_HTTP_OK, "application/json");

    // Request for an interface
    } else if (strcmp(url,"/") != 0 && strcmp(url, "/favicon.ico")) {
        
        std::string urlAsString(url);
        // As a JSON
        if (urlAsString.find("JSON") != std::string::npos) {
            
            std::string portNumber(url);
            portNumber = portNumber.substr(1, portNumber.size()-6);
            
            return redirectJsonRequest(connection, portNumber);
        }  else {
        // As an html interface
            std::string portNumber(url);
            portNumber = portNumber.substr(1, portNumber.size()-1);
            
            ss << responseHead << "http://"<< searchLocalIP().toStdString().c_str() <<":"<<portNumber.c_str()<< responseTail;
            return sendPage(connection, ss.str().c_str (), ss.str().size(), MHD_HTTP_OK, "text/html");
        }
    }
     
    ss<<responseHead<<responseTail; 
    return sendPage(connection, ss.str().c_str (), ss.str().size(), MHD_HTTP_OK, "text/html");
}

int FLServerHttp::handlePost(MHD_Connection* connection, const char* /**url**/, void* info)
{
    connection_info *con_info = (connection_info*)info;
    int port = 0;
    
    if (con_info->winUrl.compare("") != 0 && con_info->winUrl.compare(fServerAddress) != 0) {
        size_t pos = con_info->winUrl.rfind(":");
        std::string portNumber = con_info->winUrl.substr(pos+1, con_info->winUrl.size()-pos-2);
        port = atoi(portNumber.c_str()); 
    }
    
    emit compile(con_info->data.c_str(), port);
    
    while (fPosted != true){}
        
    fPosted = false;
    
    if (fCompiled) {
        con_info->answerstring = fUrl;
        return sendPage(connection, con_info->answerstring.c_str(), con_info->answerstring.size(), MHD_HTTP_OK, "text/plain");
    }
        
    return MHD_YES;
}

//Callback that parses the content of a post request
int FLServerHttp::iteratePost(void* coninfo_cls, MHD_ValueKind, const char* key, const char* /*filename*/, const char* /*content_type*/, const char* /*transfer_encoding*/, const char* data, uint64_t /*off*/, size_t size)
{
    connection_info* con_info = (connection_info*)coninfo_cls;
    
    if (size > 0) {
        
        if (strcmp(key,"var") == 0)
            con_info->data += data;
        
        if (strcmp(key,"interfaceurl") == 0)
            con_info->winUrl = data;
    }
    
    con_info->answercode = MHD_HTTP_OK;
    return MHD_YES;
}

//---------------------- HANDLE INCOMING CNX ------------------------

//Callback answering to any request to the server
int FLServerHttp::answerToConnection(void* cls,
                                    MHD_Connection* connection,
                                    const char* url, 
                                    const char* method, 
                                    const char* /*version*/, 
                                    const char* upload_data, 
                                    size_t* upload_data_size,
                                    void** con_cls)
{
    FLServerHttp* server = (FLServerHttp*)cls;
    std::string errorPage = kErrorPage;
    
    if (NULL == *con_cls) {
        connection_info* con_info;
    
        if (fUploadingClients >= server->getMaxClients()) {
            std::string busyServer(kBusyPage);
            return server->sendPage(connection, busyServer.c_str (), busyServer.size (), MHD_HTTP_SERVICE_UNAVAILABLE, "text/html");
        }
        
        con_info = new connection_info();
        con_info->data = "";
        con_info->winUrl = "";
        
        if (0 == strcmp(method, "POST")) {
            
            con_info->postprocessor = MHD_create_post_processor(connection, POSTBUFFERSIZE, (MHD_PostDataIterator)iteratePost, (void*)con_info);
            
            if (NULL == con_info->postprocessor) {
                free(con_info);
                return MHD_NO;
            }
            
            fUploadingClients++;
            
            con_info->connectiontype = POST;
            con_info->answercode = MHD_HTTP_OK;
            con_info->answerstring = kErrorPage;
        } else {
            con_info->connectiontype = GET;
        }
        
        *con_cls = (void*)con_info;
        return MHD_YES;
    }
    
    if (0 == strcmp(method, "GET")) {
        return server->handleGet(connection, url);
    } else if (0 == strcmp(method, "POST")) {
    
        connection_info *con_info = (connection_info*)*con_cls;
        
        if (0 != *upload_data_size) {
            MHD_post_process(con_info->postprocessor, upload_data, *upload_data_size);
            *upload_data_size = 0;
            return MHD_YES;
        } else {
            return server->handlePost(connection, url, (void*)con_info);
        }
    } else {
        return server->sendPage(connection, errorPage.c_str(), errorPage.size(), MHD_HTTP_BAD_REQUEST, "text/html");
    }
}

//---------------------- CREATE RETURNING PAGE ------------------------
int FLServerHttp::sendPage(struct MHD_Connection* connection, const char* page, int length, int status_code, const char* type)
{
    int ret;
    MHD_Response *response;
    
    response = MHD_create_response_from_buffer(length, (void*)page, MHD_RESPMEM_MUST_COPY);
    if (!response) {
        return MHD_NO;
    }
    
    MHD_add_response_header(response, "Content-Type", type ? type : "text/plain");
    std::cout << "FLServerHttp::sendPage " << status_code << std::endl;
    ret = MHD_queue_response(connection, status_code, response);
    
    return ret;
}

//Callback ending a client connection
void FLServerHttp::requestCompleted(void*, MHD_Connection*, void** con_cls, MHD_RequestTerminationCode)
{
    connection_info* con_info = (connection_info*)*con_cls;
    
    if (!con_info) {
        return;
    }
    
    if (con_info->connectiontype == POST) {
        if (NULL != con_info->postprocessor) {
            MHD_destroy_post_processor(con_info->postprocessor);
            fUploadingClients--;
        }
    }
    
    delete con_info;
    *con_cls = NULL;
}

//---------------- FAUST RECOMPILATION RESULT -----------------------------
void FLServerHttp::compileSuccessfull(const std::string& url)
{
    fUrl = url;
    fCompiled = true;
    fPosted = true;
}

void FLServerHttp::compileFailed(const std::string& error)
{
    fCompiled = false;
    fError = error;    
    fPosted = true;
}

//--------------- HANDLE AVAILABLE HTTP INTERFACES ----------------

void FLServerHttp::declareHttpInterface(int port,  const std::string& name)
{
     fDeclaredNames[port] = name;
    updateAvailableInterfaces();
}

void FLServerHttp::removeHttpInterface(int port)
{
    fDeclaredNames.erase(port);
    updateAvailableInterfaces();
}

void FLServerHttp::updateAvailableInterfaces()
{
    std::stringstream json;
    std::stringstream html;
    
    std::string interfacesHead = fHome + "/ServerAvailableInterfacesHead.html";
    
    json << '{';
    html << readFile(interfacesHead.c_str()).toStdString();
    
    html<<"<table width=\"90%\" border=\"0\" cellspacing=\"10\" cellpadding=\"10\" align=\"center\">";
    
    for (std::map<int, std::string>::iterator it = fDeclaredNames.begin(); it != fDeclaredNames.end(); it++) {
        
        if (it != fDeclaredNames.begin())
            json<<',';
        json << std::endl << '"' << it->second << '"' << ": [" << '"' << it->first << '"' << ']';
        
        html<<"<tr>"<<std::endl;
        html<<"<td>"<<it->second<<"</td>";
        html<<"<td><a href=\""<<it->first<<"\">"<<fServerAddress<<it->first<<"</a></td>"<<std::endl;
        html<<"<td><iframe width=\"30%\" name=\"iframe name\" height=\"90\" src=\""<<"http://"<<searchLocalIP().toStdString()<<":"<<it->first<<"\" border=\"0\" frameborder=\"0\" scrolling=\"no\" align=\"left\" hspace=\"0\" vspace=\"0\"></iframe></td>"<<std::endl;
        html<<"</tr>"<<std::endl;
    }
    
    json << std::endl << "}";
    fJson = json.str();
    
    std::string interfacesTail = fHome + "/ServerAvailableInterfacesTail.html";
    
    html<<"</table>"<<std::endl;
    html<<std::endl<<readFile(interfacesTail.c_str()).toStdString();
    fHtml = html.str();
}

//-------------- Special treatement for the JSON Request ----------

// Standard Callback to store the server response to IPadd:5510/JSON
static size_t store_Response(void* buf, size_t size, size_t nmemb, void* userp)
{
    std::ostream* os = static_cast<std::ostream*>(userp);
    std::streamsize len = size * nmemb;
    return (os->write(static_cast<char*>(buf), len)) ? len : 0;
}

// A request for the JSON, written as :
//IPadd:7777/5510/JSON is well redirected to IPadd:5510/JSON
int FLServerHttp::redirectJsonRequest(struct MHD_Connection *connection, std::string portNumber)
{
#ifndef WIN32
	std::string resultingPage = "";
    std::stringstream url; 
    
    url<<"http://"<< searchLocalIP().toStdString().c_str() <<":"<<portNumber.c_str()<< "/JSON";
    std::string finalURL = url.str();
    CURL *curl = curl_easy_init();
    
    long respcode = MHD_HTTP_BAD_REQUEST; 
    
    if (curl) {
        
        std::ostringstream oss;
        
        curl_easy_setopt(curl, CURLOPT_URL, finalURL.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &store_Response);
        curl_easy_setopt(curl, CURLOPT_FILE, &oss);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 60); 
        curl_easy_setopt(curl,CURLOPT_TIMEOUT, 600);
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {

            curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);
            
            if (respcode == 200)
                resultingPage = oss.str();
        }
        
        curl_easy_cleanup(curl);
    }    
    
    return sendPage(connection, resultingPage.c_str(), resultingPage.size(), respcode, "application/json");
#else
return 1;
#endif
}

//----------Accessor to Max Client Number--------
int FLServerHttp::getMaxClients()
{ 
    return fMaxCients; 
}




