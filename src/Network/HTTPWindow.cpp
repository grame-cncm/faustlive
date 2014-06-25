//
//  HTTPWindow.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// HTTPWindow creates a QrCode which correspond to an httpd interface of your faust application so that you can control it within distance. 

#include "HTTPWindow.h"

#include "utilities.h"

using namespace std;

//---------------------------HTTPWINDOW IMPLEMENTATION

HTTPWindow::HTTPWindow(){}

HTTPWindow::~HTTPWindow(){}

//Brings window on front end and titles the window
void HTTPWindow::frontShow_Httpd(QString windowTitle){
    
    setWindowTitle(windowTitle);
    raise();
    show();
    adjustSize();
}

//Right Click reaction = Export to png
void HTTPWindow::contextMenuEvent(QContextMenuEvent* ev){
    
    QMenu *menu = new QMenu();
    
    QAction* exportToPNG = new QAction("Export to PNG",this);
    
    menu->addAction(exportToPNG);
    QAction* act = menu->exec(ev->globalPos());
    
    if(act == exportToPNG)
        emit toPNG();
    
    delete exportToPNG;
    delete menu;
    
}

void HTTPWindow::closeEvent(QCloseEvent* /*event*/){
    
    this->hide();
    
    if(QApplication::keyboardModifiers() == Qt::AltModifier) emit closeAll();
}



