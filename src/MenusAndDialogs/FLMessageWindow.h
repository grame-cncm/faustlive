//
//  FLMessageWindow.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//
// This Window is the interface to comunicate messages to the user

#ifndef _FLMessageWindow_h
#define _FLMessageWindow_h

#include <string>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

using namespace std;

class FLMessageWindow : public QDialog{
    
    private:
    
        Q_OBJECT
        QLabel*    fMessage;
    
    public:
    
        FLMessageWindow();
        virtual ~FLMessageWindow();   
        
        static FLMessageWindow* _msgWindow;
        static FLMessageWindow* _Instance();
    
        void   displayMessage(const QString& msg);
};

#endif
