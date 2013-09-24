//
//  FLExportManager.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _FLExportManager_h
#define _FLExportManager_h

#include <string>

#include <QObject>
#include <QDialog>
#include <QtNetwork>

#include <QGroupBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QString>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QPushButton>

using namespace std;

class FLExportManager : public QObject{
    
    Q_OBJECT
    
    private :
        
        QUrl            fServerUrl;
        string          fFileToExport;
        string          fFilenameToExport;
        string          fFilenameToSave;
    
    //Export graphical parameters 
    
        QDialog*        dialogWindow;
    
        QGroupBox*      menu1Export;
        QGroupBox*      menu2Export;
        QComboBox*      exportFormat;
        QComboBox*      exportPlatform;
        QComboBox*      exportArchi;
        QComboBox*      exportChoice;
    
        QPushButton*    save;
    
    public :
    
        FLExportManager(QUrl server, string file, string filename);
        virtual ~FLExportManager();
    
        void init();
        
    signals:
    
        void             error(const char*);
        void            start_progressing(const char*);
        void            stop_progressing();
        void            processEnded();
    
    private slots :
    
        void            postExport();
        void            exportChecked(bool on);
        void            readKey();
        void            networkError(QNetworkReply::NetworkError msg);
        void            getFileFromKey(const char* key);
        void            endProcess();
};

#endif
