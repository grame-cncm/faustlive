//
//  audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//


#ifndef _AudioFactory_h
#define _AudioFactory_h

#include "AudioSettings.h"
#include "AudioManager.h"

class QWidget;

class AudioFactory{

    public :
    AudioFactory(){}
    ~AudioFactory(){}

        virtual AudioSettings* createAudioSettings(string homeFolder, QGroupBox* parent) = 0;
        virtual AudioManager* createAudioManager(AudioSettings* audioParameters) = 0;
    
};

#endif
