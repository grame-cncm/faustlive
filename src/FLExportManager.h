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
    
    QString              fHome;
    QString              fFileToExport;
    QString              fFilenameToSave;
    QString              fCodeToSend;
    
    //Export graphical parameters 
    QNetworkReply*      fTargetReply;
    QNetworkReply *     fPostReply;
    QNetworkReply*      fGetKeyReply;
    
    QDialog*        fDialogWindow;  //Export Manager to choose your export parameters
    QDialog*        fMessageWindow; //Window that displays the progress of the export process
    
    QGroupBox*          fMenu2Export;
    QGridLayout*        fMenu2Layout;
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
    
    int                 fStep;  // To know what step has crashed the export
    
    void                continueProgress(QCheckBox* toCheckBox , string followingMsg);
    
    void                init_MessageWindow();
    void                init_DialogWindow();
    void                abortReply(QNetworkReply* reply);
    
    public :
    
    FLExportManager(string url, string sessionHome, string file, string filename);
    FLExportManager(QString url, QString sessionHome);
    virtual ~FLExportManager();
    
    void                exportFile(QString file, QString faustCode);
    void                set_URL(const QString& url);
    
    public slots :
    
    void            postExport();
    void            readKey();
    void            networkError(QNetworkReply::NetworkError msg);
    void            getFileFromKey(const char* key);
    void            saveFileOnDisk();
    void            targetsDescriptionReceived();
    void            platformChanged(const QString& index);
    void            showSaveB();
    void            setLastState();
    void            redirectAbort();
};

#endif
