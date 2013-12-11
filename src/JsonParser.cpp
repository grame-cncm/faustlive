#include <fstream>
#include <iostream>
#include <ctype.h>
#include "JsonParser.h"

// ---------------------------------------------------------------------
// Parse full json record describing a json/faust interface :
// {"metadatas": "...", "ui": [{ "type": "...", "label": "...", "items": [...], "address": "...","init": "...", "min": "...", "max": "...","step": "..."}]}
//
// and store the result in map Metadatas and vector containing the items of the interface. Returns true if parsing was successfull.
// This function is used by targetsDescriptionReceived() the remote  DSP to decode the result of 
//
bool parseJson(const char*& p, map<string,string>& metadatas, vector<itemInfo*>& uiItems)
{
    parseCh(p, '{');
    do {
        string          key;
        string          value;
        
        if (parseMetaData(p, key, value, metadatas)) {
            metadatas[key] = value;
//            printf("KEY %s|| Value = %s\n", key.c_str(), value.c_str());
        } else {
            if(key.compare("ui") == 0){
                int numItems = 0;
                parseCh(p,'[') && parseUI(p, uiItems, numItems);
            }
        }
    } while (tryCh(p,','));
    return parseCh(p, '}');
}


// ---------------------------------------------------------------------
// Parse metadatas of the interface:
//  "name" : "...", "inputs" : "...", "outputs" : "...", ...
// and store the result as key/value
//
bool parseMetaData (const char*& p, string& key, string& value, map<string,string>& metadatas)
{
    if(parseWord(p,key)){
        if(key.compare("meta") == 0){
            if(parseCh(p,':') && parseCh(p,'[') && parseCh(p,'{')){
                do {
                    string          key1;
                    string          value1;
                    
                    if (parseMetaData(p, key1, value1, metadatas)) {
                        metadatas[key1] = value1;
                        printf("METADATAS = %s || %s\n", key1.c_str(), value1.c_str());
                    }
                } while (tryCh(p,','));
                
                return parseCh(p,'}') && parseCh(p,']');
            }
            else
                return false;
        }
        else
            return  parseCh(p,':') && parseWord(p, value);
    }
    else
        return false;
}


// ---------------------------------------------------------------------
// Parse gui:
//  "type" : "...", "label" : "...", "address" : "...", ...
// and store the result in uiItems Vector
//

bool parseUI (const char*& p, vector<itemInfo*>& uiItems, int& numItems)
{
    
    if(parseCh(p,'{')){
    
        string label;
        string value;
        
        do {
            
            if(parseWord(p, label)){
                
                if(label.compare("type") == 0){
                    
                    if(uiItems.size() != 0)
                        numItems++;
                    
                    if(parseCh(p, ':') && parseWord(p, value)){   
                        
                        itemInfo* item = new itemInfo;
                        item->type = value;
                        uiItems.push_back(item);
                    }
                }
                
                else if(label.compare("label") == 0){
                    if(parseCh(p, ':') && parseWord(p, value)){
                        itemInfo* item = uiItems[numItems];
                        item->label = value;
                    }
                }
                
                else if(label.compare("address") == 0){
                    if(parseCh(p, ':') && parseWord(p, value)){
                        itemInfo* item = uiItems[numItems];
                        item->address = value;
                    }
                }
                
                else if(label.compare("meta") == 0){
                    
                    string metaKey, metaValue;
                    
                    if(parseCh(p, ':') && parseCh(p,'[')){
                    
                        do{ 
                            if( parseCh(p,'{') && parseWord(p, metaKey) && parseCh(p, ':') && parseWord(p, metaValue) && parseCh(p,'}')){
                                itemInfo* item = uiItems[numItems];
                                item->meta[metaKey] = metaValue;
                            }
                                
                        }while (tryCh(p,','));
                        if(!parseCh(p,']'))
                            return false;
                    }
                
                }
                
                else if(label.compare("init") == 0){
                    if(parseCh(p, ':') && parseWord(p, value)){
                        itemInfo* item = uiItems[numItems];
                        item->init = value;
                    }
                }
                
                else if(label.compare("min") == 0){
                    if(parseCh(p, ':') && parseWord(p, value)){
                        itemInfo* item = uiItems[numItems];
                        item->min = value;
                    }
                }
                
                else if(label.compare("max") == 0){
                    if(parseCh(p, ':') && parseWord(p, value)){
                        itemInfo* item = uiItems[numItems];
                        item->max = value;
                    }
                }
                
                else if(label.compare("step") == 0){
                    if(parseCh(p, ':') && parseWord(p, value)){
                        itemInfo* item = uiItems[numItems];
                        item->step = value;
                    }
                }
                
                else if(label.compare("items") == 0){
                    
                    if(parseCh(p, ':') && parseCh(p,'[')){
                        
                        do{ 
                            if(!parseUI(p, uiItems, numItems))
                                return false;
                        }while (tryCh(p,','));
                        if(parseCh(p,']')){
                            itemInfo* item = new itemInfo;
                            item->type = "close";
                            uiItems.push_back(item);
                            numItems++;
                        }
                    }
                }
            }
            else
                return false;
            
        } while (tryCh(p,','));
        if(parseCh(p,'}'))
            return true;
        else
            return false;
    }
    else
        return false;
}

// ---------------------------------------------------------------------
//                          Elementary parsers
// ---------------------------------------------------------------------


// Advance pointer p to the first non blank character
void skipSpace(const char*& p)
{
    while (isspace(*p)) { p++; }
}

// Report a parsing error
bool parseErr(const char*& p, const char* errmsg )
{
    cerr << "Parse error : " << errmsg << " here : " << p << endl;
    return true;
}

// Parse character x, but don't report error if fails
bool tryCh(const char*& p, char x)
{
    skipSpace(p);
    if (x == *p) {
        p++;
        return true;
    } else {
        return false;
    }
}

//Parse character x, reports an error if it fails
bool parseCh(const char*& p, char x)
{
    skipSpace(p);
    if (x == *p) {
        p++;
        return true;
    } else {
//        cerr << "parsing error : expoected character '" << x << "'" << ", instead got : " << p << endl;
        return false;
    }
}

// Parse a quoted string "..." and store the result in s, reports an error if it fails
bool parseWord(const char*& p, string& s)
{
    string      str;

    skipSpace(p);

    const char* saved = p;

    if (*p++ == '"') {
        while ( (*p != 0) && (*p != '"') ) {
            str += *p++;
        }
        if (*p++=='"') {
            s = str;
            return true;
        }
    }
    p = saved;
//    std::cerr << "parsing error : expected quoted string, instead got : "<< p << std::endl;
    return false;
}



