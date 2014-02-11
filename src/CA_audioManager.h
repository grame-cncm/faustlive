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
        int                 fBufferSize;
    
        CA_audioFader*      fCurrentAudio;
        CA_audioFader*      fFadeInAudio;
    
    public :
        CA_audioManager(AudioSettings* as);
        virtual ~CA_audioManager();
    
        virtual bool init(const char*, dsp* DSP);
        virtual bool initAudio(QString& error, const char* name, dsp* DSP, const char* port_name);
//        virtual void set_dsp(dsp* DSP);
        virtual bool start();
        virtual void stop();
    
        virtual bool init_FadeAudio(QString& error, const char* name, dsp* DSP);
        virtual void start_Fade();
        virtual void wait_EndFade();
    
        virtual int get_buffer_size();
        virtual int get_sample_rate();
};

#endif
