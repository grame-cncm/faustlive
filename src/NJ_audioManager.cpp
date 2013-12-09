//
//  NJ_audioManager.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// NJ_audioManager controls 2 NJ_audioFader. It can switch from one to another with a crossfade or it can act like a simple netjack-dsp

#include "NJ_audioManager.h"

#include "NJ_audioFader.h"

NJ_audioManager::NJ_audioManager(AudioSettings* as): AudioManager(as){
    
    fSettings = dynamic_cast<NJ_audioSettings*>(as);
    
//    printf("SETTINGS = %i || %s || %i || %i\n", fSettings->get_compressionValue(), fSettings->get_IP().c_str(), fSettings->get_Port(), fSettings->get_latency());
    
    fCurrentAudio = new NJ_audioFader(fSettings->get_compressionValue(), fSettings->get_IP(), fSettings->get_Port(), fSettings->get_latency());
    
    connect(fCurrentAudio, SIGNAL(error(const char*)), this, SLOT(send_Error(const char*)));
}

NJ_audioManager::~NJ_audioManager(){

    delete fCurrentAudio;
}

//INIT interface to correspond to JackAudio init interface
bool NJ_audioManager::initAudio(string& error, const char* name, dsp* DSP, const char* /*port_name*/){
    
    if(init(name, DSP))
        return true;
    else{
        error = "Impossible to init NetJackAudio";
        return false;
    }
}

//INIT/START/STOP on Current NetJackAudio
bool NJ_audioManager::init(const char* name, dsp* DSP){

    return fCurrentAudio->init(name, DSP);
}

bool NJ_audioManager::start(){
    return fCurrentAudio->start();
}

void NJ_audioManager::stop(){
    fCurrentAudio->stop();
}

//Init new audio, that will fade in current audio
bool NJ_audioManager::init_FadeAudio(string& error, const char* name, dsp* DSP){
    
    printf("fSettings = %p \n", fSettings);
    
    fFadeInAudio = new NJ_audioFader(fSettings->get_compressionValue(), fSettings->get_IP(), fSettings->get_Port(), fSettings->get_latency());
    
//    printf("INIT_FADEAUDIO THIS = %p \n",fFadeInAudio);
    
    connect(fFadeInAudio, SIGNAL(error(const char*)), this, SLOT(send_Error(const char*)));
    
    if(fFadeInAudio->init(name, DSP))
        return true;
    else{
        error = "Impossible to fade NetJack Client";
        return false;
    }
}

//Crossfade start
void NJ_audioManager::start_Fade(){
    
    fCurrentAudio->launch_fadeOut();
    fFadeInAudio->launch_fadeIn();
    
    fFadeInAudio->start();
}

//When the crossfade ends, FadeInAudio becomes the current audio 
void NJ_audioManager::wait_EndFade(){
 
    while(fCurrentAudio->get_FadeOut() == 1){}
    
    fCurrentAudio->stop();
    NJ_audioFader* intermediate = fCurrentAudio;
    fCurrentAudio = fFadeInAudio;
    fFadeInAudio = intermediate;
    delete fFadeInAudio;
}

//In case of Network failure, the application is notified
void NJ_audioManager::send_Error(const char* msg){
    
    emit errorSignal(msg);
}
