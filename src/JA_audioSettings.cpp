//
//  JA_audioSettings.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "JA_audioSettings.h"

JA_audioSettings::JA_audioSettings(std::string home, QGroupBox* parent) : AudioSettings(home, parent){}

JA_audioSettings::~JA_audioSettings(){}

void JA_audioSettings::readSettings(){}
void JA_audioSettings::writeSettings(){}
void JA_audioSettings::setVisualSettings(){}
void JA_audioSettings::storeVisualSettings(){}

bool JA_audioSettings::isEqual(AudioSettings* /*as*/){
    
    return true;
}

std::string JA_audioSettings::get_ArchiName(){
    return "Jackaudio";
}