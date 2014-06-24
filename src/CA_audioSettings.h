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
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#define CA_SAVINGFILE "CA_Settings.rf"

using namespace std;

class CA_audioSettings : public AudioSettings{

    private :
    
    //Graphical CoreAudio Settings
        QTextBrowser*       fsplRate;
        QLineEdit*          fBufSize;
    
    private slots :
    
    //The sample rate cannot be modified internally, it is redirected in Configuration Audio and Midi
        void linkClicked(const QUrl& link); 
    
    public :
        CA_audioSettings(QGroupBox* parent);
        ~CA_audioSettings();

        virtual void    setVisualSettings();
        virtual void    storeVisualSettings();
    
        virtual bool    isEqual(AudioSettings* as);
        virtual QString get_ArchiName();
        int             get_BufferSize();
        int             get_SampleRate();
};

#endif
