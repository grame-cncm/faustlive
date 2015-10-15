//
//  PA_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Concrete factory specific to JackAudio
// It can create 2 "products" : AudioSettings (PA_AudioSettings) and AudioManager (PA_audioManager)

#ifndef _PA_audioFactory_h
#define _PA_audioFactory_h

#include "AudioFactory.h"

class PA_audioFactory : public AudioFactory {
    
    public:
    
        PA_audioFactory();
        virtual ~PA_audioFactory();
        
        virtual AudioSettings* createAudioSettings(QGroupBox* parent);
        virtual AudioManager* createAudioManager(AudioShutdownCallback cb, void* arg);
};

#endif
