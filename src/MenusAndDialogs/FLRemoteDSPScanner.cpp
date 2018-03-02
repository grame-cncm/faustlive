//
//  FLRemoteDSPScanner.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

//	This file is not yet used in FaustLive implementation
#include "FLRemoteDSPScanner.h"

#ifdef REMOTE
#include "faust/dsp/remote-dsp.h"
#endif
using namespace std;

/*****************************FLRemoteDSPScanner**********************************/

FLRemoteDSPScanner::FLRemoteDSPScanner(QWidget* parent):QDialog(parent, 0){
    
    setWindowFlags(Qt::FramelessWindowHint);
    
    fCurrentItem = NULL;
    fSHAKey = "";
    
    init();
}

FLRemoteDSPScanner::~FLRemoteDSPScanner(){
    
    QList<QListWidgetItem*> items =  fItemToSHA.keys();
    QList<QListWidgetItem*>::iterator it = items.begin();
    
    while(it != items.end()){
        
        delete *it;
        items.removeOne(*it);
        it = items.begin();
    }
}

void FLRemoteDSPScanner::getVueFromInterfaceList(std::vector<std::pair<std::string, std::string> >  interfaceList, QListWidget* vue){
    
    for(size_t i = 0; i< interfaceList.size() ; i ++){
        QListWidgetItem* item = new QListWidgetItem(interfaceList[i].second.c_str());
        vue->addItem(item);
        fItemToSHA[item] = interfaceList[i].first.c_str();
    }
        
    connect(vue, SIGNAL(itemDoubleClicked( QListWidgetItem *)), this, SLOT(itemDblClick(QListWidgetItem *)));
    
    connect(vue, SIGNAL(itemClicked( QListWidgetItem *)), this, SLOT(itemClick(QListWidgetItem *)));
}

void FLRemoteDSPScanner::init(){
    fLayout = new QVBoxLayout;
    
    std::map<std::string, std::pair<std::string, int> > machineList;
    
//    if(getRemoteDSPMachines(&machineList)){
//        
//        for(std::map<std::string, std::pair<std::string, int> >::iterator it = machineList.begin(); it != machineList.end(); it++){
//            
//            std::vector<std::pair<std::string, std::string> > factoriesList;
//            
//            if(getRemoteFactoriesAvailable(it->second.first, it->second.second, &factoriesList)){
//                
//                QGroupBox* group = new QGroupBox(it->first.c_str());
//                QVBoxLayout* layout = new QVBoxLayout;
//                
//                QListWidget *vue = new QListWidget;
//                
//                getVueFromInterfaceList(factoriesList, vue);
//                
//                group->setLayout(layout);
//                fLayout->addWidget(group);
//            }
            //            else
            //                error : impossible to get Remote Factories available
//        }
//    }
    //    else{
    //        error : impossible to get Remote Machines available
    //    }
    
    QWidget* intermediateWidget = new QWidget;
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    QPushButton* saveButton = new QPushButton;
    connect(saveButton, SIGNAL(clicked()), this, SLOT(openRemoteDSP()));
    
    QPushButton* cancelButton = new QPushButton;
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelDialog()));
    
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(new QLabel(""));
    buttonLayout->addWidget(saveButton);
    
    intermediateWidget->setLayout(buttonLayout);
    fLayout->addWidget(intermediateWidget);
    
    setLayout(fLayout);
}

void FLRemoteDSPScanner::itemDblClick(QListWidgetItem* item){
 
    fCurrentItem = item;
    openRemoteDSP();
}

void FLRemoteDSPScanner::itemClicked(QListWidgetItem* item){
    
    fCurrentItem = item;
}

void FLRemoteDSPScanner::openRemoteDSP(){

    if(fCurrentItem){
        fSHAKey = fItemToSHA[fCurrentItem];
        
        accept();
    }
    else
        reject();
}

void FLRemoteDSPScanner::cancelDialog(){
    reject();
}

QString FLRemoteDSPScanner::shaKey(){
    return fSHAKey;
}

