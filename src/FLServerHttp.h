//
//  FLServerHttp.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLServer wraps another httpPage (typically a httpdInterface) in a droppable page that can send a new source to FaustLive

#ifndef _FLSERVERHTTP_h
#define _FLSERVERHTTP_h

#define kResponseHead "<!DOCTYPE html PUBLIC>\n\
<html>\n\
<head>\n\
\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n\
\n\
<style type=\"text/css\">\n\
\n\
#filedrag{\n\
font-weight: bold;\n\
text-align: center;\n\
padding: 1em 0;\n\
margin: 1em 0;\n\
color: #CECECE;\n\
border: 2px dashed #CECECE;\n\
border-radius: 7px;\n\
cursor: default;\n\
font-size : 20px;\n\
}\n\
\n\
#filedrag.hover{\n\
color: #FF7F00;\n\
border-color: #FF7F00;\n\
border-style: solid;\n\
}\n\
\n\
</style>\n\
\n\
</head>\n\
<body bgcolor= black> >\n\
<div id=\"filedrag\">\n\
Drop your .dsp file here\n\
</div>\n\
<form id=\"upload\" action=\"CompilerResponse\" method=\"POST\" enctype=\"multipart/form-data\">\n\
<input type=\"hidden\" id=\"MAX_FILE_SIZE\" name=\"MAX_FILE_SIZE\" value=\"300000\" />\n\
</form>\n\
\n\
<iframe id=\"httpInterface\" src=\""

#define kResponseTail  "\" style=\"min-width:100%; min-height:90%;\" frameBorder=\"0\">\n</iframe>\n\
\n\
<script type=text/javascript>\n\
(function(){\n\
\n\
function FileDragHover(e) {\n\
\n\
e.stopPropagation();\n\
e.preventDefault();\n\
e.target.className = (e.type == \"dragover\" ? \"hover\" : \"\");\n\
}\n\
\n\
function FileSelectHandler(e) {\n\
\n\
FileDragHover(e);\n\
var files = e.target.files || e.dataTransfer.files;\n\
f = files[0];\n\
\n\
UploadFile(f);\n\
}\n\
\n\
function UploadFile(e) {\n\
\n\
FileDragHover(e);\n\
var files = e.target.files || e.dataTransfer.files;\n\
var file = files[0];\n\
\n\
if (location.host.indexOf(\"sitepointstatic\") >= 0) return\n\
\n\
var request = new XMLHttpRequest();\n\
if (request.upload && file.size <= document.getElementById(\"MAX_FILE_SIZE\").value) {\n\
\n\
var reader = new FileReader();\n\
\n\
var ext = file.name.split('.').pop();\n\
\n\
if(ext == \"dsp\"){\n\
\n\
reader.readAsText(file);  \n\
}\n\
reader.onloadend = function(e) {\n\
var allText = reader.result;\n\
var xhr = new XMLHttpRequest();\n\
var params = \"var=\" + encodeURIComponent(allText);\n\
xhr.open(\"POST\", '', true);\n\
xhr.setRequestHeader(\"Content-type\", \"application/x-www-form-urlencoded\");\n\
xhr.setRequestHeader(\"Content-length\", params.length);\n\
xhr.setRequestHeader(\"Connection\", \"close\");\n\
xhr.onreadystatechange = function() {\n\
    if(xhr.readyState == 4 && xhr.status == 200) {\n\
        document.getElementById(\"httpInterface\").src = xhr.responseText;\n\
    }\n\
}\n\
xhr.send(params);\n\
};\n\
}\n\
}\n\
\n\
function Init() {\n\
var filedrag1 = document.getElementById(\"filedrag\");\n\
\n\
var xx = new XMLHttpRequest();\n\
if (xx.upload) {\n\
filedrag1.addEventListener(\"dragover\", FileDragHover, false);\n\
filedrag1.addEventListener(\"dragleave\", FileDragHover, false);\n\
filedrag1.addEventListener(\"drop\", UploadFile, false);\n\
}\n\
}\n\
\n\
if (window.File && window.FileList && window.FileReader) {\n\
\n\
Init();\n\
}\n\
})();\n\
\n\
</script>\n\
\n\
</body>\n\
</html>"

#define kBusyPage   "<html><body>FaustLive Server is busy, please try again later.</body></html>"

#define kErrorPage "<html>\n<body>ERROR</body>\n</html>"

#define kErrorCompile1 "<html>\n<body>\nImpossible to compile this file : \n"

#define kErrorCompile2 "\n</body>\n</html>"

#include <sstream>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <stdlib.h>

#include <microhttpd.h>

#include <map>
#include <vector>
#include <string>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

using namespace std;

#define POSTBUFFERSIZE 512

#define GET 0
#define POST 1

struct connection_info_struct {
    int connectiontype; // GET or POST
    struct MHD_PostProcessor *postprocessor; // the POST processor used internally by microhttpd
    int answercode; // used internally by microhttpd to see where things went wrong or right
    
    string data;
    string compilationOptions;
    std::string answerstring; // the answer sent to the user after upload
};

class FLServerHttp : public QObject{
    
    Q_OBJECT
    
    int             fMax_clients;   // Maximum of clients the server can support
    string          fError;         // Not important right now
    string          fUrl;           // Url of wrapped http page 
    
    public :
    
    bool            fPosted;        // Post request completed
    bool            fCompiled;      // Compilation sucess
    
    static int      fNr_of_uploading_clients;
    
    struct          MHD_Daemon* fDaemon;
    
                    FLServerHttp();
                    ~FLServerHttp();
    
    int             getMaxClients();
    
    bool            start(unsigned short port);
    void            stop();

    int             send_page(struct MHD_Connection *connection, const char *page, int length, int status_code, const char * type = 0);
    
    static int      answer_to_connection	(void *cls, struct MHD_Connection *connection,
                                     const char *url, const char *method,
                                     const char *version, const char *upload_data,
                                     size_t *upload_data_size, void **con_cls);
    
    static void request_completed(void *cls, MHD_Connection *connection, void **con_cls, MHD_RequestTerminationCode toe);
    
    static int iterate_post(void *coninfo_cls, MHD_ValueKind kind, const char *key, const char *filename, const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size);
    
    void            compile_Successfull(string& url);
    void            compile_Failed(string error);
    
    signals :
    
    void        compile_Data(const char*, int);
  
};

#endif
