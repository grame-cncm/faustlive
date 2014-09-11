//
//  FLPresentationWindow.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//
// This window is the presentation menu of FaustLive

#ifndef _FLPresentationWindow_h
#define _FLPresentationWindow_h

#include <string>
#include <map>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

using namespace std;

class FLPresentationWindow : public QDialog{
    
    Q_OBJECT
    
    private :
    
        QString     fExampleToOpen;
    
    public :
    
        FLPresentationWindow();
        virtual ~FLPresentationWindow();
    
        void    init();
    
        virtual void closeEvent(QCloseEvent* event);
    
    public slots:
    //Store the item clicked to open it when the open button is pressed
        void    itemClick(QListWidgetItem *item);
        void    itemChosen();
    
    //Opens directly a double clicked item
        void itemDblClick(QListWidgetItem* item);
    
        void    hideWindow();
    
        void    newWindow();
        void    openWindow();
        void    session();
        void    pref();
        void    help();
    
    signals:
        void    openExample(const QString&);
        void    newWin();
        void    openWin();
        void    openSession();
        void    openPref();
        void    openHelp();
};

#endif
