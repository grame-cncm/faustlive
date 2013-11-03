#ifndef SIMPLEPARSER_H
#define SIMPLEPARSER_H

#include <map>
#include <vector>
#include <string>

using namespace std;

bool parseOperatingSystemsList (const char*& p, vector<string>& platforms, map<string, vector<string> >& M);

// ------------------------- implementation ------------------------------
bool parseOperatingSystem (const char*& p, string& os, vector<string>& al);
bool parseArchitecturesList (const char*& p, vector<string>& v);

void skipBlank(const char*& p);
bool parseError(const char*& p, const char* errmsg );
bool tryChar(const char*& p, char x);
bool parseChar(const char*& p, char x);
bool parseString(const char*& p, string& s);

#endif // SIMPLEPARSER_H
