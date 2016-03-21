//
//  NJs_audioFader.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class re-implement the netjackaudio calcul process to be adaptated to FadeIn and FadeOut processes

#include "NJs_audioFader.h"

NJs_audioFader::NJs_audioFader(int celt, const std::string master_ip, int master_port, int mtu, int latency, QObject* parent)
: QObject(parent), netjackaudio(celt, master_ip, master_port, mtu, latency)
{
    reset_Values();
    fNumberRestartAttempts = 0;
}

NJs_audioFader::~NJs_audioFader(){}

//CallBack in case of network failure
int NJs_audioFader::restart_cb()
{
    // printf("Network failure, restart...\n");
    
    if (fNumberRestartAttempts < 10) {
        emit error("Network failure, restart...");    
        fNumberRestartAttempts++;
        return 0;
    } else {
        emit error("Impossible to restart Network");
        fNumberRestartAttempts = 0;
    }
    
    return 1;
}

//Reimplementing Audio callback to add the crossfade procedure
void NJs_audioFader::process(int count,  float** inputs, float** outputs)
{
     AVOIDDENORMALS;
     fDSP->compute(count, inputs, outputs);
     crossfade_Calcul(count, fDSP->getNumOutputs(), outputs);
}

bool NJs_audioFader::init(const char* name, dsp* DSP) 
{
    return init_aux(name, DSP, DSP->getNumInputs(), DSP->getNumOutputs(), 0, 0);
}

bool NJs_audioFader::init(const char* name, int numInputs, int numOutputs) 
{
    return init_aux(name, numInputs, numOutputs, 0, 0);
}

bool NJs_audioFader::set_dsp(dsp* DSP)
{
     netjackaudio::set_dsp(DSP); // SL le 30/06/15
    //set_dsp_aux(DSP);
    return true;
}

void NJs_audioFader::launch_fadeIn()
{
    set_doWeFadeIn(true);
}

void NJs_audioFader::launch_fadeOut()
{
    set_doWeFadeOut(true);
}

bool NJs_audioFader::get_FadeOut()
{
    return get_doWeFadeOut();
}

void NJs_audioFader::force_stopFade()
{
    reset_Values();
}
