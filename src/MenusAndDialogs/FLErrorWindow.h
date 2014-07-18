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
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

using namespace std;

class FLErrorWindow : public QMainWindow{
    
    Q_OBJECT
    
    private :
        QVBoxLayout*    fLayout;
        QWidget*        fWidget;
        QTextEdit*      fErrorText;
        QPushButton*    fButton;
    
    public :
    
        FLErrorWindow();
        virtual ~FLErrorWindow();
   
        static FLErrorWindow* _errorWindow;
        static FLErrorWindow* _getInstance();
        
        void            init_Window();
        void            print_Error(const QString& text);
        virtual void    closeEvent(QCloseEvent* event);
    
    signals :
        void            closeAll();
    
    public slots :
        void            hideWin();
        void            redirectClose();
};

#endif
