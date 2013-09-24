//
//  crossfade_coreaudio.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "crossfade_coreaudio.h"
/******************************************************************************
 *******************************************************************************
 
 COREAUDIO INTERNAL INTERFACE
 
 *******************************************************************************
 *******************************************************************************/

OSStatus crossfade_TCoreAudioRenderer::Render(AudioUnitRenderActionFlags *ioActionFlags,
                                              const AudioTimeStamp *inTimeStamp,
                                              UInt32 inNumberFrames,
                                              AudioBufferList *ioData)
{
    
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

//******************************************************************************
// *******************************************************************************
// 
// CORE AUDIO INTERFACE
// 
// *******************************************************************************
// *******************************************************************************/


crossfade_coreaudio::crossfade_coreaudio(int srate, int fpb){
    
    fSampleRate = srate;
    fFramesPerBuf = fpb;
}

crossfade_coreaudio::crossfade_coreaudio(int fpb){ 
    
    fSampleRate = -1;
    fFramesPerBuf = fpb;
    //    printf("BUFFER SIZE = %i\n", fpb);
}

crossfade_coreaudio::~crossfade_coreaudio() {}

bool crossfade_coreaudio::init(const char* /*name*/, dsp* DSP) 
{
    if (fAudioDevice.OpenDefault(DSP, DSP->getNumInputs(), DSP->getNumOutputs(), fFramesPerBuf, fSampleRate) < 0) {
        printf("Cannot open CoreAudio device\n");
        return false;
    }
    // If -1 was given, fSampleRate will be changed by OpenDefault
    DSP->init(fSampleRate);
    return true;
}

bool crossfade_coreaudio::start() 
{
    if (fAudioDevice.Start() < 0) {
        printf("Cannot start CoreAudio device\n");
        return false;
    }
    return true;
}

void crossfade_coreaudio::stop() 
{
    fAudioDevice.Stop();
    fAudioDevice.Close();
}

void crossfade_coreaudio::launch_fadeOut(){
    fAudioDevice.set_doWeFadeOut(true);
}

void crossfade_coreaudio::launch_fadeIn(){
    fAudioDevice.set_doWeFadeIn(true);
}

bool crossfade_coreaudio::get_FadeOut(){
    //    printf("COREAUDIO fade out\n");
    return fAudioDevice.get_doWeFadeOut();
}
