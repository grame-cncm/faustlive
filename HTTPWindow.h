//
//  HTTPWindow.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _HTTPWindow_h
#define _HTTPWindow_h

#include <string>
#include <QMainWindow>
#include <QObject>
#include <QPixmap>

#include "faust/gui/httpdUI.h"
#include "faust/audio/dsp.h"

using namespace std;

class HTTPWindow : public QMainWindow{
    
    Q_OBJECT
    
    private : 
    
    string      IPaddress;
    char*       winTitle;
    httpdUI*    httpdinterface; //Httpd interface for distance control
    QPixmap     myQrCode;
    bool        shortcut;
    
    public : 
    
    //------------------------HTTPD WINDOW ACTIONS
    
    HTTPWindow();
    
    ~HTTPWindow();
    
    void displayQRCode(char* url);
    
    void frontShow_Httpd(string windowTitle);
    
    void display_HttpdWindow(int x, int y);
    
    bool is_httpdWindow_active();
    
    void hide_httpdWindow();

    void search_IPadress();
    
    bool build_httpdInterface(char* error, string windowTitle, dsp* current_DSP);
    
    void launch_httpdInterface();
    
    void toPNG();
    
    virtual void contextMenuEvent(QContextMenuEvent* ev);
    
    void    keyPressEvent(QKeyEvent* event);
    void    keyReleaseEvent(QKeyEvent* event);
    void    closeEvent(QCloseEvent* event);
    
signals:
    void    closeAll();
};

#endif
