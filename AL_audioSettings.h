//
//  AL_audioSettings.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class describes the Settings of a coreaudio : it contains its values and visual objects to represent them. 

#ifndef _AL_audioSettings_h
#define _AL_audioSettings_h

#include "AudioSettings.h"

#include <string>
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#define AL_SAVINGFILE "AL_Settings.rf"

using namespace std;

class AL_audioSettings : public AudioSettings{

    private :

        int                 fBufferSize; //Real Buffer Size setting
        int                 fFrequency;
        int                 fPeriods;
        string              fCardName;
    
    //Graphical CoreAudio Settings
        QLineEdit*          fFreqLine;
        QLineEdit*          fBufLine;
        QLineEdit*          fPeriodLine;
        QLineEdit*          fCardLine;
    
    private slots :
    
    public :
        AL_audioSettings(string home, QGroupBox* parent);
        ~AL_audioSettings();
    
        virtual void readSettings();
        virtual void writeSettings();
        virtual void setVisualSettings();
        virtual void storeVisualSettings();
    
        virtual bool isEqual(AudioSettings* as);    
        int get_BufferSize();
		int get_Period();
		int get_Frequency();
		string get_DeviceName();
        virtual string get_ArchiName();
};

#endif
