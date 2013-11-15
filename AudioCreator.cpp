//
//  AudioCreator.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

//This class is a SINGLETON (as described in DESIGN PATTERNS)
//
//The goal of this class is to control the type of audio architecture used in the application. Therefore it creates the right type of audioFactory

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

#ifdef ALSA
#include "AL_audioFactory.h"
#endif

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

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
#ifdef ALSA
    kAlsaaudio
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
#ifdef ALSA
    fAudioArchi->addItem("Alsa");
#endif
    
    readSettings();
    
    printf("fAudioIndex = %i\n", fAudioIndex);
    
    fFactory = createFactory(fAudioIndex);
        
    connect(fAudioArchi, SIGNAL(activated(int)), this, SLOT(indexChanged(int)));
    
    setCurrentSettings(fAudioIndex);
    
    fLayout->addRow(new QLabel("Audio Architecture"), fAudioArchi);
    
    fSettingsBox = new QGroupBox;
    fUselessBox = new QGroupBox;
    
    fIntermediateSettings = fFactory->createAudioSettings(fHome, fSettingsBox);
    
    printf("fIntermediateSettings = %p\n", fIntermediateSettings);
    
    fLayout->addRow(fSettingsBox);
    
    fMenu->setLayout(fLayout);

    fCurrentSettings = fFactory->createAudioSettings(fHome, fUselessBox);
    printf("fIntermediateSettings = %p\n", fCurrentSettings);
}

//Returns the instance of the audioCreator
AudioCreator* AudioCreator::_Instance(string homeFolder, QGroupBox* box){
    if(_instance == 0)
        _instance = new AudioCreator(homeFolder, box);
    
    return _instance;
}

AudioCreator::~AudioCreator(){

    writeSettings();
    
    delete fFactory;
    delete fSettingsBox;
    delete fUselessBox;
    delete fCurrentSettings;
    delete fIntermediateSettings;
}

//Set or Save fAudioIndex with the visual parameter chosen
void AudioCreator::setCurrentSettings(int index){
    
    fAudioArchi->setCurrentIndex(index);
}

void AudioCreator::saveCurrentSettings(){
    
    fAudioIndex = fAudioArchi->currentIndex();
    
    delete fCurrentSettings;
    delete fUselessBox;
    fUselessBox = new QGroupBox;
    
    reset_Settings();
    fCurrentSettings = fFactory->createAudioSettings(fHome, fUselessBox);
}

//Dynamic change when the audio index (= audio architecture) changes
void AudioCreator::indexChanged(int index){
    
    if(fFactory != NULL)
        delete fFactory;
    
    if(fIntermediateSettings != NULL){
        delete fIntermediateSettings;
    }
    
    if(fSettingsBox != NULL)
        delete fSettingsBox;
    
    fFactory = createFactory(index);
    fSettingsBox = new QGroupBox;
    fIntermediateSettings = fFactory->createAudioSettings(fHome, fSettingsBox);
    
    fLayout->addRow(fSettingsBox);
    fMenu->setLayout(fLayout);
}

//Creation of the Factory/Settings/Manager depending on audio index
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
            
#ifdef ALSA
        case kAlsaaudio:
            
            return new AL_audioFactory();
            break;
#endif  
        default:
            return NULL;
    }
}

AudioSettings* AudioCreator::createAudioSettings(string homeFolder, QGroupBox* parent){

    return fFactory->createAudioSettings(homeFolder, parent);
    
}

AudioManager* AudioCreator::createAudioManager(AudioSettings* audioParameters){

    return fFactory->createAudioManager(audioParameters);
}

//Save and read settings in the saving file
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

//Accessors to the Settings
string AudioCreator::get_ArchiName(){
    return fCurrentSettings->get_ArchiName();
}

AudioSettings* AudioCreator::getCurrentSettings(){
    
    return fCurrentSettings;
}

AudioSettings* AudioCreator::getNewSettings(){
    
    return fIntermediateSettings;
}

void AudioCreator::reset_Settings(){
    
    indexChanged(fAudioIndex);
    setCurrentSettings(fAudioIndex);
    fIntermediateSettings->writeSettings();
}

//Does the visual parameters concord to the stored settings?
//Determines if the audio has to be reloaded
bool AudioCreator::didSettingChanged(){
    
    fIntermediateSettings->storeVisualSettings();
    
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

