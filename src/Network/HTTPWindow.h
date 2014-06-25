//
//  HTTPWindow.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// HTTPWindow creates a QrCode which correspond to an httpd interface of your faust application so that you can control it within distance. 

#ifndef _HTTPWindow_h
#define _HTTPWindow_h

//#include <string>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include <QtNetwork>
//
//#include "faust/audio/dsp.h"


//using namespace std;

class HTTPWindow : public QMainWindow{
    
    Q_OBJECT
    
    private : 
    
    public : 
    
    //------------------------HTTPD WINDOW ACTIONS
    
    HTTPWindow();
    ~HTTPWindow();
    
    void        frontShow_Httpd(QString windowTitle);
    
    virtual void contextMenuEvent(QContextMenuEvent* ev);
    
    void        closeEvent(QCloseEvent* event);
    
signals:
    void        closeAll();
    void        toPNG();
};

#endif
