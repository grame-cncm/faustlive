//
//  FLToolBar.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLToolBar.h"

#include "FLSettings.h"

#include "utilities.h"

#ifdef REMOTE
    #include "faust/remote-dsp.h"
#endif

//--------------------------FLToolBar

FLToolBar::FLToolBar(QSettings* settings, QWidget* parent) : QToolBar(parent){
    
    fSettings = settings;
    
    setAutoFillBackground(true);
    
    QLineEdit* myLine = new QLineEdit(this);
    myLine->setReadOnly(true);
    myLine->setStyleSheet("*{background-color:transparent; border-color:white;margin: 0px;padding: 0px 0px 0px 0px;spacing: 0px;}*:selected{color:black; border-color : white;margin: 0px;padding: 0px;spacing: 0px;}");
    myLine->setAutoFillBackground(true);
    
    fTreeWidget = new QTreeWidget(myLine);
    fTreeWidget->setAttribute(Qt::WA_MacShowFocusRect, 0);
    fTreeWidget->setAutoFillBackground(true);
    
    fTreeWidget->setStyleSheet("*{background-color:transparent; border-color:transparent;margin: 0px;padding: 0px 0px 0px 0px;spacing: 0px;} *:item{background-color:transparent;border-color:transparent;margin: 0px;padding: 0px 0px 0px 0px;spacing: 0px;} ");
    
    
    fItem = new QTreeWidgetItem( fTreeWidget, QStringList(QString("Window Options")), QTreeWidgetItem::UserType);
    fItem2 = new QTreeWidgetItem( fItem, QStringList(QString("Window Options")), QTreeWidgetItem::UserType);
    
    fTreeWidget->setFrameShape(QFrame::NoFrame);
    fTreeWidget->header()->setVisible(false);
    
    
    setOrientation(Qt::Vertical);
    addWidget(myLine);    
    connect(fTreeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(expansionAction(QTreeWidgetItem*)));
    connect(fTreeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(collapseAction(QTreeWidgetItem*)));
    
    fWidget1 = new QWidget;
    fWidget1->setStyleSheet("*{background-color:#CECECE;}");
    
    fOptionLine = new QLineEdit(tr(""), fWidget1);
    fOptionLine->setStyleSheet("*{background-color:white;}");
    fOptValLine = new QLineEdit(tr(""), fWidget1);
    fOptValLine->setStyleSheet("*{background-color:white;}");

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
    
#ifdef HTTPCTRL

    fHttpBox = new QGroupBox(tr("Enable Http Remote Interface"));
    fHttpBox->setCheckable(true);
    QVBoxLayout* httpLayout = new QVBoxLayout();
    
    connect(fHttpBox, SIGNAL(toggled(bool)), this, SLOT(redirectHttp(bool)));
    
    fHttpBox->setChecked(settings->value("isHttpOn", FLSettings::getInstance()->value("General/Network/HttpDefaultChecked", false).toBool()).toBool());
    
    fPortLine = new QLineEdit(tr(""), fWidget1);
    httpLayout->addWidget(new QLabel(tr("Http Port"), fWidget1));
    httpLayout->addWidget(fPortLine);
    fPortLine->setStyleSheet("*{background-color:white;}");
    connect(fPortLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
    fHttpBox->setLayout(httpLayout);
    fLayout1->addWidget(fHttpBox);
    
    fOscBox = new QGroupBox(tr("Enable Osc Remote Interface"));
    fOscBox->setCheckable(true);
    
    QVBoxLayout* oscLayout = new QVBoxLayout();
    
    connect(fOscBox, SIGNAL(toggled(bool)), this, SLOT(redirectOsc(bool)));
    
    fOscBox->setChecked(false);
    
    fPortOscLine = new QLineEdit(tr(""), fWidget1);
    fPortOscLine->setStyleSheet("*{background-color:white;}");
    oscLayout->addWidget(new QLabel(tr("Osc Port"), fWidget1));
    oscLayout->addWidget(fPortOscLine);
    connect(fPortOscLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
    fOscBox->setLayout(oscLayout);
    fLayout1->addWidget(fOscBox);
#endif


#ifdef REMOTE
    fRemoteEnabled = false;
    fRemoteButton = new QPushButton();
    setRemote(fSettings->value("MachineName", "local processing").toString(), fSettings->value("MachineIP", "").toString(), fSettings->value("MachinePort", "").toInt());
    
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
        fIPToHostName[string("local processing")] = make_pair("", 0);
        
        map<string, pair <string, int> >::iterator it = fIPToHostName.begin();
        
        while(it!= fIPToHostName.end()){
            
            // Add the machines to the menu passed in parameter 
            QAction* machineAction = new QAction(it->first.c_str(), fRemoteButton->menu());
            connect(machineAction, SIGNAL(triggered()), this, SLOT(update_remoteMachine()));
            
            fRemoteButton->menu()->addAction(machineAction); 
            
            it++;
        }
    }
    
#endif
}

void FLToolBar::setRemote(const QString& name, const QString& ipServer, int port){
    
    printf("SET REMOTE WITH NAME = %s\n", name.toStdString().c_str());
    
    fRemoteButton->setText(name);
    fSettings->setValue("MachineName", name);
    fSettings->setValue("MachineIP", ipServer);
}

//Reaction to a click cancellation
void FLToolBar::remoteFailed(){

    printf("Remote Failed with name = %s\n", fFormerName.toStdString().c_str());
    
    setRemote(fFormerName, fFormerIp, fFormerPort);
}

void FLToolBar::collapseAction(QTreeWidgetItem* /*item*/){
    
    removeAction(fAction1);
    
    adjustSize();
    emit sizeReduction();
    
    setOrientation(Qt::Horizontal);
}

FLToolBar::~FLToolBar(){}

//Reaction to enter one of the QLineEdit
void FLToolBar::modifiedOptions(){
    
    QString text = fOptionLine->text();
    
    QString val = fOptValLine->text();
    
    bool ok;
    int value = val.toInt(&ok);
	if(!ok)
        value = 3;
    
    if(text != (fSettings->value("FaustOptions", "").toString()) || value != fSettings->value("OptValue", 3).toInt()){
        
        fSettings->setValue("OptValue", value); 
        fSettings->setValue("FaustOptions", text); 
        
        sleep(4);
        
        emit compilationOptionsChanged();
    }
#ifdef HTTPCTRL
    int port = fPortLine->text().toInt(&ok);
    
	if(!ok)
        port = 5510;
    
    if(port != fSettings->value("HttpPort", 5510)){
        fSettings->setValue("HttpPort", port); 
        emit httpPortChanged();
    }
    
#endif
#ifdef OSCVAR
    QString portOscText = fPortOscLine->text();
    
    int portOsc = fPortOscLine->text().toInt(&ok);
    
	if(!ok)
        portOsc = 5510;
        
    if(portOsc != fSettings->value("OscPort", 5510)){
        fSettings->setValue("OscPort", portOsc); 
        emit oscPortChanged();
    }
#endif
}

//Accessors to FLToolBar values
void FLToolBar::setOptions(QString options){
    fOptionLine->setText(options);
}

void FLToolBar::setVal(int value){
    
    fOptValLine->setText(QString::number(value));
}

void FLToolBar::setPort(int port){

#ifdef HTTPCTRL
    fPortLine->setText(QString::number(port));
#endif
}

void FLToolBar::setPortOsc(int port){
    
//#ifndef _WIN32 || OSCVAR
#ifdef OSCVAR
    fPortOscLine->setText(QString::number(port));
#endif
}

QString FLToolBar::machineName(){

#ifdef REMOTE
    return fRemoteButton->text();
#else
	return "";
#endif

}

QString FLToolBar::ipServer(){
#ifdef REMOTE
    return fSettings->value("MachineIP", "127.0.0.1").toString();
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
    if(fSettings->value("MachineName", "local processing").toString() != action->text()){
        
        fFormerIp = fSettings->value("MachineIP", "").toString();
        fFormerPort = fSettings->value("MachinePort", 0).toInt();
        fFormerName = fRemoteButton->text();
        
        setRemote(toto.c_str(), fIPToHostName[toto].first.c_str(), fIPToHostName[toto].second);
        
        emit switchMachine();
    }
    
#endif
}

void FLToolBar::redirectHttp(bool on){
    emit switch_http(on);
}

void FLToolBar::switchHttp(bool on){
//#ifndef _WIN32 || HTTPDVAR
#ifdef HTTPCTRL
    fHttpBox->setChecked(on);
#endif
}

bool FLToolBar::isHttpOn(){
//#ifndef _WIN32 || HTTPDVAR
#ifdef HTTPCTRL
    return fHttpBox->isChecked();
#else
    return false;
#endif
}

bool FLToolBar::isOscOn(){
//#ifndef _WIN32 || OSCVAR
#ifdef OSCVAR

    return fOscBox->isChecked();
#else
    return false;
#endif
}

void FLToolBar::redirectOsc(bool on){
    emit switch_osc(on);
}

void FLToolBar::switchOsc(bool on){
//#ifndef _WIN32 || OSCVAR
#ifdef OSCVAR
    fOscBox->setChecked(on);
#endif
}
