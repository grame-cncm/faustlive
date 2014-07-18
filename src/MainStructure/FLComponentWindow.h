//
//  FLWindow.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FAUSTLIVE WINDOW. This class describes the behavior of a window that contains a DSP. 
// Its principal characteristics are : 
//      - to accept drag'n drop
//      - to accept right click
//      - to enable a control within distance of its interface through http protocol (see HTTPDWindow)

#ifndef _FLComponentWindow_h
#define _FLComponentWindow_h

#include <string>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

class FLComponentWindow : public QMainWindow
{
    Q_OBJECT
    
    private : 
    
        QRectF fFirstRect;
        QRectF fSecondRect;
    
        QString         fFirstSource;
        QString         fSecondSource;
        QString         fSourceFolder;
    
        QPushButton*    fSaveB;
    
        QLabel*          fFirstLabel;
        QLabel*          fSecondLabel;
    
        void            init();
    
        QString         handleDrop(QDropEvent * event);
    
    private slots:
    
        void            createComponent();
        void            cancel();
    
    public :
    
    //####CONSTRUCTOR

    FLComponentWindow(const QString& sourceFolder);
        virtual ~FLComponentWindow();   
    
//    void paintEvent(QPaintEvent *e);
    
    virtual void dropEvent ( QDropEvent * event );
    
    virtual void dragEnterEvent ( QDragEnterEvent * event );
        
    signals :
        void            newComponent(const QString&);
        void            deleteIt(); 
};

#endif
