 //
//  NJ_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Concrete factory specific to NetJackAudio
// It can create 2 "products" : AudioSettings (NJ_AudioSettings) and AudioManager (NJ_audioManager)

#include "NJ_audioFactory.h"
#include "NJ_audioSettings.h"
#include "NJ_audioManager.h"

NJ_audioFactory::NJ_audioFactory(){}

NJ_audioFactory::~NJ_audioFactory(){}

//Creates Settings, corresponding to a NetJack object
AudioSettings* NJ_audioFactory::createAudioSettings(QString homeFolder, QGroupBox* parent){
    
    return new NJ_audioSettings(homeFolder, parent);
}

//Creates an AudioManager, corresponding to a NetJack object
AudioManager* NJ_audioFactory::createAudioManager(AudioSettings* audioParameters, AudioShutdownCallback cb, void* arg){
    
    return new NJ_audioManager(audioParameters, cb, arg);
}

