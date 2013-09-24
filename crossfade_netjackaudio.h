//
//  crossfade_netjackaudio.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _crossfade_netjackaudio_h
#define _crossfade_netjackaudio_h

#include <QObject>
#include "netjack-dsp.h"
#include "audioFader_Interface.h"
#include "audioFader_Implementation.h"

class crossfade_netjackaudio : public QObject, public netjackaudio, public audioFader_Interface, public audioFader_Implementation
{
    Q_OBJECT
        
    int     NumberOutput;           // DSP's number of outputs
    int     numberRestartAttempts;  // Number of attempts of reconnexion before considering that the connection is lost
    
    static int net_restart(void* arg);
    
    static int net_process(jack_nframes_t buffer_size,
                           int, float** audio_input_buffer, int, void**, int, float** audio_output_buffer, int, void**, void* arg);    
    public:
    
        crossfade_netjackaudio(int celt, const std::string master_ip, int master_port, int latency, QObject* parent);
    
        virtual ~crossfade_netjackaudio();
    
        virtual bool init(const char* name, dsp* DSP);
    
        virtual bool start();
        virtual void stop();
    
        virtual void launch_fadeIn();
        virtual void launch_fadeOut();
        virtual bool get_FadeOut();
    
    signals :
        void error(char*);
};

#endif
