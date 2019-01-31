//
//  FLComponentWindow.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// The component window is a feature allowing the user to compose DSPs in a parallel/sequence/recursive way to create a new DSP. 
// As in the FLWindow, it is possible to drop files and urls, so that you can combine elements from the web with local DSPs.

#ifndef _FLComponentWindow_h
#define _FLComponentWindow_h


#include <QPushButton>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QVector>

#include "smartpointer.h"

class dsp;

class FLComponentItem;

/****************** General window in which DSPs are composed *************/
class FLComponentWindow : public QMainWindow
{
    private: 
    
        Q_OBJECT
    
        QList< QList<FLComponentItem*> >    fItems;
        QVector<QPair<QLabel*, QGroupBox*> > fVerticalElements;
        QHBoxLayout*                        fHComponentLayout;
        FLComponentItem* fFeedBackItem;
        QPushButton*    fSaveB;
    
        void            init();
    
    QList<QList<FLComponentItem*> > componentListWithoutEmptyItem(QList<QList<FLComponentItem*> > items);
    
    private slots:
        void            addComponentRow();
        void            addComponentColumn();
        void            deleteComponentRow();
        void            deleteComponentColumn();
        void            createComponent();
        virtual void    closeEvent(QCloseEvent* event);
        void            cancel();
    
    public:
        		 FLComponentWindow();
        virtual ~FLComponentWindow();   
            
        signals :
            void            newComponent(const QString&);
            void            deleteIt(); 
};

#endif
