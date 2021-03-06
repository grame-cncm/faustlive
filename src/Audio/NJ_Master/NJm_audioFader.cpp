//
//  NJm_audioFader.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class re-implement the netjackaudio calcul process to be adaptated to FadeIn and FadeOut processes

#include "NJm_audioFader.h"

NJm_audioFader::NJm_audioFader(int celt, const std::string master_ip, int master_port, int mtu, int latency, QObject* parent)
: QObject(parent), netjackaudio_midicontrol(celt, master_ip, master_port, mtu, latency)
{
    reset_Values();
    fNumberRestartAttempts = 0;
}

NJm_audioFader::~NJm_audioFader(){}

//CallBack in case of network failure
int NJm_audioFader::restart_cb()
{
    printf("NJm_audioFader::RESTART = %i\n", fNumberRestartAttempts);
    
    if (fNumberRestartAttempts < 9) {
        // emit error("Network failure, restart...");    
        fNumberRestartAttempts++;
        return 0;
    } else {
        printf("IMPOSSIBLE TO RESTART = %p\n", this);
        emit errorPRINT("Impossible to restart Network");
        fNumberRestartAttempts = 0;
    }
    
    return 1;
}

//Reimplementing Audio callback to add the crossfade procedure
void NJm_audioFader::process(int count, float** audio_inputs, float** audio_outputs, void** midi_inputs, void** midi_outputs)
{
    AVOIDDENORMALS;
    
    float** inputs_tmp = (float**)alloca(fDSP->getNumInputs()*sizeof(float*));
    float** outputs_tmp = (float**)alloca(fDSP->getNumOutputs()*sizeof(float*));
    
    for(int i = 0; i < fDSP->getNumInputs();i++) {
        inputs_tmp[i] = audio_inputs[i];
    }
    
    for(int i = 0; i < fDSP->getNumOutputs();i++) {
        outputs_tmp[i] = audio_outputs[i];
    }
    
    // Control buffer always use buffer_size, even if uncomplete data buffer (count < buffer_size) is received
    decodeMidiControl(midi_inputs[0], fResult.buffer_size);
    
    // "count" may be less than buffer_size
    fDSP->compute(count, inputs_tmp, outputs_tmp);
    crossfade_Calcul(count, fDSP->getNumOutputs(), outputs_tmp);
    
    // Control buffer always use buffer_size, even if uncomplete data buffer (count < buffer_size) is received
    encodeMidiControl(midi_outputs[0], fResult.buffer_size);
}

bool NJm_audioFader::init(const char* name, dsp* dsp) 
{
    fDSP = dsp;
    fDSP->buildUserInterface(this);
    return initAux(name, fDSP, fDSP->getNumInputs(), fDSP->getNumOutputs(), 1, 1);
}

bool NJm_audioFader::init(const char* name, int numInputs, int numOutputs) 
{
     return initAux(name, numInputs, numOutputs, 1, 1);
}

bool NJm_audioFader::set_dsp(dsp* dsp)
{
    fDSP = dsp;
    fDSP->buildUserInterface(this);
    netjackaudio::setDsp(fDSP); // SL le 30/06/15
    //set_dsp_aux(DSP);
    return true;
}

void NJm_audioFader::launch_fadeIn()
{
    set_doWeFadeIn(true);
}

void NJm_audioFader::launch_fadeOut()
{
    set_doWeFadeOut(true);
}

bool NJm_audioFader::get_FadeOut()
{
    return get_doWeFadeOut();
}

void NJm_audioFader::force_stopFade()
{
    reset_Values();
}
