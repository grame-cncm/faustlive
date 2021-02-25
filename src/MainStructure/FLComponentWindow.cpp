//
//  FLComponentWindow.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#if defined(_WIN32) && !defined(GCC)
# pragma warning (disable: 4100)
#else
# pragma GCC diagnostic ignored "-Wunused-parameter"
# pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include "faust/gui/QTUI.h"
#include "faust/dsp/dsp.h"

#include "QTDefs.h"
#include "FLComponentItem.h"
#include "FLComponentLabel.h"
#include "FLComponentWindow.h"
#include "FLSessionManager.h"
#include "FLNodes.h"
#include "FLWinSettings.h"
#include "utilities.h"

#include "FLErrorWindow.h"
#include "FLMessageWindow.h"

/****************************LAYOUT OPTIMIZATION TREE*****************/

binaryNode* createBestContainerTree(binaryNode* node1, binaryNode* node2)
{
#ifdef QT6
    QSize screenSize = QGuiApplication::primaryScreen()->size();
#else
    QSize screenSize = QApplication::desktop()->geometry().size();
#endif

    int screenWidth = screenSize.width();
    int screenHeight = screenSize.height();
    
    int hWidth = node1->rectSurface().width() + node2->rectSurface().width();
    int hHeight = max(node1->rectSurface().height(), node2->rectSurface().height());
    int hSurface = hWidth * hHeight;
    
    int vWidth = max(node1->rectSurface().width(), node2->rectSurface().width());
    int vHeight = node1->rectSurface().height() + node2->rectSurface().height();
    int vSurface = vWidth * vHeight;
    
//In case vertical group surface is better and doesnt overuse screen space
    if(hSurface < vSurface && hHeight < screenHeight && hWidth < screenWidth)
        return new horizontalNode(node1, node2, QRect( 0, 0, hWidth, hHeight));
//In case horizontal group surface is better but overuse screen space
    else if(vHeight > screenHeight || vWidth > screenWidth)
        return new horizontalNode(node1, node2, QRect( 0, 0, hWidth, hHeight));
//Otherwise
    else
        return new verticalNode(node1, node2, QRect( 0, 0, vWidth, vHeight));
}

//Returns a list of Root Nodes
QList<binaryNode*> createListTrees(QList<FLComponentItem*> components)
{
    QList<binaryNode*> newListTrees;
    
    if(components.size() == 0){}
    else if(components.size() == 1){
        newListTrees.push_back(new leafNode(*(components.begin())));
    }
    else if(components.size() == 2){
        
        binaryNode* componentNode1 = new leafNode(*(components.begin()));
        binaryNode* componentNode2 = new leafNode(*(components.begin()+1));
        
        binaryNode* rootNode = createBestContainerTree(componentNode1, componentNode2);
        newListTrees.push_back(rootNode);
    }
    else{

        QList<FLComponentItem*> restOfComponents(components);
        restOfComponents.pop_front();
        
        FLComponentItem* itemToDispatch = *(components.begin());
        
        newListTrees = dispatchComponentOnListOfTrees(itemToDispatch, createListTrees(restOfComponents));
    }
    
    return newListTrees;
}

QList<binaryNode*> dispatchComponentOnListOfTrees(FLComponentItem* component, QList<binaryNode*> existingTrees)
{
    QList<binaryNode*>newListTrees;
    
    for(QList<binaryNode*>::iterator it = existingTrees.begin(); it != existingTrees.end(); it++){
        
        //        FIRST DISPACTH TREE
        binaryNode* componentNode1 = new leafNode(component);
        
        binaryNode* newRoot1 = createBestContainerTree(componentNode1, *it);
        
        newListTrees.push_back(newRoot1);
        
        //        SECOND DISPATCH TREE
        binaryNode* componentNode2 = new leafNode(component);
        
        binaryNode* intermediateNode = createBestContainerTree(componentNode2, (*it)->left);
        
        binaryNode* newRoot2 = createBestContainerTree(intermediateNode, (*it)->right);
        
        newListTrees.push_back(newRoot2);
    }
    
    return newListTrees;
}

