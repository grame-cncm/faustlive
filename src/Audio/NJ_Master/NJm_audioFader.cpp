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
    
    // printf("Network failure, restart...\n");
    
    if(fNumberRestartAttempts < 9){
//        emit error("Network failure, restart...");    
        fNumberRestartAttempts++;
        return 0;
    }
    else{
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
    
    float* inputs_tmp[fDsp->getNumInputs()];
    float* outputs_tmp[fDsp->getNumOutputs()];
    
    for(int i = 0; i < fDsp->getNumInputs();i++) {
        inputs_tmp[i] = audio_inputs[i];
    }
    
    for(int i = 0; i < fDsp->getNumOutputs();i++) {
        outputs_tmp[i] = audio_outputs[i];
    }
    
    // Control buffer always use buffer_size, even if uncomplete data buffer (count < buffer_size) is received
    decode_midi_control(midi_inputs[0], fResult.buffer_size);
    
    // "count" may be less than buffer_size
    fDsp->compute(count, inputs_tmp, outputs_tmp);
    crossfade_Calcul(count, fDsp->getNumOutputs(), outputs_tmp);
    
    // Control buffer always use buffer_size, even if uncomplete data buffer (count < buffer_size) is received
    encode_midi_control(midi_outputs[0], fResult.buffer_size);
}

bool NJm_audioFader::init(const char* name, dsp* DSP) {
    
    fDsp = DSP;
    
    DSP->buildUserInterface(this);
    return init_aux(name, DSP, DSP->getNumInputs(), DSP->getNumOutputs(), 1, 1);
}

bool NJm_audioFader::init(const char* name, int numInputs, int numOutputs) {
    
    return init_aux(name, numInputs, numOutputs, 1, 1);
}

bool NJm_audioFader::set_dsp(dsp* DSP){
    
    fDsp = DSP;
    DSP->buildUserInterface(this);
    set_dsp_aux(DSP);
    return true;
}

void NJm_audioFader::launch_fadeIn(){
    set_doWeFadeIn(true);
}

void NJm_audioFader::launch_fadeOut(){
    set_doWeFadeOut(true);
}

bool NJm_audioFader::get_FadeOut(){
    return get_doWeFadeOut();
}

void NJm_audioFader::force_stopFade(){
    reset_Values();
}
