//
//  NJ_audioManager.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "NJ_audioManager.h"

#include "NJ_audioFader.h"

NJ_audioManager::NJ_audioManager(AudioSettings* as): AudioManager(as){

    fSettings = dynamic_cast<NJ_audioSettings*>(as);
    
    fCurrentAudio = new NJ_audioFader(fSettings->get_compressionValue(), fSettings->get_IP(), fSettings->get_Port(), fSettings->get_latency());
    
    connect(fCurrentAudio, SIGNAL(error(const char*)), this, SLOT(send_Error(const char*)));
}

NJ_audioManager::~NJ_audioManager(){

    delete fCurrentAudio;
}

bool NJ_audioManager::init(const char* name, dsp* DSP){

    return fCurrentAudio->init(name, DSP);
}

bool NJ_audioManager::start(){
    return fCurrentAudio->start();
}

void NJ_audioManager::stop(){
    fCurrentAudio->stop();
}

bool NJ_audioManager::initAudio(char* error, const char* name, dsp* DSP, const char* port_name){
    
    if(init(name, DSP))
        return true;
    else{
        snprintf(error, 255,"%s","Impossible to init NetJackAudio");
        return false;
    }
}

bool NJ_audioManager::init_FadeAudio(char* error, const char* name, dsp* DSP){
    
    fFadeInAudio = new NJ_audioFader(fSettings->get_compressionValue(), fSettings->get_IP(), fSettings->get_Port(), fSettings->get_latency());
    
//    printf("INIT_FADEAUDIO THIS = %p \n",fFadeInAudio);
    
    connect(fFadeInAudio, SIGNAL(error(const char*)), this, SLOT(send_Error(const char*)));
    
    if(fFadeInAudio->init(name, DSP))
        return true;
    else{
        snprintf(error, 255, "Impossible to fade NetJack Client");
        return false;
    }
}

void NJ_audioManager::start_Fade(){
    
    fCurrentAudio->launch_fadeOut();
    fFadeInAudio->launch_fadeIn();
    
    fFadeInAudio->start();
}

void NJ_audioManager::wait_EndFade(){
 
    while(fCurrentAudio->get_FadeOut() == 1){}
    
    fCurrentAudio->stop();
    NJ_audioFader* intermediate = fCurrentAudio;
    fCurrentAudio = fFadeInAudio;
    fFadeInAudio = intermediate;
    delete fFadeInAudio;
}

void NJ_audioManager::send_Error(const char* msg){
    
    emit errorSignal(msg);
}
