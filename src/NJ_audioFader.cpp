//
//  NJ_audioFader.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class re-implement the netjackaudio calcul process to be adaptated to FadeIn and FadeOut processes

#include "NJ_audioFader.h"

NJ_audioFader::NJ_audioFader(int celt, const std::string master_ip, int master_port, int latency, QObject* parent)
: QObject(parent), netjackaudio(celt, master_ip, master_port, latency)
{
    fCelt = celt;
    fMasterIP = master_ip;
    fMasterPort = master_port;
    fLatency = latency;
    
    reset_Values();
    fNumberRestartAttempts = 0;
}

NJ_audioFader::~NJ_audioFader(){}

//CallBack in case of network failure
int NJ_audioFader::net_restart(void* arg) 
{
    AVOIDDENORMALS;
    NJ_audioFader* obj = (NJ_audioFader*)arg;
    
    //        printf("Network failure, restart...\n");
    
    if(obj->fNumberRestartAttempts < 10){
        obj->emit error("Network failure, restart...");    
        obj->fNumberRestartAttempts++;
        return 0;
    }
    else{
        obj->emit error("Impossible to restart Network");
        obj->fNumberRestartAttempts = 0;
    }
    
    return 1;
}

//Reimplementing Audio callback to add the crossfade procedure
int NJ_audioFader::net_process(jack_nframes_t buffer_size,
                                        int,
                                        float** audio_input_buffer,
                                        int,
                                        void**,
                                        int,
                                        float** audio_output_buffer,
                                        int,
                                        void**,
                                        void* arg) {
    AVOIDDENORMALS;
    NJ_audioFader* obj = (NJ_audioFader*)arg;
    printf("OBJ = %p\n", obj);
    obj->fDsp->compute(buffer_size, audio_input_buffer, audio_output_buffer);
    
    obj->crossfade_Calcul(buffer_size, obj->fNumberOutput, audio_output_buffer);
    return 0;
}

bool NJ_audioFader::init(const char* name, dsp* DSP) {
    
    fDsp = DSP;
    
    fNumberOutput = DSP->getNumOutputs();
    
    jack_slave_t request = {
        DSP->getNumInputs(),
        DSP->getNumOutputs(),
        0, 0,
        DEFAULT_MTU,
        2,
        (fCelt > 0) ? JackCeltEncoder : JackFloatEncoder,
        (fCelt > 0) ? fCelt : 0,
        fLatency
    };
    
    if ((fNet = jack_net_slave_open(fMasterIP.c_str(), fMasterPort, name, &request, &fResult)) == 0) {
        printf("jack remote server not running ?\n");
        return false;
    }
    
    
    printf("BUFFER SIZE = %i\n", fResult.buffer_size);
    
    jack_set_net_slave_process_callback(fNet, net_process, this);
#ifdef RESTART_CB_API
    jack_set_net_slave_restart_callback(fNet, net_restart, this);
#else
    jack_set_net_slave_shutdown_callback(fNet, net_shutdown, this);
#endif
    jack_set_net_slave_sample_rate_callback(fNet, net_sample_rate, this);
    
    fDsp->init(fResult.sample_rate);
    return true;
}

bool NJ_audioFader::start() {
    if (jack_net_slave_activate(fNet)) {
        printf("cannot activate net");
        return false;
    }
    return true;
}

void NJ_audioFader::stop() {
    jack_net_slave_deactivate(fNet);
//    printf("NET DEACTiVATE\n");
    jack_net_slave_close(fNet);
//    printf("NET CLOSE = %p\n", this);
}

void NJ_audioFader::launch_fadeIn(){
    set_doWeFadeIn(true);
}

void NJ_audioFader::launch_fadeOut(){
    set_doWeFadeOut(true);
}

bool NJ_audioFader::get_FadeOut(){
    return get_doWeFadeOut();
}
