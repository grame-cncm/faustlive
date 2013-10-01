//
//  CA_audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//


#ifndef _CA_audioFactory_h
#define _CA_audioFactory_h

#include "AudioFactory.h"

class CA_audioFactory : public AudioFactory{

    public :
        CA_audioFactory();
        ~CA_audioFactory();

        virtual AudioSettings* createAudioSettings(string homeFolder, QGroupBox* parent);
        virtual AudioManager* createAudioManager(AudioSettings* audioParameters);
};

#endif
