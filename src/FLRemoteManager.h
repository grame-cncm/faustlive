//
//  FLRemoteManager.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//
// FLRemoteManager is the interface that scans the network to find remote processing service

#ifndef _FLRemoteManager_h
#define _FLRemoteManager_h

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <QtNetwork>

using namespace std;

class FLRemoteManager : public QObject{
    
    Q_OBJECT
    
    private :
    
    
    public :
    
    FLRemoteManager();
    ~FLRemoteManager();
    
    bool init();
    
    public slots :

    
};

#endif
