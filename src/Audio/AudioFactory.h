//
//  audioFactory.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Abstract AudioFactory. 
// To implement a new type of audio architecture, this class has to be inherited to create a concret audioFactory

#ifndef _AudioFactory_h
#define _AudioFactory_h

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

class AudioSettings;
class AudioManager;

typedef void (* AudioShutdownCallback)(const char* message, void* arg);

class AudioFactory{

    public:
    
        AudioFactory(){}
        virtual ~AudioFactory(){}

        virtual AudioSettings* createAudioSettings(QGroupBox* parent) = 0;
        virtual AudioManager* createAudioManager(AudioShutdownCallback cb, void* arg) = 0;
    
};

#endif
