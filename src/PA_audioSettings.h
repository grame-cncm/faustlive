//
//  PA_audioSettings.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class describes the Settings of a jackaudio, which are non-existant 

#ifndef _PA_audioSettings_h
#define _PA_audioSettings_h

#include "AudioSettings.h"

#include <string>
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#define PA_SAVINGFILE "PA_Settings.rf"

using namespace std;

class PA_audioSettings : public AudioSettings{

    private :
    
    long            fBufferSize; //Real Buffer Size setting
    long            fSampleRate;
    
    //Graphical CoreAudio Settings
    QLineEdit*       fsplRate;
    QLineEdit*       fBufSize;
    QFormLayout*     fLayout;
    QComboBox*       fDeviceBox;
    
    public :
        PA_audioSettings(QString home, QGroupBox* parent);
    
        virtual ~PA_audioSettings();
    
        virtual void readSettings();
        virtual void writeSettings();
        virtual void setVisualSettings();
        virtual void storeVisualSettings();
    
        virtual bool isEqual(AudioSettings* as);
        long    get_BufferSize();
        long    get_SampleRate();
        virtual QString get_ArchiName();
        QString get_audioDevice();
        void    add_audioDevice(const QString& deviceName);
};

#endif
