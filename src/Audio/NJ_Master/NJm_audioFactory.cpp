 //
//  NJm_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Concrete factory specific to NetJackAudio
// It can create 2 "products" : AudioSettings (NJm_AudioSettings) and AudioManager (NJm_audioManager)

#include "NJm_audioFactory.h"
#include "NJm_audioSettings.h"
#include "NJm_audioManager.h"

NJm_audioFactory::NJm_audioFactory(){}

NJm_audioFactory::~NJm_audioFactory(){}

//Creates Settings, corresponding to a NetJack object
AudioSettings* NJm_audioFactory::createAudioSettings(QGroupBox* parent){
    
    return new NJm_audioSettings(parent);
}

//Creates an AudioManager, corresponding to a NetJack object
AudioManager* NJm_audioFactory::createAudioManager(AudioShutdownCallback cb, void* arg){
    
    return new NJm_audioManager(cb, arg);
}

