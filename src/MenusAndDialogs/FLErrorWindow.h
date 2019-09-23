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

#include <QMainWindow>
#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>


class FLErrorWindow : public QMainWindow{
    
    private:
    
        Q_OBJECT
        QVBoxLayout*    fLayout;
        QWidget*        fWidget;
        QTextEdit*      fErrorText;
        QPushButton*    fButton;
    
    public:
    
        FLErrorWindow();
        virtual ~FLErrorWindow();
   
        static FLErrorWindow* _errorWindow;
        static FLErrorWindow* _Instance();
        
        void            init_Window();
        void            print_Error(const QString& text);
        virtual void    closeEvent(QCloseEvent* event);
    
    signals:
        void            closeAll();
    
    public slots:
        void            hideWin();
        void            redirectClose();
};

#endif
