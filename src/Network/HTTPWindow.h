//
//  HTTPWindow.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// HTTPWindow creates a QrCode which correspond to an httpd interface of your faust application so that you can control it within distance. 

#ifndef _HTTPWindow_h
#define _HTTPWindow_h

#include <string>

#include <QMainWindow>

#include "faust/dsp/dsp.h"

using namespace std;

class HTTPWindow : public QMainWindow {
    
    private:
    
        Q_OBJECT
          
        void        frontShow_Httpd(QString windowTitle);
        
        virtual void contextMenuEvent(QContextMenuEvent* ev);
        
        void        closeEvent(QCloseEvent* event);
    
    public: 
        
        //------------------------HTTPD WINDOW ACTIONS
        
        HTTPWindow();
        virtual ~HTTPWindow();
         
    signals:
    
        void        closeAll();
        void        toPNG();
};

#endif
