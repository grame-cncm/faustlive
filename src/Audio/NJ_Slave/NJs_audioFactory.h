//
//  NJs_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Concrete factory specific to NetJackAudio
// It can create 2 "products" : AudioSettings (NJs_AudioSettings) and AudioManager (NJs_audioManager)

#ifndef _NJs_audioFactory_h
#define _NJs_audioFactory_h

#include "AudioFactory.h"

class NJs_audioFactory : public AudioFactory{
    
    public:
    
        NJs_audioFactory();
        virtual ~NJs_audioFactory();
        
        virtual AudioSettings* createAudioSettings(QGroupBox* parent);
        virtual AudioManager* createAudioManager(AudioShutdownCallback cb, void* arg);
};

#endif
