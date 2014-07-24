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

#include "smartpointer.h"

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
        
    private:
    
//    Place in the Component Window for group name
        QString         fIndex;
        
        QString         fSource;
        dsp*            fCompiledDSP;
    
        QVBoxLayout*    fLayout;
        QWidget*        fCurrentWidget;
    
        QString         handleDrop(QDropEvent * event);
    
    public:
        FLComponentItem(const QString& index, QWidget* parent);
        FLComponentItem(const QString& source, QRect rect, QWidget* parent);
        ~FLComponentItem();
            
        QString     source();
    
        void        createInterfaceInRect(const QString& source);
    
        QString     faustComponent();
    
    virtual void dropEvent ( QDropEvent * event );
    virtual void dragEnterEvent ( QDragEnterEvent * event );
    
};


/****************************LAYOUT OPTIMIZATION TREE*****************/
class binaryNode : public smartable{
    
public :
    SMARTP<binaryNode> left;
    SMARTP<binaryNode> right;
    
    //  Representing the surface of the interface
    virtual QRect rectSurface() = 0;
    
    virtual QString renderToFaust(const QString& faustOperator) = 0;
    
    int surface(){
        return rectSurface().width() * rectSurface().height();
    }
};

class treeNode : public binaryNode{

public :
    
    QRect rect;
    
    virtual QRect rectSurface(){
        return rect;
    }
};

class verticalNode : public treeNode{
  
public :
    verticalNode(binaryNode* node1, binaryNode* node2, QRect r){
        left = node1;
        right = node2;
        rect = r;
    }
    
    virtual QString renderToFaust(const QString& faustOperator){
        
        QString faustCode = "vgroup(\"\"," + left->renderToFaust(faustOperator) + faustOperator + right->renderToFaust(faustOperator)+")";
        
        return faustCode;
    }
};

class horizontalNode : public treeNode{

public :
    horizontalNode(binaryNode* node1, binaryNode* node2, QRect r){
        left = node1;
        right = node2;
        rect = r;
    }
    
    virtual QString renderToFaust(const QString& faustOperator){
        
        QString faustCode = "hgroup(\"\"," + left->renderToFaust(faustOperator) + faustOperator + right->renderToFaust(faustOperator)+")";
        
        return faustCode;
    } 
};

class leafNode : public binaryNode{
    
public :
    FLComponentItem* item;
    
    leafNode(FLComponentItem* i){
        item = i;
        left = NULL;
        right = NULL;
    }
    
    virtual QString renderToFaust(const QString& /*faustOperator*/){
        return item->faustComponent();
    }
    
    virtual QRect rectSurface(){
        return item->rect();
    }
};

binaryNode* createBestContainerTree(binaryNode* node1, binaryNode* node2);
QList<binaryNode*> createListTrees(QList<FLComponentItem*> components);
QList<binaryNode*> dispatchComponentOnListOfTrees(FLComponentItem* component, QList<binaryNode*> existingTrees);
binaryNode* calculateBestDisposition(QList<FLComponentItem*> components);

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
