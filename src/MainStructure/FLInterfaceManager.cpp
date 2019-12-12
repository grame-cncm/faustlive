//
//  FLInterfaceManager.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLInterfaceManager.h"

FLInterfaceManager* FLInterfaceManager::_interfaceManagerInstance = NULL;

//----------------------CONSTRUCTOR/DESTRUCTOR---------------------------

FLInterfaceManager::FLInterfaceManager(){}

FLInterfaceManager::~FLInterfaceManager(){}

FLInterfaceManager* FLInterfaceManager::_Instance()
{
    if (_interfaceManagerInstance == NULL) {
        FLInterfaceManager::_interfaceManagerInstance = new FLInterfaceManager;
    }
    
    return FLInterfaceManager::_interfaceManagerInstance;
}

void FLInterfaceManager::updateAllGuis()
{
    if (fLocker.Lock()) {
        GUI::updateAllGuis();
        fLocker.Unlock();
    }
}

void FLInterfaceManager::registerGUI(GUI* ui)
{
    if (fLocker.Lock()) {
        fGuiList.push_back(ui);
        fLocker.Unlock();
    }
}

void FLInterfaceManager::unregisterGUI(GUI* ui)
{
    if (fLocker.Lock()) {
        fGuiList.remove(ui);
        fLocker.Unlock();
    }
}
