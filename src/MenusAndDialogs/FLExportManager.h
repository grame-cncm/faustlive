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

class FLTargetChooser : public QDialog{
   Q_OBJECT
    
private:
    
    static FLTargetChooser*    _targetChooser;
    
    vector<string>                  fPlatforms;     // list of available export platforms
    map<string, vector<string> >    fTargets;       // plateform -> available targets
    
    QNetworkReply*      fTargetReply;
    
//    Graphical elements
    QGroupBox*          fMenu2Export;
    QGridLayout*        fMenu2Layout;
    QComboBox*          fExportFormat;
    QComboBox*          fExportPlatform;
    QComboBox*          fExportArchi;
    QComboBox*          fExportChoice;
    
    QLabel*             fErrorText;
    
//    Saving the user choices 
    QString             fLastPlatform;
    QString             fLastArchi;
    QString             fLastChoice;
    
    void                init();
    void                sendTargetRequest();
public:
    
    FLTargetChooser(QWidget* parent = NULL);
    virtual ~FLTargetChooser();
    
    static FLTargetChooser* _Instance();
    
    QString         platform();
    QString         architecture();
    QString         binOrSource();
    
    public slots:
    
    void            acceptDialog();
    void            cancelDialog();
    
    void                abortReply();
    void            targetsDescriptionReceived(); 
    void            platformChanged(const QString& index);
    void            setLastState();
    virtual void    closeEvent(QCloseEvent* event);
    
};

class FLExportManager : public QDialog{
    Q_OBJECT
    
private:
    
    QString              fAppName;
    QString              fCodeToSend;
    
    //Saving repository chosen by the user
    QString				fLastOpened;
    
    //Target characteristics
    QString             fUrl;
    QString             fPlatform;
    QString             fArchi;
    QString             fChoice;
    
    //Expected network replies 
    QNetworkReply *     fPostReply;
    QNetworkReply*      fGetKeyReply;
    
    int                 fStep;  // To know what step has crashed the export
    
    QByteArray          fDataReceived;
    
    //Dialog for export progress and its graphical elements
    QGridLayout*        fMsgLayout;
    QLabel*             fConnectionLabel;
    QLabel*             fCompilationLabel;
    QProgressDialog*    fPrgBar;
    QLabel*             fCheck1;
    QLabel*             fCheck2;
    
    QTextEdit*          fTextZone;
    
    QPixmap             fCheckImg;
    QPixmap             fNotCheckImg;
    
    QPushButton*        fSaveB;   
    QPushButton*        fOkB;
    
    void                init();
    void                abortReply(QNetworkReply* reply);

    static FLExportManager*    _exportManager;
    
public:
    FLExportManager();
    ~FLExportManager();
    static FLExportManager*    _Instance();
    
    void            exportFile(const QString& name, const QString& faustCode, const QString& p, const QString& a, const QString& sb);
    
    public slots:
    
    void            postExport();
    void            readKey();
    void            networkError(QNetworkReply::NetworkError msg);
    void            getFileFromKey(const char* key);
    void            saveFileOnDisk();
    void            showSaveB();
    void            redirectAbort();
    virtual void    closeEvent(QCloseEvent* event);
  
};

#endif
