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

//#define kNumberOfCrossFadeProcess 30
//#define kFadeInCoefficient 1.000356
//#define kFadeOutCoefficient 1.000186

#include <atomic>

// Gain step per sample: a crossfade lasts 1/kFadeCoefficient samples (about 0.7 s at 44.1 kHz),
// whatever the buffer size
#define kFadeCoefficient 0.00003255f
//#define kFadeOutCoefficient 0.000386

class AudioFader_Implementation
{
    protected:
    
        // Set by the GUI thread, cleared by the audio thread at the end of the fade.
        // Atomic (not volatile) so that the data written before the flag (JACK buffers, DSP) is visible to the other thread.
        std::atomic<bool> fDoWeFadeOut;
        std::atomic<bool> fDoWeFadeIn;
        
        // Only used by the audio thread (and before the stream starts)
        float   fInCoef;                 // Coefficients of multiplication   
        float   fOutCoef;                // during audio crossfade
        int     fNumberOfFadeProcess;    // Number of audio callbacks of the current fade
        bool    fFadedOut;               // The fade out is finished: silence until the stream is stopped
        
        void    increment_crossFade();
        
        //Specific to the 2 clients crossfade
        void    crossfade_Calcul(int numFrames, int numOutputs, float** outBuffer);
        
    public:
    
        AudioFader_Implementation();
        virtual ~AudioFader_Implementation();
    
        void set_doWeFadeOut(bool val);
        void set_doWeFadeIn(bool val);
        bool get_doWeFadeOut();
        // Not to be called while the audio thread runs (it writes the coefficients)
        void reset_Values();
        // Can be called from the GUI thread: the stream then plays without fade
        void stop_Fade();

};

#endif
