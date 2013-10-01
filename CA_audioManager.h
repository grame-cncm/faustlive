//
//  CA_audioManager.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//


#ifndef _CA_audioManager_h
#define _CA_audioManager_h

#include "AudioManager.h"
#include "CA_audioSettings.h"

class CA_audioFader;

class CA_audioManager : public AudioManager{

    Q_OBJECT
    
    CA_audioSettings*    fSettings;
    
    CA_audioFader* fCurrentAudio;
    CA_audioFader* fFadeInAudio;
    
    public :
        CA_audioManager(AudioSettings* as);
        virtual ~CA_audioManager();
    
    virtual bool init(const char*, dsp* DSP);
    virtual bool start();
    virtual void stop();
    
    virtual bool initAudio(char* error, const char* name, dsp* DSP, const char* port_name);
    
    virtual bool init_FadeAudio(char* error, const char* name, dsp* DSP);
    
    virtual void start_Fade();
    
    virtual void wait_EndFade();
    
};

#endif
