//
//  FLExportManager.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "Faust_Exporter.h"
#include "SimpleParser.h"

// Standard Callback to store a server response in strinstream
static size_t store_Response(void *buf, size_t size, size_t nmemb, void* userp)
{
    std::ostream* os = static_cast<std::ostream*>(userp);
    std::streamsize len = size * nmemb;
    return (os->write(static_cast<char*>(buf), len)) ? len : 0;
}

// Standard Callback to store a server response in file
static size_t store_File(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}

// Access to FaustWeb service -Sends a request to faustweb compilation service to know platforms and architecture supported by this
bool get_available_targets(const std::string& url, std::vector<std::string>& platforms, std::map<std::string, std::vector<std::string> >& targets, string& error){
    
    string finalURL = url + "/targets";
    
    CURL *curl = curl_easy_init();
    
    bool isInitSuccessfull = false;
    
    if (curl) {
        
        std::ostringstream oss;
        
        curl_easy_setopt(curl, CURLOPT_URL, finalURL.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &store_Response);
        curl_easy_setopt(curl, CURLOPT_FILE, &oss);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15); 
        curl_easy_setopt(curl,CURLOPT_TIMEOUT, 15);
        
        CURLcode res = curl_easy_perform(curl);
        
        if(res == CURLE_OK){
            
            long respcode; //response code of the http transaction
            
            curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);
            
            if(respcode == 200){
                
                string response(oss.str());
                const char* json = response.c_str();
                     
                if(parseOperatingSystemsList(json, platforms, targets))
                    isInitSuccessfull = true;
                else
                    error = "Targets Could not be parsed.";
            }
        }
        else
            error = "Impossible to start connection";
        curl_easy_cleanup(curl);
    }
    else
        error = "Impossible to start connection";
    
    return isInitSuccessfull;
}


// Access to FaustWeb service - Upload your faust application given a sourceFile, an operating system and an architecture
bool export_file(const std::string& url, const std::string& file, const std::string& os, const std::string& architecture, const std::string& output_type, const std::string& output_file, std::string& error){

    string key("");
    
    if(get_shaKey(url, file, key, error))
        return get_file_from_key(url, key, os, architecture, output_type, output_file, error);
    else
        return false;

}

// Access to FaustWeb service - Post your faust file and get a corresponding SHA-Key
bool get_shaKey(const std::string& url, const std::string& file, std::string& key, std::string& error){

    CURL *curl = curl_easy_init();
    struct curl_httppost* formpost = NULL;
    struct curl_httppost* lastptr = NULL;
    
    struct curl_forms forms[3]; 
    
    forms[0].option=CURLFORM_FILE; 
    forms[0].value=strdup(file.c_str()); 
    forms[1].option=CURLFORM_CONTENTTYPE;
    forms[1].value=strdup("text/plain");
    forms[2].option=CURLFORM_END;
    
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file", 
                 CURLFORM_ARRAY, forms, CURLFORM_END); 
    
    bool isInitSuccessfull = false;
    
    if (curl) {
        
        std::ostringstream oss;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &store_Response);
        curl_easy_setopt(curl, CURLOPT_FILE, &oss);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT ,15); 
        curl_easy_setopt(curl,CURLOPT_TIMEOUT, 15);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        CURLcode res = curl_easy_perform(curl);
        
        if(res != CURLE_OK)
            error = oss.str();
        else{
            
            long respcode; //response code of the http transaction
            
            curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);
            
            if(respcode == 200){
                key = oss.str();
                isInitSuccessfull = true;
            }
            else if(respcode == 400)
                error = "Impossible to generate SHA Key";
        }
        
        curl_easy_cleanup(curl);
        curl_formfree(formpost);
    }
    else
        error = "Connection Impossible To Start";
    
    return isInitSuccessfull;
}


// Access to FaustWeb service - Upload your faust application given the SHA-Key, an operating system and an architecture
bool get_file_from_key(const std::string& url, const std::string& key, const std::string& os, const std::string& architecture, const std::string& output_type, const std::string& output_file, std::string& error){
    
    bool isInitSuccessfull = false;
    
    string finalURL = url + "/" + key + "/" + os + "/" + architecture + "/" + output_type;
    
    FILE* file = fopen(output_file.c_str(), "wb");
    
    if(file){
        
        CURL *curl = curl_easy_init();
        
        if (curl) {
            
            curl_easy_setopt(curl, CURLOPT_URL, finalURL.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &store_File);
            curl_easy_setopt(curl, CURLOPT_FILE, file);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT ,15); 
            curl_easy_setopt(curl,CURLOPT_TIMEOUT, 15);
            //        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            
            CURLcode res = curl_easy_perform(curl);
            
            if(res == CURLE_OK){
                
                long respcode; //response code of the http transaction
                
                curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);
                
                if(respcode == 200)
                    isInitSuccessfull = true;
                else
                    error = "Impossible to write file";
            }
            else
                error = "Compilation failed";
            
            curl_easy_cleanup(curl);
        }
        else
            error = "Connection Impossible To Start";
        
        fclose(file);
    }
    else
        error = "Impossible to open " + output_file;
    
    return isInitSuccessfull;
}


