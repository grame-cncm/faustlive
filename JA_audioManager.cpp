//
//  JA_audioManager.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

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

bool JA_audioManager::init(const char* name, dsp* DSP){}

bool JA_audioManager::start(){
    return fCurrentAudio->start();
}
void JA_audioManager::stop(){
    fCurrentAudio->stop();
}

bool JA_audioManager::initAudio(char* error, const char* name, dsp* DSP, const char* port_name){

    if(fCurrentAudio->init(port_name, DSP, name))
        return true;
    else{
        snprintf(error, 255, "Impossible to init JackAudio Client");
        return false;
    }
}

bool JA_audioManager::init_FadeAudio(char* error, const char* name, dsp* DSP){

    fCurrentAudio->init_FadeIn_Audio(DSP, name);
    return true;
}

void JA_audioManager::connect_Audio(string homeDir){
    
    if(QFileInfo(homeDir.c_str()).exists()){
        
        list<pair<string, string> > connection = fInterface->recallConnections(homeDir.c_str());
        
        fCurrentAudio->reconnect(connection);
    }
    else
        fCurrentAudio->default_connections();  

}

void JA_audioManager::save_Connections(string homeDir){

    fInterface->saveConnections(homeDir.c_str(), fCurrentAudio->get_audio_connections());
    
}

void JA_audioManager::change_Connections(string homeDir, list<pair<string, string> > changeTable){

    fInterface->update(homeDir.c_str(), changeTable);
}

void JA_audioManager::start_Fade(){

    fCurrentAudio->launch_fadeOut();
}

void JA_audioManager::wait_EndFade(){
    
    while(fCurrentAudio->get_FadeOut() == 1){}
    
    fCurrentAudio->upDate_DSP();
}

