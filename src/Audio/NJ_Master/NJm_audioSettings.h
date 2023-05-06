//
//  NJm_audioSettings.h
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

#ifndef _NJm_audioSettings_h
#define _NJm_audioSettings_h

#include "AudioSettings.h"
#include <string>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif


class NJm_audioSettings : public AudioSettings {

    public:
    
        NJm_audioSettings(QGroupBox* parent);
        virtual ~NJm_audioSettings();
    
        virtual void setVisualSettings();
        virtual void storeVisualSettings();
    
        virtual bool isEqual(AudioSettings* as);
        virtual QString get_ArchiName();
};

#endif
