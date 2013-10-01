//
//  AudioManager.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//


#ifndef _AudioManager_h
#define _AudioManager_h

#include "AudioSettings.h"
#include "faust/audio/audio.h"
//#include "faust/audio/dsp.h"

#include <QObject>

class AudioManager : public QObject, public audio{

    Q_OBJECT
    
    AudioSettings*  fParams;
    
    public :
    
    AudioManager(AudioSettings* as){
    
        fParams = as;
    }
    virtual ~AudioManager(){}
    
    virtual bool init(const char*, dsp* DSP){}
    virtual bool start(){}
    virtual void stop(){}
    
    virtual bool initAudio(char* error, const char* name, dsp* DSP, const char* port_name){}
    
    virtual bool init_FadeAudio(char* error, const char* name, dsp* DSP){}
    
    virtual void connect_Audio(string homeFolder){}
    
    virtual void save_Connections(string homeFolder){}
    
    virtual void change_Connections(string homeFolder, list<pair<string, string> > changeTable){}
    
    virtual void start_Fade(){}
    virtual void wait_EndFade(){}
    
    signals : 
    
    void errorSignal(const char*);

};

#endif
