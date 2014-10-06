//
//  FLMessageWindow.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLMessageWindow.h"
#include "utilities.h"

//-----------------------ERRORWINDOW IMPLEMENTATION

FLMessageWindow* FLMessageWindow::_msgWindow = NULL;

FLMessageWindow::FLMessageWindow(){
    
    setWindowFlags(Qt::FramelessWindowHint);
    
    fMessage = new QLabel;
    fMessage->setAlignment(Qt::AlignCenter);
    
    QLabel* tittle = new QLabel(tr("<h2>FAUSTLIVE</h2>"));
    tittle->setAlignment(Qt::AlignCenter);

    QVBoxLayout* layoutSave = new QVBoxLayout;
    
    layoutSave->addWidget(tittle);
    layoutSave->addWidget(new QLabel(tr("")));
    layoutSave->addWidget(fMessage);
    layoutSave->addWidget(new QLabel(tr("")));
    
    setLayout(layoutSave);
    
    centerOnPrimaryScreen(this);
    adjustSize();
}

FLMessageWindow* FLMessageWindow::_Instance(){
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


