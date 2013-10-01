//
//  CA_audioSettings.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//


#ifndef _CA_audioSettings_h
#define _CA_audioSettings_h

#include "AudioSettings.h"

#include <string>

#include <QTextBrowser>
#include <QLineEdit>
#include <QUrl>
#include <QString>
#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QFile>
#include <QTextStream>

#define CA_SAVINGFILE "CA_Settings.rf"

using namespace std;

class CA_audioSettings : public AudioSettings{

    private :

        int                 fBufferSize;
    
        QTextBrowser*       fsplRate;
        QLineEdit*          fBufSize;
    
    private slots :
    
        void linkClicked(const QUrl& link); 
        void yaouh(const QString& text);
    
    public :
        CA_audioSettings(string home, QGroupBox* parent);
        ~CA_audioSettings();
    
        virtual void readSettings();
        virtual void writeSettings();
        virtual void setCurrentSettings();
        virtual void getCurrentSettings();
    
        virtual bool isEqual(AudioSettings* as);
        bool isStringInt(const char* word);
    
        int get_BufferSize();
        virtual string get_ArchiName();
};

#endif
