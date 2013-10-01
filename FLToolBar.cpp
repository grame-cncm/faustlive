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
    
    myTreeWidget = new QTreeWidget(myLine);
    myTreeWidget->setAttribute(Qt::WA_MacShowFocusRect, 0);
    
    myTreeWidget->setStyleSheet("*{background-color:transparent; alternate-background-color: white;selection-background-color : white; selection-color : black;border-color:transparent;margin: 0px;padding: 0px 0px 0px 0px;spacing: 0px;} *:item{background-color:transparent;border-color:transparent;margin: 0px;padding: 0px 0px 0px 0px;spacing: 0px;} ");
    
    
    item = new QTreeWidgetItem( myTreeWidget, QStringList(QString("Compilation Options")), QTreeWidgetItem::UserType);
    
    item2 = new QTreeWidgetItem( item, QStringList(QString("")), QTreeWidgetItem::UserType);
    
    myTreeWidget->setFrameShape(QFrame::NoFrame);
    myTreeWidget->header()->setVisible(false);
    
    
    setOrientation(Qt::Vertical);
    addWidget(myLine);    
    connect(myTreeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(expansionAction(QTreeWidgetItem*)));
    connect(myTreeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(collapseAction(QTreeWidgetItem*)));
    
    widget1 = new QWidget;
    widget2 = new QWidget;
    
    myText = new QLabel(tr("FAUST Options "), widget1);
    myText->setAlignment(Qt::AlignCenter);
    myOptText = new QLabel(tr("LLVM Optimization "), widget2);
    myOptText->setAlignment(Qt::AlignCenter);
    
    optionLine = new QLineEdit(tr(""), widget1);
    optValLine = new QLineEdit(tr(""), widget2);
    optValLine->setMaxLength(3);
    optValLine->adjustSize();
    
    layout1 = new QVBoxLayout;
    layout2 = new QVBoxLayout;
    
    layout1->addWidget(myText);
    layout1->addWidget(optionLine);
    
    layout2->addWidget(myOptText);
    layout2->addWidget(optValLine);
    
    widget1->setLayout(layout1);
    widget2->setLayout(layout2);
    connect (optionLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    connect(optValLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
}

//item is useless but QT signal forces the slot parameters
void FLToolBar::expansionAction(QTreeWidgetItem * item){
    
    widget1->show();
    widget2->show();
    
    action1 = addWidget(widget1);
    action2 = addWidget(widget2);
    
    setOrientation(Qt::Vertical);
    
    adjustSize();
    emit sizeChanged();
}

void FLToolBar::collapseAction(QTreeWidgetItem* item){
    
    removeAction(action2);
    removeAction(action1);
    
    adjustSize();
    emit sizeChanged();
}

FLToolBar::~FLToolBar(){
    
    //    delete layout1;
    //    delete layout2;
    //    delete optionLine;
    //    delete optValLine;
    //    delete myOptText;
    //    delete myText;
    //    delete widget1;
    //    delete widget2;
    //    delete item2;
    //    delete item;
    //    delete myTreeWidget;
}

void FLToolBar::modifiedOptions(){
    
    string text = optionLine->text().toStdString();
    
    int value = 3;
    
    string val = optValLine->text().toStdString();
    if(isStringInt(val.c_str()))
        value = atoi(val.c_str());
    
    printf("value = %i// text = %s\n", value, text.c_str());
    
    emit modified(text, value);
}

void FLToolBar::setOptions(string options){
    optionLine->setText(options.c_str());
}
void FLToolBar::setVal(int value){
    
    stringstream ss;
    ss<<value;
    
    optValLine->setText(ss.str().c_str());
}

string FLToolBar::getOptions(){
    return optionLine->text().toStdString();
}
int FLToolBar::getVal(){
    
    string val = optValLine->text().toStdString();
    if(isStringInt(val.c_str()))
        return atoi(val.c_str());
}
