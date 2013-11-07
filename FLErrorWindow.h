//
//  FLErrorWindow.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//
// This Window is the interface to comunicate the errors or other messages to the user 

#ifndef _FLErrorWindow_h
#define _FLErrorWindow_h

#include <string>
#include <QtGui>

using namespace std;

class FLErrorWindow : public QMainWindow{
    
    Q_OBJECT
    
    private :
        QVBoxLayout*    fLayout;
        QWidget*        fWidget;
        QTextEdit*      fErrorText;
        bool            fShortcut;
        QPushButton*    fButton;
    
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
