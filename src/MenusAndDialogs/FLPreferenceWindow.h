//
//  FLPreferenceWindow.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// The role of the preference Window is to display the preferences of FaustLive and to handle the changes of settings. 

#ifndef _FLPreferenceWindow_h
#define _FLPreferenceWindow_h

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include "AudioCreator.h"

class FLPreferenceWindow : public QDialog
{
    private:
    
        Q_OBJECT
        
        QLineEdit*          fCompilModes;
        QLineEdit*          fOptVal;
        QLineEdit*          fServerLine;
        QLineEdit*          fRemoteServerLine;
        QLineEdit*          fPortLine;
        QCheckBox*          fHttpAuto;
        QCheckBox*          fOscAuto;
        QCheckBox*          fMIDIAuto;
        QCheckBox*          fPolyAuto;
        
        AudioCreator*       fAudioCreator;
        QGroupBox*          fAudioBox;
        
        void                init();
        void                resetVisualObjects();
        
    public: 

        FLPreferenceWindow(QWidget* parent = 0);
        virtual  ~FLPreferenceWindow();

        static FLPreferenceWindow* _prefWindow;
        static FLPreferenceWindow* _Instance();
        
    private slots:
        
        void save();
        void cancel();
        void styleClicked();
        virtual void closeEvent(QCloseEvent* event);
        
    signals:
        
        void newStyle(const QString&);
        void urlChanged();
        void dropPortChange();
        void remoteServerPortChanged();
};

#endif
