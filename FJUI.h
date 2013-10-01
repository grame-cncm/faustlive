//
//  FJUI.h
//  TEST_QT_DND
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _FJUI_h
#define _FJUI_h

#include <iostream>
#include <fstream>
#include <list>
#include <utility>

using namespace std;

class FJUI
{
    
    public : 
    
    //Saves the connections into the filename
    void saveConnections(const char* filename, std::list<std::pair<std::string, std::string> > 	Connections)
	{
        std::ofstream f(filename, ios::trunc);
        
        std::list<std::pair<string, string> > ::const_iterator it;
        
		for (it=Connections.begin(); it!=Connections.end(); it++) {
			f << it->first.c_str() << ' ' << it->second.c_str() << endl;
        }
		f.close();
	}
    
	// Returns the connections saved in filename
	std::list<std::pair<std::string, std::string> >  recallConnections(const char* filename)
	{
		std::ifstream f(filename);
		std::string  g;
		std::string	n;
        std::list<std::pair<std::string, std::string> >	Connections;
        
		while (f.good()) {
			f >> g >> n;
            
//            printf("RECALL = %s ---> %s\n", g.c_str(), n.c_str());
            Connections.push_back(make_pair(g,n));
		}
		f.close();
        
        
        return Connections;
    }
    
    //Updating the connections in the file following the changeTable
    void  update(const char* filename, std::list<pair<string,string> > changeTable)
	{
        std::list<std::pair<std::string, std::string> > 	Connections;
        
        std::ifstream readF(filename);
		string  port1, port2;
        
		while (readF.good()) {
			readF >> port1 >> port2;
            
            std::list<std::pair<std::string,std::string> >::iterator it;
            for(it = changeTable.begin(); it != changeTable.end() ; it++){
                
                int pos = port1.find(it->first);
                if(pos != std::string::npos){
                    char nextCharacter = port1[pos+it->first.length()];
                    //This way freeverb1 is not recognized in freeverb12!
                    if(nextCharacter == ':' || nextCharacter == ' ' || nextCharacter == '_'){
                        port1.erase(pos, it->first.length());
                        port1.insert(pos, it->second);
                        break;
                    }
                }
            }
            for(it = changeTable.begin(); it != changeTable.end() ; it++){
                int pos = port2.find(it->first);
                if(pos != std::string::npos){
                    char nextCharacter = port2[pos+it->first.length()];
                    if(nextCharacter == ':' || nextCharacter == ' ' || nextCharacter == '_'){
                        port2.erase(pos, it->first.length());
                        port2.insert(pos, it->second);
                        break;
                    }
                }
            }
            
            Connections.push_back(make_pair(port1, port2));
		}
		readF.close();
    
        saveConnections(filename, Connections);
    }
};


#endif
