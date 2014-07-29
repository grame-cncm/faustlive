//
//  NJs_audioSettings.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class describes the Settings of netjack clients : 
//  - Its compression Value
//  - Its IPadress
//  - Its Port
//  - Its Latencty
// All of them are described visually by a QLineEdit

#ifndef _NJs_audioSettings_h
#define _NJs_audioSettings_h

#include "AudioSettings.h"

#include <string>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

using namespace std;

class NJs_audioSettings : public AudioSettings{

    private :
    
    //Graphical parameters
        QLineEdit*          fCVLine;
        QLineEdit*          fIPLine;
        QLineEdit*          fPortLine;
        QLineEdit*          fLatLine;
        QLineEdit*          fMTULine;
    
    public :
        NJs_audioSettings(QGroupBox* parent);
        virtual ~NJs_audioSettings();
    
        virtual void setVisualSettings();
        virtual void storeVisualSettings();
    
        virtual bool isEqual(AudioSettings* as);
    
        int         get_compressionValue();
        QString    get_IP();
        int         get_Port();
        int         get_latency();
        int         get_mtu();
    
        virtual QString get_ArchiName();
};

#endif
