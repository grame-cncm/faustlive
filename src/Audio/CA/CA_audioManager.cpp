//
//  CA_audioManager.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 GRAME. All rights reserved.
//

// CA_audioManager controls 2 CA_audioFader. It can switch from one to another with a crossfade or it can act like a simple coreaudio-dsp

#include <QDateTime>

#if defined(_WIN32) && !defined(GCC)
# pragma warning (disable: 4100)
#else
# pragma GCC diagnostic ignored "-Wunused-parameter"
# pragma GCC diagnostic ignored "-Wunused-variable"
# pragma GCC diagnostic ignored "-Wunused-function"
#endif

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
bool CA_audioManager::initAudio(QString& error, const char* name, bool /*midi*/)
{
    error = "";
    fName = name;
    fInit = false;
    return true;
}

bool CA_audioManager::initAudio(QString& error, const char* /*name*/, const char* port_name, int numInputs, int numOutputs, bool midi)
{
    if (numInputs == 0 && numOutputs == 0) {
        return initAudio(error, port_name, midi);
    }
    
    if (fCurrentAudio->init(port_name, numInputs, numOutputs)) {        
        FLSettings::_Instance()->setValue("General/Audio/CoreAudio/BufferSize", getBufferSize());
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
        fCurrentAudio->setDsp(DSP);
        return true;
    } else if (init(fName, DSP)) {
        FLSettings::_Instance()->setValue("General/Audio/CoreAudio/BufferSize", getBufferSize());
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
    fFadeInAudio = new CA_audioFader(fBufferSize);
    
    if (fFadeInAudio->init(name, DSP)) {
        return true;
    } else {
        error = "Impossible to init new CoreAudio client";
        return false;
    }
}

//Crossfade start
void CA_audioManager::start_Fade()
{
    fFadeInAudio->launch_fadeIn();
    fCurrentAudio->launch_fadeOut();
    
    if (!fFadeInAudio->start()) {
        printf("CoreAudio did not start\n");
    } else {
        printf("CoreAudio totally started\n");
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

int CA_audioManager::getBufferSize()
{
    return fCurrentAudio->getBufferSize();
}

int CA_audioManager::getSampleRate()
{
    return fCurrentAudio->getSampleRate();
}


