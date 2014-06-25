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

AudioFader_Implementation::AudioFader_Implementation(){
    reset_Values();
}

AudioFader_Implementation::~AudioFader_Implementation(){}

void AudioFader_Implementation::set_doWeFadeIn(bool val){
    fDoWeFadeIn = val;
}

void AudioFader_Implementation::set_doWeFadeOut(bool val){
    
    fDoWeFadeOut = val;
}

bool AudioFader_Implementation::get_doWeFadeOut(){
    return fDoWeFadeOut;
}

void AudioFader_Implementation::reset_Values(){
    fNumberOfFadeProcess = 0;
    fInCoef = 0.01;
    fOutCoef = 1;
    fDoWeFadeOut = false;
    fDoWeFadeIn = false;
}

void AudioFader_Implementation::increment_crossFade(){
    
    if(fNumberOfFadeProcess != kNumberOfCrossFadeProcess){
        if(fInCoef < 1)
            fInCoef = fInCoef*kFadeInCoefficient;  
        
        fOutCoef = fOutCoef*kFadeOutCoefficient;
        fNumberOfFadeProcess++;
    }
    else{
        reset_Values();
    }
}

void AudioFader_Implementation::crossfade_Calcul(int numFrames, int numOutputs, float** outBuffer){
    
    if(fDoWeFadeOut){
//        printf("Fade OUt");
        for(int j = 0; j < numFrames ; j++){
            for(int i = 0; i < numOutputs; i++){
                outBuffer[i][j] = outBuffer[i][j] /fOutCoef;
            }
            
            fOutCoef = fOutCoef*kFadeOutCoefficient; 
        }
    }
    else if(fDoWeFadeIn){
//        printf("Fade In");   
        for(int j = 0; j < numFrames ; j++){
            
            for(int i = 0; i < numOutputs; i++){
                outBuffer[i][j] = outBuffer[i][j] * fInCoef;
            } 
            if(fInCoef < 1)   
                fInCoef = fInCoef*kFadeInCoefficient;      
        }  
    }
    
    if(fDoWeFadeIn || fDoWeFadeOut)
        increment_crossFade();
}
