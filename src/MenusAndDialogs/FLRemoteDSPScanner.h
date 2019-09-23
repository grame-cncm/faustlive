//
//  FLRemoteDSPScanner.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//
// FLRemoteDSPScanner is the interface that establish a connection to discover the remote compilation services.
// It could then be possible to create a remote instance of a published remote factory


//	This file is not yet used in FaustLive implementation

#ifndef _FLRemoteDSPScanner_h
#define _FLRemoteDSPScanner_h

#include <string>
#include <iostream>
#include <list>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include <QtNetwork>

using namespace std;

class FLRemoteDSPScanner : public QDialog{
    
    private:
    
        Q_OBJECT
        QVBoxLayout*            fLayout;
        QListWidgetItem*        fCurrentItem;
        QMap<QListWidgetItem*, QString>    fItemToSHA;
        QString                 fSHAKey;
    
        void    getVueFromInterfaceList(std::vector<std::pair<std::string, std::string> >  interfaceList, QListWidget* vue);
        void    init();
    
    public:
    
        FLRemoteDSPScanner(QWidget* parent = NULL);
        virtual ~FLRemoteDSPScanner();
    
        QString  shaKey();
    
    public slots:
    
        void    itemDblClick(QListWidgetItem* item);    
        void    itemClicked(QListWidgetItem* item);    
        void    cancelDialog();
        void    openRemoteDSP();
};

#endif





