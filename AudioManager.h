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

#include "faust/audio/audio.h"

#include <QObject>

class AudioSettings;

class AudioManager : public QObject, public audio{

    Q_OBJECT
    
    public :
    
    AudioManager(AudioSettings* as){}
    virtual ~AudioManager(){}
    
    virtual bool initAudio(char* error, const char* name, dsp* DSP, const char* port_name) = 0;

    virtual bool init_FadeAudio(char* error, const char* name, dsp* DSP) = 0;
    virtual void start_Fade() = 0;
    virtual void wait_EndFade() = 0;
    
    virtual void connect_Audio(std::string homeFolder){}
    virtual void save_Connections(std::string homeFolder){}
    virtual void change_Connections(std::string homeFolder, std::list<std::pair<std::string, std::string> > changeTable){}
    
    signals : 
    
    void errorSignal(const char*);

};

#endif
