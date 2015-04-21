//
//  FLInterfaceManager.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLInterfaceManager keeps track of the interfaces registered and can update them, protecting multiple access to the interface list with a Mutex.
// It is a singleton in order to be easily acccessible from any another class.

#ifndef _FLInterfaceManager_h
#define _FLInterfaceManager_h

//#include <QtGui>
//#if QT_VERSION >= 0x050000
//#include <QtWidgets>
//#endif

#include <list>
#include "faust/gui/GUI.h"
#include "TMutex.h"

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
