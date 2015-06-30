//
//  NJm_audioManager.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// NJm_audioManager controls 2 NJm_audioFader. It can switch from one to another with a crossfade or it can act like a simple netjack-dsp

#include "NJm_audioManager.h"

#include "NJm_audioFader.h"

#include "FLSettings.h"

NJm_audioManager::NJm_audioManager(AudioShutdownCallback cb, void* arg): AudioManager(cb, arg)
{
    FLSettings* settings = FLSettings::_Instance();
    
    fCV = settings->value("General/Audio/NetJackMaster/CV", -1).toInt();
    fIP = settings->value("General/Audio/NetJackMaster/IP", "225.3.19.154").toString();
    fPort = settings->value("General/Audio/NetJackMaster/Port", 19000).toInt();
    fLatency = settings->value("General/Audio/NetJackMaster/Latency", 2).toInt();
    fMTU = settings->value("General/Audio/NetJackMaster/MTU", 1500).toInt();
    
    fCurrentAudio = new NJm_audioFader(fCV, fIP.toStdString(), fPort, fMTU, fLatency);
    connect(fCurrentAudio, SIGNAL(errorPRINT(const char*)), this, SLOT(send_Error(const char*)));
    printf("FCURRENTAUDIO = %p\n", fCurrentAudio);
    fInit = false; //Indicator of which init has been used
}

NJm_audioManager::~NJm_audioManager()
{
    delete fCurrentAudio;
}

//INIT interface to correspond to JackAudio init interface
bool NJm_audioManager::initAudio(QString& /*error*/, const char* /*name*/)
{
    fInit = false;
    return true;
}

bool NJm_audioManager::initAudio(QString& error, const char* /*name*/, const char* port_name, int numInputs, int numOutputs)
{
    if (fCurrentAudio->init(port_name, numInputs, numOutputs)) {
        fInit = true;
        return true;
    } else {
        error = "Impossible to init NetJack Master";
        return false;
    }
}

bool NJm_audioManager::setDSP(QString& error, dsp* DSP, const char* port_name){
    
    if (fInit) {
        return fCurrentAudio->set_dsp(DSP);
    } else if(fCurrentAudio->init(port_name, DSP)) {
        return true;
    } else {
        error = "Impossible to init NetJack Master";
        return false;
    }
}

//INIT/START/STOP on Current NetJackAudio
bool NJm_audioManager::init(const char* name, dsp* DSP)
{
    return fCurrentAudio->init(name, DSP);
}

bool NJm_audioManager::start()
{
    return fCurrentAudio->start();
}

void NJm_audioManager::stop()
{
    fCurrentAudio->stop();
}

//Init new audio, that will fade in current audio
bool NJm_audioManager::init_FadeAudio(QString& error, const char* name, dsp* DSP)
{
    fFadeInAudio = new NJm_audioFader(fCV, fIP.toStdString(), fPort, fMTU, fLatency);
    connect(fFadeInAudio, SIGNAL(errorPRINT(const char*)), this, SLOT(send_Error(const char*)));
    
    if (fFadeInAudio->init(name, DSP)) {
        return true;
    } else {
        error = "Impossible to fade NetJack Master";
        return false;
    }
}

//Crossfade start
void NJm_audioManager::start_Fade()
{
    fCurrentAudio->launch_fadeOut();
    fFadeInAudio->launch_fadeIn();
    fFadeInAudio->start();
}

//When the crossfade ends, FadeInAudio becomes the current audio 
void NJm_audioManager::wait_EndFade()
{
    QDateTime currentTime(QDateTime::currentDateTime());
    
    while (fCurrentAudio->get_FadeOut() == 1) {
        printf("STOPED PROGRAMATICALLY\n");
        fFadeInAudio->force_stopFade();
        fCurrentAudio->force_stopFade();
    }
    
    fCurrentAudio->stop();
    NJm_audioFader* intermediate = fCurrentAudio;
    fCurrentAudio = fFadeInAudio;
    fFadeInAudio = intermediate;
    delete fFadeInAudio;
}

//In case of Network failure, the application is notified
void NJm_audioManager::send_Error(const char* msg)
{
    printf("NJm_audioManager::send_Error\n");
    emit errorSignal(msg);
}

int NJm_audioManager::get_buffer_size()
{
    return fCurrentAudio->get_buffer_size();
}

int NJm_audioManager::get_sample_rate()
{
    return fCurrentAudio->get_sample_rate();
}

bool NJm_audioManager::is_connexion_active()
{
    return fCurrentAudio->is_connexion_active();
}

