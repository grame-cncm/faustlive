#ifndef FLSIMPLEPARSER_H
#define FLSIMPLEPARSER_H

#include <map>
#include <vector>
#include <string>

// Simple parser is made to parse the platforms and architectures available in FaustWeb from the JSON encoded returned by FaustWeb


bool parseOperatingSystemsList(const char*& p, std::vector<std::string>& platforms, std::map<std::string, std::vector<std::string> >& M);

#endif // FLSIMPLEPARSER_H
