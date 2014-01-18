//
//  PA_audioManager.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// PA_audioManager controls 1 PA_audioFader. It can switch from one DSP to another with a crossfade or it can act like a simple jackaudio-dsp
// PA_audioManager also controls the jack connections of the audio. 

#ifndef _PA_audioManager_h
#define _PA_audioManager_h

#include "AudioManager.h"
#include "PA_audioSettings.h"
#include "FJUI.h"

class PA_audioFader;

class PA_audioManager : public AudioManager{

    Q_OBJECT
    
    PA_audioSettings*       fSettings;
    PA_audioFader*          fCurrentAudio;
    PA_audioFader*          fFadeInAudio;
    
    public :
        PA_audioManager(AudioSettings* as);
        virtual ~PA_audioManager();
    
    virtual bool init(const char*, dsp* DSP);
    virtual bool initAudio(string& error, const char* name, dsp* DSP, const char* port_name);
    virtual bool start();
    virtual void stop();
    
    virtual bool init_FadeAudio(string& error, const char* name, dsp* DSP);
    virtual void start_Fade();
    virtual void wait_EndFade();
    
    virtual int get_buffer_size();
    virtual int get_sample_rate();
};

#endif
