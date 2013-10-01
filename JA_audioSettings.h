//
//  JA_audioSettings.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//


#ifndef _JA_audioSettings_h
#define _JA_audioSettings_h

#include "AudioSettings.h"

class JA_audioSettings : public AudioSettings{

    public :
        JA_audioSettings(string home, QGroupBox* parent);
    
        virtual ~JA_audioSettings();
    
        virtual void readSettings();
        virtual void writeSettings();
        virtual void setCurrentSettings();
        virtual void getCurrentSettings();
    
        virtual bool isEqual(AudioSettings* as);
    
        virtual string get_ArchiName();
};

#endif
