//
//  CA_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Concrete factory specific to JackAudio
// It can create 2 "products" : AudioSettings (JA_AudioSettings) and AudioManager (JA_audioManager)

#ifndef _JA_audioFactory_h
#define _JA_audioFactory_h

#include "AudioFactory.h"

class JA_audioFactory : public AudioFactory{
    
    public :
    JA_audioFactory();
    ~JA_audioFactory();
    
    virtual AudioSettings* createAudioSettings(QGroupBox* parent);
    virtual AudioManager* createAudioManager(AudioShutdownCallback cb, void* arg);
};

#endif
