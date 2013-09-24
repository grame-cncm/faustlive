//
//  crossfade_netjackaudio.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "crossfade_netjackaudio.h"

crossfade_netjackaudio::crossfade_netjackaudio(int celt, const std::string master_ip, int master_port, int latency = 2, QObject* parent = NULL)
: QObject(parent), netjackaudio(celt, master_ip, master_port, latency)
{
    fCelt = celt;
    fMasterIP = master_ip;
    fMasterPort = master_port;
    fLatency = latency;
    
    doWeFadeOut = false;
    doWeFadeIn = false;
    InCoef = 0.01;
    OutCoef = 1;
    NumberOfFadeProcess = 0;
    numberRestartAttempts = 0;
}

crossfade_netjackaudio::~crossfade_netjackaudio(){}

int crossfade_netjackaudio::net_restart(void* arg) 
{
    AVOIDDENORMALS;
    crossfade_netjackaudio* obj = (crossfade_netjackaudio*)arg;
    
    //        printf("Network failure, restart...\n");
    
    if(obj->numberRestartAttempts < 10){
        obj->emit error("Network failure, restart...");    
        obj->numberRestartAttempts++;
        return 0;
    }
    else{
        obj->emit error("Impossible to restart Network");
        obj->numberRestartAttempts = 0;
    }
    
    return 1;
}

int crossfade_netjackaudio::net_process(jack_nframes_t buffer_size,
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
    crossfade_netjackaudio* obj = (crossfade_netjackaudio*)arg;
    obj->fDsp->compute(buffer_size, audio_input_buffer, audio_output_buffer);
    
    obj->crossfade_Calcul(buffer_size, obj->NumberOutput, audio_output_buffer);
    return 0;
}

bool crossfade_netjackaudio::init(const char* name, dsp* DSP) {
    fDsp = DSP;
    
    NumberOutput = DSP->getNumOutputs();
    
    
    //Potentiellement il va falloir modifier la valeur du time out pour être sur qu'on 
    jack_slave_t request = {
        DSP->getNumInputs(),
        DSP->getNumOutputs(),
        0, 0,
        DEFAULT_MTU,
        5,
        (fCelt > 0) ? JackCeltEncoder : JackFloatEncoder,
        (fCelt > 0) ? fCelt : 0,
        fLatency
    };
    
    jack_master_t result;
    
    printf("COMPR = %i // MASTER IP = %s // PORT = %i // LAT = %i\n", fCelt,fMasterIP.c_str(), fMasterPort, fLatency);
    
    //        fMasterIP = "127.0.0.1";
    
    if ((fNet = jack_net_slave_open(fMasterIP.c_str(), fMasterPort, name, &request, &result)) == 0) {
        printf("jack remote server not running ?\n");
        return false;
    }
    
    jack_set_net_slave_process_callback(fNet, net_process, this);
    jack_set_net_slave_restart_callback(fNet, net_restart, this);
    
    fDsp->init(result.sample_rate);
    return true;
}

bool crossfade_netjackaudio::start() {
    if (jack_net_slave_activate(fNet)) {
        printf("cannot activate net");
        return false;
    }
    return true;
}

void crossfade_netjackaudio::stop() {
    jack_net_slave_deactivate(fNet);
    printf("NET DEACTiVATE\n");
    jack_net_slave_close(fNet);
    printf("NET CLOSE\n");
}

void crossfade_netjackaudio::launch_fadeIn(){
    set_doWeFadeIn(true);
}

void crossfade_netjackaudio::launch_fadeOut(){
    set_doWeFadeOut(true);
}

bool crossfade_netjackaudio::get_FadeOut(){
    return get_doWeFadeOut();
}
