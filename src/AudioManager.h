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

#include <string>
#include <QObject>

using namespace std;

class AudioSettings;

class AudioManager : public QObject, public audio{

    Q_OBJECT
    
    public :
    
    AudioManager(AudioSettings* as){Q_UNUSED(as);}
    virtual ~AudioManager(){}
    
    virtual bool initAudio(QString& error, const char* name, dsp* DSP, const char* port_name) = 0;

    virtual bool init_FadeAudio(QString& error, const char* name, dsp* DSP) = 0;
    virtual void start_Fade() = 0;
    virtual void wait_EndFade() = 0;
    
    virtual void connect_Audio(QString homeFolder){Q_UNUSED(homeFolder);}
    virtual void save_Connections(QString homeFolder){Q_UNUSED(homeFolder);}
    
    virtual void change_Connections(QString homeFolder, QList<std::pair<QString, QString> > changeTable){Q_UNUSED(homeFolder); Q_UNUSED(changeTable);}
    
    virtual int get_buffer_size() = 0;
    virtual int get_sample_rate() = 0;
    
    signals : 
    
    void errorSignal(const char*);

};

#endif
