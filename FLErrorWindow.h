//
//  FLErrorWindow.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _FLErrorWindow_h
#define _FLErrorWindow_h

#include <string>
#include <QMainWindow>
#include <QObject>
#include <QVBoxLayout>
#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QKeyEvent>
#include <QCloseEvent>

using namespace std;

//To display easily the errors during the execution
class FLErrorWindow : public QMainWindow{
    
    Q_OBJECT
    
    private :
        QVBoxLayout*    layout;
        QWidget*        widget;
        QTextEdit*      errorText;
        bool            shortcut;
        QPushButton*    button;
    
    public :
    
        FLErrorWindow();
        virtual ~FLErrorWindow();
    
        void            init_Window();
        void            print_Error(const char* text);
        virtual void    closeEvent(QCloseEvent* event);
    
        virtual void    keyPressEvent(QKeyEvent* event);
        virtual void    keyReleaseEvent(QKeyEvent* event);
    
    signals :
        void            closeAll();
    
    public slots :
        void            hideWin();
};

#endif
