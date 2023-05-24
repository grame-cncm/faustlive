//
//  JA_audioFader.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class adds new features to jackaudio so that the dsp of the audioClient can be dynamically changed. 
// Moreover, the two dsp will be switched with a crossfade between them. 

#ifndef _JA_audioFader_h
#define _JA_audioFader_h

#include <list>
#include <string>
#include "faust/audio/jack-dsp.h"
#include "AudioFader_Interface.h"
#include "AudioFader_Implementation.h"


class JA_audioFader : public jackaudio_midi, public AudioFader_Interface, public AudioFader_Implementation {    
    
    private:
    
        dsp* fDSPIn;
      
        float** fIntermediateFadeOut;
        float** fIntermediateFadeIn;
    
        std::list<std::pair<std::string, std::string> > fConnectionsIn;		// Connections list
        
        virtual void processAudio(jack_nframes_t nframes);
    
        float crossfade_calculation(int i, int j);
    
    public:
    
        JA_audioFader();
        virtual ~JA_audioFader();
        
        // Special version that names the JACK ports
        bool set_dsp(dsp* DSP, const char* portsName);
    
        virtual bool start();
    
        virtual void init_FadeIn_Audio(dsp* DSP, const char* portsName);  
        
        int reconnect(std::list<std::pair<std::string, std::string> > Connections);
        
        std::list<std::pair<std::string, std::string> > get_audio_connections(bool& saved);
    
        virtual void launch_fadeOut();
        virtual void launch_fadeIn();
        virtual bool get_FadeOut();
    
        virtual void upDate_DSP();
};

#endif
