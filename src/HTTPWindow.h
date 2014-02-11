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

#include "faust/audio/dsp.h"

class httpdUI;

//using namespace std;

class HTTPWindow : public QMainWindow{
    
    Q_OBJECT
    
    private : 
    
    QString      fIPaddress;     //IP adress
    QString      fEntireAddress; //IP + Port adress
    QString      fTitle;         //Htpp Window Title
    httpdUI*    fInterface;     //Httpd interface for distance control
    QPixmap     fQrCode;
    
    bool        fShortcut;      //In case ALT + x button is pressed, shortcut is activated
    
    public : 
    
    //------------------------HTTPD WINDOW ACTIONS
    
    HTTPWindow();
    
    ~HTTPWindow();
    
    void        displayQRCode(QString url);
    
    void        frontShow_Httpd(QString windowTitle);
    
    void        display_HttpdWindow(int x, int y, int port);
    
    bool        is_httpdWindow_active();
    
    void        hide_httpdWindow();

    void        search_IPadress();
    QString      getUrl();
        int         get_Port();
    
    bool        build_httpdInterface(QString& error, QString windowTitle, dsp* current_DSP, int port);
    
    void        launch_httpdInterface();
    
    void        toPNG();
    
    virtual void contextMenuEvent(QContextMenuEvent* ev);
    
    void        keyPressEvent(QKeyEvent* event);
    void        keyReleaseEvent(QKeyEvent* event);
    void        closeEvent(QCloseEvent* event);
    
signals:
    void        closeAll();
};

#endif
