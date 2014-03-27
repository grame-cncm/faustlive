//
//  FLExportManager.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//
// FLExportManager is the interface that establish a connection with faustweb-server to export a faust application. 

#ifndef _FLExportManager_h
#define _FLExportManager_h

#include <string>
#include <iostream>
#include <list>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <QtNetwork>

using namespace std;

class FLExportManager : public QObject{
    
    Q_OBJECT
    
    private :
        
        QUrl                fServerUrl;         //Web Service URL        
    
        vector<std::string>                  fPlatforms;     // list of available export platforms
        map<std::string, vector<std::string> >    fTargets;       // plateform -> available targets

        QString              fHome;
        QString              fFileToExport;
        QString              fTemporaryFile;
    
    //Export graphical parameters 
    
        QDialog*            fDialogWindow;  //Export Manager to choose your export parameters
        QDialog*            fMessageWindow; //Window that displays the progress of the export process
    
        QGroupBox*          fMenu2Export;
        QFormLayout*        fMenu2Layout;
        QComboBox*          fExportFormat;
        QComboBox*          fExportPlatform;
        QComboBox*          fExportArchi;
        QComboBox*          fExportChoice;
    
        QTextEdit*          fTextZone;
        QLabel*             fErrorText;
    
        QString             fLastPlatform;
        QString             fLastArchi;
        QString             fLastChoice;
    
    //Dialog for export progress and its graphical elements
    
        QPixmap             fCheckImg;
        QPixmap             fNotCheckImg;
    
        QGridLayout*        fMsgLayout;
        QLabel*             fConnectionLabel;
        QLabel*             fCompilationLabel;
        QProgressDialog*    fPrgBar;
        QLabel*             fCheck1;
        QLabel*             fCheck2;
    
        QPushButton*        fCloseB;
        QPushButton*        fSaveB;
        QPushButton*        fOkB;
        QByteArray          fDataReceived;
    
        void                init_MessageWindow();
        void                init_DialogWindow();
    
    public :
    
        FLExportManager(QString url, QString sessionHome);
        virtual ~FLExportManager();
    
        void                exportFile(QString file);
        void                set_URL(const QString& url);
    
    public slots :
    
        void                postExport();
        void                getFileFromKey(const char* key);
        void                targetsDescriptionReceived();
        void                platformChanged(const QString& index);
        void                saveFile();
};

#endif
