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

#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QAction>

using namespace std;

class FLToolBar : public QToolBar{
    
    Q_OBJECT
    
    private:
    
        bool                isStringInt(const char* word);
    
        QTreeWidget*        fTreeWidget;
        QTreeWidgetItem *   fItem;
        QTreeWidgetItem *   fItem2;
        QLineEdit*          fOptionLine;     //Allows the addition of compilation options
        QLineEdit*          fOptValLine;     //And the change of the llvm optimization level
    
    
        QWidget*            fWidget1;
        QWidget*            fWidget2;
        QLabel*             fText;
        QLabel*             fOptText;
        QVBoxLayout*        fLayout1;
        QVBoxLayout*        fLayout2;
        QAction*            fAction1;
        QAction*            fAction2;
    
    public:
    
    FLToolBar(QWidget* parent = NULL);
    ~FLToolBar();
    
    void setOptions(string options);
    void setVal(int value);
    
    string getOptions();
    int getVal();
    
    
public slots: 
    void    modifiedOptions();
    void    expansionAction(QTreeWidgetItem * item);
    void    collapseAction(QTreeWidgetItem* item);
    
    signals :
    
       void  modified(string option, int val);
        void sizeChanged();
    
};

#endif
