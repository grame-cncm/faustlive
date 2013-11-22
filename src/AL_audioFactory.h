//
//  AL_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Concrete factory specific to CoreAudio
// It can create 2 "products" : AudioSettings (AL_AudioSettings) and AudioManager (AL_audioManager)

#ifndef _AL_audioFactory_h
#define _AL_audioFactory_h

#include "AudioFactory.h"

class AL_audioFactory : public AudioFactory{

    public :
        AL_audioFactory();
        ~AL_audioFactory();

    virtual AudioSettings* createAudioSettings(std::string homeFolder, QGroupBox* parent);
        virtual AudioManager* createAudioManager(AudioSettings* audioParameters);
};

#endif
