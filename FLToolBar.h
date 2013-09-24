//
//  FLToolBar.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

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
    
        bool        isStringInt(const char* word);
    
        QTreeWidget*    myTreeWidget;
        QTreeWidgetItem * item;
        QTreeWidgetItem * item2;
        QLineEdit*      optionLine;     //Allows the addition of compilation options
        QLineEdit*      optValLine;     //And the change of the llvm optimization level
    
    
        QWidget*        widget1;
        QWidget*        widget2;
        QLabel*         myText;
        QLabel*         myOptText;
        QVBoxLayout*    layout1;
        QVBoxLayout*    layout2;
        QAction*        action1;
        QAction*        action2;
    
    public:
    
    FLToolBar(QWidget* parent);
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
