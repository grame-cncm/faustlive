//
//  audioFader_Implementation.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _audioFader_Implementation_h
#define _audioFader_Implementation_h

#define NumberOfCrossFadeProcess 30
#define FadeInCoefficient 1.000356
#define FadeOutCoefficient 1.000186

class audioFader_Implementation
{
    protected:
    
    bool    doWeFadeOut;
    bool    doWeFadeIn;
    
    float   InCoef;                 // Coefficients of multiplication   
    float   OutCoef;                // during audio crossfade
    int     NumberOfFadeProcess;    // Number of fade out process before considering that the signal is almost null
    
    void increment_crossFade();
    void reset_Values();
    void crossfade_Calcul(int numFrames, int numOutputs, float** outBuffer);
    
    public:
    
        audioFader_Implementation();
        ~audioFader_Implementation();
    
        void set_doWeFadeOut(bool val);
        void set_doWeFadeIn(bool val);
        bool get_doWeFadeOut();
};

#endif