binaryNode* calculateBestDisposition(QList<FLComponentItem*> components)
{
    QList<binaryNode*> binaryTrees = createListTrees(components);
    
    binaryNode* minSurfaceTree = *(binaryTrees.begin());
    
    for(QList<binaryNode*>::iterator it = binaryTrees.begin(); it != binaryTrees.end(); it++){
        int nodeSurface = (*it)->surface();
        if( nodeSurface < minSurfaceTree->surface())
            minSurfaceTree = *it;
    }
    
    return minSurfaceTree;
}

/****************************COMPONENT WINDOW***************************/
FLComponentWindow::FLComponentWindow()
{
    setAcceptDrops(true);
//    setGeometry(300,300, 300,300);
    init();
    setGeometry(0, 0, 800,450);
    centerOnPrimaryScreen(this);
}

FLComponentWindow::~FLComponentWindow()
{
    for(QList<QList<FLComponentItem*> >::iterator it = fItems.begin(); it != fItems.end(); it++ ){
        
        for(QList<FLComponentItem*>::iterator it2 = it->begin(); it2 != it->end(); it2++){
            
            delete *it2;
        }
    }

    delete fSaveB;
    delete fHComponentLayout;
}

void FLComponentWindow::init()
{
    QScrollArea *sa = new QScrollArea(this);
    
    sa->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded);
    sa->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded);
    
    QWidget* vcontainer = new QWidget;
    QVBoxLayout* vlayout = new QVBoxLayout;
    
//------Initialize the feedback label
    QVBoxLayout* feedbackBoxLayout = new QVBoxLayout;
    QGroupBox* feedbackBox = new QGroupBox("Recursive composition");

    fFeedBackItem = new FLComponentItem("Feedback");
    feedbackBoxLayout->addWidget(fFeedBackItem);
    
    feedbackBox->setLayout(feedbackBoxLayout);
    
    QWidget* hcontainer = new QWidget;
    QHBoxLayout* hlayout = new QHBoxLayout;
    
    fHComponentLayout = new QHBoxLayout;
    QGroupBox*  componentGroup = new QGroupBox("Sequentiel composition");
    componentGroup->setLayout(fHComponentLayout);
    
//------Initialize with a 2 items row
    QList<FLComponentItem*> newColumn;
    QGroupBox* newVbox = new QGroupBox("Parallel composition");
    QVBoxLayout* newVboxLayout = new QVBoxLayout;
    newVbox->setLayout(newVboxLayout);
    
    FLComponentItem* item = new FLComponentItem("11");
    newVboxLayout->addWidget(item);
    fHComponentLayout->addWidget(newVbox);
    
    newColumn.push_back(item);
    fItems.push_back(newColumn);
    fVerticalElements.push_back(qMakePair(new QLabel(""), newVbox));
    
    addComponentColumn();
//-------PLUS/MOINS IMAGES
    QPixmap plusImage(":/Images/plus.png");
    QPixmap minusImage(":/Images/minus.png");
    
    QGroupBox*  hbuttongroup = new QGroupBox;
    QHBoxLayout* hbuttonlayout = new QHBoxLayout;
    
    MyLabel *rowplusimage = new MyLabel();
    rowplusimage->setAlignment(Qt::AlignCenter);
    rowplusimage->setPixmap(plusImage.scaled(30, 30, Qt::KeepAspectRatio));
    connect(rowplusimage, SIGNAL(imageClicked()), this, SLOT(addComponentRow()));
    
    MyLabel *rowminusimage = new MyLabel();
    rowminusimage->setAlignment(Qt::AlignCenter);
    rowminusimage->setPixmap(minusImage.scaled(30, 30, Qt::KeepAspectRatio));
    connect(rowminusimage, SIGNAL(imageClicked()), this, SLOT(deleteComponentRow()));
    
    hbuttonlayout->addWidget(rowplusimage);
    hbuttonlayout->addWidget(rowminusimage);
    hbuttongroup->setLayout(hbuttonlayout);
    
    QGroupBox*  vbuttongroup = new QGroupBox;
    QVBoxLayout* vbuttonlayout = new QVBoxLayout;
    
    MyLabel *colplusimage = new MyLabel();
    colplusimage->setAlignment(Qt::AlignCenter);
    colplusimage->setPixmap(plusImage.scaled(30, 30, Qt::KeepAspectRatio));
    connect(colplusimage, SIGNAL(imageClicked()), this, SLOT(addComponentColumn()));
    
    MyLabel *colminusimage = new MyLabel();
    colminusimage->setAlignment(Qt::AlignCenter);
    colminusimage->setPixmap(minusImage.scaled(30, 30, Qt::KeepAspectRatio));
    connect(colminusimage, SIGNAL(imageClicked()), this, SLOT(deleteComponentColumn()));
    
    vbuttonlayout->addWidget(colplusimage);
    vbuttonlayout->addWidget(colminusimage);
    vbuttongroup->setLayout(vbuttonlayout);

