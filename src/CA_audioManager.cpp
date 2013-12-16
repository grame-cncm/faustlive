//
//  CA_audioManager.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// CA_audioManager controls 2 CA_audioFader. It can switch from one to another with a crossfade or it can act like a simple coreaudio-dsp

#include "CA_audioSettings.h"
#include "CA_audioFader.h"
#include "CA_audioManager.h"

CA_audioManager::CA_audioManager(AudioSettings* as) : AudioManager(as){

    CA_audioSettings* settings = dynamic_cast<CA_audioSettings*>(as);
    
    fBufferSize = settings->get_BufferSize();
    
    fCurrentAudio = new CA_audioFader(fBufferSize);
    
}

CA_audioManager::~CA_audioManager(){

    delete fCurrentAudio;
}

//INIT interface to correspond to JackAudio init interface
bool CA_audioManager::initAudio(string& error, const char* name, dsp* DSP, const char* /*port_name*/){
    
    if(init(name, DSP))
        return true;
    else{
        error = "Impossible to init CoreAudio Client";
        return false;
    }
}

//INIT/START/STOP on Current CoreAudio
bool CA_audioManager::init(const char* name, dsp* DSP){

    return fCurrentAudio->init(name, DSP);

}

bool CA_audioManager::start(){
    return fCurrentAudio->start();
}

void CA_audioManager::stop(){
    fCurrentAudio->stop();
}

//Init new audio, that will fade in current audio
bool CA_audioManager::init_FadeAudio(string& error, const char* name, dsp* DSP){

    printf("CA_audioManager::init_FadeAudio\n");
    
    fFadeInAudio = new CA_audioFader(fBufferSize);
    
    if(fFadeInAudio->init(name, DSP))
        return true;
    else{
        error = "Impossible to init new Core Audio Client";
        return false;
    }
}

//Crossfade start
void CA_audioManager::start_Fade(){

    printf("CA_audioManager::start_FadeIn\n");
    
    fFadeInAudio->launch_fadeIn();
    
    printf("CA_audioManager::start_FadeOut\n");
    
    fCurrentAudio->launch_fadeOut();
    
    printf("CA_audioManager::start_FadeInAudio\n");
    
    fFadeInAudio->start();
}

//When the crossfade ends, FadeInAudio becomes the current audio 
void CA_audioManager::wait_EndFade(){

    int i=0;
    
    while(fCurrentAudio->get_FadeOut() == 1){ 
        
//   In case of CoreAudio Bug : If the Render function is not called, the loop could be infinite. This way, it isn't.
        if(i > 300) 
            break; 
        else 
            i++;
    }
    
    fCurrentAudio->stop();
    CA_audioFader* intermediate = fCurrentAudio;
    fCurrentAudio = fFadeInAudio;
    fFadeInAudio = intermediate;
    delete fFadeInAudio;
    
}

int CA_audioManager::get_buffer_size(){
    return fCurrentAudio->get_buffer_size();
}

int CA_audioManager::get_sample_rate(){
    return fCurrentAudio->get_sample_rate();
}


