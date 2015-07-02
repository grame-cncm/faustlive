//
//  FLServerHttp.h
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//
// FLServer wraps another httpPage (typically a httpdInterface) in a dropable page that can send a new source to FaustLive
//
// The GET requests treated by FLServer are :
//         /availableInterfaces --> returns an HTML page describing all available HTML interfaces
//         /availableInterfaces/JSON --> returns the available interfaces as a JSON description
//         /                    --> HTML page with only a drop zone
//         /<portNumber>        --> HTML page with drop zone and interface connresponding to <portNumber>
//
// The POST request treated by FLServer is :
//         receiving Faust code --> compilation --> return url to HTML interface
//
// The HTML wrappers are stored in Resources/Html
//
// See ServerHTTPSpecification in documentation for further implementation

#ifndef _FLSERVERHTTP_h
#define _FLSERVERHTTP_h

#define kBusyPage  "<html><body>FaustLive Server is busy, please try again later.</body></html>"
#define kErrorPage "<html>\n<body>ERROR</body>\n</html>"
#define kErrorCompile1 "<html>\n<body>\nImpossible to compile this file : \n"
#define kErrorCompile2 "\n</body>\n</html>"

#include <sstream>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <microhttpd.h>

#include <map>
#include <vector>
#include <string>
#undef min
#undef max
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

using namespace std;

#define POSTBUFFERSIZE 512

#define GET 0
#define POST 1

struct connection_info {

    int connectiontype;                         // GET or POST
    struct MHD_PostProcessor *postprocessor;    // the POST processor used internally by microhttpd
    int answercode;                             // used internally by microhttpd to see where things went wrong or right
    
    string data;
    string compilationOptions;
    string winUrl;                              // To be able to replace faust content in the right FLWindow
    std::string answerstring;                   // the answer sent to the user after upload
    
};

class FLServerHttp : public QObject
{
    
    private:

        Q_OBJECT
        
        int             fMax_clients;
        string          fError;         // Not important right now
        string          fUrl;           // Url of wrapped http page 
        
        bool            fPosted;        // Post request completed
        bool            fCompiled;      // Compilation sucess
        
        string          fServerAddress;
        
        string          fJson;
        string          fHtml;
        
        string          fHome;
        
        map<int, string>     fDeclaredNames;
        
        static FLServerHttp*    _serverInstance;
        
        static int      fNr_of_uploading_clients;
        
        struct          MHD_Daemon* fDaemon;
        
        int             handleGet(MHD_Connection *connection, const char* url);
        int             handlePost(MHD_Connection *connection, const char* url, void *info);
        
        void            updateAvailableInterfaces();
        int             getMaxClients();
        
        int             sendPage(struct MHD_Connection *connection, const char *page, int length, int status_code, const char * type = 0);
        
        static int      answerToConnection(void *cls, struct MHD_Connection *connection,
                                         const char *url, const char *method,
                                         const char *version, const char *upload_data,
                                         size_t *upload_data_size, void **con_cls);
                                         
        
        int             redirectJsonRequest(struct MHD_Connection *connection, string portNumber);
        
        static void requestCompleted(void *cls, MHD_Connection *connection, void **con_cls, MHD_RequestTerminationCode toe);
        
        static int iteratePost(void *coninfo_cls, MHD_ValueKind kind, 
                                const char *key, const char *filename, 
                                const char *content_type, 
                                const char *transfer_encoding, 
                                const char *data, uint64_t off, 
                                size_t size);
     
    public:
       
        FLServerHttp(const string& home);
        virtual ~FLServerHttp();
         
        bool        start();
        void        stop();
        
        void        declareHttpInterface(int port, const string& name);
        void        removeHttpInterface(int port);
        
        void        compileSuccessfull(const string& url);
        void        compileFailed(const string& error);
      
        static void createInstance(const string& homeFolder);
        static void deleteInstance();
          
        static      FLServerHttp* _Instance();
          
    signals:
        
        void compile(const char*, int);
    
};

#endif
