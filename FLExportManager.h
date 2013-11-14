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
    
        vector<string>                  fPlatforms;     // list of available export platforms
        map<string, vector<string> >    fTargets;       // plateform -> available targets

        string              fHome;
        string              fFileToExport;
        string              fFilenameToExport;
        string              fFilenameToSave;
    
    //Export graphical parameters 
    
        QDialog*        fDialogWindow;  //Export Manager to choose your export parameters
        QDialog*        fMessageWindow; //Window that displays the progress of the export process
    
        QGroupBox*          fMenu2Export;
        QFormLayout*        fMenu2Layout;
        QComboBox*          fExportFormat;
        QComboBox*          fExportPlatform;
        QComboBox*          fExportArchi;
        QComboBox*          fExportChoice;
    
    
    //Dialog for export progress and its graphical elements
    
        QFormLayout*        fMsgLayout;
        QLabel*             fConnectionLabel;
        QLabel*             fRemoteComp;
        QProgressDialog*    fPrgBar;
        QLabel*             fCheck1;
        QLabel*             fCheck2;
    
        QPushButton*        fCloseB;
        QPushButton*        fSaveB;
        QByteArray          fDataReceived;
    
        int                 fStep;  // To know what step has crashed the export
    
        void                display_progress();
        void                continueProgress(QCheckBox* toCheckBox , string followingMsg);
    
    public :
    
        FLExportManager(string url, string sessionHome, string file, string filename);
        virtual ~FLExportManager();
    
        void init();
    
    public slots :
    
        void            postExport();
        void            readKey();
        void            networkError(QNetworkReply::NetworkError msg);
        void            getFileFromKey(const char* key);
        void            saveFileOnDisk();
        void            targetsDescriptionReceived();
        void            platformChanged(const QString& index);
        void            StopProgressSlot();
        void            showSaveB();
};

#endif
