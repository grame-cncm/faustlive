#ifndef FLSIMPLEPARSER_H
#define FLSIMPLEPARSER_H

#include <map>
#include <vector>
#include <string>

// Simple parser is made to parse the platforms and architectures available in FaustWeb from the JSON encoded returned by FaustWeb

using namespace std;

bool parseOperatingSystemsList(const char*& p, vector<string>& platforms, map<string, vector<string> >& M);

#endif // FLSIMPLEPARSER_H
