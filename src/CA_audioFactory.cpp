//
//  CA_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Concrete factory specific to CoreAudio
// It can create 2 "products" : AudioSettings (CA_AudioSettings) and AudioManager (CA_audioManager)

#include "CA_audioFactory.h"
#include "CA_audioSettings.h"
#include "CA_audioManager.h"


CA_audioFactory::CA_audioFactory(){}

CA_audioFactory::~CA_audioFactory(){}

//Creates Settings, corresponding to a CoreAudio object
AudioSettings* CA_audioFactory::createAudioSettings(QString homeFolder, QGroupBox* parent){

    return new CA_audioSettings(homeFolder, parent);

}
//Creates an AudioManager, corresponding to a CoreAudio object
AudioManager* CA_audioFactory::createAudioManager(AudioSettings* audioParameters, AudioShutdownCallback cb, void* arg){

    return new CA_audioManager(audioParameters, cb, arg);

}

