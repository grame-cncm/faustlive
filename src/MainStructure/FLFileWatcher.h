//
//  FaustLiveApp.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLApp is the centerpiece of FaustLive. The class controls all the windows, menu and actions of a user. 

#ifndef _FLFileWatcher_h
#define _FLFileWatcher_h

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include <map>
#include <iostream>
#include <string>

using namespace std;

class FLWindow;


class FLFileWatcher : public QSettings
{
    Q_OBJECT
    
    QFileSystemWatcher*             fWatcher;
//    QFileSystemWatcher*             fTempWatcher;
    
    QTimer*                         fSynchroTimer;
    
    QString                         fSourceToChanged;
    QList<FLWindow*>                fWinChanged;
    
    std::map<QString, QList<FLWindow*> > fMap;
    
    
//    Map the dir 
    QMap<QString, QList<QString> >       fDirToChildren;
    
    static FLFileWatcher*           _fileWatcher;
    
    private slots:
 
    void         reset_Timer(const QString fileModified);
//    void         reset_Temp_Timer(const QString fileModified);
    void         dirChanged(const QString&);
    void         fileChanged();
    
public: 
    
    FLFileWatcher();
    ~FLFileWatcher();

    static FLFileWatcher*           _Instance();
    
    void    startWatcher(QVector<QString> paths, FLWindow* win);
//    void    startTempWatcher(const QString& path, FLWindow* win);
    void    stopWatcher(QVector<QString> paths, FLWindow* win);
};

#endif
