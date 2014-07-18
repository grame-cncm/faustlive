//
//  FLToolBar.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// ToolBar of the FaustLive Windows. 

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
    
    Q_OBJECT
    
    private:
    
        int                 fButtonState;
        QPushButton*        fWindowOptions;
    
        QSettings*          fSettings;
    
        QPushButton*        fSaveButton;
        
        QLineEdit*          fOptionLine;     //Allows the addition of compilation options
        QLineEdit*          fOptValLine;     //And the change of the llvm optimization level
        
        QCheckBox*          fHttpCheckBox;
        QLabel*             fHttpPort;      //Edit port http
        
        QCheckBox*          fOSCCheckBox;
        QLineEdit*          fPortInOscLine;   //Edit osc port
        QLineEdit*          fPortOutOscLine;   //Edit osc port
        QLineEdit*          fPortErrOscLine;
    
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
        bool                wasOscSwitched();
        bool                hasOscOptionsChanged();
        bool                wasHttpSwitched();
        bool                hasRemoteOptionsChanged();
        bool                hasReleaseOptionsChanged();;
    
    public:
    
        FLToolBar(QSettings* settings, QWidget* parent = NULL);
        ~FLToolBar();
    
        void                syncVisualParams();
    
        void    switchHttp(bool on);
        void    switchOsc(bool on);
    
    public slots: 
        void    modifiedOptions();
        void    expansionAction();
        void    collapseAction();
        void    buttonStateChanged();
        void    enableButton(const QString&);
        void    enableButton(int);
    
    signals :
    
        void    oscPortChanged();
        void    compilationOptionsChanged();
        void    sizeGrowth();
        void    sizeReduction();
        void    switch_http(bool on);
        void    switch_osc(bool on);
        void    switch_release(bool on);
};

#endif
