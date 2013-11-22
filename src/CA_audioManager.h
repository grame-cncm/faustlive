//
//  CA_audioManager.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// CA_audioManager controls 2 CA_audioFader. It can switch from one to another with a crossfade or it can act like a simple coreaudio-dsp

#ifndef _CA_audioManager_h
#define _CA_audioManager_h

#include "AudioManager.h"

class CA_audioFader;
class CA_audioSettings;

class CA_audioManager : public AudioManager{

    Q_OBJECT
    
    //Settings are common to the 2 audioFader that can co-exist during a crossfade
        CA_audioSettings*   fSettings;
    
        CA_audioFader*      fCurrentAudio;
        CA_audioFader*      fFadeInAudio;
    
    public :
        CA_audioManager(AudioSettings* as);
        virtual ~CA_audioManager();
    
        virtual bool init(const char*, dsp* DSP);
        virtual bool initAudio(string& error, const char* name, dsp* DSP, const char* port_name);
        virtual bool start();
        virtual void stop();
    
        virtual bool init_FadeAudio(string& error, const char* name, dsp* DSP);
        virtual void start_Fade();
        virtual void wait_EndFade();
    
};

#endif
