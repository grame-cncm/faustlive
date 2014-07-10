//
//  FaustLiveApp.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLApp is the centerpiece of FaustLive. The class controls all the windows, menu and actions of a user. 

#ifndef _FLPreferenceWindow_h
#define _FLPreferenceWindow_h

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include "AudioCreator.h"

class FLPreferenceWindow : public QDialog
{
    Q_OBJECT
    
    QLineEdit*          fCompilModes;
    QLineEdit*          fOptVal;
    QLineEdit*          fServerLine;
    QLineEdit*          fPortLine;
    QCheckBox*          fHttpAuto;
    
    AudioCreator*       fAudioCreator;
    QGroupBox*          fAudioBox;
    
    void                init();
    void                resetVisualObjects();
public: 

    FLPreferenceWindow(QWidget * parent = 0);
    
    ~FLPreferenceWindow();

    private slots:
    
    void save();
    void cancel();
    void styleClicked();
    
signals:
    
    void newStyle(const QString&);
    void urlChanged();
    void dropPortChange();
};

#endif
