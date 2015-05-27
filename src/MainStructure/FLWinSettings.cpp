//
//  FLWinSettings.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLWinSettings.h"
#include "FLSettings.h"

//----------------------CONSTRUCTOR/DESTRUCTOR---------------------------

FLWinSettings::FLWinSettings(int index , const QString & fileName, Format format, QObject * parent) : QSettings(fileName, format, parent){

    fIndex = index;
}

//Deleting the window settings in the general settings
FLWinSettings::~FLWinSettings(){
    
    FLSettings* generalSettings = FLSettings::_Instance();

    generalSettings->beginGroup("Windows");
    
    QString settingToErase = QString::number(fIndex) + "/Path";
    generalSettings->remove(settingToErase);
    
    settingToErase =  QString::number(fIndex) + "/Name";
    generalSettings->remove(settingToErase);
    
    settingToErase =  QString::number(fIndex) + "/SHA";
    generalSettings->remove(settingToErase);
    
    generalSettings->endGroup();
}

//Adds the pair <key, value> to the window settings 
// AND synchronizes Path, Name and SHA in the general settings
void FLWinSettings::setValue (const QString & key, const QVariant & value ){
    
    QSettings::setValue(key, value);
    
    FLSettings* generalSettings = FLSettings::_Instance();
    
    if(key == "Path" || key == "Name" || key == "SHA"){
    
        generalSettings->beginGroup("Windows");
        
        QString generalKey = QString::number(fIndex) + "/" + key;
        generalSettings->setValue(generalKey, value);
        
        generalSettings->endGroup();
    }
}