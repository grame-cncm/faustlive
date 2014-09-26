//
//  FaustLiveApp.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLFileWatcher.h"

#include "FLWindow.h"
#include "utilities.h"

FLFileWatcher* FLFileWatcher::_fileWatcher = 0;
//----------------------CONSTRUCTOR/DESTRUCTOR---------------------------

FLFileWatcher::FLFileWatcher(){

    fWatcher = new QFileSystemWatcher;
    fTempWatcher = new QFileSystemWatcher;
    fSynchroTimer = new QTimer();
    connect(fSynchroTimer, SIGNAL(timeout()), this, SLOT(fileChanged()));
    
    connect(fWatcher, SIGNAL(fileChanged(const QString)), this, SLOT(reset_Timer(const QString)));
    
    connect(fTempWatcher, SIGNAL(fileChanged(const QString)), this, SLOT(reset_Temp_Timer(const QString)));
    
    connect(fWatcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(dirChanged(const QString&)));
}

FLFileWatcher::~FLFileWatcher(){}

FLFileWatcher* FLFileWatcher::_Instance(){
    
    if(FLFileWatcher::_fileWatcher == 0)
        FLFileWatcher::_fileWatcher = new FLFileWatcher();
    
    return FLFileWatcher::_fileWatcher;
}

void FLFileWatcher::startWatcher(QVector<QString> paths, FLWindow* win){
    
    for(int i = 0; i<paths.size(); i++){
    
        printf("START PATH = %s\n", paths[i].toStdString().c_str());
        
        QString path = paths[i];
        
        if(path != ""){
            fWatcher->addPath(path);
            
            QList<FLWindow*> list = fMap[path];
            list.push_back(win);
            
            printf("Size of Path List = %i\n", list.size());
            
            fMap[path] = list;
            
            //      Watches the changes in the folder containing the file... in case of name changes
            QString absolutePath = QFileInfo(path).absolutePath();
            
            QStringList filters;
            filters << "*.dsp"<<"*.lib";
            
            QDir path(absolutePath);
            fDirToChildren[absolutePath] = path.entryList(filters, QDir::Files | QDir::NoDotAndDotDot);
            
            fWatcher->addPath(absolutePath);
        }
    }
}

void FLFileWatcher::startTempWatcher(const QString& path, FLWindow* win){
    
    if(path != ""){
        fTempWatcher->addPath(path);
    
        QList<FLWindow*> list = fMap[path];
        list.push_back(win);
        
        fMap[path] = list;
    }
}

void FLFileWatcher::stopWatcher(QVector<QString> paths, FLWindow* win){
    
    for(int i = 0; i<paths.size(); i++){
        
        QString path = paths[i];

        if(path != ""){
            
            QList<FLWindow*> list = fMap[path];
            list.removeOne(win);
            
            fMap[path] = list;
        }
    }
}

void FLFileWatcher::dirChanged(const QString& dirModified){
    
    QStringList oldChildren = fDirToChildren[dirModified];
    
    QStringList filters;
    filters << "*.dsp"<<"*.lib";
    
    QDir path(dirModified);
    QStringList newChildren =  path.entryList(filters, QDir::Files | QDir::NoDotAndDotDot);
    
//    In case of deleted or renamed file (added file is ignored)
    if(oldChildren.size()>=newChildren.size()){
        
        QStringList filesWatched = fWatcher->files();
        
        QString oldName("");
        QString newName("");
        
        for(QStringList::iterator it = filesWatched.begin(); it != filesWatched.end(); it++){
//        Find modified file
            if(!QFileInfo(*it).exists()){
                oldName = *it;
                
//            If a file was deleted, it's not worth going name searching
                if(oldChildren.size()>newChildren.size())
                    break;
                
//            Search in newList the one File that is not in the old list
                for(QStringList::iterator it2 = newChildren.begin(); it2 != newChildren.end(); it2++){
                    if(oldChildren.indexOf(*it2) == -1){
                        newName = *it2;
                        break;
                    }
                }
                
                break;
            }
        }
        
        QList<FLWindow*> list = fMap[oldName];
        
        QList<FLWindow*>::iterator it;
        
        for(it = list.begin() ; it != list.end(); it++){
            
            if(newName == "")
                (*it)->source_Deleted();
            else{
                QString nameModified = dirModified + "/" + newName;
                (*it)->update_Window(nameModified);
            }
        }
        
    }
    
    fDirToChildren[dirModified] = newChildren;
}

void FLFileWatcher::reset_Timer(const QString fileModified){

    if(!QFileInfo(fileModified).exists()){
        fSynchroTimer->stop();
        return;
    }
    
    fSourceToChanged = fileModified;
    fWinChanged =  fMap[fileModified];
    
    //If the signal is triggered multiple times in 2 second, only 1 is taken into account
    if(fSynchroTimer->isActive()){
        fSynchroTimer->stop();
        fSynchroTimer->start(2000);
    }
    else
        fSynchroTimer->start(2000);
}

void FLFileWatcher::reset_Temp_Timer(const QString fileModified){
    
    fSourceToChanged = pathToContent(fileModified);
    fWinChanged =  fMap[fileModified];
    
    //If the signal is triggered multiple times in 2 second, only 1 is taken into account
    if(fSynchroTimer->isActive()){
        fSynchroTimer->stop();
        fSynchroTimer->start(2000);
    }
    else
        fSynchroTimer->start(2000);
}

void FLFileWatcher::fileChanged(){
    
    fSynchroTimer->stop();

    QList<FLWindow*>::iterator it;
        
    for(it = fWinChanged.begin() ; it != fWinChanged.end(); it++)
        (*it)->update_Window(fSourceToChanged);
}



