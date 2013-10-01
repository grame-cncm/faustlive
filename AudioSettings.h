//
//  audioSettings.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//


#ifndef _AudioSettings_h
#define _AudioSettings_h

#include <string>

#include <QObject>
#include <QGroupBox>
#include <QLayout>
#include <QUrl>

using namespace std;

class AudioSettings : public QObject{

    Q_OBJECT
    
    protected:

            string              fSavingFile;
    
    public :
        AudioSettings(string home, QGroupBox* parent){}
        ~AudioSettings(){}
    
    virtual void readSettings() = 0;
    virtual void writeSettings() = 0;
    virtual void setCurrentSettings() = 0;
    virtual void getCurrentSettings() = 0;
    
    virtual bool isEqual(AudioSettings* as) = 0;
    virtual bool operator==(AudioSettings& as){return isEqual(&as);}
    
    virtual string get_ArchiName() = 0;

    protected slots:
    virtual void linkClicked(const QUrl&){}
    virtual void yaouh(const QString& text){}
};

#endif
