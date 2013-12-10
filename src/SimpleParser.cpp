#include <fstream>
#include <iostream>
#include <ctype.h>
#include "SimpleParser.h"


// ---------------------------------------------------------------------
// Parse full json record describing available operating systems and
// associated architectures :
// {"os1" : ["arch1, "arch2",...], "os2" : ["arch1, "arch2",...],...}
// and store the result in map M. Returns true if parsing was successfull.
// This function is used by targetsDescriptionReceived() to parse the JSON
// record sent by the webservice.
//
bool parseOperatingSystemsList (const char*& p, vector<string>& platforms, map<string, vector<string> >& M)
{
    parseChar(p, '{');
    do {
        string          os;
        vector<string>  archlist;
        if (parseOperatingSystem(p, os, archlist)) {
            platforms.push_back(os);
            M[os] = archlist;
        } else {
            return false;
        }
    } while (tryChar(p,','));
    return parseChar(p, '}');
}


// ---------------------------------------------------------------------
// Parse operating system record :
//  "os" : ["arch1, "arch2",...]
// and store the result in os and al
//
bool parseOperatingSystem (const char*& p, string& os, vector<string>& al)
{
    return  parseString(p,os) && parseChar(p,':')
            && parseChar(p,'[')
            && parseArchitecturesList(p,al)
            && parseChar(p,']');
}


// ---------------------------------------------------------------------
// Parse an architecture list
//  "arch1, "arch2",...
// and store the result in a vector v
//
bool parseArchitecturesList (const char*& p, vector<string>& v)
{
    string s;
    do {

        if (parseString(p,s)) {
            v.push_back(s);
        } else {
            return false;
        }

    } while ( tryChar(p,','));
    return true;
}


// ---------------------------------------------------------------------
//                          Elementary parsers
// ---------------------------------------------------------------------


// Advance pointer p to the first non blank character
void skipBlank(const char*& p)
{
    while (isspace(*p)) { p++; }
}

// Report a parsing error
bool parseError(const char*& p, const char* errmsg )
{
    cerr << "Parse error : " << errmsg << " here : " << p << endl;
    return true;
}

// Parse character x, but don't report error if fails
bool tryChar(const char*& p, char x)
{
    skipBlank(p);
    if (x == *p) {
        p++;
        return true;
    } else {
        return false;
    }
}

//Parse character x, reports an error if it fails
bool parseChar(const char*& p, char x)
{
    skipBlank(p);
    if (x == *p) {
        p++;
        return true;
    } else {
        cerr << "parsing error : expoected character '" << x << "'" << ", instead got : " << p << endl;
        return false;
    }
}

// Parse a quoted string "..." and store the result in s, reports an error if it fails
bool parseString(const char*& p, string& s)
{
    string str;

    skipBlank(p);

    const char* saved = p;

    if (*p++ == '"') {
        while ((*p != 0) && (*p != '"')) {
            str += *p++;
        }
        if (*p++=='"') {
            s = str;
            return true;
        }
    }
    p = saved;
    std::cerr << "parsing error : expected quoted string, instead got : "<< p << std::endl;
    return false;
}



