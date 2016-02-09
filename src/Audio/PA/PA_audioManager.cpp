//
//  PA_audioManager.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// PA_audioManager controls 1 PA_audioFader. It can switch from one DSP to another with a crossfade or it can act like a simple jackaudio-dsp
// PA_audioManager also controls the jack connections of the audio. 

#include "PA_audioManager.h"
#include "PA_audioFader.h"

#include "FLSettings.h"

#include <QFileInfo>

PA_audioManager::PA_audioManager(AudioShutdownCallback cb, void* arg): AudioManager(cb, arg)
{
	long bs = FLSettings::_Instance()->value("General/Audio/PortAudio/BufferSize", 1024).toInt();
    long sr = FLSettings::_Instance()->value("General/Audio/PortAudio/SampleRate", 44100).toInt();
    fCurrentAudio = new PA_audioFader(sr, bs);
    fFadeInAudio = 0;
    fInit = false;
}

PA_audioManager::~PA_audioManager()
{
    delete fCurrentAudio;
}

//INIT/START/STOP on Current PortAudio
bool PA_audioManager::initAudio(QString& error, const char* /*name*/)
{
    error = "";
    fInit = false;
    return true;
}

bool PA_audioManager::initAudio(QString& error, const char* /*name*/, const char* port_name, int numInputs, int numOutputs)
{
	if (fCurrentAudio->init(port_name, numInputs, numOutputs/*, fSettings->get_inputDevice(), fSettings->get_ouputDevice()*/)) {
        fInit = true;
        return true;
    } else {
        error = "Impossible to init PortAudio Client";
        return false;
    }
}

bool PA_audioManager::setDSP(QString& error, dsp* DSP, const char* port_name)
{
    if (fInit) {
        fCurrentAudio->set_dsp(DSP);
        return true;
    } else if (init(port_name, DSP)) {
        return true;
    } else {
        error = "Impossible to init PortAudio Client";
        return false;
    }
}

bool PA_audioManager::init(const char* name, dsp* DSP)
{
	return fCurrentAudio->init(name, DSP);
}

bool PA_audioManager::start()
{
    return fCurrentAudio->start();
}

void PA_audioManager::stop()
{
    fCurrentAudio->stop();
}

//Init new dsp, that will fade in current audio
bool PA_audioManager::init_FadeAudio(QString& error, const char* name, dsp* DSP)
{
	long bs = FLSettings::_Instance()->value("General/Audio/PortAudio/BufferSize", 1024).toInt();
    long sr = FLSettings::_Instance()->value("General/Audio/PortAudio/SampleRate", 44100).toInt();
   
    fFadeInAudio = new PA_audioFader(sr, bs);
    
	if (fFadeInAudio->init(name, DSP)) {
		return true;
	} else {
        error = "Impossible to init new Port Audio Client";
        return false;
    }
}

//Crossfade start
void PA_audioManager::start_Fade()
{
    fFadeInAudio->launch_fadeIn();
    fCurrentAudio->launch_fadeOut();
    fFadeInAudio->start();
}

//When the crossfade ends, the DSP is updated in jackaudio Fader
void PA_audioManager::wait_EndFade()
{
    while (fCurrentAudio->get_FadeOut()) {}
    fCurrentAudio->stop();
    PA_audioFader* intermediate = fCurrentAudio;
    fCurrentAudio = fFadeInAudio;
    fFadeInAudio = intermediate;
    delete fFadeInAudio;
}

int PA_audioManager::get_buffer_size()
{
    // TODO
    return fCurrentAudio->get_buffer_size();
}

int PA_audioManager::get_sample_rate()
{
    // TODO
    return fCurrentAudio->get_sample_rate();
}


