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
FLComponentItem::FLComponentItem(QWidget* parent = NULL) : QGroupBox(parent){
   
    setAcceptDrops(true);
    
    fSource = "";
    fLayout = new QVBoxLayout;
    
    QLabel *image = new QLabel("<h2>DROP YOUR FAUST DSP</h2>");
    
    fLayout->addWidget(image);
    fCurrentWidget = image;
    
    setLayout(fLayout);
}

FLComponentItem::~FLComponentItem(){
    
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
    
    dsp* compiledDSP = sessionManager->createDSP(factorySetts, NULL, NULL, NULL, errorMsg);
    
    if (compiledDSP == NULL)
        return;
    
    fSource = source;
    setTitle(QFileInfo(fSource).baseName());
    
    
    QTGUI* interface = new QTGUI(this);
    compiledDSP->buildUserInterface(interface);
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

FLComponentWindow::~FLComponentWindow(){}

void FLComponentWindow::init()
{
    QWidget* vcontainer = new QWidget;
    QVBoxLayout* vlayout = new QVBoxLayout;
    QWidget* hcontainer = new QWidget;
    QHBoxLayout* hlayout = new QHBoxLayout;
    
    fLayout = new QGridLayout();
    QGroupBox*  componentGroup = new QGroupBox;
    componentGroup->setLayout(fLayout);
    
    QList<FLComponentItem*> newColumn;
    FLComponentItem* item = new FLComponentItem();
    fLayout->addWidget(item, 0, 0);
    newColumn.push_back(item);
    
    FLComponentItem* item2 = new FLComponentItem();
    fLayout->addWidget(item2, 1, 0);
    newColumn.push_back(item2);
    
    fItems.push_back(newColumn);
    
    QPixmap plusImage("/Users/denoux/FLReconstruct/Resources/Images/plus.ico");
//    
//    dropImage.scaledToHeight(10, Qt::SmoothTransformation);
    
    MyLabel *rowimage = new MyLabel();
    rowimage->setPixmap(plusImage);
    rowimage->setAlignment(Qt::AlignCenter);
    rowimage->setFixedSize(50,50);
    connect(rowimage, SIGNAL(imageClicked()), this, SLOT(addComponentRow()));
    
    MyLabel *colimage = new MyLabel();
    colimage->setPixmap(plusImage);
    colimage->setAlignment(Qt::AlignCenter);
    colimage->setFixedSize(50,50);
    connect(colimage, SIGNAL(imageClicked()), this, SLOT(addComponentColumn()));
    
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
    
    hlayout->addWidget(componentGroup);
    hlayout->addWidget(colimage);
    hcontainer->setLayout(hlayout);
    
    vlayout->addWidget(new QLabel("Sequential Composition"));
    vlayout->addWidget(hcontainer);
    vlayout->addWidget(rowimage);
    vlayout->addWidget(intermediateWidget);
    
    vcontainer->setLayout(vlayout);
    setCentralWidget(vcontainer);
    
    adjustSize();
}

void FLComponentWindow::createComponent(){
    
    QString faustToCompile = "process = ";
    
    for(QList<QList<FLComponentItem*> >::iterator it = fItems.begin(); it != fItems.end(); it++ ){
        
        if(it != fItems.begin())
            faustToCompile+=":";
        
        QString composition("");
        
        for(QList<FLComponentItem*>::iterator it2=it->begin(); it2 != it->end(); it2++){
            
            if(it2 != it->begin())
                composition+=",";
            
            composition = "component(\"" + (*it2)->source() + "\")";
        }
        faustToCompile += composition;
        faustToCompile+=")";
    }
    
    
    faustToCompile += ";";

    
    hide();
    printf("EMIT = %s\n", faustToCompile.toStdString().c_str());
    emit newComponent(faustToCompile);
}

void FLComponentWindow::addComponentRow(){
    
    int index = 0;
    
    for(QList<QList<FLComponentItem*> >::iterator it = fItems.begin(); it != fItems.end(); it++ ){
        
        FLComponentItem* item = new FLComponentItem();
        fLayout->addWidget(item, it->size(), index);
        
        it->push_back(item);
        index++;
    }
}

void FLComponentWindow::addComponentColumn(){
    
    QList<FLComponentItem*> newColumn;
    
    for(int i=0; i<fItems.begin()->size(); i++){
        FLComponentItem* item = new FLComponentItem();
        fLayout->addWidget(item, i, fItems.size());
        
        newColumn.push_back(item);
    }
    
    fItems.push_back(newColumn);
}
//
//void FLComponentWindow::organizeLayout(){
// 
//    for(QList<QList<FLComponentItem*> >::iterator it = ; it )
//    
//}

void FLComponentWindow::cancel(){
    emit deleteIt();
}
