//
//  NJs_audioFader.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class re-implement the netjackaudio calcul process to be adaptated to FadeIn and FadeOut processes

#ifndef _NJs_audioFader_h
#define _NJs_audioFader_h

#include <QObject>
#include <string>
#include <iostream>
#include <sstream>
#include "faust/audio/netjack-dsp.h"
#include "AudioFader_Interface.h"
#include "AudioFader_Implementation.h"

class NJs_audioFader : public QObject, public netjackaudio, public AudioFader_Interface, public AudioFader_Implementation
{
    Q_OBJECT
        
    int     fNumberRestartAttempts;  // Number of attempts of reconnexion before considering that the connection is lost
    
    //static int net_restart(void* arg);
        
    virtual int restart_cb();
        
    virtual int set_sample_rate(jack_nframes_t nframes)
    {
        printf("New sample rate = %u\n", nframes);
        fDsp->init(nframes);
        return 0;
    }
    
    virtual void error_cb(int error_code)
    {
        std::stringstream err;
        err<<error_code;
        
        emit error(err.str().c_str());
    
    }
    
    void process(int count, float** inputs, float** outputs);
    
    public:
    
        NJs_audioFader(int celt, const std::string master_ip, int master_port, int mtu, int latency = 2, QObject* parent = NULL);
    
        virtual ~NJs_audioFader();
    
        virtual bool init(const char* name, dsp* DSP);
        bool         init(const char* name, int numInputs, int numOutputs);
        virtual bool set_dsp(dsp* DSP);
    
//        virtual bool start();
//        virtual void stop();
    
        virtual void launch_fadeIn();
        virtual void launch_fadeOut();
        virtual bool get_FadeOut();
    void        force_stopFade();
    
    signals :
        void error(const char*);
};

#endif
