//
//  JA_audioManager.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// JA_audioManager controls 1 JA_audioFader. It can switch from one DSP to another with a crossfade or it can act like a simple jackaudio-dsp
// JA_audioManager also controls the jack connections of the audio. 

#include "JA_audioManager.h"

#include "JA_audioFader.h"

#include <QFileInfo>

JA_audioManager::JA_audioManager(AudioSettings* as): AudioManager(as){

    fSettings = dynamic_cast<JA_audioSettings*>(as);
    
    fCurrentAudio = new JA_audioFader;
}

JA_audioManager::~JA_audioManager(){

    delete fCurrentAudio;
}

//INIT/START/STOP on Current JackAudio
bool JA_audioManager::initAudio(QString& error, const char* name){
    
    printf("NAME INIT = %s\n", name);
    
    if(fCurrentAudio->init(name))
        return true;
    else{
        error = "Impossible to init JackAudio Client";
        return false;
    }
}

bool JA_audioManager::setDSP(QString& error, dsp* DSP, const char* port_name){
 
    printf("SET DSP = %s\n", port_name);
    
    if(fCurrentAudio->set_dsp(DSP, port_name))
        return true;
    else{
        error = "Impossible to init JackAudio Client";
        return false;
    }
    
}

bool JA_audioManager::init(const char* name, dsp* DSP){
    return fCurrentAudio->init(name, DSP);
}

bool JA_audioManager::start(){
    return fCurrentAudio->start();
}

void JA_audioManager::stop(){
    fCurrentAudio->stop();
}

//Init new dsp, that will fade in current audio
bool JA_audioManager::init_FadeAudio(QString& error, const char* name, dsp* DSP){

    error = "";
    
    fCurrentAudio->init_FadeIn_Audio(DSP, name);
    return true;
}

//Crossfade start
void JA_audioManager::start_Fade(){

    fCurrentAudio->launch_fadeOut();
}

//When the crossfade ends, the DSP is updated in jackaudio Fader
void JA_audioManager::wait_EndFade(){
    
    while(fCurrentAudio->get_FadeOut() == 1){}
    
    fCurrentAudio->upDate_DSP();
}

//Recall Connections from saving file
void JA_audioManager::connect_Audio(string homeDir){
    
    if(QFileInfo(homeDir.c_str()).exists()){
        
        list<pair<string, string> > connection = fInterface->recallConnections(homeDir.c_str());
        
        fCurrentAudio->reconnect(connection);
    }
    else
        fCurrentAudio->default_connections();  
    
}

//Save connections in file
void JA_audioManager::save_Connections(string homeDir){
    
    fInterface->saveConnections(homeDir.c_str(), fCurrentAudio->get_audio_connections());
    
}

//Update connection file following the change table
void JA_audioManager::change_Connections(string homeDir, list<pair<string, string> > changeTable){
    
    fInterface->update(homeDir.c_str(), changeTable);
}

int JA_audioManager::get_buffer_size(){
    return fCurrentAudio->get_buffer_size();
}

int JA_audioManager::get_sample_rate(){
    return fCurrentAudio->get_sample_rate();
}
