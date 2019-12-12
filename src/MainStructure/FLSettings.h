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

#include <QSettings>

class FLSettings : public QSettings
{

    private:

        Q_OBJECT
        
        static FLSettings* _settingsInstance;
        
    public: 
        
        FLSettings( const QString & fileName, Format format, QObject * parent = 0);
        virtual ~FLSettings();

        static FLSettings* _Instance();
        static void createInstance(const QString homePath);
        static void deleteInstance();
        
};

#endif
