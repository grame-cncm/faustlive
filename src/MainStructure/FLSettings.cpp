//
//  FaustLiveApp.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLSettings.h"


FLSettings* FLSettings::_settingsInstance = 0;
//----------------------CONSTRUCTOR/DESTRUCTOR---------------------------

FLSettings::FLSettings( const QString & fileName, Format format, QObject * parent) : QSettings(fileName, format, parent){}

FLSettings::~FLSettings(){}

FLSettings* FLSettings::getInstance(){
    return FLSettings::_settingsInstance;
}

void FLSettings::initInstance(const QString homePath){

    QString settingPath = homePath + "/Settings.ini";
    
    FLSettings::_settingsInstance = new FLSettings(settingPath, QSettings::IniFormat);
}

void FLSettings::deleteInstance(){
    delete FLSettings::_settingsInstance;
}
