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

class crossfade_TCoreAudioRenderer: public TCoreAudioRenderer, public AudioFader_Implementation {
    
    public:

        crossfade_TCoreAudioRenderer()
        {
           reset_Values();
        }
        
        //  Reimplementing audio callback to add the crossfade procedure
        virtual OSStatus Render(AudioUnitRenderActionFlags *ioActionFlags,
                                const AudioTimeStamp *inTimeStamp,
                                UInt32 inNumberFrames,
                                AudioBufferList *ioData)
        {
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
                fDSP->compute(double(AudioConvertHostTimeToNanos(inTimeStamp->mHostTime))/1000., inNumberFrames, fInChannel, fOutChannel);
                
                // ADDED LINE COMPARING TO BASIC COREAUDIO
                crossfade_Calcul(inNumberFrames, fDevNumOutChans, fOutChannel);
            } else {
                printError(err);
            }
            return err;
        }
};

class CA_audioFader : public audio, public AudioFader_Interface
{
    
    protected:
    
        crossfade_TCoreAudioRenderer fCrossFadeDevice;
        int fSampleRate, fBufferSize;
        
    public:
    
        CA_audioFader(int srate, int fpb)
        {
            fSampleRate = srate;
            fBufferSize = fpb;
        }
        
        CA_audioFader(int fpb)
        { 
            fSampleRate = -1;
            fBufferSize = fpb;
        }
        
        virtual ~CA_audioFader()
        {
            fCrossFadeDevice.Stop();
            fCrossFadeDevice.Close();
        }
        
        virtual bool init(const char* /*name*/, dsp* DSP)
        {
            if (fCrossFadeDevice.OpenDefault(DSP->getNumInputs(), DSP->getNumOutputs(), fBufferSize, fSampleRate) < 0) {
                printf("Cannot open CoreAudio device\n");
                return false;
            } else {
                fCrossFadeDevice.set_dsp(DSP);
                // If -1 was given, fSampleRate will be changed by OpenDefault
                DSP->init(fSampleRate);
                return true;
            }
        }
        
        bool init(const char* /*name*/, int numInputs, int numOutputs)
        {
            if (fCrossFadeDevice.OpenDefault(numInputs, numOutputs, fBufferSize, fSampleRate) < 0) {
                printf("Cannot open CoreAudio device\n");
                return false;
            } else {
                return true;
            }
        }
        
        void set_dsp(dsp* DSP)
        {
            fCrossFadeDevice.set_dsp(DSP);
            DSP->init(fSampleRate);
        }
        
        virtual bool start()
        {
            if (fCrossFadeDevice.Start() < 0) {
                printf("Cannot start CoreAudio device\n");
                return false;
            } else {
                return true;
            }
        }
        
        virtual void stop()
        {
            fCrossFadeDevice.Stop();
        }
        
        virtual void launch_fadeOut()
        {
            fCrossFadeDevice.set_doWeFadeOut(true);
        }
        
        virtual void launch_fadeIn()
        {
            fCrossFadeDevice.set_doWeFadeIn(true);
        }
        
        virtual bool get_FadeOut()
        {
            return fCrossFadeDevice.get_doWeFadeOut();
        }
        
        virtual int get_buffer_size() { return fCrossFadeDevice.GetBufferSize(); }
        virtual int get_sample_rate() { return fCrossFadeDevice.GetSampleRate(); }
        
        void force_stopFade()
        {
            fCrossFadeDevice.reset_Values();
        }
    
};

#endif
