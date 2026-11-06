//
//  AudioFader_Implementation.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// 

#include "AudioFader_Implementation.h"
#include <stdio.h>

/******************************************************************************
 *******************************************************************************
 
 AUDIO FADER Implementation
 
 *******************************************************************************
 *******************************************************************************/

AudioFader_Implementation::AudioFader_Implementation()
{
    reset_Values();
}

AudioFader_Implementation::~AudioFader_Implementation() {}

void AudioFader_Implementation::set_doWeFadeIn(bool val)
{
    fDoWeFadeIn = val;
}

void AudioFader_Implementation::set_doWeFadeOut(bool val)
{
    fDoWeFadeOut = val;
}

bool AudioFader_Implementation::get_doWeFadeOut()
{
    return fDoWeFadeOut;
}

void AudioFader_Implementation::reset_Values()
{
    fNumberOfFadeProcess = 0;
    fInCoef = 1;
    fOutCoef = 1;
    fFadedOut = false;
    fDoWeFadeOut = false;
    fDoWeFadeIn = false;
}

void AudioFader_Implementation::stop_Fade()
{
    fDoWeFadeOut = false;
    fDoWeFadeIn = false;
}

// Used by the JACK fader (both DSP in the same client), once per audio callback.
// The fade ends when the coefficient reaches 0 (it used to end after 60 callbacks,
// so before 0 with buffers smaller than 512 frames, and the old DSP came back at full volume).
void AudioFader_Implementation::increment_crossFade()
{
    if (fOutCoef > 0) {
        fInCoef = fInCoef - kFadeCoefficient;  
        fOutCoef = fInCoef;
        fNumberOfFadeProcess++;
    } else {
        reset_Values();
    }
}

// Used by the faders where each DSP has its own audio stream (CoreAudio, PortAudio, NetJack)
void AudioFader_Implementation::crossfade_Calcul(int numFrames, int numOutputs, float** outBuffer)
{
    if (fFadedOut) {
        // The stream is about to be stopped by the GUI thread (wait_EndFade): stay silent
        for (int i = 0; i < numOutputs; i++) {
            for (int j = 0; j < numFrames; j++) {
                outBuffer[i][j] = 0.f;
            }
        }
    } else if (fDoWeFadeOut) {
        
        for (int j = 0; j < numFrames ; j++) {
            
            for (int i = 0; i < numOutputs; i++)
                outBuffer[i][j] = outBuffer[i][j] * fOutCoef;
            
            fOutCoef = (fOutCoef > kFadeCoefficient) ? (fOutCoef - kFadeCoefficient) : 0.f;
        }
        
        if (fOutCoef <= 0.f) {
            fFadedOut = true;
            fNumberOfFadeProcess = 0;
            fDoWeFadeOut = false;   // last: wait_EndFade then stops this stream
        } else {
            fNumberOfFadeProcess++;
        }
        
    } else if (fDoWeFadeIn) {
        
        for (int j = 0; j < numFrames ; j++) {
            
            for (int i = 0; i < numOutputs; i++)
                outBuffer[i][j] = outBuffer[i][j] * (1-fInCoef);
            
            fInCoef = (fInCoef > kFadeCoefficient) ? (fInCoef - kFadeCoefficient) : 0.f;
        }
        
        if (fInCoef <= 0.f) {
            fInCoef = 1;
            fNumberOfFadeProcess = 0;
            fDoWeFadeIn = false;    // full volume from now on
        } else {
            fNumberOfFadeProcess++;
        }
    }
}
