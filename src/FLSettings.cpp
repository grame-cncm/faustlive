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

FLSettings::FLSettings( const QString & fileName, Format format, QObject * parent) : QSettings(fileName, format, parent){}

FLSettings::~FLSettings(){}

FLSettings* FLSettings::getInstance(){
    if(FLSettings::_settingsInstance == 0)
//        FLSettings::_settingsInstance = new FLSethtings("Grame", "FaustLive");
        FLSettings::_settingsInstance = new FLSettings("/Users/denoux/.FaustLive-CurrentSession-1.1/Settings.ini", QSettings::IniFormat);
    
//    setPath(QSettings::IniFormat, QSettings::UserScope, "");
    
    return FLSettings::_settingsInstance;
}

void FLSettings::deleteInstance(){
    delete FLSettings::_settingsInstance;
}