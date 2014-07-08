//
//  FLWinSettings.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLWinSettings contains the settings of a window. It synchronizes some parameters with the general settings of the application.

#ifndef _FLWinSettings_h
#define _FLWinSettings_h

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

class FLWinSettings : public QSettings
{
    Q_OBJECT
    
        int     fIndex;
    
    public: 
    
        //@param index : index of the window which settings it is
        //@param filename : path to the settings file
        //@param format : format of the settings
        //@param parent : parent object in the hierarchy
        FLWinSettings(int index , const QString & fileName, Format format, QObject * parent = 0);
        ~FLWinSettings();
    
        virtual void setValue(const QString & key, const QVariant & value);
};

#endif
