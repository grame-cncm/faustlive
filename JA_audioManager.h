//
//  JA_audioManager.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//


#ifndef _JA_audioManager_h
#define _JA_audioManager_h

#include "AudioManager.h"
#include "JA_audioSettings.h"
#include "FJUI.h"

class JA_audioFader;

class JA_audioManager : public AudioManager{

    Q_OBJECT
    
    JA_audioSettings*       fSettings;
    
    FJUI*                   fInterface;
    JA_audioFader*    fCurrentAudio;
    
    public :
        JA_audioManager(AudioSettings* as);
        virtual ~JA_audioManager();
    
    virtual bool init(const char*, dsp* DSP);
    virtual bool start();
    virtual void stop();
    
    virtual bool initAudio(char* error, const char* name, dsp* DSP, const char* port_name);
    
    virtual bool init_FadeAudio(char* error, const char* name, dsp* DSP);
    
    virtual void connect_Audio(string homeDir);
    
    virtual void save_Connections(string homeDir);
    
    virtual void change_Connections(string homeDir, list<pair<string, string> > changeTable);
    
    virtual void start_Fade();
    virtual void wait_EndFade();
    
};

#endif
