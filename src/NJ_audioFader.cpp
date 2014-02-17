//
//  NJ_audioFader.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class re-implement the netjackaudio calcul process to be adaptated to FadeIn and FadeOut processes

#include "NJ_audioFader.h"

NJ_audioFader::NJ_audioFader(int celt, const std::string master_ip, int master_port, int mtu, int latency, QObject* parent)
: QObject(parent), netjackaudio(celt, master_ip, master_port, mtu, latency)
{
    reset_Values();
    fNumberRestartAttempts = 0;
}

NJ_audioFader::~NJ_audioFader(){}

//CallBack in case of network failure
int NJ_audioFader::restart_cb()
{
    // printf("Network failure, restart...\n");
    
    if(fNumberRestartAttempts < 10){
        emit error("Network failure, restart...");    
        fNumberRestartAttempts++;
        return 0;
    }
    else{
        emit error("Impossible to restart Network");
        fNumberRestartAttempts = 0;
    }
    
    return 1;
}

//Reimplementing Audio callback to add the crossfade procedure
void NJ_audioFader::process(int count,  float** inputs, float** outputs)
{
     AVOIDDENORMALS;
     fDsp->compute(count, inputs, outputs);
     crossfade_Calcul(count, fDsp->getNumOutputs(), outputs);
}

bool NJ_audioFader::init(const char* name, dsp* DSP) {
    
    return init_aux(name, DSP, DSP->getNumInputs(), DSP->getNumOutputs(), 0, 0);
}

bool NJ_audioFader::init(const char* name, int numInputs, int numOutputs) {
    
    return init_aux(name, numInputs, numOutputs, 0, 0);
}

bool NJ_audioFader::set_dsp(dsp* DSP){
    
    set_dsp_aux(DSP);
    return true;
}

bool NJ_audioFader::start() 
{
    if (jack_net_slave_activate(fNet)) {
        printf("cannot activate net");
        return false;
    }
    return true;
}

void NJ_audioFader::stop() 
{
    jack_net_slave_deactivate(fNet);
    jack_net_slave_close(fNet);
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

void NJ_audioFader::force_stopFade(){
    reset_Values();
}
