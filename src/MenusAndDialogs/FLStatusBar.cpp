//
//  FLStatusBar.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLStatusBar.h"

#include "FLSettings.h"

#include "utilities.h"

#ifdef REMOTE
    #include "faust/dsp/remote-dsp.h"
#endif

//--------------------------FLStatusBar

FLStatusBar::FLStatusBar(QSettings* settings, QWidget* parent) : QStatusBar(parent){

    fSettings = settings;
    
    setAutoFillBackground(true);
    
    init();
    
    setFixedHeight(40);
}

FLStatusBar::~FLStatusBar(){
    setRemoteSettings("local processing", "127.0.0.1", 7777);
}

void FLStatusBar::init(){

#ifdef REMOTE
    fRemoteEnabled = false;
    fRemoteButton = new QPushButton();
    setRemoteSettings(fSettings->value("RemoteProcessing/MachineName", "local processing").toString(), fSettings->value("RemoteProcessing/MachineIP", "127.0.0.1").toString(), fSettings->value("RemoteProcessing/MachinePort", 7777).toInt());
    
    fRemoteButton->setFlat(true);
    
    fRemoteButton->setStyleSheet("*{"
                                 "background-color: transparent;"
                                 "border-color: transparent;"
                                 "padding:0px 0px 0px 0px;"
                                 "margin-left: 5px;"
                                 "}");
    
    fRemoteMenu = new QMenu();
    fRemoteButton->setMenu(fRemoteMenu);
    
    addPermanentWidget(fRemoteButton, 1);
    
    connect(fRemoteMenu, SIGNAL(aboutToShow()), this, SLOT(openMachineMenu()));
#endif

}

//Reaction to a click on the remote enabling button
void FLStatusBar::openMachineMenu(){
    
#ifdef REMOTE    
    fRemoteMenu->clear();
    fIPToHostName.clear();
    
    // Browse the remote machines available
    if(getRemoteDSPMachines(&fIPToHostName)){
        
        // Add localhost to the machine list
        fIPToHostName[string("local processing")] = make_pair("", 0);
        
        map<string, pair <string, int> >::iterator it = fIPToHostName.begin();
        
        while(it!= fIPToHostName.end()){
            
            // Add the machines to the menu passed in parameter 
            QAction* machineAction = new QAction(it->first.c_str(), fRemoteMenu);
            connect(machineAction, SIGNAL(triggered()), this, SLOT(switch_Machine()));
            
            fRemoteMenu->addAction(machineAction); 
            
            it++;
        }
        
        fRemoteMenu->setFixedWidth(fRemoteButton->geometry().width());
    }
#endif
}

void FLStatusBar::setRemoteSettings(const QString& name, const QString& ipServer, int portServer){
    
    fRemoteButton->setText(name);
    fSettings->setValue("RemoteProcessing/MachineName", name);
    fSettings->setValue("RemoteProcessing/MachineIP", ipServer);
    fSettings->setValue("RemoteProcessing/MachinePort", portServer);
}

//Reaction to a click cancellation
void FLStatusBar::remoteFailed(){

    setRemoteSettings(fFormerName, fFormerIp, fFormerPort);
}


//--- Update when new processing machine is chosen
void FLStatusBar::switch_Machine(){
    
#ifdef REMOTE
    QAction* action = qobject_cast<QAction*>(sender());
    string toto(action->text().toStdString());
    
    //    If the server is the same, there is no update
    if(fSettings->value("RemoteProcessing/MachineName", "local processing").toString() != action->text()){
        
        fFormerIp = fSettings->value("RemoteProcessing/MachineIP", "127.0.0.1").toString();
        fFormerPort = fSettings->value("RemoteProcessing/MachinePort", 7777).toInt();
        fFormerName = fRemoteButton->text();
        
        setRemoteSettings(toto.c_str(), fIPToHostName[toto].first.c_str(), fIPToHostName[toto].second);
        
        emit switchMachine();
    }
    
#endif
}



