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

    private:
    
        map<string, int> fInputdevices;
        map<string, int> fOutputdevices;

        //Graphical PortAudio Settings
        QLineEdit*       fsplRate;
        QLineEdit*       fBufSize;
        QFormLayout*     fLayout;
        //QComboBox*       fInputDeviceBox;
        //QComboBox*       fOutputDeviceBox;

        long get_BufferSize();
        long get_SampleRate();

    public:
    
        PA_audioSettings(QGroupBox* parent);
        virtual ~PA_audioSettings();
    
        virtual void setVisualSettings();
        virtual void storeVisualSettings();
    
        virtual bool isEqual(AudioSettings* as);
        virtual QString get_ArchiName();
        
//      int		get_inputDevice();
//		int		get_ouputDevice();
//		void	set_deviceList();
};

#endif
