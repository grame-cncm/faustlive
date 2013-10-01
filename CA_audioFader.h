//
//  NJ_audioFader.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _NJ_audioFader_h
#define _NJ_audioFader_h

#include "faust/audio/coreaudio-dsp.h"
#include "audioFader_Interface.h"
#include "audioFader_Implementation.h"

class crossfade_TCoreAudioRenderer: public TCoreAudioRenderer, public audioFader_Implementation{
    
public:
    virtual OSStatus Render(AudioUnitRenderActionFlags *ioActionFlags,
                    const AudioTimeStamp *inTimeStamp,
                    UInt32 inNumberFrames,
                            AudioBufferList *ioData){
        
        //    printf("Tcoreaudio fils = %p!\n", this);
        
        AudioUnitRender(fAUHAL, ioActionFlags, inTimeStamp, 1, inNumberFrames, fInputData);
        for (int i = 0; i < fDevNumInChans; i++)
            fInChannel[i] = (float*)fInputData->mBuffers[i].mData;
        
        for (int i = 0; i < fDevNumOutChans; i++)
            fOutChannel[i] = (float*)ioData->mBuffers[i].mData;
        
        fDSP->compute((int)inNumberFrames, fInChannel, fOutChannel);
        crossfade_Calcul(inNumberFrames, fDevNumOutChans, fOutChannel);
        
        return 0;
    }
};


class CA_audioFader : public audio, public audioFader_Interface
{
    
protected:
    crossfade_TCoreAudioRenderer fCrossFadeDevice;
	int fSampleRate, fFramesPerBuf;
    
public:
    CA_audioFader(int srate, int fpb){
        
        fSampleRate = srate;
        fFramesPerBuf = fpb;
    }
    
    CA_audioFader(int fpb){ 
        
        fSampleRate = -1;
        fFramesPerBuf = fpb;
        //    printf("BUFFER SIZE = %i\n", fpb);
    }
    
	virtual ~CA_audioFader(){}
    
	virtual bool init(const char* /*name*/, dsp* DSP){
        if (fCrossFadeDevice.OpenDefault(DSP, DSP->getNumInputs(), DSP->getNumOutputs(), fFramesPerBuf, fSampleRate) < 0) {
            printf("Cannot open CoreAudio device\n");
            return false;
        }
        // If -1 was given, fSampleRate will be changed by OpenDefault
        DSP->init(fSampleRate);
        return true;
    }
    
	virtual bool start(){
        if (fCrossFadeDevice.Start() < 0) {
            printf("Cannot start CoreAudio device\n");
            return false;
        }
        return true;
    }
    
	virtual void stop(){
        fCrossFadeDevice.Stop();
        fCrossFadeDevice.Close();
    }
    
    virtual void launch_fadeOut(){
        fCrossFadeDevice.set_doWeFadeOut(true);
    }
    
    virtual void launch_fadeIn(){
        fCrossFadeDevice.set_doWeFadeIn(true);
    }
    
    virtual bool get_FadeOut(){
        //    printf("COREAUDIO fade out\n");
        return fCrossFadeDevice.get_doWeFadeOut();
    }
};

#endif
