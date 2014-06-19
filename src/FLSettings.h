//
//  FaustLiveApp.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLApp is the centerpiece of FaustLive. The class controls all the windows, menu and actions of a user. 

#ifndef _FLSettings_h
#define _FLSettings_h

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

class FLSettings : public QSettings
{
    Q_OBJECT
    
public: 
    
    FLSettings(const QString & organization, const QString & application = QString(), QObject * parent = 0);
    
    FLSettings(Format format, Scope scope, const QString & organization, const QString & application = QString(), QObject * parent = 0 );
    
    FLSettings( const QString & fileName, Format format, QObject * parent = 0);
    
    ~FLSettings();

    
    static FLSettings* getInstance();
    static FLSettings* _settingsInstance;
    static void deleteInstance();
};

#endif
