//
//  FLToolBar.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLToolBar.h"

#include <QHeaderView>
#include <sstream>

bool FLToolBar::isStringInt(const char* word){
    
    bool returning = true;
    
    for(int i=0; i<strlen(word); i++){
        if(!isdigit(word[i])){
            returning = false;
            break;
        }
    }
    return returning;
}

//--------------------------FLToolBar

FLToolBar::FLToolBar(QWidget* parent) : QToolBar(parent){
    
    QLineEdit* myLine = new QLineEdit(this);
    myLine->setReadOnly(true);
    myLine->setStyleSheet("*{background-color:transparent; selection-background-color : white; border-color:white;margin: 0px;padding: 0px 0px 0px 0px;spacing: 0px;}*:selected{color:black; border-color : white;margin: 0px;padding: 0px;spacing: 0px;}");
    myLine->setAutoFillBackground(true);
    
    fTreeWidget = new QTreeWidget(myLine);
    fTreeWidget->setAttribute(Qt::WA_MacShowFocusRect, 0);
    
    fTreeWidget->setStyleSheet("*{background-color:transparent; alternate-background-color: white;selection-background-color : white; selection-color : black;border-color:transparent;margin: 0px;padding: 0px 0px 0px 0px;spacing: 0px;} *:item{background-color:transparent;border-color:transparent;margin: 0px;padding: 0px 0px 0px 0px;spacing: 0px;} ");
    
    
    fItem = new QTreeWidgetItem( fTreeWidget, QStringList(QString("Window Options")), QTreeWidgetItem::UserType);
    
    fItem2 = new QTreeWidgetItem( fItem, QStringList(QString("")), QTreeWidgetItem::UserType);
    
    fTreeWidget->setFrameShape(QFrame::NoFrame);
    fTreeWidget->header()->setVisible(false);
    
    
    setOrientation(Qt::Vertical);
    addWidget(myLine);    
    connect(fTreeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(expansionAction(QTreeWidgetItem*)));
    connect(fTreeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(collapseAction(QTreeWidgetItem*)));
    
    fWidget1 = new QWidget;
    fWidget2 = new QWidget;
    fWidget3 = new QWidget;
    
    fText = new QLabel(tr("FAUST Compiler Options"), fWidget1);
    fText->setAlignment(Qt::AlignCenter);
    fOptText = new QLabel(tr("LLVM Optimization"), fWidget2);
    fOptText->setAlignment(Qt::AlignCenter);
    fPortText = new QLabel(tr("HTTPD PORT"), fWidget3);
    fPortText->setAlignment(Qt::AlignCenter);
    
    fOptionLine = new QLineEdit(tr(""), fWidget1);
    fOptValLine = new QLineEdit(tr(""), fWidget2);
    fPortLine = new QLineEdit(tr(""), fWidget3);
    fOptValLine->setMaxLength(3);
    fOptValLine->adjustSize();
    
    fLayout1 = new QVBoxLayout;
    fLayout2 = new QVBoxLayout;
    fLayout3 = new QVBoxLayout;
    
    fLayout1->addWidget(fText);
    fLayout1->addWidget(fOptionLine);
    
    fLayout2->addWidget(fOptText);
    fLayout2->addWidget(fOptValLine);
    
    fLayout3->addWidget(fPortText);
    fLayout3->addWidget(fPortLine);
    
    fWidget1->setLayout(fLayout1);
    fWidget2->setLayout(fLayout2);
    fWidget3->setLayout(fLayout3);
    connect (fOptionLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    connect(fOptValLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    connect(fPortLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
}

//item is useless but QT signal forces the slot parameters
void FLToolBar::expansionAction(QTreeWidgetItem * item){
    
    fWidget1->show();
    fWidget2->show();
    fWidget3->show();
        
    fAction1 = addWidget(fWidget1);
    fAction2 = addWidget(fWidget2);
    fAction3 = addWidget(fWidget3);
    
    setOrientation(Qt::Vertical);
    
    adjustSize();
    emit sizeGrowth();
}

void FLToolBar::collapseAction(QTreeWidgetItem* item){
    
    removeAction(fAction3);
    removeAction(fAction2);
    removeAction(fAction1);
    
    adjustSize();
    emit sizeReduction();
}

FLToolBar::~FLToolBar(){

//    delete fOptionLine;
//    delete fOptValLine;
//    delete fOptText;
//    delete fText;
//    delete fWidget1;
//    delete fWidget2;
//    delete fItem2;
//    delete fItem;
//    delete fTreeWidget;
//    delete fLayout1;
//    delete fLayout2;

}

void FLToolBar::modifiedOptions(){
    
    string text = fOptionLine->text().toStdString();
    
    int value = 3;
    
    string val = fOptValLine->text().toStdString();
    if(isStringInt(val.c_str()))
        value = atoi(val.c_str());
    
    int port = 5510;
    
    string portText = fPortLine->text().toStdString();
    if(isStringInt(portText.c_str()))
        port = atoi(portText.c_str());
    
//    printf("value = %i// text = %s\n", value, text.c_str());
    
    emit modified(text, value, port);
}

void FLToolBar::setOptions(string options){
    fOptionLine->setText(options.c_str());
}

string FLToolBar::getOptions(){
    return fOptionLine->text().toStdString();
}

void FLToolBar::setVal(int value){
    
    stringstream ss;
    ss<<value;
    
    fOptValLine->setText(ss.str().c_str());
}

int FLToolBar::getVal(){
    
    string val = fOptValLine->text().toStdString();
    if(isStringInt(val.c_str()))
        return atoi(val.c_str());
}

int FLToolBar::getPort(){
    
    string val = fPortLine->text().toStdString();
    if(isStringInt(val.c_str()))
        return atoi(val.c_str());
}

void FLToolBar::setPort(int port){
    
    stringstream ss;
    ss<<port;
    
    fPortLine->setText(ss.str().c_str());
}

