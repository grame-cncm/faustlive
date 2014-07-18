//
//  FLMessageWindow.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLMessageWindow.h"

//-----------------------ERRORWINDOW IMPLEMENTATION

FLMessageWindow* FLMessageWindow::_msgWindow = NULL;

FLMessageWindow::FLMessageWindow(){
    
    setWindowFlags(Qt::FramelessWindowHint);
    
    fMessage = new QLabel;
    fMessage->setAlignment(Qt::AlignCenter);
    fMessage->setStyleSheet("*{color: black}");
    
    QLabel* tittle = new QLabel(tr("<h2>FAUSTLIVE</h2>"));
    tittle->setAlignment(Qt::AlignCenter);

    QVBoxLayout* layoutSave = new QVBoxLayout;
    
    layoutSave->addWidget(tittle);
    layoutSave->addWidget(new QLabel(tr("")));
    layoutSave->addWidget(fMessage);
    layoutSave->addWidget(new QLabel(tr("")));
    
    setLayout(layoutSave);
    
    QSize screenSize = QApplication::desktop()->geometry().size(); 
    int screenWidth = screenSize.width();
    int screenHeight = screenSize.height();
    
    move((screenWidth-width())/2, (screenHeight-height())/2);
    adjustSize();
}

FLMessageWindow* FLMessageWindow::_getInstance(){
    if(_msgWindow == NULL)
        _msgWindow = new FLMessageWindow;
    
    return _msgWindow;
}

FLMessageWindow::~FLMessageWindow(){
    delete fMessage;
}

void FLMessageWindow::displayMessage(const QString& msg){
    fMessage->setText(msg);
}


