//
//  FLToolBar.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// ToolBar of the FaustLive Windows. 

#ifndef _FLToolBar_h
#define _FLToolBar_h

#include <string>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
using namespace std;

class FLToolBar : public QToolBar{
    
    Q_OBJECT
    
    private:
    
        QTreeWidget*        fTreeWidget;
        QTreeWidgetItem *   fItem;
        QTreeWidgetItem *   fItem2;

        QLineEdit*          fOptionLine;     //Allows the addition of compilation options
        QLineEdit*          fOptValLine;     //And the change of the llvm optimization level
        QLineEdit*          fPortLine;      //Edit port http
        QLineEdit*          fPortOscLine;   //Edit osc port
    
        QMenu*              fRemoteMenu;
        QPushButton*        fRemoteButton;
        bool                fRemoteEnabled;
    
        QWidget*            fWidget1;;  
        QVBoxLayout*        fLayout1;
        QAction*            fAction1;
    
    public:
    
    FLToolBar(QWidget* parent = NULL);
    ~FLToolBar();
    
    void setOptions(string options);
    void setVal(int value);
    void setPort(int port);
    void setPortOsc(int port);
    
    string getOptions();
    int getVal();
    int getPort();
    int getPortOsc();
    
    void    remoteFailed(bool fromNotToRemote);
    
    void    setRemoteButtonName(const string& name);
public slots: 
    void    modifiedOptions();
    void    expansionAction(QTreeWidgetItem * item);
    void    collapseAction(QTreeWidgetItem* item);
    void    openRemoteBox();
    
    signals :
    
       void  modified(const string& option, int val, int port, int portOsc);
        void sizeGrowth();
        void sizeReduction();
        void update_Menu(QMenu*);
        void remoteStateChanged(int state);
};

#endif
