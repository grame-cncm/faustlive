


#include "Remote_DSP_Factory.h"

#include "utilities.h"

#include <vector>


// Callback to gather the response from the server.  Comes in chunks (typically 16384 characters at a time), so needs to be stitched together.
size_t Remote_DSP_Factory::store_Response(void *buf, size_t size, size_t nmemb, void* userp){
    
    std::ostream& os = *static_cast<std::ostream*>(userp);
    std::streamsize len = size * nmemb;
    if(os.write(static_cast<char*>(buf), len))
        return len;
    else
        return 0;
}

string getValue(int argc, char** argv, const string& key, vector<int>* toBePassed){
     
    for(int i = 0; i < argc ; i++){
    
        if(key.compare(argv[i]) == 0){
            toBePassed->push_back(i);
            toBePassed->push_back(i+1);
            return argv[i+1];;
        }
    }
     
    return "";
}

Remote_DSP_Factory::Remote_DSP_Factory(){}

bool Remote_DSP_Factory::init(string ipServer, string dspContent, int argc, char** argv, int opt_level, string& error){
    
    bool isInit = false;
    
    fServerIP = ipServer;
    
    CURL *curl;
    CURLcode res;
    
    std::ostringstream oss;
    
    curl = curl_easy_init();
    
    string finalString = "data=";
    finalString += curl_easy_escape(curl , dspContent.c_str() , dspContent.size());
    
    string IP, Port, Latency, CV;
    
    vector<int> toBePassed;
    
//    IP = lopts(argv, "--NJ_ip", "");
//    Port = lopts(argv, "--NJ_port", "");
//    Latency = lopts(argv, "--NJ_latency", "");
//    CV = lopts(argv, "--NJ_compression", "");
    
    fIP = (char*)(searchLocalIP().toStdString().c_str());
    fPort = "19000";
    fLatency = "2";
    fCV = "-1";
    
//    finalString += "&options=";
//    
//    for(int i=1; i<argc; i++){
//        
//        if(string(argv[i]).find("--")!=string::npos){
//            i ++;
//        }
//        else{
//            finalString += argv[i];
//            finalString += "%";
//        }
//    }
    
    finalString += "&opt_level=";
    stringstream ol;
    ol<<opt_level;
    finalString +=ol.str(); 
    
    
    printf("finalString = %s\n", finalString.c_str());
    
    if (curl) {
        
        string ip = ipServer;
        ip += "/GetJson";
        
        curl_easy_setopt(curl, CURLOPT_URL, ip.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, finalString.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(finalString.c_str()));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &Remote_DSP_Factory::store_Response);
        curl_easy_setopt(curl, CURLOPT_FILE, &oss);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        
        res = curl_easy_perform(curl);
        
        long respcode; //response code of the http transaction
        
        curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);
        
        printf("RESPONSE CODE= %ld\n", respcode);
        
        if(res != CURLE_OK){
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            error =  curl_easy_strerror(res);
        }
        else{
            if(respcode == 200){
                fJsonResponse = oss.str();
            //            fprintf(stderr, "curl_easy_perform() success: %s\n", oss.str().c_str());
//            printf("fJsonResponse = %s\n", fJsonResponse.c_str());
                isInit = true;
            }
            else if(respcode == 400)
                error = oss.str();
                
        }
        curl_easy_cleanup(curl);
    }
    
    return isInit;
}

Remote_DSP_Factory::~Remote_DSP_Factory(){}

void Remote_DSP_Factory::stop(){
    
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    
    string finalString = "factoryIndex=";
    finalString += fIndex;
    
    //    printf("finalString = %s\n", finalString.c_str());
    
    if (curl) {
        
        string ip = fServerIP;
        ip += "/DeleteFactory";
        
        curl_easy_setopt(curl, CURLOPT_URL, ip.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, finalString.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(finalString.c_str()));
        
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        //        else
        //            fprintf(stderr, "curl_easy_perform() success \n");
        
        curl_easy_cleanup(curl);
    }
}

string Remote_DSP_Factory::json(){

    return fJsonResponse;
}

//------------- CREATE/DELETE FACTORY------

Remote_DSP_Factory* createRemoteFactory(string ipServer, string dspContent, int argc, char** argv, int opt_level, string& error){
    
    printf("CREATE REMOTE FACTORY\n");
    
    Remote_DSP_Factory* factory = new Remote_DSP_Factory();
    
    if(factory->init(ipServer, dspContent, argc, argv, opt_level, error))
        return factory;
    else{
        delete factory;
        return NULL;
    }
        
}

void deleteRemoteFactory(Remote_DSP_Factory* factory){

    printf("DELETE REMOTE FACTORY\n");
    factory->stop();
    delete factory;
}







