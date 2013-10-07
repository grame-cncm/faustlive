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

#include <QtGui>
#include <QtNetwork>

using namespace std;

class FLExportManager : public QObject{
    
    Q_OBJECT
    
    private :
        
        QUrl            fServerUrl;
        QLineEdit*       fServIPLine;
    
        string          fHome;
        string          fFileToExport;
        string          fFilenameToExport;
        string          fFilenameToSave;
    
    //Export graphical parameters 
    
        QDialog*        fDialogWindow;
    
        QGroupBox*      fMenu1Export;
        QGroupBox*      fMenu2Export;
        QComboBox*      fExportFormat;
        QComboBox*      fExportPlatform;
        QComboBox*      fExportArchi;
        QComboBox*      fExportChoice;
    
        QPushButton*    fSaveButton;
    
        void            writeURL(QUrl server);
        QUrl            readURL();
    
    public :
    
        FLExportManager(string sessionHome, string file, string filename);
        virtual ~FLExportManager();
    
        void init();
        
    signals:
    
        void            error(const char*);
        void            start_progressing(const char*);
        void            stop_progressing();
        void            processEnded();
    
    private slots :
    
        void            cancelDialog();
        void            postExport();
        void            exportChecked(bool on);
        void            readKey();
        void            networkError(QNetworkReply::NetworkError msg);
        void            getFileFromKey(const char* key);
        void            endProcess();
};

#endif
