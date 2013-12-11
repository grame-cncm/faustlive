//
//  FLToolBar.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLToolBar.h"

#include <sstream>
#include "utilities.h"

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
    
    QLabel* text = new QLabel(tr("FAUST Compiler Options"), fWidget1);
//    text->setAlignment(Qt::AlignCenter);
    
    QLabel* optText = new QLabel(tr("LLVM Optimization"), fWidget1);
//    optText->setAlignment(Qt::AlignCenter);
    
    QLabel* portText = new QLabel(tr("HTTPD Port"), fWidget1);
//    portText->setAlignment(Qt::AlignCenter);
    
    fRemoteCheckBox = new QCheckBox(tr("REMOTE Processing IP"), fWidget1);
    
    fOptionLine = new QLineEdit(tr(""), fWidget1);
    fOptValLine = new QLineEdit(tr(""), fWidget1);
    fPortLine = new QLineEdit(tr(""), fWidget1);
    fRemoteLine = new QLineEdit(tr(""), fWidget1);

    fOptValLine->setMaxLength(3);
    fOptValLine->adjustSize();
    
    fLayout1 = new QVBoxLayout;
    
    fLayout1->addWidget(text);
    fLayout1->addWidget(fOptionLine);
    
    fLayout1->addWidget(optText);
    fLayout1->addWidget(fOptValLine);
    
    fLayout1->addWidget(portText);
    fLayout1->addWidget(fPortLine);

    fLayout1->addWidget(fRemoteCheckBox);
    fLayout1->addWidget(fRemoteLine);
    
    fWidget1->setLayout(fLayout1);
    
    connect (fOptionLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    connect(fOptValLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    connect(fPortLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    connect(fRemoteLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    connect(fRemoteCheckBox, SIGNAL(stateChanged (int)), this, SLOT(sendRemoteProcessing(int)));
}

///*item*/ is useless but QT signal forces the slot parameters

//TRICK to be able to add/remove objects from the toolbar 
void FLToolBar::expansionAction(QTreeWidgetItem * /*item*/){
    
    fWidget1->show();
        
    fAction1 = addWidget(fWidget1);
    
    setOrientation(Qt::Vertical);
    
    adjustSize();
    emit sizeGrowth();
}

void FLToolBar::collapseAction(QTreeWidgetItem* /*item*/){
    
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

//Reaction to enter one of the QlineEdit
void FLToolBar::modifiedOptions(){
    
    string text = fOptionLine->text().toStdString();
    
    string textIP = fRemoteLine->text().toStdString();
    
    int value = 3;
    
    string val = fOptValLine->text().toStdString();
    if(isStringInt(val.c_str()))
        value = atoi(val.c_str());
    
    int port = 5510;
    
    string portText = fPortLine->text().toStdString();
    if(isStringInt(portText.c_str()))
        port = atoi(portText.c_str());
    
//    printf("value = %i// text = %s\n", value, text.c_str());
    
    emit modified(text, value, port, textIP);
}

//Accessors to FLToolBar values
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
    else
        return 0;
}

int FLToolBar::getPort(){
    
    string val = fPortLine->text().toStdString();
    if(isStringInt(val.c_str()))
        return atoi(val.c_str());
    else
        return 0;
}

void FLToolBar::setPort(int port){
    
    stringstream ss;
    ss<<port;
    
    fPortLine->setText(ss.str().c_str());
}

void FLToolBar::setIP(const string& ip){
    
    fRemoteLine->setText(ip.c_str());
}

string FLToolBar::getIP(){
    
    return fRemoteLine->text().toStdString();
}

void FLToolBar::sendRemoteProcessing(int state){
    
    emit remoteStateChanged(state);
}

