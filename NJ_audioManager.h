//
//  NJ_audioManager.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//


#ifndef _NJ_audioManager_h
#define _NJ_audioManager_h

#include "AudioManager.h"
#include "NJ_audioSettings.h"

class NJ_audioFader;

class NJ_audioManager : public AudioManager{

    Q_OBJECT
    
    NJ_audioSettings*       fSettings;
    
    NJ_audioFader* fCurrentAudio;
    NJ_audioFader* fFadeInAudio;
    
    public :
        NJ_audioManager(AudioSettings* as);
        virtual ~NJ_audioManager();
    
        virtual bool init(const char*, dsp* DSP);
        virtual bool start();
        virtual void stop();
    
        virtual bool initAudio(char* error, const char* name, dsp* DSP, const char* port_name);
    
        virtual bool init_FadeAudio(char* error, const char* name, dsp* DSP);
    
        virtual void start_Fade();
    
        virtual void wait_EndFade();
    
    private slots :
    
        void send_Error(const char*);
    
};

#endif
