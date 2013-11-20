//
//  AL_audioManager.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// AL_audioManager controls 2 AL_audioFader. It can switch from one to another with a crossfade or it can act like a simple coreaudio-dsp

#include <sstream>
#include "AL_audioSettings.h"
#include "AL_audioFader.h"
#include "AL_audioManager.h"

AL_audioManager::AL_audioManager(AudioSettings* as) : AudioManager(as){

    fSettings = dynamic_cast<AL_audioSettings*>(as);
}

AL_audioManager::~AL_audioManager(){

    delete fCurrentAudio;
}

//INIT interface to correspond to JackAudio init interface
bool AL_audioManager::initAudio(string& error, const char* name, dsp* DSP, const char* /*port_name*/){
    
	char* argv[8];
    
	stringstream f, bs, p;
	f<<fSettings->get_Frequency();
	bs<<fSettings->get_BufferSize();
	p<<fSettings->get_Period();


	printf("DEVICE = %s || FREQ = %s || Buff = %s || Period = %s\n",fSettings->get_DeviceName().c_str(), (char*)(f.str().c_str()), (char*)(bs.str().c_str()), (char*)(p.str().c_str()));


    argv[0] = "-d";
    argv[1] = (char*)(fSettings->get_DeviceName().c_str());
    argv[2] = "-f";
    argv[3] = (char*)(f.str().c_str());
    argv[4] = "-b";
    argv[5] = (char*)(bs.str().c_str());
    argv[6] = "-p";
    argv[7] = "2";

//    argv[0] = "-d";
//    argv[1] = "hw:0";
//    argv[2] = "-f";
//    argv[3] = "44100";
//    argv[4] = "-b";
//    argv[5] = "512";
//    argv[6] = "-p";
//    argv[7] = "2";

    fCurrentAudio = new AL_audioFader(8, argv, DSP);


    if(init(name, DSP))
        return true;
    else{
        error = "Impossible to init Alsa Client";
        return false;
    }
}

//INIT/START/STOP on Current CoreAudio
bool AL_audioManager::init(const char* name, dsp* DSP){

    return fCurrentAudio->init(name, DSP);

}

bool AL_audioManager::start(){
    return fCurrentAudio->start();
}

void AL_audioManager::stop(){
    fCurrentAudio->stop();
}

//Init new audio, that will fade in current audio
bool AL_audioManager::init_FadeAudio(string& error, const char* name, dsp* DSP){

    printf("AL_audioManager::init_FadeAudio\n");
    
	 char* argv[8];
    
	stringstream f, bs, p;
	f<<fSettings->get_Frequency();
	bs<<fSettings->get_BufferSize();
	p<<fSettings->get_Period();

    argv[0] = "-d";
    argv[1] = (char*)(fSettings->get_DeviceName().c_str());
    argv[2] = "-f";
    argv[3] = (char*)(f.str().c_str());
    argv[4] = "-b";
    argv[5] = (char*)(bs.str().c_str());
    argv[6] = "-p";
    argv[7] = (char*)(p.str().c_str());

    fFadeInAudio = new AL_audioFader(8, argv, DSP);
    
    if(fFadeInAudio->init(name, DSP))
        return true;
    else{
        error = "Impossible to init new Alsa Client";
        return false;
    }
}

//Crossfade start
void AL_audioManager::start_Fade(){

    printf("AL_audioManager::start_FadeIn\n");
    
    fFadeInAudio->launch_fadeIn();
    
    printf("AL_audioManager::start_FadeOut\n");
    
    fCurrentAudio->launch_fadeOut();
    
    printf("AL_audioManager::start_FadeInAudio\n");
    
    fFadeInAudio->start();
}

//When the crossfade ends, FadeInAudio becomes the current audio 
void AL_audioManager::wait_EndFade(){

    while(fCurrentAudio->get_FadeOut() == 1){}
    
    fCurrentAudio->stop();
    AL_audioFader* intermediate = fCurrentAudio;
    fCurrentAudio = fFadeInAudio;
    fFadeInAudio = intermediate;
    delete fFadeInAudio;
    
}
