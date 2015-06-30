//
//  PA_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Concrete factory specific to PortAudio
// It can create 2 "products" : AudioSettings (PA_AudioSettings) and AudioManager (PA_audioManager)

#include "PA_audioFactory.h"
#include "PA_audioSettings.h"
#include "PA_audioManager.h"

PA_audioFactory::PA_audioFactory(){}

PA_audioFactory::~PA_audioFactory(){}

//Creates Settings, corresponding to a JackAudio object
AudioSettings* PA_audioFactory::createAudioSettings(QGroupBox* parent)
{
    return new PA_audioSettings(parent);
}

//Creates an AudioManager, corresponding to a JackAudio object 
AudioManager* PA_audioFactory::createAudioManager(AudioShutdownCallback cb, void* arg)
{
    return new PA_audioManager(cb, arg);
}

