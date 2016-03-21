//
//  NJm_audioSettings.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class describes the Settings of netjack clients : 
//  - Its compression Value
//  - Its IPadress
//  - Its Port
//  - Its Latencty
// All of them are described visually by a QLineEdit

#include "NJm_audioSettings.h"
#include "utilities.h"
#include "FLSettings.h"

NJm_audioSettings::NJm_audioSettings(QGroupBox* parent) : AudioSettings(parent){}

NJm_audioSettings::~NJm_audioSettings(){}

//Real Params TO Visual params
void NJm_audioSettings::setVisualSettings(){}

//Visual Params TO Real params
void NJm_audioSettings::storeVisualSettings(){}

//Operator== for NetJack Settings
bool NJm_audioSettings::isEqual(AudioSettings*)
{
    return true;
}

QString NJm_audioSettings::get_ArchiName()
{
    return "NetJackMaster";
}




