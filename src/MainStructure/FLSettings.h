//
//  FLSettings.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLSettings contains the settings of the application. Its hierarchy is described in FaustLive documentation.
// It is a singleton in order to be easily acccessible from any another class.

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
    
    FLSettings( const QString & fileName, Format format, QObject * parent = 0);
    ~FLSettings();

    static FLSettings* getInstance();
    static void initInstance(const QString homePath);
    static FLSettings* _settingsInstance;
    static void deleteInstance();
};

#endif
