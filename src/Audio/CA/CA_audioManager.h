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

class CA_audioManager : public AudioManager {

    private:
    
        Q_OBJECT
    
        //Settings are common to the 2 audioFader that can co-exist during a crossfade    
        int                 fBufferSize;
        const char*         fName;
        bool                fInit; //To know which way the audio was initialized
    
        CA_audioFader*      fCurrentAudio;
        CA_audioFader*      fFadeInAudio;
    
    public :
        CA_audioManager(AudioShutdownCallback cb, void* arg);
        virtual ~CA_audioManager();
    
        virtual bool init(const char*, dsp* DSP);
        virtual bool initAudio(QString& error, const char* name);
        virtual bool setDSP(QString& error, dsp* DSP, const char* port_name);
        virtual bool initAudio(QString& error, const char* name, const char* port_name, int numInputs, int numOutputs);
    
        virtual bool start();
        virtual void stop();
    
        virtual bool init_FadeAudio(QString& error, const char* name, dsp* DSP);
        virtual void start_Fade();
        virtual void wait_EndFade();
    
        virtual int get_buffer_size();
        virtual int get_sample_rate();
};

#endif
