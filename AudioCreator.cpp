//
//  AudioCreator.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "AudioCreator.h"

#include "AudioSettings.h"
#include "AudioManager.h"
#include "AudioFactory.h"

#ifdef COREAUDIO
    #include "CA_audioFactory.h"
#endif

#ifdef JACK
    #include "JA_audioFactory.h"
#endif

#ifdef NETJACK
    #include "NJ_audioFactory.h"
#endif

#include <QLabel>
#include <QFile>
#include <QTextStream>

enum audioArchi{
   
#ifdef COREAUDIO
    kCoreaudio, 
#endif
#ifdef JACK
    kJackaudio,
#endif
#ifdef NETJACK
    kNetjackaudio
#endif
};

AudioCreator* AudioCreator::_instance = 0;

AudioCreator::AudioCreator(string homeFolder, QGroupBox* parent) : QObject(NULL){

    fHome = homeFolder;
    fSavingFile = fHome + "/" + SAVINGFILE;
    
    fMenu = parent;
    fLayout = new QFormLayout;
    
    fAudioArchi = new QComboBox(fMenu);
    
//Conditionnal compilation | the options are disabled when not chosen as qmake options
#ifdef COREAUDIO
    fAudioArchi->addItem("Core Audio");
#endif

#ifdef JACK
    fAudioArchi->addItem("Jack");
#endif

#ifdef NETJACK
    fAudioArchi->addItem("NetJack");
#endif
    
    readSettings();
    
    printf("fAudioIndex = %i\n", fAudioIndex);
    
    fFactory = createFactory(fAudioIndex);
        
    connect(fAudioArchi, SIGNAL(activated(int)), this, SLOT(indexChanged(int)));
    
    setCurrentSettings(fAudioIndex);
    
    fLayout->addRow(new QLabel("Audio Architecture"), fAudioArchi);
    
    fSettingsBox = new QGroupBox(fMenu);
    
    fIntermediateSettings = fFactory->createAudioSettings(fHome, fSettingsBox);
    fLayout->addRow(fSettingsBox);
    fMenu->setLayout(fLayout);

    fCurrentSettings = fFactory->createAudioSettings(fHome, fSettingsBox);
}

AudioCreator::~AudioCreator(){

    writeSettings();
    
    delete fFactory;
    delete fSettingsBox;
    delete fCurrentSettings;
    delete fIntermediateSettings;
}

void AudioCreator::setCurrentSettings(int index){
    
    fAudioArchi->setCurrentIndex(index);
}

void AudioCreator::saveCurrentSettings(){
    
    fAudioIndex = fAudioArchi->currentIndex();
    
    delete fCurrentSettings;

    reset_Settings();
    fCurrentSettings = fFactory->createAudioSettings(fHome, fSettingsBox);
}

void AudioCreator::indexChanged(int index){
    
    if(fFactory != NULL)
        delete fFactory;
    
    if(fIntermediateSettings != NULL){
        delete fIntermediateSettings;
    }
    
    if(fSettingsBox != NULL)
        delete fSettingsBox;
    
    fFactory = createFactory(index);
    fSettingsBox = new QGroupBox(fMenu);
    fIntermediateSettings = fFactory->createAudioSettings(fHome, fSettingsBox);
    
    fLayout->addRow(fSettingsBox);
    fMenu->setLayout(fLayout);
}

AudioFactory* AudioCreator::createFactory(int index){
    
    switch(index){
#ifdef COREAUDIO
        case kCoreaudio:
            
            return new CA_audioFactory();
            break;
#endif
#ifdef JACK
        case kJackaudio:
            
            return new JA_audioFactory();
            break;
#endif
            
#ifdef NETJACK
        case kNetjackaudio:
            
            return new NJ_audioFactory();
            break;
#endif            
        default:
            return NULL;
    }
}

void AudioCreator::reset_Settings(){
    
    indexChanged(fAudioIndex);
    setCurrentSettings(fAudioIndex);
    fIntermediateSettings->writeSettings();
}

AudioSettings* AudioCreator::getCurrentSettings(){
    
    return fCurrentSettings;
}

AudioSettings* AudioCreator::getNewSettings(){

    return fIntermediateSettings;
}

AudioSettings* AudioCreator::createAudioSettings(string homeFolder, QGroupBox* parent){

    return fFactory->createAudioSettings(homeFolder, parent);
    
}

AudioManager* AudioCreator::createAudioManager(AudioSettings* audioParameters){

    return fFactory->createAudioManager(audioParameters);
}
 
bool AudioCreator::didSettingChanged(){
    
    fIntermediateSettings->getCurrentSettings();
    
    if(fAudioIndex != fAudioArchi->currentIndex()){
        return true;
    }
    else{
                
        if(!((*fCurrentSettings)==(*fIntermediateSettings))){
            return true;
        }
        else{
            return false;
        }
    }
}

AudioCreator* AudioCreator::_Instance(string homeFolder, QGroupBox* box){
        if(_instance == 0)
            _instance = new AudioCreator(homeFolder, box);
    
    return _instance;
}

void AudioCreator::readSettings(){
    
    QString boxText;
    
    QFile f(fSavingFile.c_str()); 
    
    if(f.open(QFile::ReadOnly)){
        
        QTextStream textReading(&f);
        textReading>>boxText;

        f.close();
        
        for(int i=0; i<fAudioArchi->count() ; i++){
        
            if(boxText == fAudioArchi->itemText(i)){
                fAudioIndex = i;
                break;
            }
            else
                fAudioIndex = 0;
        }
    }
    else
        fAudioIndex = 0;
}

void AudioCreator::writeSettings(){
    
    //fSavedSettings = fSettings + Modifier le fichier
    
    QFile f(fSavingFile.c_str()); 
    
    QString boxText = fAudioArchi->itemText(fAudioIndex);
    
    if(f.open(QFile::WriteOnly | QIODevice::Truncate)){
        
        QTextStream textWriting(&f);
        
        textWriting<<boxText;
        
        f.close();
    }    
}

string AudioCreator::get_ArchiName(){
    return fCurrentSettings->get_ArchiName();
}


