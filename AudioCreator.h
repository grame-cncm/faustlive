//
//  audioCreator.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

//This class is a SINGLETON (as described in DESIGN PATTERNS)
//
//The goal of this class is to control the type of audio architecture used in the application. Therefore it creates the right type of audioFactory

#ifndef _AudioCreator_h
#define _AudioCreator_h

#include <string>
#include <QtGui>

class AudioSettings;
class AudioManager;
class AudioFactory;

using namespace std;

#define SAVINGFILE "AudioSettings.rf"

class AudioCreator : public QObject{
   
    Q_OBJECT
    
    protected : 
    
        int             fAudioIndex;        //Audio architecture currently used
    
        string          fHome;              //HomeFolder where to save the settings
        string          fSavingFile;        //File saving the audio architecture used
    
        static AudioCreator*    _instance;
    
        AudioFactory*   fFactory;           //Type of factory depending on audio Architecture
    
    
    //BOTH ARE NEEDED IN CASE THE UPDATE AUDIO DOES NOT WORK AND INTERMEDIATE SETTINGS HAVE TO BE SWITCHED BACK
        AudioSettings*  fIntermediateSettings;  //Visual settings that can change multiple times before being saved
        AudioSettings*  fCurrentSettings;       //Current audio settings saved
    
    //Layout and menus that contain the audioArchi ComboBox and the Settings
        QFormLayout*    fLayout;                
        QGroupBox*      fMenu;
        QGroupBox*      fSettingsBox;
        QGroupBox*      fSettingsIntermediateBox;
        QComboBox*      fAudioArchi;

    //Save and read settings in the saving file
        void            writeSettings();
        void            readSettings();
    
    //Set fAudioIndex with the visual parameter chosen
        void            setCurrentSettings(int index);
    
    public :
                        AudioCreator(string homeFolder, QGroupBox* parent);
        virtual         ~AudioCreator();
    
    //Returns the instance of the audioCreator
        static AudioCreator*   _Instance(string homeFolder, QGroupBox* box);
    
    //Creates an audioManager depending on the current Audio Architecture
        AudioFactory*   createFactory(int index);
        AudioManager*   createAudioManager(AudioSettings* audioParameters);
        AudioSettings*  createAudioSettings(string homeFolder, QGroupBox* parent);
    
    //Accessors to the settings
        string          get_ArchiName();
        AudioSettings*  getCurrentSettings();
        AudioSettings*  getNewSettings();
        bool            didSettingChanged();
        void            reset_Settings();
    
        void            saveCurrentSettings();
    
    private slots :
    
        void            indexChanged(int index);
    
};

#endif
