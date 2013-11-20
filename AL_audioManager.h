//
//  AL_audioManager.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// AL_audioManager controls 2 AL_audioFader. It can switch from one to another with a crossfade or it can act like a simple coreaudio-dsp

#ifndef _AL_audioManager_h
#define _AL_audioManager_h

#include "AudioManager.h"

class AL_audioFader;
class AL_audioSettings;

class AL_audioManager : public AudioManager{

    Q_OBJECT
    
    //Settings are common to the 2 audioFader that can co-exist during a crossfade
        AL_audioSettings*   fSettings;
    
        AL_audioFader*      fCurrentAudio;
        AL_audioFader*      fFadeInAudio;
    
    public :
        AL_audioManager(AudioSettings* as);
        virtual ~AL_audioManager();
    
        virtual bool init(const char*, dsp* DSP);
        virtual bool initAudio(string& error, const char* name, dsp* DSP, const char* port_name);
        virtual bool start();
        virtual void stop();
    
        virtual bool init_FadeAudio(string& error, const char* name, dsp* DSP);
        virtual void start_Fade();
        virtual void wait_EndFade();
    
};

#endif
