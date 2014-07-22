//
//  FLWindow.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "faust/llvm-dsp.h"
#include "faust/gui/faustqt.h"

#include "FLComponentWindow.h"
#include "FLSessionManager.h"
#include "FLWinSettings.h"

/****************************COMPONENT ITEM***************************/
FLComponentItem::FLComponentItem(QWidget* parent = NULL) : QWidget(parent){
   
    setAcceptDrops(true);
    
    fSource = "";
    fLayout = new QVBoxLayout;
    
    QLabel *image = new QLabel("<h2>DROP YOUR FAUST DSP</h2>");
    
    fLayout->addWidget(image);
    fCurrentWidget = image;
    
    setLayout(fLayout);
    
    fCompiledDSP = NULL;
}

FLComponentItem::~FLComponentItem(){
    
    FLSessionManager* sessionManager = FLSessionManager::_Instance();
    
    if(fCompiledDSP)
        sessionManager->deleteDSPandFactory(fCompiledDSP);
}

QString FLComponentItem::source(){
    if(fSource == "")
        fSource = "process = _;";
    return fSource;
}

void FLComponentItem::dragEnterEvent ( QDragEnterEvent * event ){
    
    if (event->mimeData()->hasFormat("text/uri-list")){
        
        QList<QString>    sourceList;
        QList<QUrl> urls = event->mimeData()->urls();
        QList<QUrl>::iterator i;
        
        for (i = urls.begin(); i != urls.end(); i++) {
            if(QFileInfo(i->toLocalFile()).completeSuffix() == "dsp" || i->toString().indexOf("http://") == 0)
                event->acceptProposedAction();
        }
    }
    if(event->mimeData()->hasFormat("text/plain")){
        
        if(event->mimeData()->text().indexOf("http://") == 0 || event->mimeData()->text().indexOf("file://") == 0)
            event->acceptProposedAction();
    }
}

QString FLComponentItem::handleDrop(QDropEvent * event){
    
    QList<QUrl> urls = event->mimeData()->urls();
    QList<QUrl>::iterator i;
    
    QString sourceToSave("");
    
    if (event->mimeData()->hasFormat("text/uri-list")){
        for (i = urls.begin(); i != urls.end(); i++){
            if(i->isLocalFile())
                sourceToSave = i->toLocalFile();
            else
                sourceToSave = i->toString();
        }
    }
    else if(event->mimeData()->hasFormat("text/plain") && sourceToSave == "")
        sourceToSave = event->mimeData()->text();
    
    
    return sourceToSave;
}

void FLComponentItem::createInterfaceInRect(const QString& source){
    
    QString errorMsg("");
    
    FLSessionManager* sessionManager = FLSessionManager::_Instance();
    
    printf("Source to compile = %s\n", source.toStdString().c_str());
    
    QPair<QString, void*> factorySetts = sessionManager->createFactory(source, NULL, errorMsg);
    
    if(factorySetts.second == NULL){
        printf("Is factory Not Compiled %s\n", errorMsg.toStdString().c_str());
        //ICI IL FAUT AFFICHER "COMPILING ERROR DANS LE CARRE"
        return;
    }
    
    fCompiledDSP = sessionManager->createDSP(factorySetts, NULL, NULL, NULL, errorMsg);
    
    if (fCompiledDSP == NULL)
        return;
    
    fSource = source;
//    setTitle(QFileInfo(fSource).baseName());
    
    
    QTGUI* interface = new QTGUI(this);
    fCompiledDSP->buildUserInterface(interface);
    interface->setEnabled(false);
    interface->resize(150,100);
//    interface->run();
    
    delete fLayout;
    delete fCurrentWidget;
    fLayout = new QVBoxLayout;
    fLayout->addWidget(interface);
    setLayout(fLayout);
    
    fCurrentWidget = interface;
}

void FLComponentItem::dropEvent ( QDropEvent * event ){
    
    QString source = handleDrop(event);
        
    if(source != ""){
        event->acceptProposedAction();
        createInterfaceInRect(source);
    }
}

/****************************COMPONENT WINDOW***************************/
FLComponentWindow::FLComponentWindow(){
    
    setAcceptDrops(true);
    
    setGeometry(300,300, 300,300);
    
    init();
    
}

