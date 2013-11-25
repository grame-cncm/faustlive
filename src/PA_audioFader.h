//
//  PA_audioFader.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class adds new features to jackaudio so that the dsp of the audioClient can be dynamically changed. MoreOver, the two dsp will be switched with a crossfade between them. 

#ifndef _PA_audioFader_h
#define _PA_audioFader_h

#include <string>
#include "faust/audio/portaudio-dsp.h"
#include "AudioFader_Interface.h"
#include "AudioFader_Implementation.h"

using namespace std;

class PA_audioFader : public portaudio, public AudioFader_Interface, public AudioFader_Implementation
{    
    
    public:
    
        PA_audioFader(long srate, long bsize);
    
        virtual ~PA_audioFader();
    
        virtual bool init(const char* name, dsp* DSP); 
    
        virtual bool init(const char* name, dsp* DSP, const char* portsName);
        virtual bool start();
        virtual void stop();
    
        virtual int processAudio(float** ibuf, float** obuf, unsigned long frames);
    
        virtual void launch_fadeOut();
        virtual void launch_fadeIn();
        virtual bool get_FadeOut();
    
        virtual void upDate_DSP();

};

#endif
