//
//  FJUI.h
//  TEST_QT_DND
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Interface to save Jack Connections in a file

#ifndef _FJUI_h
#define _FJUI_h

#include <iostream>
#include <fstream>
#include <list>
#include <utility>

using namespace std;

struct FJUI
{
    
    // Saves the connections into the filename
    static void saveConnections(const char* filename, std::list<std::pair<std::string, std::string> > Connections)
	{
        std::ofstream f(filename, ios::trunc);
        std::list<std::pair<string, string> > ::const_iterator it;
        
		for (it = Connections.begin(); it != Connections.end(); it++) {
			f << endl<< "\"" <<it->first.c_str()<< "\"" << ' ' << "\""<< it->second.c_str()<< "\"";
        }
        
		f.close();
	}
    
	// Returns the connections saved in filename
	static std::list<std::pair<std::string, std::string> > recallConnections(const char* filename)
	{
		std::ifstream f(filename);
        f >> std::noskipws;
		std::string g = "";
		std::string	n = "";
//      Storing in g when 1 to 2, storing to n when 3 to 4
        int gORn = 0;
        char cote;
        std::list<std::pair<std::string, std::string> >	Connections;
        
		while (f.good()) {
//          f>>cote;
            cote = f.get();
            
            if (f.good()) {
                
                if (cote == '\"') {
                    gORn++;
                } else if (gORn == 1) {
                    g += cote;
                } else if (gORn == 3) {
                    n += cote;
                }
                
                if (gORn == 4) { 
                    Connections.push_back(make_pair(g,n));
//                    printf("Connect = %s To %s\n", g.c_str(), n.c_str());
                    gORn = 0;
                    g = "";
                    n = "";
                }
//              printf("g = %s || n = %s\n", g.c_str(), n.c_str());
            }
		}
		f.close();
        return Connections;
    }
    
    //Updating the connections in the file following the changeTable
    static void update(const char* filename, std::map<std::string, std::string> changeTable)
	{
        std::list<std::pair<std::string, std::string> > Connections;
        
        std::ifstream readF(filename);
		string  port1, port2;
        
		while (readF.good()) {
			readF >> port1 >> port2;
            
            std::map<std::string,std::string>::iterator it;
            for (it = changeTable.begin(); it != changeTable.end() ; it++) {
                size_t pos = port1.find(it->first);
                if (pos != std::string::npos) {
                    char nextCharacter = port1[pos+it->first.length()];
                    //This way freeverb1 is not recognized in freeverb12!
                    if (nextCharacter == ':') {
                        port1.erase(pos, it->first.length());
                        port1.insert(pos, it->second);
                        break;
                    }
                }
            }
            for (it = changeTable.begin(); it != changeTable.end() ; it++) {
                size_t pos = port2.find(it->first);
                if (pos != std::string::npos) {
                    char nextCharacter = port2[pos+it->first.length()];
                    if (nextCharacter == ':') {
                        port2.erase(pos, it->first.length());
                        port2.insert(pos, it->second);
                        break;
                    }
                }
            }
            
            Connections.push_back(make_pair(port1, port2));
		}
		readF.close();
        FJUI::saveConnections(filename, Connections);
    }
};

#endif
