//
//  NJs_audioManager.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// NJs_audioManager controls 2 NJs_audioFader. It can switch from one to another with a crossfade or it can act like a simple netjack-dsp

#include "NJs_audioManager.h"

#include "NJs_audioFader.h"

#include "FLSettings.h"

NJs_audioManager::NJs_audioManager(AudioShutdownCallback cb, void* arg): AudioManager(cb, arg){
    
    FLSettings* settings = FLSettings::_Instance();
    
    fCV = settings->value("General/Audio/NetJackSlave/CV", -1).toInt();
    fIP = settings->value("General/Audio/NetJackSlave/IP", "225.3.19.154").toString();
    fPort = settings->value("General/Audio/NetJackSlave/Port", 19000).toInt();
    fLatency = settings->value("General/Audio/NetJackSlave/Latency", 2).toInt();
    fMTU = settings->value("General/Audio/NetJackSlave/MTU", 1500).toInt();
    
    fCurrentAudio = new NJs_audioFader(fCV, fIP.toStdString(), fPort, fMTU, fLatency);
    
    connect(fCurrentAudio, SIGNAL(error(const char*)), this, SLOT(send_Error(const char*)));
    
    fInit = false; //Indicator of which init has been used
}

NJs_audioManager::~NJs_audioManager(){

    delete fCurrentAudio;
}

//INIT interface to correspond to JackAudio init interface
bool NJs_audioManager::initAudio(QString& /*error*/, const char* /*name*/){

    fInit = false;
//    fName = name;
    return true;
}

bool NJs_audioManager::initAudio(QString& error, const char* /*name*/, const char* port_name, int numInputs, int numOutputs){
    
    if(fCurrentAudio->init(port_name, numInputs, numOutputs)){
        fInit = true;
        return true;
    }
    else{
        error = "Impossible to init NetJackAudio";
        return false;
    }
}

bool NJs_audioManager::setDSP(QString& error, dsp* DSP, const char* port_name){
    
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
bool NJs_audioManager::init(const char* name, dsp* DSP){
    return fCurrentAudio->init(name, DSP);
}

bool NJs_audioManager::start(){
    return fCurrentAudio->start();
}

void NJs_audioManager::stop(){
    fCurrentAudio->stop();
}

//Init new audio, that will fade in current audio
bool NJs_audioManager::init_FadeAudio(QString& error, const char* name, dsp* DSP){
    
    fFadeInAudio = new NJs_audioFader(fCV, fIP.toStdString(), fPort, fMTU, fLatency);
    
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
void NJs_audioManager::start_Fade(){
    
    fCurrentAudio->launch_fadeOut();
    fFadeInAudio->launch_fadeIn();
    
    fFadeInAudio->start();
}

//When the crossfade ends, FadeInAudio becomes the current audio 
void NJs_audioManager::wait_EndFade(){

    QDateTime currentTime(QDateTime::currentDateTime());
    
    while(fCurrentAudio->get_FadeOut() == 1){
        printf("STOPED PROGRAMATICALLY\n");
        fFadeInAudio->force_stopFade();
        fCurrentAudio->force_stopFade();
    }
    
    fCurrentAudio->stop();
    NJs_audioFader* intermediate = fCurrentAudio;
    fCurrentAudio = fFadeInAudio;
    fFadeInAudio = intermediate;
    delete fFadeInAudio;
}

//In case of Network failure, the application is notified
void NJs_audioManager::send_Error(const char* msg){
    emit errorSignal(msg);
}

int NJs_audioManager::get_buffer_size(){
    return fCurrentAudio->get_buffer_size();
}

int NJs_audioManager::get_sample_rate(){
    return fCurrentAudio->get_sample_rate();
}


