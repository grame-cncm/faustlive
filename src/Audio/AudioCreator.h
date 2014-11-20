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
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

class AudioSettings;
class AudioManager;

#include "AudioFactory.h"

using namespace std;

class AudioCreator : public QObject{
   
    Q_OBJECT
    
    protected : 
    
        int             fTempAudioIndex;        //Audio architecture currently used
    
        static AudioCreator*    _instance;
    
        AudioFactory*   fFactory;           //Type of factory depending on audio Architecture
    
    //Layout and menus that contain the audioArchi ComboBox and the Settings
        QFormLayout*    fLayout;                
        QGroupBox*      fMenu;
        QGroupBox*      fSettingsBox;
        QGroupBox*      fTempBox;
        QComboBox*      fAudioArchi;
    
    //BOTH ARE NEEDED IN CASE THE UPDATE AUDIO DOES NOT WORK AND INTERMEDIATE SETTINGS HAVE TO BE SWITCHED BACK
        AudioSettings*  fTempSettings;  //Visual settings that can change multiple times before being saved (when audio update is tested, fTempSetting stores the previous settings)
        AudioSettings*  fCurrentSettings;       //Current audio settings saved
    
        int             driverNameToIndex(const QString& driverName);
    
    public :
    
        AudioCreator(QGroupBox* parent);
        virtual         ~AudioCreator();
    
    //Returns the instance of the audioCreator
        static AudioCreator*   _Instance(QGroupBox* box);
    
    //Creates an audioManager depending on the current Audio Architecture
        AudioFactory*   createFactory(int index);
        AudioManager*   createAudioManager(AudioShutdownCallback cb = NULL, void* arg = NULL);
        AudioSettings*  createAudioSettings(QGroupBox* parent);
    
    //        When the parameters are not saved
        void savedSettingsToVisualSettings();
    
    //    When the parameters are saved, there are 2 steps : 1- Save them temporarily 2- Test them in the application --> In case of success they are saved // In case of failure, previous parameters are restored
        void visualSettingsToTempSettings();
        void tempSettingsToSavedSettings();
        void restoreSavedSettings();
    
    //Accessors to the settings
        QString          get_ArchiName();
        bool            didSettingChanged();

    //Switch back to default audio architecture
        void            reset_AudioArchitecture();
    
    private slots :
    
        void            indexChanged(int index);
    
    
    
};

#endif
