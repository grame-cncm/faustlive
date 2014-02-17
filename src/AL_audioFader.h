//
//  AL_audioFader.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// These classes re-implement the coreaudio calcul process to be adaptated to FadeIn and FadeOut processes

#ifndef _AL_audioFader_h
#define _AL_audioFader_h

#include "faust/audio/alsa-dsp.h"
#include "AudioFader_Interface.h"
#include "AudioFader_Implementation.h"


class AL_audioFader : public alsaaudio, public AudioFader_Interface, public AudioFader_Implementation
{
    
public:
    AL_audioFader(int argc, char *argv[], dsp* DSP) : alsaaudio(argc,argv,DSP){}
    
	virtual ~AL_audioFader(){}
    
	virtual bool init(const char* /*name*/, dsp* DSP){
		fAudio->open();
	    DSP->init(fAudio->frequency());
		fAudio->longinfo();
 		return true;
    }
    
	virtual bool start(){
		fRunning = true;
		if (pthread_create( &fAudioThread, 0, __run, this))
			fRunning = false;
		return fRunning;
    }
    
	virtual void stop(){
        if (fRunning) {
			fRunning = false;
			pthread_join (fAudioThread, 0);
		}
    }
    
    virtual void run() {
		bool rt = setRealtimePriority();
		printf(rt ? "RT : ":"NRT: "); fAudio->shortinfo();
		if (fAudio->duplexMode()) {
            
			fAudio->write();
			fAudio->write();
			while(fRunning) {
				fAudio->read();
				fDSP->compute(fAudio->buffering(), fAudio->inputSoftChannels(), fAudio->outputSoftChannels());
//            ADDED LINE COMPARING TO BASIC ALSAAUDIO
                crossfade_Calcul(fAudio->fChanInputs, fAudio->fChanOutputs, fAudio->outputSoftChannels());
				fAudio->write();
			}
            
		} else {
            
			fAudio->write();
			while(fRunning) {
				fDSP->compute(fAudio->buffering(), fAudio->inputSoftChannels(), fAudio->outputSoftChannels());
//            ADDED LINE COMPARING TO BASIC ALSAAUDIO
                crossfade_Calcul(fAudio->fChanInputs, fAudio->fChanOutputs, fAudio->outputSoftChannels());
				fAudio->write();
			}
		}
	}
    
    virtual void launch_fadeOut(){
        set_doWeFadeOut(true);
    }
    
    virtual void launch_fadeIn(){
        set_doWeFadeIn(true);
    }
    
    virtual bool get_FadeOut(){
        return get_doWeFadeOut();
    }
};

#endif
