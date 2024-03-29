#include <fstream>
#include <iostream>
#include <ctype.h>
#include "SimpleParser.h"

// ---------------------------------------------------------------------
//                          Elementary parsers
// ---------------------------------------------------------------------

static bool parseArchitecturesList(const char*& p, std::vector<std::string>& v);

// Advance pointer p to the first non blank character
static void skipBlank(const char*& p)
{
    while (isspace(*p)) { p++; }
}

// Report a parsing error
//static bool parseError(const char*& p, const char* errmsg)
//{
//    cerr << "Parse error : " << errmsg << " here : " << p << endl;
//    return true;
//}

// Parse character x, but don't report error if fails
static bool tryChar(const char*& p, char x)
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
static bool parseChar(const char*& p, char x)
{
    skipBlank(p);
    if (x == *p) {
        p++;
        return true;
    } else {
        std::cerr << "parsing error : expected character '" << x << "'" << ", instead got : " << p << std::endl;
        return false;
    }
}

// Parse a quoted string "..." and store the result in s, reports an error if it fails
static bool parseString(const char*& p, std::string& s)
{
    std::string str;
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
    std::cerr << "parsing error : expected quoted std::string, instead got : "<< p << std::endl;
    return false;
}

// ---------------------------------------------------------------------
// Parse operating system record :
//  "os" : ["arch1, "arch2",...]
// and store the result in os and al
//
static bool parseOperatingSystem(const char*& p, std::string& os, std::vector<std::string>& al)
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
static bool parseArchitecturesList(const char*& p, std::vector<std::string>& v)
{
    std::string s;
    do {
        
        if (parseString(p,s)) {
            v.push_back(s);
        } else {
            return false;
        }
        
    } while (tryChar(p,','));
    return true;
}

// ---------------------------------------------------------------------
// Parse full json record describing available operating systems and
// associated architectures :
// {"os1" : ["arch1, "arch2",...], "os2" : ["arch1, "arch2",...],...}
// and store the result in map M. Returns true if parsing was successfull.
// This function is used by targetsDescriptionReceived() to parse the JSON
// record sent by the webservice.
//
bool parseOperatingSystemsList(const char*& p, std::vector<std::string>& platforms, std::map<std::string, std::vector<std::string> >& M)
{
    parseChar(p, '{');
    do {
        std::string          os;
        std::vector<std::string>  archlist;
        if (parseOperatingSystem(p, os, archlist)) {
            platforms.push_back(os);
            M[os] = archlist;
        } else {
            return false;
        }
    } while (tryChar(p,','));
    return parseChar(p, '}');
}



