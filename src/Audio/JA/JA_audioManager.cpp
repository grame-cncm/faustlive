//
//  JA_audioManager.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// JA_audioManager controls 1 JA_audioFader. It can switch from one DSP to another with a crossfade or it can act like a simple jackaudio-dsp
// JA_audioManager also controls the JACK connections of the audio. 

#include "JA_audioManager.h"
#include "JA_audioFader.h"
#include <QFileInfo>
#include "FLSettings.h"

void JA_audioManager::shutdown_message(const char * msg, void* arg)
{
    Q_UNUSED(arg);
    Q_UNUSED(msg);
}

JA_audioManager::JA_audioManager(shutdown_callback cb, void* arg): AudioManager(cb, arg)
{
    fCurrentAudio = new JA_audioFader;
    fCurrentAudio->shutdown(cb, arg);
}

JA_audioManager::~JA_audioManager()
{
    delete fCurrentAudio;
}

//INIT/START/STOP on Current JackAudio
bool JA_audioManager::initAudio(QString& error, const char* name, bool midi)
{
    if (fCurrentAudio->init(name, 0)) {
        return true;
    } else {
        error = "Impossible to init JackAudio Client";
        return false;
    }
}

bool JA_audioManager::setDSP(QString& error, dsp* DSP, const char* port_name)
{
    fCurrentAudio->set_dsp(DSP, port_name);
    return true;
}

bool JA_audioManager::init(const char* name, dsp* DSP)
{
    return fCurrentAudio->init(name, DSP);
}

bool JA_audioManager::start()
{
    return fCurrentAudio->start();
}

void JA_audioManager::stop()
{
    fCurrentAudio->stop();
}

//Init new dsp, that will fade in current audio
bool JA_audioManager::init_FadeAudio(QString& error, const char* name, dsp* DSP)
{
    error = "";
    fCurrentAudio->init_FadeIn_Audio(DSP, name);
    return true;
}

//Crossfade start
void JA_audioManager::start_Fade()
{
    fCurrentAudio->launch_fadeOut();
}

//When the crossfade ends, the DSP is updated in jackaudio Fader
void JA_audioManager::wait_EndFade()
{
    while (fCurrentAudio->get_FadeOut()) {}
    fCurrentAudio->upDate_DSP();
}

//Recall Connections from saved file
void JA_audioManager::connect_Audio(string homeDir)
{
    if (FLSettings::_Instance()->value("General/Audio/Jack/AutoConnect", true).toBool()) {
        if (QFileInfo(homeDir.c_str()).exists()) {
            list<pair<string, string> > connection = FJUI::recallConnections(homeDir.c_str());
            fCurrentAudio->reconnect(connection);
        } else {
            fCurrentAudio->defaultConnections();
        }
    } else {
        printf("Do not connect\n");
    }
}

//Save connections in file
void JA_audioManager::save_Connections(string homeDir)
{
    bool saved;
    list<pair<string, string> > connections = fCurrentAudio->get_audio_connections(saved);
    if (saved) {
        FJUI::saveConnections(homeDir.c_str(), connections);
    }
}

int JA_audioManager::getBufferSize()
{
    return fCurrentAudio->getBufferSize();
}

int JA_audioManager::getSampleRate()
{
    return fCurrentAudio->getSampleRate();
}
