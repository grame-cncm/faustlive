//
//  NJ_audioFader.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class re-implement the netjackaudio calcul process to be adaptated to FadeIn and FadeOut processes

#ifndef _NJ_audioFader_h
#define _NJ_audioFader_h

#include <QObject>
#include "faust/audio/netjack-dsp.h"
#include "AudioFader_Interface.h"
#include "AudioFader_Implementation.h"

class NJ_audioFader : public QObject, public netjackaudio, public AudioFader_Interface, public AudioFader_Implementation
{
    Q_OBJECT
        
    int     fNumberOutput;           // DSP's number of outputs
    int     fNumberRestartAttempts;  // Number of attempts of reconnexion before considering that the connection is lost
    
    static int net_restart(void* arg);
    
    static int net_process(jack_nframes_t buffer_size,
                           int, float** audio_input_buffer, int, void**, int, float** audio_output_buffer, int, void**, void* arg);    
    public:
    
        NJ_audioFader(int celt, const std::string master_ip, int master_port, int latency = 2, QObject* parent = NULL);
    
        virtual ~NJ_audioFader();
    
        virtual bool init(const char* name, dsp* DSP);
    
        virtual bool start();
        virtual void stop();
    
        virtual void launch_fadeIn();
        virtual void launch_fadeOut();
        virtual bool get_FadeOut();
//        void         set_NumOutput(int num);
    
    signals :
        void error(const char*);
};

#endif
