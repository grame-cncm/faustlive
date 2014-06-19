//
//  FaustLiveApp.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLSettings.h"

FLSettings* FLSettings::_settingsInstance = 0;
//----------------------CONSTRUCTOR/DESTRUCTOR---------------------------

FLSettings::FLSettings( const QString & organization, const QString & application, QObject * parent) : QSettings(organization, application, parent){}

FLSettings::FLSettings(Format format, Scope scope, const QString & organization, const QString & application, QObject * parent) : QSettings(format, scope, organization, application, parent){}

FLSettings::~FLSettings(){}

FLSettings* FLSettings::getInstance(){
    if(FLSettings::_settingsInstance == 0)
        FLSettings::_settingsInstance = new FLSettings("Grame", "FaustLive");
//        FLSettings::_settingsInstance = new FLSettings(QSettings::IniFormat, QSettings::UserScope, "FaustList", "Grame");
    
    setPath(QSettings::IniFormat, QSettings::UserScope, "/Users/denoux/.FaustLive-CurrentSession-1.1");
    
    return FLSettings::_settingsInstance;
}

void FLSettings::deleteInstance(){
    delete FLSettings::_settingsInstance;
}