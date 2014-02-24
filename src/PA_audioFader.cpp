//
//  PA_audioFader.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class adds new features to jackaudio so that the dsp of the audioClient can be dynamically changed. MoreOver, the two dsp will be switched with a crossfade between them. 

#include "PA_audioFader.h"

PA_audioFader::PA_audioFader(long srate, long bsize) : portaudio(srate, bsize){}

PA_audioFader::~PA_audioFader(){   
    stop(); 
    Pa_Terminate();
}

bool PA_audioFader::init(const char* name, dsp* DSP){

    fDsp = DSP;
    if (pa_error(Pa_Initialize())) {
        return false;
    }
    
    const PaDeviceInfo*	idev = Pa_GetDeviceInfo(Pa_GetDefaultInputDevice());
    const PaDeviceInfo*	odev = Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice());
    
    if (fDsp->getNumInputs() > idev->maxInputChannels || fDsp->getNumOutputs() > odev->maxOutputChannels) {
        printf("DSP has %d inputs and %d outputs, physical inputs = %d physical outputs = %d \n", 
               fDsp->getNumInputs(), fDsp->getNumOutputs(), 
               idev->maxInputChannels, odev->maxOutputChannels);
        return false;
    }
    
    fDevNumInChans = idev->maxInputChannels;
    fDevNumOutChans = odev->maxOutputChannels;
    
    fInputParameters.device = Pa_GetDefaultInputDevice();
    fInputParameters.sampleFormat = paFloat32 | paNonInterleaved;;
    fInputParameters.channelCount = fDevNumInChans;
    fInputParameters.hostApiSpecificStreamInfo = 0;
    
    fOutputParameters.device = Pa_GetDefaultOutputDevice();
    fOutputParameters.sampleFormat = paFloat32 | paNonInterleaved;;
    fOutputParameters.channelCount = fDevNumOutChans;
    fOutputParameters.hostApiSpecificStreamInfo = 0;
    
    PaError err;
    if ((err = Pa_IsFormatSupported(
                                    ((fDevNumInChans > 0) ? &fInputParameters : 0),
                                    ((fDevNumOutChans > 0) ? &fOutputParameters : 0), fSampleRate)) != 0) {
        printf("stream format is not supported err = %d\n", err);
        return false;
    }
    
    fDsp->init(fSampleRate);
    return true;


} 

// INIT/START of Jack audio Client
bool PA_audioFader::init(const char* name, dsp* DSP, const char* /*portsName*/) 
{
    return init(name, DSP);
}

//bool PA_audioFader::init(const char* name){
//    
//}
//
//bool PA_audioFader::set_dsp(dsp* DSP, const char* /*portsName*/){
//    
//}

bool PA_audioFader::start(){

    if (pa_error(Pa_OpenStream(&fAudioStream, &fInputParameters, &fOutputParameters, fSampleRate, fBufferSize, paNoFlag, audioCallback, this))) {
        return false;
    }      
    
    if (pa_error(Pa_StartStream(fAudioStream))) {
        return false;
    }
    return true;
}

void PA_audioFader::stop() 
{
    if (fAudioStream) {
        Pa_StopStream(fAudioStream);
        Pa_CloseStream(fAudioStream);
        fAudioStream = 0;
    }
}

int PA_audioFader::processAudio(float** ibuf, float** obuf, unsigned long frames) 
{
    // process samples
    fDsp->compute(frames, ibuf, obuf);
    crossfade_Calcul(fBufferSize, fDevNumOutChans, obuf);
    return paContinue;
}

// UpDate the list of ports needed by new DSP
void PA_audioFader::launch_fadeOut(){
    set_doWeFadeOut(true);
}

//Fade In is not needed, because the fade in and out are both launched in the same process
void PA_audioFader::launch_fadeIn(){
    set_doWeFadeIn(true);
}

bool PA_audioFader::get_FadeOut(){
    return get_doWeFadeOut();
}

// The inFading DSP becomes the current one
void PA_audioFader::upDate_DSP(){}
