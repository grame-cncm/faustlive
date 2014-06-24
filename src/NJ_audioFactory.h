//
//  NJ_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Concrete factory specific to NetJackAudio
// It can create 2 "products" : AudioSettings (NJ_AudioSettings) and AudioManager (NJ_audioManager)

#ifndef _NJ_audioFactory_h
#define _NJ_audioFactory_h

#include "AudioFactory.h"

class NJ_audioFactory : public AudioFactory{
    
    public :
    NJ_audioFactory();
    ~NJ_audioFactory();
    
    virtual AudioSettings* createAudioSettings(QGroupBox* parent);
    virtual AudioManager* createAudioManager(AudioShutdownCallback cb, void* arg);
};

#endif
