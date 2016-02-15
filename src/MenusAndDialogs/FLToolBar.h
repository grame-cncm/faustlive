//
//  FLToolBar.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLToolBar shapes the settings of the FLWindow. Whenever a change in the settings is applied, the toolbar reacts by emitting signals, connected to the FLWindow.

// Some Remote Control/ Publish features are commented because not finished to be implemented

#ifndef _FLToolBar_h
#define _FLToolBar_h

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

enum{
    fFold,
    fUnFold
};

class FLToolBar : public QToolBar{
     
    private:
    
        Q_OBJECT
    
        int                 fButtonState;
        QPushButton*        fWindowOptions;
    
        QSettings*          fSettings;
    
        QPushButton*        fSaveButton;
        
        QLineEdit*          fOptionLine;        //Allows the addition of compilation options
        QLineEdit*          fOptValLine;        //And the change of the llvm optimization level
    
        QLineEdit*          fAutomaticExportLine;
        QLineEdit*          fScriptLine;
        
        QCheckBox*          fHttpCheckBox;
        QLabel*             fHttpPort;          //Edit http port 
        
        QCheckBox*          fMIDICheckBox;      //MIDI interface
        
        QCheckBox*          fPolyCheckBox;         //Polyphonic support
        QCheckBox*          fPolyGroupCheckBox;    //Polyphonic group
        QLineEdit*          fPolyLine;             //Edit voices number
  
        QCheckBox*          fOSCCheckBox;
        QLineEdit*          fPortInOscLine;     //Edit osc port
        QLineEdit*          fPortOutOscLine;    //Edit osc port
        QLineEdit*          fPortErrOscLine;
    
//        QCheckBox*          fRemoteControlCheckBox;
//        QLabel*             fRemoteControlIP;
    
        QLineEdit*          fCVLine;  
        QLineEdit*          fMTULine;   
        QLineEdit*          fLatLine;  
        QLineEdit*          fDestHostLine;
    
        QCheckBox*          fPublishBox;
    
        QToolBox*           fContainer;  
    
        QAction*            fAction1;
        QAction*            fAction2;
    
        void                init();
    
        bool                hasStateChanged();
        bool                hasCompilationOptionsChanged();
        bool                wasAutomaticExportSwitched();
        bool                hasAutomaticExportChanged();
        bool                hasScriptChanged();
        bool                wasOscSwitched();
        bool                hasOscOptionsChanged();
        bool                wasHttpSwitched();
        bool                wasMIDISwitched();
        bool                wasPolyphonySwitched();
        bool                wasRemoteControlSwitched();
        bool                hasRemoteOptionsChanged();
        bool                hasReleaseOptionsChanged();;
    
    public:
    
        FLToolBar(QSettings* settings, QWidget* parent = NULL);
        virtual ~FLToolBar();
    
        void                syncVisualParams();
    
        void    switchHttp(bool on);
        void    switchOsc(bool on);
        void    switchMIDI(bool on);
        void    switchPoly(bool on);
    
    public slots: 
    
        void    modifiedOptions();
        void    expansionAction();
        void    collapseAction();
        void    buttonStateChanged();
        void    enableButton(const QString&);
        void    enableButton(int);
    
    signals:
    
        void    oscPortChanged();
        void    compilationOptionsChanged();
        void    generateNewAuxFiles();
        void    sizeGrowth();
        void    sizeReduction();
        void    switch_http(bool on);
        void    switch_midi(bool on);
        void    switch_poly(bool on);
        void    switch_osc(bool on);
        void    switch_remotecontrol(bool on);    
        void    switch_release(bool on);
//        void    execScript();
};

#endif
