//
//  JA_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Concrete factory specific to JackAudio
// It can create 2 "products" : AudioSettings (JA_AudioSettings) and AudioManager (JA_audioManager)

#include "JA_audioFactory.h"
#include "JA_audioSettings.h"
#include "JA_audioManager.h"

JA_audioFactory::JA_audioFactory(){}

JA_audioFactory::~JA_audioFactory(){}

//Creates Settings, corresponding to a JackAudio object
AudioSettings* JA_audioFactory::createAudioSettings(QGroupBox* parent){
    
    return new JA_audioSettings(parent);
    
}

//Creates an AudioManager, corresponding to a JackAudio object 
AudioManager* JA_audioFactory::createAudioManager(AudioShutdownCallback cb, void* arg){
    
    return new JA_audioManager(cb, arg);
    
}