//-------Save/Cancel Buttons    
    QWidget* intermediateWidget = new QWidget;
    QHBoxLayout* intermediateLayout = new QHBoxLayout;
    
    QPushButton* cancel = new QPushButton(tr("Cancel"), this);
    cancel->setDefault(false);
    
    fSaveB = new QPushButton(tr("Create Component"), this);
    fSaveB->setDefault(true);
    
    connect(cancel, SIGNAL(released()), this, SLOT(cancel()));
    connect(fSaveB, SIGNAL(released()), this, SLOT(createComponent()));
    
    intermediateLayout->addWidget(cancel);
    intermediateLayout->addWidget(new QLabel(tr("")));
    intermediateLayout->addWidget(fSaveB);
    intermediateWidget->setLayout(intermediateLayout);
    
//------Window Layout
    hlayout->addWidget(componentGroup);
    hlayout->addWidget(vbuttongroup);
    hcontainer->setLayout(hlayout);
    
    vlayout->addWidget(feedbackBox);
    vlayout->addWidget(hcontainer);
    vlayout->addWidget(hbuttongroup);
    vlayout->addWidget(intermediateWidget);
    
    vcontainer->setLayout(vlayout);
//    setCentralWidget(vcontainer);
        
    sa->setWidgetResizable( true );
    sa->setWidget( vcontainer );
    sa->setAutoFillBackground(true);
    
    setCentralWidget(sa);
    
//    adjustSize();
}

//Create a faust program that puts in parallel each column component then compose them sequentially.
void FLComponentWindow::createComponent()
{
    hide();
    fItems = componentListWithoutEmptyItem(fItems);
    
    if(fItems.size() !=0) {
        
        FLMessageWindow::_Instance()->displayMessage("Creating component...");
        FLMessageWindow::_Instance()->show();
        FLMessageWindow::_Instance()->raise();
        
        QString faustToCompile = "";
        
        QList<FLComponentItem*> parallelItems;
        
        int layoutIndex = 1;
        
        for(QList<QList<FLComponentItem*> >::iterator it = fItems.begin(); it != fItems.end(); it++ ){
            
            if(it->size() != 0){
                
                binaryNode* rootDisposition = calculateBestDisposition(*it);
                QString composition = "stereoize(" + rootDisposition->renderToFaust(",", QString::number(layoutIndex)) + ")";
                
                FLComponentItem* parallelItem = new FLComponentItem(composition, rootDisposition->rectSurface());
                parallelItems.push_back(parallelItem);
                
                if(layoutIndex == 1)
                    layoutIndex++;
                else
                    layoutIndex--;
            }
        }
        
        faustToCompile += calculateBestDisposition(parallelItems)->renderToFaust(":", "");
        
        if(fFeedBackItem->faustComponent("") != ""){
            faustToCompile = "import(\"music.lib\");\n\n\nprocess = recursivize("+ faustToCompile + "," + fFeedBackItem->faustComponent("") + ")";
        }
        else{
            faustToCompile = "import(\"music.lib\");\n\n\nprocess = " + faustToCompile;
        }
        
        faustToCompile += ";";
//        
//        printf("Component code = %s\n", faustToCompile.toStdString().c_str());
        
        emit newComponent(faustToCompile);
        FLMessageWindow::_Instance()->hide();
    }
    else
        emit newComponent("");
}

