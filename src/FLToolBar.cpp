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

#ifdef REMOTE
    #include "faust/remote-dsp.h"
#endif

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
    
    fOptionLine = new QLineEdit(tr(""), fWidget1);
    fOptValLine = new QLineEdit(tr(""), fWidget1);

    fOptValLine->setMaxLength(3);
    fOptValLine->adjustSize();
    
    fLayout1 = new QVBoxLayout;
    
    fLayout1->addWidget(new QLabel(tr("FAUST Compiler Options"), fWidget1));
    fLayout1->addWidget(fOptionLine);
    connect (fOptionLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));

    fLayout1->addWidget(new QLabel(tr("LLVM Optimization"), fWidget1));
    fLayout1->addWidget(fOptValLine);    
    connect(fOptValLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
//    fPublishBox = new QCheckBox(tr("Publish Faust Application"));
//    fLayout1->addWidget(fPublishBox);
//    connect(fPublishBox, SIGNAL(checkStateSet()), this, SLOT());
    
#ifndef _WIN32 || HTTPDVAR
    
    fHttpBox = new QGroupBox(tr("Enable Http Remote Interface"));
    fHttpBox->setCheckable(true);
    QVBoxLayout* httpLayout = new QVBoxLayout();
    
    connect(fHttpBox, SIGNAL(toggled(bool)), this, SLOT(redirectHttp(bool)));
    
    fHttpBox->setChecked(false);
    
    fPortLine = new QLineEdit(tr(""), fWidget1);
    httpLayout->addWidget(new QLabel(tr("Http Port"), fWidget1));
    httpLayout->addWidget(fPortLine);
    connect(fPortLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
    fHttpBox->setLayout(httpLayout);
    fLayout1->addWidget(fHttpBox);
    
    fOscBox = new QGroupBox(tr("Enable Osc Remote Interface"));
    fOscBox->setCheckable(true);
    
    QVBoxLayout* oscLayout = new QVBoxLayout();
    
    connect(fOscBox, SIGNAL(toggled(bool)), this, SLOT(redirectOsc(bool)));
    
    fOscBox->setChecked(false);
    
    fPortOscLine = new QLineEdit(tr(""), fWidget1);
    oscLayout->addWidget(new QLabel(tr("Osc Port"), fWidget1));
    oscLayout->addWidget(fPortOscLine);
    connect(fPortOscLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
    fOscBox->setLayout(oscLayout);
    fLayout1->addWidget(fOscBox);
#endif


#ifdef REMOTE
    fIpRemoteServer = "localhost";
    fRemoteEnabled = false;
    fRemoteButton = new QPushButton();
    fRemoteButton->setText(tr("local processing"));
    
    fRemoteMenu = new QMenu();
    fRemoteButton->setMenu(fRemoteMenu);
    
    addWidget(fRemoteButton);
    
    connect(fRemoteMenu, SIGNAL(aboutToShow()), this, SLOT(openRemoteBox()));
#endif
    
    fWidget1->setLayout(fLayout1);
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

//Reaction to a click on the remote enabling button
void FLToolBar::openRemoteBox(){
    
#ifdef REMOTE    
    fRemoteButton->menu()->clear();
    fIPToHostName.clear();
    
    // Browse the remote machines available
    if(getRemoteMachinesAvailable(&fIPToHostName)){
        
        // Add localhost to the machine list
        fIPToHostName[string("local processing")] = make_pair("localhost", 0);
        
        map<string, pair <string, int> >::iterator it = fIPToHostName.begin();
        
        while(it!= fIPToHostName.end()){
            
            printf("IPOFHOSTNAME = %s\n", it->second.first.c_str());
            
            // Add the machines to the menu passed in parameter 
            QAction* machineAction = new QAction(it->first.c_str(), fRemoteButton->menu());
            connect(machineAction, SIGNAL(triggered()), this, SLOT(update_remoteMachine()));
            
            fRemoteButton->menu()->addAction(machineAction); 
            
            it++;
        }
    }
    
#endif
}

void FLToolBar::setRemoteButtonName(const QString& name){
    
    fRemoteButton->setText(name);
}

//Reaction to a click cancellation
void FLToolBar::remoteFailed(){

    fIpRemoteServer = fFormerIp;
    fPortRemoteServer = fFormerPort;
}

void FLToolBar::remoteSuccessfull(){
    setRemoteButtonName(fNewName);
}

void FLToolBar::setNewOptions(const QString& ip, int port, const QString& newName){
    
    fFormerIp = fIpRemoteServer;
    fFormerPort = fPortRemoteServer;
    fFormerName = fRemoteButton->text();
    
    fIpRemoteServer = ip;
    fPortRemoteServer = port;
    fNewName = newName;
    
}

void FLToolBar::collapseAction(QTreeWidgetItem* /*item*/){
    
    removeAction(fAction1);
    
    adjustSize();
    emit sizeReduction();
    
    setOrientation(Qt::Horizontal);
}

FLToolBar::~FLToolBar(){}

//Reaction to enter one of the QlineEdit
void FLToolBar::modifiedOptions(){
    
    QString text = fOptionLine->text();
    
    int value = 3;
    
    QString val = fOptValLine->text();
	if(isStringInt(val.toLatin1().data()))
        value = atoi(val.toStdString().c_str());
  
#ifndef _WIN32 || HTTPDVAR
    int port = 5510;
    
    QString portText = fPortLine->text();
	if(isStringInt(portText.toStdString().c_str()))
		port = atoi(portText.toStdString().c_str());
    
    int portOsc = 5510;
    
    QString portOscText = fPortOscLine->text();
	if(isStringInt(portOscText.toStdString().c_str()))
		portOsc = atoi(portOscText.toStdString().c_str());
#endif
    
//    printf("value = %i// text = %s\n", value, text.c_str());
    
    emit modified(text, value, port, portOsc);
}

//Accessors to FLToolBar values
void FLToolBar::setOptions(QString options){
    fOptionLine->setText(options);
}

QString FLToolBar::getOptions(){
    return fOptionLine->text();
}

void FLToolBar::setVal(int value){
    
    stringstream ss;
    ss<<value;
    
    fOptValLine->setText(ss.str().c_str());
}

int FLToolBar::getVal(){
    
    QString val = fOptValLine->text();
	if(isStringInt(val.toStdString().c_str()))
		return atoi(val.toStdString().c_str());
    else
        return 0;
}

int FLToolBar::getPort(){

#ifndef _WIN32 || HTTPDVAR
    QString val = fPortLine->text();
	if(isStringInt(val.toStdString().c_str()))
		return atoi(val.toStdString().c_str());
    else
#endif
        return 0;
}

void FLToolBar::setPort(int port){
#ifndef _WIN32 || HTTPDVAR
    stringstream ss;
    ss<<port;
    
    fPortLine->setText(ss.str().c_str());
#endif
}

int FLToolBar::getPortOsc(){
    
#ifndef _WIN32 || OSCVAR    
    QString val = fPortOscLine->text();
	if(isStringInt(val.toStdString().c_str()))
		return atoi(val.toStdString().c_str());
    else
#endif
        return 0;
}

void FLToolBar::setPortOsc(int port){
    
#ifndef _WIN32 || OSCVAR
    stringstream ss;
    ss<<port;
    
    fPortOscLine->setText(ss.str().c_str());
#endif
}

QString FLToolBar::machineName(){

#ifdef REMOTE
    return fRemoteButton->text();
#else
	return "";
#endif

}

//--- Update when new processing machine is chosen
void FLToolBar::update_remoteMachine(){
    
#ifdef REMOTE
    QAction* action = qobject_cast<QAction*>(sender());
    string toto(action->text().toStdString());
    
    //    If the server is the same, there is no update
    if((fIpRemoteServer.toStdString()).compare((fIPToHostName[toto]).first) == 0)
        return;
    else{
        
        fFormerIp = fIpRemoteServer;
        fFormerPort = fPortRemoteServer;
        fFormerName = fRemoteButton->text();
        
        fIpRemoteServer = fIPToHostName[toto].first.c_str();
        fPortRemoteServer = fIPToHostName[toto].second;
        
        printf("IP clicked = %s || %i\n", fIpRemoteServer.toLatin1().data(), fPortRemoteServer);
        
        fNewName = toto.c_str();
        
        emit switchMachine(fIpRemoteServer, fPortRemoteServer);
    }
    
#endif
}

void FLToolBar::redirectHttp(bool on){
    emit switch_http(on);
}

void FLToolBar::switchHttp(bool on){
#ifndef _WIN32 || HTTPDVAR
    fHttpBox->setChecked(on);
#endif
}

bool FLToolBar::isHttpOn(){
#ifndef _WIN32 || HTTPDVAR
    return fHttpBox->isChecked();
#else
    return false;
#endif
}

bool FLToolBar::isOscOn(){
#ifndef _WIN32 || OSCVAR
    return fOscBox->isChecked();
#else
    return false;
#endif
}

void FLToolBar::redirectOsc(bool on){
    emit switch_osc(on);
}

void FLToolBar::switchOsc(bool on){
#ifndef _WIN32 || OSCVAR
    fOscBox->setChecked(on);
#endif
}