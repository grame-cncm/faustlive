//
//  FLFILEWATCHER.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLFileWatcher takes care of placing watchers over DSP files and their dependencies to make sure
// FaustLive is synchronized with its files 
// Moreover, if a file is deleted, moved or renamed, the filewatcher handles it

#ifndef _FLFileWatcher_h
#define _FLFileWatcher_h

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include <map>
#include <iostream>
#include <string>


class FLWindow;

class FLFileWatcher : public QSettings
{

    private:
    
        Q_OBJECT
        
        QFileSystemWatcher*             fWatcher;
        
        QTimer*                         fSynchroTimer;
        
        QString                         fSourceToChanged;
        QList<FLWindow*>                fWinChanged;
        
        std::map<QString, QList<FLWindow*> > fMap;
        
    //    Map the dir 
        QMap<QString, QList<QString> >  fDirToChildren;
        
        static FLFileWatcher*           _fileWatcher;
        
        private slots:
     
        void         reset_Timer(const QString fileModified);

        void         dirChanged(const QString&);
        void         fileChanged();
    
    public: 
        
        FLFileWatcher();
        ~FLFileWatcher();

        static FLFileWatcher*           _Instance();
        
        void    startWatcher(QVector<QString> paths, FLWindow* win);
        void    stopWatcher(QVector<QString> paths, FLWindow* win);
};

#endif
