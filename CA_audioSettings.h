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

        int                 fBufferSize;
    
        QTextBrowser*       fsplRate;
        QLineEdit*          fBufSize;
    
    private slots :
    
        void linkClicked(const QUrl& link); 
    
    public :
        CA_audioSettings(string home, QGroupBox* parent);
        ~CA_audioSettings();
    
        virtual void readSettings();
        virtual void writeSettings();
        virtual void setCurrentSettings();
        virtual void getCurrentSettings();
    
        virtual bool isEqual(AudioSettings* as);
        bool isStringInt(const char* word);
    
        int get_BufferSize();
        virtual string get_ArchiName();
};

#endif
