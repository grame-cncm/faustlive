//
//  JA_audioFader.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class adds new features to jackaudio so that the dsp of the audioClient can be dynamically changed. MoreOver, the two dsp will be switched with a crossfade between them. 

#ifndef _JA_audioFader_h
#define _JA_audioFader_h

#include <string>
#include "faust/audio/jack-dsp.h"
#include "AudioFader_Interface.h"
#include "AudioFader_Implementation.h"

using namespace std;

class JA_audioFader : public jackaudio, public AudioFader_Interface, public AudioFader_Implementation
{    
    
        dsp*            fDspIn;
        int				fNumInDspFade;      // number of input channels of inFading DSP
        int				fNumOutDspFade;     // number of output channels of inFading DSP
    
        float**         fIntermediateFadeOut;
        float**         fIntermediateFadeIn;
    
//      string          fPortName;
        jack_nframes_t  fBufferSize;
    
        list<pair<string, string> > fConnectionsIn;		// Connections list
    
        float crossfade_calculation(int i, int j);
    
    public:
    
        JA_audioFader(const void* icon_data = NULL, size_t icon_size = 0);
    
        virtual ~JA_audioFader();
    
        virtual int	process(jack_nframes_t nframes);
    
        virtual bool init(const char* name, dsp* DSP); 
    
        virtual bool init(const char* name, dsp* DSP, const char* portsName);
        virtual bool start();
    
        virtual void init_FadeIn_Audio(dsp* DSP, const char* portsName);  
        
        int reconnect(list<pair<string, string> > Connections);
        
        list<pair<string, string> > get_audio_connections();
    
        virtual void launch_fadeOut();
        virtual void launch_fadeIn();
        virtual bool get_FadeOut();
    
        virtual void upDate_DSP();
};

#endif
