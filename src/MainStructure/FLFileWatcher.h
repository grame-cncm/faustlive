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

class FLWindow;

class FLFileWatcher : public QSettings
{
    Q_OBJECT
    
    QFileSystemWatcher*             fWatcher;
    QTimer*                         fSynchroTimer;
    
    QString         fFileChanged;
    
    std::map<QString, QList<FLWindow*> > fMap;
    
    static FLFileWatcher*           _Instance;
    
    private slots:
 
    void         reset_Timer(const QString fileModified);
    void         fileChanged();
    
    
public: 
    
    FLFileWatcher();
    ~FLFileWatcher();

    static FLFileWatcher*           getInstance();
    
    void    startWatcher(const QString& path, FLWindow* win);
    void    stopWatcher(const QString& path, FLWindow* win);
};

#endif
