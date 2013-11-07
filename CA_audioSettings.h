//
//  CA_audioSettings.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class describes the Settings of a coreaudio : it contains its values and visual objects to represent them. 

#ifndef _CA_audioSettings_h
#define _CA_audioSettings_h

#include "AudioSettings.h"

#include <string>
#include <QtGui>

#define CA_SAVINGFILE "CA_Settings.rf"

using namespace std;

class CA_audioSettings : public AudioSettings{

    private :

        int                 fBufferSize; //Real Buffer Size setting
    
    //Graphical CoreAudio Settings
        QTextBrowser*       fsplRate;
        QLineEdit*          fBufSize;
    
    private slots :
    
    //The sample rate cannot be modified internally, it is redirected in Configuration Audio and Midi
        void linkClicked(const QUrl& link); 
    
    public :
        CA_audioSettings(string home, QGroupBox* parent);
        ~CA_audioSettings();
    
        virtual void readSettings();
        virtual void writeSettings();
        virtual void setVisualSettings();
        virtual void storeVisualSettings();
    
        virtual bool isEqual(AudioSettings* as);
        bool isStringInt(const char* word);
    
        int get_BufferSize();
        virtual string get_ArchiName();
};

#endif
