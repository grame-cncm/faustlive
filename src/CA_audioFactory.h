//
//  CA_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Concrete factory specific to CoreAudio
// It can create 2 "products" : AudioSettings (CA_AudioSettings) and AudioManager (CA_audioManager)

#ifndef _CA_audioFactory_h
#define _CA_audioFactory_h

#include "AudioFactory.h"

class CA_audioFactory : public AudioFactory{

    public :
        CA_audioFactory();
        ~CA_audioFactory();

    virtual AudioSettings* createAudioSettings(QString homeFolder, QGroupBox* parent);
        virtual AudioManager* createAudioManager(AudioSettings* audioParameters, AudioShutdownCallback cb, void* arg);
};

#endif
