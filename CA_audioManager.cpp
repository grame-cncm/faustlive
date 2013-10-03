//
//  CA_audioManager.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "CA_audioSettings.h"
#include "CA_audioFader.h"
#include "CA_audioManager.h"

CA_audioManager::CA_audioManager(AudioSettings* as) : AudioManager(as){

    fSettings = dynamic_cast<CA_audioSettings*>(as);
    
    fCurrentAudio = new CA_audioFader(fSettings->get_BufferSize());
    
}

CA_audioManager::~CA_audioManager(){

    delete fCurrentAudio;
}

bool CA_audioManager::init(const char* name, dsp* DSP){

    return fCurrentAudio->init(name, DSP);

}

bool CA_audioManager::start(){
    return fCurrentAudio->start();
}

void CA_audioManager::stop(){
    fCurrentAudio->stop();
}

bool CA_audioManager::initAudio(char* error, const char* name, dsp* DSP, const char* port_name){
    
    if(init(name, DSP))
        return true;
    else{
        snprintf(error, 255, "Impossible to init CoreAudio Client");
        return false;
    }
}

bool CA_audioManager::init_FadeAudio(char* error, const char* name, dsp* DSP){

    fFadeInAudio = new CA_audioFader(fSettings->get_BufferSize());
    
    if(fFadeInAudio->init(name, DSP))
        return true;
    else{
        snprintf(error, 255, "Impossible to init new Core Audio Client");
        return false;
    }
}

void CA_audioManager::start_Fade(){

    fFadeInAudio->launch_fadeIn();
    fCurrentAudio->launch_fadeOut();
    
    fFadeInAudio->start();
}

void CA_audioManager::wait_EndFade(){

    while(fCurrentAudio->get_FadeOut() == 1){}
    
    fCurrentAudio->stop();
    CA_audioFader* intermediate = fCurrentAudio;
    fCurrentAudio = fFadeInAudio;
    fFadeInAudio = intermediate;
    delete fFadeInAudio;
    
}
