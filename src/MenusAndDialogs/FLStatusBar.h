//
//  FLStatusBar.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// ToolBar of the FaustLive Windows. 

#ifndef _FLStatusBar_h
#define _FLStatusBar_h

#include <string>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
using namespace std;

class FLStatusBar : public QStatusBar{
    
    private:
    
        Q_OBJECT
        QSettings*          fSettings;
    
        QMenu*              fRemoteMenu;
        QPushButton*        fRemoteButton;
        bool                fRemoteEnabled;
        map<string, pair<string, int> > fIPToHostName;  //Correspondance of remote machine IP to its name
    
        QString             fFormerIp;
        int                 fFormerPort;
        QString             fFormerName;
    
        void                init();
    
    public:
    
        FLStatusBar(QSettings* settings, QWidget* parent = NULL);
        ~FLStatusBar();
    
        void        setNewOptions(const QString& ip, int port, const QString& newName);
    
        void        remoteFailed();
        void        setRemoteSettings(const QString& name, const QString& ipServer, int portServer);
    
    public slots: 
        void        openMachineMenu();
        void        switch_Machine();    
    
    signals:
        void        switchMachine();
};

#endif
