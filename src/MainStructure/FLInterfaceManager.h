//
//  FLInterfaceManager.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//
// This class was added to try questionning the update of the interfaces and their synchronization. 
// I'm not so sure it has its use right now BUT it could be of some use with all the remote features.

// When there is a QT interface, it is the one that handles the update of all guis but in a case where ther would  be none, it has to be handled differently
// FLInterfaceManager keeps track of the interfaces registered and can update them, protecting multiple access to the interface list with a Mutex.
// It is a singleton in order to be easily acccessible from any another class.

#ifndef _FLInterfaceManager_h
#define _FLInterfaceManager_h

#include <list>
#include "TMutex.h"

#if defined(_WIN32) && !defined(GCC)
# pragma warning (disable: 4100)
#else
# pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "faust/gui/GUI.h"

class FLInterfaceManager
{
    private:
    
        TMutex fLocker;
        std::list<GUI*>	fGuiList;
        
        static FLInterfaceManager* _interfaceManagerInstance;
        
    public: 
        
        FLInterfaceManager();
        virtual ~FLInterfaceManager();

        static FLInterfaceManager* _Instance();
  
        void updateAllGuis();
        void registerGUI(GUI* ui);
        void unregisterGUI(GUI* ui);
};

#endif
