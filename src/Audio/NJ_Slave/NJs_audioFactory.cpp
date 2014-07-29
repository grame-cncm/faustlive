 //
//  NJs_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Concrete factory specific to NetJackAudio
// It can create 2 "products" : AudioSettings (NJs_AudioSettings) and AudioManager (NJs_audioManager)

#include "NJs_audioFactory.h"
#include "NJs_audioSettings.h"
#include "NJs_audioManager.h"

NJs_audioFactory::NJs_audioFactory(){}

NJs_audioFactory::~NJs_audioFactory(){}

//Creates Settings, corresponding to a NetJack object
AudioSettings* NJs_audioFactory::createAudioSettings(QGroupBox* parent){
    
    return new NJs_audioSettings(parent);
}

//Creates an AudioManager, corresponding to a NetJack object
AudioManager* NJs_audioFactory::createAudioManager(AudioShutdownCallback cb, void* arg){
    
    return new NJs_audioManager(cb, arg);
}

