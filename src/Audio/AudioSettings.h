//
//  audioSettings.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class is an abstract description of the Settings specific to an audio architecture.

#ifndef _AudioSettings_h
#define _AudioSettings_h

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

class AudioSettings : public QObject{

    private:
    
        Q_OBJECT
    
    public:
    
        AudioSettings(QGroupBox* parent){ Q_UNUSED(parent); }
        virtual ~AudioSettings(){}
    
        virtual void setVisualSettings() = 0;
        virtual void storeVisualSettings() = 0;
    
        virtual bool isEqual(AudioSettings* as) = 0;
        virtual bool operator==(AudioSettings& as){return isEqual(&as);}
        
        virtual QString get_ArchiName() = 0;

    protected slots:
    
        virtual void linkClicked(const QUrl&){}
        
};

#endif
