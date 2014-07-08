//
//  FaustLiveApp.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLFileWatcher.h"

#include "FLWindow.h"

FLFileWatcher* FLFileWatcher::_Instance = 0;
//----------------------CONSTRUCTOR/DESTRUCTOR---------------------------

FLFileWatcher::FLFileWatcher(){

    fWatcher = new QFileSystemWatcher;
    fSynchroTimer = new QTimer(fWatcher);
    connect(fSynchroTimer, SIGNAL(timeout()), this, SLOT(fileChanged()));
    
    connect(fWatcher, SIGNAL(fileChanged(const QString)), this, SLOT(reset_Timer(const QString)));
}

FLFileWatcher::~FLFileWatcher(){}

FLFileWatcher* FLFileWatcher::getInstance(){
    
    if(FLFileWatcher::_Instance == 0)
        FLFileWatcher::_Instance = new FLFileWatcher();
    
    return FLFileWatcher::_Instance;
}

void FLFileWatcher::startWatcher(const QString& path, FLWindow* win){

    fWatcher->addPath(path);
    
    if(path != ""){
        
        QList<FLWindow*> list = fMap[path];
        list.push_back(win);
        
        fMap[path] = list;
    }
}

void FLFileWatcher::stopWatcher(const QString& path, FLWindow* win){
    
    if(path != ""){
        
        QList<FLWindow*> list = fMap[path];
        list.removeOne(win);
        
        fMap[path] = list;
    }
}

void FLFileWatcher::reset_Timer(const QString fileModified){

    fFileChanged = fileModified;
    
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
    
    QList<FLWindow*> list = fMap[fFileChanged];
    
    QList<FLWindow*>::iterator it;
    
    for(it = list.begin() ; it != list.end(); it++){
        (*it)->update_Window(fFileChanged);
    }
}