FLComponentWindow::~FLComponentWindow(){

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
    QWidget* vcontainer = new QWidget;
    QVBoxLayout* vlayout = new QVBoxLayout;
    QWidget* hcontainer = new QWidget;
    QHBoxLayout* hlayout = new QHBoxLayout;
    
    fHComponentLayout = new QHBoxLayout;
//    fHComponentLayout->setHorizontalSpacing(10);
    QGroupBox*  componentGroup = new QGroupBox;
    componentGroup->setLayout(fHComponentLayout);
    
//------Initialize with a 2 items row
    QList<FLComponentItem*> newColumn;
    QGroupBox* newVbox = new QGroupBox;
    QVBoxLayout* newVboxLayout = new QVBoxLayout;
    newVbox->setLayout(newVboxLayout);
    
    FLComponentItem* item = new FLComponentItem();
    newVboxLayout->addWidget(item);
    fHComponentLayout->addWidget(newVbox);
    
    newColumn.push_back(item);
    
//    
//    QPixmap flecheImage(":/Images/fleche.png");
//    
//    QLabel* flecheLabel = new QLabel;
//    flecheLabel->setPixmap(flecheImage.scaled(30, 30, Qt::KeepAspectRatio));
//    
//    fHComponentLayout->addWidget(flecheLabel);
//    
//    
//    QList<FLComponentItem*> newColumn2;
//    QGroupBox* newVbox2 = new QGroupBox;
//    QVBoxLayout* newVboxLayout2 = new QVBoxLayout;
//    newVbox2->setLayout(newVboxLayout2);
//    
//    FLComponentItem* item2 = new FLComponentItem();
//    newVboxLayout2->addWidget(item2);
//    fHComponentLayout->addWidget(newVbox2);
//    
//    newColumn2.push_back(item2);
    
    fItems.push_back(newColumn);
//    fItems.push_back(newColumn2);
    
    fVerticalElements.push_back(qMakePair(new QLabel(""), newVbox));
//    fVerticalElements.push_back(qMakePair(flecheLabel, newVbox2));
    
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
//    fSaveB->hide();
    
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
    
//    vlayout->addWidget(new QLabel("Sequential Composition"));
    vlayout->addWidget(hcontainer);
    vlayout->addWidget(hbuttongroup);
    vlayout->addWidget(intermediateWidget);
    
    vcontainer->setLayout(vlayout);
    setCentralWidget(vcontainer);
    
    adjustSize();
}

void FLComponentWindow::createComponent(){
    
    QString faustToCompile = "import(\"music.lib\");\n\n\nprocess = ";
    
    faustToCompile += "(";
    
    for(QList<QList<FLComponentItem*> >::iterator it = fItems.begin(); it != fItems.end(); it++ ){
        
        if(it != fItems.begin())
            faustToCompile+=":";
        
        QString composition("");
        
        for(QList<FLComponentItem*>::iterator it2=it->begin(); it2 != it->end(); it2++){
            
            if(it2 != it->begin())
                composition+=",";
            
            composition += "component(\"" + (*it2)->source() + "\")";
        }
        
        faustToCompile += "stereoize(" + composition + ")";
    }
    
    faustToCompile+=")";
    faustToCompile += ";";

    hide();
    
    printf("CODE TO COMPILE = %s\n", faustToCompile.toStdString().c_str());
    emit newComponent(faustToCompile);
}

void FLComponentWindow::addComponentRow(){
    
    int index = 0;
    
    printf("SIZE OF VertivalElements = %i\n", fVerticalElements.size());
    
    for(QList<QList<FLComponentItem*> >::iterator it = fItems.begin(); it != fItems.end(); it++ ){
        
        FLComponentItem* item = new FLComponentItem();
        
        fVerticalElements[index].second->layout()->addWidget(item);
        it->push_back(item);
        index++;
    }
}

void FLComponentWindow::deleteComponentRow(){
//    
////    First Row cannot be deleted
//    if(fItems.begin()->size() == 1)
//        return;
//    
//    for(QList<QList<FLComponentItem*> >::iterator it = fItems.begin(); it != fItems.end(); it++ ){
//        
//        QList<FLComponentItem*>::iterator it2 = it->end();
//        it2--;
//        FLComponentItem* item = *(it2);
//        it->removeOne(item);
//        fLayout->removeWidget(item);
//        delete item;
//    }
//    
//    adjustSize();
}

void FLComponentWindow::addComponentColumn(){
    
    QPixmap flecheImage(":/Images/fleche.png");
    
    QLabel* flecheLabel = new QLabel;
    flecheLabel->setPixmap(flecheImage.scaled(30, 30, Qt::KeepAspectRatio));
    
    fHComponentLayout->addWidget(flecheLabel);
    
    QList<FLComponentItem*> newColumn;
    QGroupBox* newVbox = new QGroupBox;
    QVBoxLayout* newVboxLayout = new QVBoxLayout;
    newVbox->setLayout(newVboxLayout);
    
    for(int i=0; i<fItems.begin()->size(); i++){
        FLComponentItem* item = new FLComponentItem();
        newVboxLayout->addWidget(item);
        
        newColumn.push_back(item);
    }

    fHComponentLayout->addWidget(newVbox);
    
    fItems.push_back(newColumn);
    fVerticalElements.push_back(qMakePair(flecheLabel, newVbox));
    
}

void FLComponentWindow::deleteComponentColumn(){
    
//    First Column cannot be deleted
    if(fItems.size() == 1)
        return;
    
    QList<QList<FLComponentItem*> >::iterator it2 = fItems.end();
    it2--;
    
    QList<FLComponentItem*> itemList = *it2;
    
    fItems.removeOne(itemList);
    
    for(QList<FLComponentItem*>::iterator it = itemList.begin(); it != itemList.end(); it++){
        
        FLComponentItem* item = *it;
        QLabel* toEraseLabel = fVerticalElements[fVerticalElements.size()-1].first;
        QGroupBox* toEraseBox = fVerticalElements[fVerticalElements.size()-1].second;
        
        fHComponentLayout->removeWidget(toEraseBox);
        fHComponentLayout->removeWidget(toEraseLabel);
        delete item;
    }
    
    adjustSize();
}

void FLComponentWindow::closeEvent(QCloseEvent* event){
    
    emit deleteIt();
}

void FLComponentWindow::cancel(){
    emit close();
}




