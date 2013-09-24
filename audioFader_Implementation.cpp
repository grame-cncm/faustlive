//
//  audioFader.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "audioFader_Implementation.h"

/******************************************************************************
 *******************************************************************************
 
 AUDIO FADER Implementation
 
 *******************************************************************************
 *******************************************************************************/

audioFader_Implementation::audioFader_Implementation(){
    reset_Values();
}

audioFader_Implementation::~audioFader_Implementation(){}

void audioFader_Implementation::set_doWeFadeIn(bool val){
    doWeFadeIn = val;
}

void audioFader_Implementation::set_doWeFadeOut(bool val){
    doWeFadeOut = val;
}

bool audioFader_Implementation::get_doWeFadeOut(){
    return doWeFadeOut;
}

void audioFader_Implementation::reset_Values(){
    NumberOfFadeProcess = 0;
    InCoef = 0.01;
    OutCoef = 1;
    doWeFadeOut = false;
    doWeFadeIn = false;
}

void audioFader_Implementation::increment_crossFade(){
    
    if(NumberOfFadeProcess != NumberOfCrossFadeProcess){
        if(InCoef < 1)
            InCoef = InCoef*FadeInCoefficient;  
        
        OutCoef = OutCoef*FadeOutCoefficient;
        NumberOfFadeProcess++;
    }
    else{
        reset_Values();
    }
}

void audioFader_Implementation::crossfade_Calcul(int numFrames, int numOutputs, float** outBuffer){
    
    if(doWeFadeOut){
//        printf("Fade OUt");
        for(int j = 0; j < numFrames ; j++){
            for(int i = 0; i < numOutputs; i++){
                outBuffer[i][j] = outBuffer[i][j] /OutCoef;
            }
            
            OutCoef = OutCoef*FadeOutCoefficient; 
        }
    }
    else if(doWeFadeIn){
//        printf("Fade In");   
        for(int j = 0; j < numFrames ; j++){
            
            for(int i = 0; i < numOutputs; i++){
                outBuffer[i][j] = outBuffer[i][j] * InCoef;
            } 
            if(InCoef < 1)   
                InCoef = InCoef*FadeInCoefficient;      
        }  
    }
    
    increment_crossFade();
}