void FLComponentWindow::addComponentRow()
{
    int verticalIndex = 0;
    int horizontalIndex = 0;
    
    for(QList<QList<FLComponentItem*> >::iterator it = fItems.begin(); it != fItems.end(); it++ ){
        QString winIndex = QString::number(horizontalIndex+1) + QString::number(it->size()+1);
        FLComponentItem* item = new FLComponentItem(winIndex);
        fVerticalElements[verticalIndex].second->layout()->addWidget(item);
        it->push_back(item);
        verticalIndex++;
        horizontalIndex++;
    }
}

void FLComponentWindow::deleteComponentRow()
{
    // First Row cannot be deleted
    if(fItems.begin()->size() == 1)
        return;
    
    int index = 0;
    
    for(QList<QList<FLComponentItem*> >::iterator it = fItems.begin(); it != fItems.end(); it++ ){
        
        QList<FLComponentItem*>::iterator it2 = it->end()-1;
        FLComponentItem* item = *(it2);
        it->removeOne(item);
        
        QGroupBox* vBox = fVerticalElements[index].second;
        
        vBox->layout()->removeWidget(item);
        delete item;
        index++;
    }
    
//    adjustSize();
}

void FLComponentWindow::addComponentColumn()
{
    
    QPixmap flecheImage(":/Images/fleche.png");
    
    QLabel* flecheLabel = new QLabel;
    flecheLabel->setPixmap(flecheImage.scaled(30, 30, Qt::KeepAspectRatio));
    
    fHComponentLayout->addWidget(flecheLabel);
    
    QList<FLComponentItem*> newColumn;
    
    QGroupBox* newVbox = new QGroupBox("Parallel composition");
    QVBoxLayout* newVboxLayout = new QVBoxLayout;

    for(int i=0; i<fItems.begin()->size(); i++){
        
        QString index = QString::number(fItems.size()+1) + QString::number(i+1);
        
        FLComponentItem* item = new FLComponentItem(index);
        newVboxLayout->addWidget(item);
        
        newColumn.push_back(item);
    }
    
    newVbox->setLayout(newVboxLayout);
    fHComponentLayout->addWidget(newVbox);
    
    fItems.push_back(newColumn);
    fVerticalElements.push_back(qMakePair(flecheLabel, newVbox));
}

void FLComponentWindow::deleteComponentColumn()
{
    // First Column cannot be deleted
    if(fItems.size() == 1)
        return;
    
    QList<QList<FLComponentItem*> >::iterator it2 = fItems.end()-1;
    
    QList<FLComponentItem*> itemList = *it2;
    
    fItems.removeOne(itemList);
    
    QList<FLComponentItem*>::iterator it = itemList.begin();
    
    while(itemList.size() != 0){
        
        FLComponentItem* item = *it;
        itemList.removeOne(item);
        delete item;
        it = itemList.begin();
    }
    
    QLabel* toEraseLabel = fVerticalElements[fVerticalElements.size()-1].first;
    QGroupBox* toEraseBox = fVerticalElements[fVerticalElements.size()-1].second;
    
    fHComponentLayout->removeWidget(toEraseBox);
    fHComponentLayout->removeWidget(toEraseLabel);
    
    delete toEraseBox->layout();
    delete toEraseBox;
    delete toEraseLabel;
    
    fVerticalElements.pop_back();
//    adjustSize();
}

QList<QList<FLComponentItem*> > FLComponentWindow::componentListWithoutEmptyItem(QList<QList<FLComponentItem*> > items)
{
    QList<QList<FLComponentItem* > > listWithoutEmptyItem;
    
    for(QList<QList<FLComponentItem*> >::iterator it = items.begin(); it != items.end(); it++){
        
        QList<FLComponentItem*> newList;
        
        for(QList<FLComponentItem*>::iterator it2 = it->begin(); it2 != it->end(); it2++){
            
            if((*it2)->faustComponent("1") != "")
                newList.push_back(*it2);
        }
        if(newList.size() != 0)
            listWithoutEmptyItem.push_back(newList);
    }
    
    return listWithoutEmptyItem;
}

void FLComponentWindow::closeEvent(QCloseEvent* /*event*/)
{
    emit deleteIt();
}

void FLComponentWindow::cancel()
{
    emit close();
}




