//
//  FLMessageWindow.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//
// This Window is the interface to comunicate the errors or other messages to the user 

#ifndef _FLMessageWindow_h
#define _FLMessageWindow_h

#include <string>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

using namespace std;

class FLMessageWindow : public QDialog{
    
    Q_OBJECT
    
    private :
        QLabel*    fMessage;
    
    public :
    
        FLMessageWindow();
        virtual ~FLMessageWindow();   
        
        static FLMessageWindow* _msgWindow;
        static FLMessageWindow* _getInstance();
    
        void   displayMessage(const QString& msg);
};

#endif
