//
//  JA_audioFader.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _JA_audioFader_h
#define _JA_audioFader_h

#include <string>
#include "faust/audio/jack-dsp.h"
#include "audioFader_Interface.h"
#include "audioFader_Implementation.h"

using namespace std;

class JA_audioFader : public jackaudio, public audioFader_Interface, public audioFader_Implementation
{    
    
    dsp*            fDspIn;
    int				fNumInDspFade;      // number of input channels of inFading DSP
    int				fNumOutDspFade;     // number of output channels of inFading DSP
    
    float**         fIntermediateFadeOut;
    float**         fIntermediateFadeIn;
    
    string          portName;
    jack_nframes_t  bufferSize;
    
    list<pair<string, string> > connectionsIn;		// Connections list
    
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
