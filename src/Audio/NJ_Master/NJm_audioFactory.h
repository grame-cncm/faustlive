//
//  NJm_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Concrete factory specific to NetJackAudio
// It can create 2 "products" : AudioSettings (NJm_AudioSettings) and AudioManager (NJm_audioManager)

#ifndef _NJm_audioFactory_h
#define _NJm_audioFactory_h

#include "AudioFactory.h"

class NJm_audioFactory : public AudioFactory {
    
    public:
    
        NJm_audioFactory();
        virtual ~NJm_audioFactory();
        
        virtual AudioSettings* createAudioSettings(QGroupBox* parent);
        virtual AudioManager* createAudioManager(AudioShutdownCallback cb, void* arg);
};

#endif
