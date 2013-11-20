//
//  AL_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Concrete factory specific to CoreAudio
// It can create 2 "products" : AudioSettings (AL_AudioSettings) and AudioManager (AL_audioManager)

#include "AL_audioFactory.h"
#include "AL_audioSettings.h"
#include "AL_audioManager.h"


AL_audioFactory::AL_audioFactory(){}

AL_audioFactory::~AL_audioFactory(){}

//Creates Settings, corresponding to a CoreAudio object
AudioSettings* AL_audioFactory::createAudioSettings(string homeFolder, QGroupBox* parent){

    return new AL_audioSettings(homeFolder, parent);

}
//Creates an AudioManager, corresponding to a CoreAudio object
AudioManager* AL_audioFactory::createAudioManager(AudioSettings* audioParameters){

    return new AL_audioManager(audioParameters);

}

