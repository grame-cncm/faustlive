//
//  NJm_audioFader.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class re-implement the netjackaudio calcul process to be adaptated to FadeIn and FadeOut processes

#ifndef _NJm_audioFader_h
#define _NJm_audioFader_h

#include <QObject>
#include <string>
#include <iostream>
#include <sstream>
#include "faust/audio/netjack-dsp.h"
#include "AudioFader_Interface.h"
#include "AudioFader_Implementation.h"

class NJm_audioFader : public QObject, public netjackaudio_midicontrol, public AudioFader_Interface, public AudioFader_Implementation
{

    private: 
    
        Q_OBJECT
            
        int fNumberRestartAttempts;  // Number of attempts of reconnexion before considering that the connection is lost
        
        //static int net_restart(void* arg);
            
        virtual int restart_cb();
            
        virtual int set_sample_rate(jack_nframes_t nframes)
        {
    //        printf("New sample rate = %u\n", nframes);
            fDsp->init(nframes);
            return 0;
        }
        
        virtual void error_cb(int error_code)
        {
            switch (error_code) {
                    
                case SOCKET_ERROR:
                    printf("NetJack : SOCKET_ERROR\n");
                    break;
                    
                case SYNC_PACKET_ERROR:
                    printf("NetJack : SYNC_PACKET_ERROR\n");
                    break;
                    
                case DATA_PACKET_ERROR:
                    printf("NetJack : DATA_PACKET_ERROR\n");
                    break;
            }
            
            std::stringstream err;
            err<<error_code;
            
            emit errorPRINT(err.str().c_str());
        
        }
        
        virtual void process(int count, float** audio_inputs, float** audio_outputs, void** midi_inputs, void** midi_outputs);
    
    public:
    
        NJm_audioFader(int celt, const std::string master_ip, int master_port, int mtu, int latency = 2, QObject* parent = NULL);
    
        virtual ~NJm_audioFader();
    
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
        void errorPRINT(const char*);
};

#endif
