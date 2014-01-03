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
        QTreeWidget*        fTreeWidget2;
        QTreeWidgetItem *   fItem;
        QTreeWidgetItem *   fItem2;
        QTreeWidgetItem *   fItem3;
        QTreeWidgetItem *   fItem4;
        QLineEdit*          fOptionLine;     //Allows the addition of compilation options
        QLineEdit*          fOptValLine;     //And the change of the llvm optimization level
        QLineEdit*          fPortLine;
    
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
    
    string getOptions();
    int getVal();
    int getPort();
    
    void    remoteFailed(bool fromNotToRemote);
    
public slots: 
    void    modifiedOptions();
    void    expansionAction(QTreeWidgetItem * item);
    void    collapseAction(QTreeWidgetItem* item);
    void    sendRemoteProcessing(int state);
    void    openRemoteBox();
    
    signals :
    
       void  modified(const string& option, int val, int port);
        void sizeGrowth();
        void sizeReduction();
    
        void remoteStateChanged(int state);
};

#endif
