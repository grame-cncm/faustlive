//
//  JA_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "JA_audioFactory.h"
#include "JA_audioSettings.h"
#include "JA_audioManager.h"

JA_audioFactory::JA_audioFactory(){}

JA_audioFactory::~JA_audioFactory(){}

AudioSettings* JA_audioFactory::createAudioSettings(string homeFolder, QGroupBox* parent){
    
    return new JA_audioSettings(homeFolder, parent);
    
}

AudioManager* JA_audioFactory::createAudioManager(AudioSettings* audioParameters){
    
    return new JA_audioManager(audioParameters);
    
}

