//
//  CA_audioManager.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// CA_audioManager controls 2 CA_audioFader. It can switch from one to another with a crossfade or it can act like a simple coreaudio-dsp

#include "CA_audioFader.h"
#include "CA_audioManager.h"
#include "FLSettings.h"

CA_audioManager::CA_audioManager(AudioShutdownCallback cb, void* arg) : AudioManager(cb, arg)
{
    fBufferSize = FLSettings::_Instance()->value("General/Audio/CoreAudio/BufferSize", 512).toInt();
    fCurrentAudio = new CA_audioFader(fBufferSize);
    fFadeInAudio = 0;
    fInit = false;
}

CA_audioManager::~CA_audioManager()
{
   delete fCurrentAudio;
}

//INIT interface to correspond to JackAudio init interface
bool CA_audioManager::initAudio(QString& error, const char* name)
{
    error = "";
    fName = name;
    fInit = false;
    return true;
}

bool CA_audioManager::initAudio(QString& error, const char* /*name*/, const char* port_name, int numInputs, int numOutputs)
{
    if (numInputs == 0 && numOutputs == 0) {
        return initAudio(error, port_name);
    }
    
    if (fCurrentAudio->init(port_name, numInputs, numOutputs)) {        
        FLSettings::_Instance()->setValue("General/Audio/CoreAudio/BufferSize", get_buffer_size());
        fInit = true;
        return true;
    } else {
        error = "Impossible to init CoreAudio Client";
        return false;
    }
}

bool CA_audioManager::setDSP(QString& error, dsp* DSP, const char* /*port_name*/)
{
    if (fInit) {
        fCurrentAudio->set_dsp(DSP);
        return true;
    } else if (init(fName, DSP)) {
        FLSettings::_Instance()->setValue("General/Audio/CoreAudio/BufferSize", get_buffer_size());
        return true;
    } else {
        error = "Impossible to init CoreAudio Client";
        return false;
    }
}

//INIT/START/STOP on Current CoreAudio
bool CA_audioManager::init(const char* name, dsp* DSP)
{
    return fCurrentAudio->init(name, DSP);
}

bool CA_audioManager::start()
{
    return fCurrentAudio->start();
}

void CA_audioManager::stop()
{
    fCurrentAudio->stop();
}

//Init new audio, that will fade in current audio
bool CA_audioManager::init_FadeAudio(QString& error, const char* name, dsp* DSP)
{
    printf("CA_audioManager::init_FadeAudio\n");
    fFadeInAudio = new CA_audioFader(fBufferSize);
    
    if (fFadeInAudio->init(name, DSP)) {
        return true;
    } else {
        error = "Impossible to init new Core Audio Client";
        return false;
    }
}

//Crossfade start
void CA_audioManager::start_Fade()
{
    fFadeInAudio->launch_fadeIn();
    fCurrentAudio->launch_fadeOut();
    
    if (!fFadeInAudio->start()) {
        printf("CoreAudio did not Start\n");
    } else {
        printf("CoreAudio totally Started\n");
    }
}

//When the crossfade ends, FadeInAudio becomes the current audio 
void CA_audioManager::wait_EndFade()
{
//   In case of CoreAudio Bug : If the Render function is not called, the loop could be infinite. This way, it isn't.
    QDateTime currentTime(QDateTime::currentDateTime());
        
    while (fCurrentAudio->get_FadeOut()) {
        QDateTime currentTime2(QDateTime::currentDateTime());
        if (currentTime.secsTo(currentTime2) > 3) {
            printf("STOPPED PROGRAMATICALLY\n");
            fFadeInAudio->force_stopFade();
            fCurrentAudio->force_stopFade();
        }
    }

    fCurrentAudio->stop();
    CA_audioFader* intermediate = fCurrentAudio;
    fCurrentAudio = fFadeInAudio;
    fFadeInAudio = intermediate;
    delete fFadeInAudio;
}

int CA_audioManager::get_buffer_size()
{
    return fCurrentAudio->get_buffer_size();
}

int CA_audioManager::get_sample_rate()
{
    return fCurrentAudio->get_sample_rate();
}


