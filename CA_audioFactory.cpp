//
//  CA_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "CA_audioFactory.h"
#include "CA_audioSettings.h"
#include "CA_audioManager.h"


CA_audioFactory::CA_audioFactory(){}

CA_audioFactory::~CA_audioFactory(){}

AudioSettings* CA_audioFactory::createAudioSettings(string homeFolder, QGroupBox* parent){

    return new CA_audioSettings(homeFolder, parent);

}

AudioManager* CA_audioFactory::createAudioManager(AudioSettings* audioParameters){

    return new CA_audioManager(audioParameters);

}

