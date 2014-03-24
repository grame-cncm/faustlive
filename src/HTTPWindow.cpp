//
//  HTTPWindow.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// HTTPWindow creates a QrCode which correspond to an httpd interface of your faust application so that you can control it within distance. 

#include "HTTPWindow.h"

#include <qrencode.h>

#include "faust/gui/httpdUI.h"
#include "utilities.h"

using namespace std;

//---------------------------HTTPWINDOW IMPLEMENTATION

HTTPWindow::HTTPWindow(){
    
    fShortcut = false;
    fInterface = NULL;
}

HTTPWindow::~HTTPWindow(){
    if(fInterface){
        delete fInterface;
    }
}

//Returns httpdUI Port
int HTTPWindow::get_Port(){
    if(fInterface != NULL)
        return fInterface->getTCPPort();
    else
        return 0;
}
//Brings window on front end and titles the window
void HTTPWindow::frontShow_Httpd(QString windowTitle){
    
    setWindowTitle(windowTitle);
    raise();
    show();
    adjustSize();
}

//bool HTTPWindow::is_httpdWindow_active(){
//    return isActiveWindow();
//}

//void HTTPWindow::hide_httpdWindow(){
//    hide();
//}

//Build Remote control interface
bool HTTPWindow::build_httpdInterface(QString& error, QString windowTitle, dsp* current_DSP, int port){
    
    //Allocation of HTTPD interface
    if(fInterface != NULL) delete fInterface;
    
    QString optionPort = "-port";
    
    char* argv[3];

    argv[0] = (char*)(windowTitle.toLatin1().data());
    argv[1] = (char*)(optionPort.toLatin1().data());
    argv[2] = (char*)(QString::number(port).toStdString().c_str());

    fInterface = new httpdUI(argv[0], 3, argv);

    if(fInterface){
        
        current_DSP->buildUserInterface(fInterface);
        return true;
    }
    
    else{
        error = "ERROR = Impossible to allocate a HTTPD Interface";
        return false;
    }
}

//Launch interface
void HTTPWindow::launch_httpdInterface(){
    fInterface->run();
}

//Right Click reaction = Export to png
void HTTPWindow::contextMenuEvent(QContextMenuEvent* ev){
    
    QMenu *menu = new QMenu();
    
    QAction* exportToPNG = new QAction("Export to PNG",this);
    
    menu->addAction(exportToPNG);
    QAction* act = menu->exec(ev->globalPos());
    
    if(act == exportToPNG){
        printf("EMIT EXPORT PNG \n");
        emit toPNG();
    }
    
    delete exportToPNG;
    delete menu;
    
}

//Tracking for close all shortcut event = ALT + x button
void HTTPWindow::keyPressEvent(QKeyEvent* event){ 
    
    if(event->key() == Qt::Key_Alt) fShortcut = true;
}

void HTTPWindow::keyReleaseEvent(QKeyEvent* event){
    
    if(event->key() == Qt::Key_Alt) fShortcut = false;
}

void HTTPWindow::closeEvent(QCloseEvent* /*event*/){
    
    this->hide();
    
    if(fShortcut) emit closeAll();
}



