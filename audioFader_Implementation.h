//
//  AudioFader_Implementation.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// audioFader_Implementation contains the variables needed by the audio classes that want to implement a crossfade audio. 
// Some calculation functions are common to the architecture audio that use 2 audio clients for their crossfade (e.g : Netjack, Coreaudio)

#ifndef _AudioFader_Implementation_h
#define _AudioFader_Implementation_h

#define kNumberOfCrossFadeProcess 30
#define kFadeInCoefficient 1.000356
#define kFadeOutCoefficient 1.000186

class AudioFader_Implementation
{
    protected:
    
    bool    fDoWeFadeOut;
    bool    fDoWeFadeIn;
    
    float   fInCoef;                 // Coefficients of multiplication   
    float   fOutCoef;                // during audio crossfade
    int     fNumberOfFadeProcess;    // Number of fade out process before considering that the signal is almost null
    
    void    reset_Values();
    void    increment_crossFade();
    
    //Specific to the 2 clients crossfade
    void    crossfade_Calcul(int numFrames, int numOutputs, float** outBuffer);
    
    public:
    
        AudioFader_Implementation();
        ~AudioFader_Implementation();
    
        void set_doWeFadeOut(bool val);
        void set_doWeFadeIn(bool val);
        bool get_doWeFadeOut();
};

#endif
