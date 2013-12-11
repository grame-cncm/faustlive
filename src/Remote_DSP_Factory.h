


#ifndef Remote_DSP_Factory__h
#define Remote_DSP_Factory__h

#include <string>
#include <sstream>

#include <curl/curl.h>


using namespace std;

class Remote_DSP_Factory{

    private:
    
        string      fJsonResponse;  //Response of Server containing the interface as json application
    
        string      fIP, fPort, fCV, fLatency, fServerIP;
    
        string      fIndex;
    
        static size_t      store_Response(void *buf, size_t size, size_t nmemb, void* userp);
    
    public: 
    
        Remote_DSP_Factory();
        virtual ~Remote_DSP_Factory();
    
        bool    init(string ipServer, string dspContent, int argc, char** argv, int opt_level, string& error);
        void    stop();
    
        string json();

    
        string  getServerIP(){return fServerIP;}
        string  getIP(){return fIP;}
        string  getPort(){return fPort;}
        string  getCV(){return fCV;}
        string  getLatency(){return fLatency;}
    
        void    setIndex(string index){fIndex = index;}
};

Remote_DSP_Factory* createRemoteFactory(string ipServer, string dspContent, int argc, char** argv, int opt_level,string& error);

void deleteRemoteFactory(Remote_DSP_Factory* factory);

#endif



