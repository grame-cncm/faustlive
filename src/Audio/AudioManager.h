//
//  AudioManager.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// AudioManager is an ameliorated audio. It controls one or two specific audio(s) to implement a crossfade audio between 2 dsp. 

#ifndef _AudioManager_h
#define _AudioManager_h

#include "AudioFactory.h"
#include "faust/audio/audio.h"

#include <string>
#include <QObject>

using namespace std;

class AudioSettings;

class AudioManager : public QObject, public audio {

    private:

        Q_OBJECT
    
    public:
    
        AudioManager(AudioShutdownCallback cb = NULL, void* arg = NULL){Q_UNUSED(cb);Q_UNUSED(arg);}
        virtual ~AudioManager(){}
        
        virtual bool initAudio(QString& error, const char* name, bool midi) = 0;
        
        virtual bool initAudio(QString& error, const char* name, const char* port_name, int numInputs, int numOutputs, bool midi)
            {Q_UNUSED(numInputs); Q_UNUSED(numOutputs); Q_UNUSED(port_name); return initAudio(error, name, midi);}
        
        virtual bool setDSP(QString& error, dsp* DSP, const char* port_name) = 0;

        virtual bool init_FadeAudio(QString& error, const char* name, dsp* DSP) = 0;
        virtual void start_Fade() = 0;
        virtual void wait_EndFade() = 0;
        
        virtual void connect_Audio(std::string homeFolder){Q_UNUSED(homeFolder);}
        virtual void save_Connections(std::string homeFolder){Q_UNUSED(homeFolder);}
    
        virtual int getNumInputs() { return -1; }
        virtual int getNumOutputs() { return -1; }
    
    signals: 
        
        void errorSignal(const char*);

};

#endif
