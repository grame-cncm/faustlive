#ifndef SIMPLEPARSER_H
#define SIMPLEPARSER_H

#include <map>
#include <vector>
#include <string>

using namespace std;

struct itemInfo{
    
    string type;
    string label;
    string address;
    string init;
    string min;
    string max;
    string step;
    map<string,string> meta;
    
};

bool parseJson(const char*& p, map<string,string>& metadatas, vector<itemInfo*>& uiItems);

// ------------------------- implementation ------------------------------
bool parseMetaData (const char*& p, string& key, string& value, map<string,string>& metadatas);
bool parseUI (const char*& p, vector<itemInfo*>& uiItems, int& numItems);

void skipSpace(const char*& p);
bool parseErr(const char*& p, const char* errmsg );
bool tryCh(const char*& p, char x);
bool parseCh(const char*& p, char x);
bool parseWord(const char*& p, string& s);

#endif // SIMPLEPARSER_H
