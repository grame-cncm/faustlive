#ifndef SIMPLEPARSER_H
#define SIMPLEPARSER_H

#include <map>
#include <vector>
#include <string>

using namespace std;

bool parseOperatingSystemsList(const char*& p, vector<string>& platforms, map<string, vector<string> >& M);

#endif // SIMPLEPARSER_H
