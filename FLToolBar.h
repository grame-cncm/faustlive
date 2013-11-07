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

using namespace std;

class FLToolBar : public QToolBar{
    
    Q_OBJECT
    
    private:
    
        QTreeWidget*        fTreeWidget;
        QTreeWidgetItem *   fItem;
        QTreeWidgetItem *   fItem2;
        QLineEdit*          fOptionLine;     //Allows the addition of compilation options
        QLineEdit*          fOptValLine;     //And the change of the llvm optimization level
        QLineEdit*          fPortLine;
    
    
        QWidget*            fWidget1;
        QWidget*            fWidget2;
        QWidget*            fWidget3;
        QLabel*             fText;
        QLabel*             fOptText;
        QLabel*             fPortText;
        QVBoxLayout*        fLayout1;
        QVBoxLayout*        fLayout2;
        QVBoxLayout*        fLayout3;
        QAction*            fAction1;
        QAction*            fAction2;
        QAction*            fAction3;
    
    public:
    
    FLToolBar(QWidget* parent = NULL);
    ~FLToolBar();
    
    void setOptions(string options);
    void setVal(int value);
    void setPort(int port);
    
    string getOptions();
    int getVal();
    int getPort();
    
    
public slots: 
    void    modifiedOptions();
    void    expansionAction(QTreeWidgetItem * item);
    void    collapseAction(QTreeWidgetItem* item);
    
    signals :
    
       void  modified(string option, int val, int port);
        void sizeGrowth();
        void sizeReduction();
    
};

#endif
