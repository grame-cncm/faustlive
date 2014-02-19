//
//  CA_audioFader.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// These classes re-implement the coreaudio calcul process to be adaptated to FadeIn and FadeOut processes

#ifndef _CA_audioFader_h
#define _CA_audioFader_h

#include "faust/audio/coreaudio-dsp.h"
#include "AudioFader_Interface.h"
#include "AudioFader_Implementation.h"

class crossfade_TCoreAudioRenderer: public TCoreAudioRenderer, public AudioFader_Implementation{
    
public:

    crossfade_TCoreAudioRenderer(){
//        printf("CA_AudioFader::crossfade_coreAudio constructor %p\n", this);
       reset_Values();
    }
    
//  Reimplementing audio callback to add the crossfade procedure
    virtual OSStatus Render(AudioUnitRenderActionFlags *ioActionFlags,
                    const AudioTimeStamp *inTimeStamp,
                    UInt32 inNumberFrames,
                            AudioBufferList *ioData){
        
//        printf("Tcoreaudio fils = %p || fadeOut? = %i\n", this, fDoWeFadeOut);
        
        OSStatus err = noErr;
        if (fDevNumInChans > 0) {
            err = AudioUnitRender(fAUHAL, ioActionFlags, inTimeStamp, 1, inNumberFrames, fInputData);
        }
        if (err == noErr) {
            for (int i = 0; i < fDevNumInChans; i++) {
                fInChannel[i] = (float*)fInputData->mBuffers[i].mData;
            }
            for (int i = 0; i < fDevNumOutChans; i++) {
                fOutChannel[i] = (float*)ioData->mBuffers[i].mData;
            }
            fDSP->compute(inNumberFrames, fInChannel, fOutChannel);
            
//            ADDED LINE COMPARING TO BASIC COREAUDIO
            crossfade_Calcul(inNumberFrames, fDevNumOutChans, fOutChannel);
        } else {
//            printf("AudioUnitRender error... %x\n", fInputData);
            printError(err);
        }
        return err;
        
    }
};


class CA_audioFader : public audio, public AudioFader_Interface
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
        if (fCrossFadeDevice.OpenDefault(DSP->getNumInputs(), DSP->getNumOutputs(), fFramesPerBuf, fSampleRate) < 0) {
            printf("Cannot open CoreAudio device\n");
            return false;
        }
        fCrossFadeDevice.set_dsp(DSP);
        // If -1 was given, fSampleRate will be changed by OpenDefault
        DSP->init(fSampleRate);
        return true;
    }
    
    bool init(const char* /*name*/, int numInputs, int numOutputs){
        if (fCrossFadeDevice.OpenDefault(numInputs, numOutputs, fFramesPerBuf, fSampleRate) < 0) {
            printf("Cannot open CoreAudio device\n");
            return false;
        }
        else
            return true;
    }
    
    bool set_dsp(dsp* DSP){
        
        fCrossFadeDevice.set_dsp(DSP);
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
//        printf("DOWEFADEOUT = %i || %p\n", fCrossFadeDevice.get_doWeFadeOut(), &fCrossFadeDevice);
        return fCrossFadeDevice.get_doWeFadeOut();
    }
    
    virtual int get_buffer_size() { return fFramesPerBuf; }
    virtual int get_sample_rate() { return fSampleRate; }
    
    void        force_stopFade(){ 
//        fCrossFadeDevice.set_doWeFadeOut(val);
        fCrossFadeDevice.reset_Values();
    }
//    void        force_doWeFadeIn(bool val){ 
//        fCrossFadeDevice.set_doWeFadeIn(val);
//        fCrossFadeDevice.reset_Values();
//    }
};

#endif
