//
//  FLErrorWindow.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLErrorWindow.h"

#include <QApplication>
#include <QDesktopWidget>

//-----------------------ERRORWINDOW IMPLEMENTATION

FLErrorWindow::FLErrorWindow(){
    errorText = new QTextEdit(this);
    shortcut = false;
}

FLErrorWindow::~FLErrorWindow(){
    delete errorText;
}

void FLErrorWindow::init_Window(){
    
    widget = new QWidget(this);
    button = new QPushButton(tr("OK"), this);
    button->setDefault(true);
    connect(button, SIGNAL(clicked()), this, SLOT(hideWin()));
    layout = new QVBoxLayout();
    
    QFont font;
    font.setFamily(QString::fromUtf8("Menlo"));
    font.setPointSize(12);
    errorText->setFont(font);
    
    errorText->setReadOnly(true);
    
    QSize screenSize = QApplication::desktop()->geometry().size();
    this->setGeometry(screenSize.width()*3/4 , 0,screenSize.width()/4,screenSize.height()/10);
    this->setCentralWidget(widget);
    
    layout->addWidget(errorText);
    layout->addWidget(button);
    
    widget->setLayout(layout);
}

void FLErrorWindow::closeEvent(QCloseEvent* event){
    
    this->hideWin();
    
    if(shortcut)
        emit closeAll();
}

void FLErrorWindow::hideWin(){
    errorText->setPlainText("");
    hide();
}

void FLErrorWindow::print_Error(const char* text){
    
    QString inter = errorText->toPlainText(); 
    string wholeText = inter.toStdString() +"\n" +text;
    
    errorText->setPlainText(wholeText.c_str());
    this->adjustSize();
    show();
    raise(); 
}

void FLErrorWindow::keyPressEvent(QKeyEvent* event){ 
    
    if(event->key() == Qt::Key_Alt)
        shortcut = true;
}

void FLErrorWindow::keyReleaseEvent(QKeyEvent* event){
    
    if(event->key() == Qt::Key_Alt)
        shortcut = false;
}

