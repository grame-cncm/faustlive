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

#include "FLSettings.h"

NJ_audioManager::NJ_audioManager(AudioShutdownCallback cb, void* arg): AudioManager(cb, arg){
    
    FLSettings* settings = FLSettings::_Instance();
    
    fCV = settings->value("General/Audio/NetJack/CV", -1).toInt();
    fIP = settings->value("General/Audio/NetJack/IP", "225.3.19.154").toString();
    fPort = settings->value("General/Audio/NetJack/Port", 19000).toInt();
    fLatency = settings->value("General/Audio/NetJack/Latency", 2).toInt();
    fMTU = settings->value("General/Audio/NetJack/MTU", 1500).toInt();
    
    fCurrentAudio = new NJ_audioFader(fCV, fIP.toStdString(), fPort, fMTU, fLatency);
    
    connect(fCurrentAudio, SIGNAL(error(const char*)), this, SLOT(send_Error(const char*)));
    
    fInit = false; //Indicator of which init has been used
}

NJ_audioManager::~NJ_audioManager(){

    delete fCurrentAudio;
}

//INIT interface to correspond to JackAudio init interface
bool NJ_audioManager::initAudio(QString& /*error*/, const char* /*name*/){

    fInit = false;
//    fName = name;
    return true;
}

bool NJ_audioManager::initAudio(QString& error, const char* /*name*/, const char* port_name, int numInputs, int numOutputs){
    
    if(fCurrentAudio->init(port_name, numInputs, numOutputs)){
        fInit = true;
        return true;
    }
    else{
        error = "Impossible to init NetJackAudio";
        return false;
    }
}

bool NJ_audioManager::setDSP(QString& error, dsp* DSP, const char* port_name){
    
    if(fInit)
        return fCurrentAudio->set_dsp(DSP);
    
    else if(fCurrentAudio->init(port_name, DSP))
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
bool NJ_audioManager::init_FadeAudio(QString& error, const char* name, dsp* DSP){
    
    fFadeInAudio = new NJ_audioFader(fCV, fIP.toStdString(), fPort, fMTU, fLatency);
    
    connect(fFadeInAudio, SIGNAL(error(const char*)), this, SLOT(send_Error(const char*)));
    
    if(fFadeInAudio->init(name, DSP)){
        return true;
    }
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

    QDateTime currentTime(QDateTime::currentDateTime());
    
    while(fCurrentAudio->get_FadeOut() == 1){
        printf("STOPED PROGRAMATICALLY\n");
        fFadeInAudio->force_stopFade();
        fCurrentAudio->force_stopFade();
    }
    
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

int NJ_audioManager::get_buffer_size(){
    return fCurrentAudio->get_buffer_size();
}

int NJ_audioManager::get_sample_rate(){
    return fCurrentAudio->get_sample_rate();
}


