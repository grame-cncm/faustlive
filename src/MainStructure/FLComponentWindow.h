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

class dsp;

class MyLabel : public QLabel{
    Q_OBJECT

public:
    MyLabel(QWidget* parent = NULL){}
    ~MyLabel(){}
    
    virtual void mouseReleaseEvent( QMouseEvent * event ){
        emit imageClicked();
    }
    
    signals:
        void imageClicked();
};

class FLComponentItem : public QWidget{
    
    Q_OBJECT
        
    private:;
        
        QString         fSource;
        dsp*            fCompiledDSP;
    
        QVBoxLayout*    fLayout;
        QWidget*        fCurrentWidget;
    
        QString         handleDrop(QDropEvent * event);
    
    public:
        FLComponentItem(QWidget* parent);
        ~FLComponentItem();
            
        QString     source();
    
        void        createInterfaceInRect(const QString& source);
    
    virtual void dropEvent ( QDropEvent * event );
    virtual void dragEnterEvent ( QDragEnterEvent * event );
    
};

class FLComponentWindow : public QMainWindow
{
    Q_OBJECT
    
    private : 
    
        QList< QList<FLComponentItem*> >    fItems; 
    
        QVector<QPair<QLabel*, QGroupBox*> > fVerticalElements;
        QHBoxLayout*                        fHComponentLayout;
    
        QPushButton*    fSaveB;
    
        void            init();
    
    private slots:
    
        void            addComponentRow();
        void            addComponentColumn();
        void            deleteComponentRow();
    
        void            deleteComponentColumn();
        void            createComponent();
        virtual void    closeEvent(QCloseEvent* event);
        void            cancel();
    
    public :
    
    //####CONSTRUCTOR

    FLComponentWindow();
    virtual ~FLComponentWindow();   
    
        
    signals :
        void            newComponent(const QString&);
        void            deleteIt(); 
};

#endif
