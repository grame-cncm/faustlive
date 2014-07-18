//
//  FLWindow.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLComponentWindow.h"

/****************************FaustLiveWindow IMPLEMENTATION***************************/

//------------CONSTRUCTION WINDOW
FLComponentWindow::FLComponentWindow(const QString& sourceFolder) : fFirstRect(10,15,150,100), fSecondRect(180,15,150,100), fFirstSource(""), fSecondSource(""){
    
    setAcceptDrops(true);
    
    setGeometry(300,300, 300,300);
    fSourceFolder = sourceFolder;
    
    init();
    
}

FLComponentWindow::~FLComponentWindow(){}

void FLComponentWindow::dragEnterEvent ( QDragEnterEvent * event ){
    
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

QString FLComponentWindow::handleDrop(QDropEvent * event){
    
    QList<QUrl> urls = event->mimeData()->urls();
    QList<QUrl>::iterator i;
    
    QString sourceToSave("");
    
    if (event->mimeData()->hasFormat("text/uri-list")){
        for (i = urls.begin(); i != urls.end(); i++)
            sourceToSave = i->toString();
    }
    else if(event->mimeData()->hasFormat("text/plain") && sourceToSave == "")
        sourceToSave = event->mimeData()->text();

    return sourceToSave;
}

void FLComponentWindow::dropEvent ( QDropEvent * event ){
    
    if(fFirstRect.contains(event->pos())){
        
        fFirstSource = handleDrop(event);
        
        if(fFirstSource != ""){
            event->acceptProposedAction();
            fFirstLabel->setText(QFileInfo(fFirstSource).baseName());
        }
    }
    else if(fSecondRect.contains(event->pos())){
        
        fSecondSource = handleDrop(event);
        
        if(fSecondSource != ""){
            event->acceptProposedAction();
            fSecondLabel->setText(QFileInfo(fSecondSource).baseName());
        }
    }
    
    if(fSecondSource != "" && fFirstSource != "")
        fSaveB->show();
}

void FLComponentWindow::init()
{
    QWidget* container = new QWidget;
    QFormLayout* layout = new QFormLayout;
    
    QWidget* intermediateWidget = new QWidget;
    QHBoxLayout* intermediateLayout = new QHBoxLayout;
    
    QPixmap pm(150,100);
    pm.fill(Qt::transparent);
    
    QPainter painter(&pm);
    painter.setPen(QColor("#c56c00"));
    painter.setBrush(QBrush("#c56c00"));
    painter.drawRect(fFirstRect);
    
    QLabel* one = new QLabel;
    one->setPixmap(pm);
    
    QPixmap px(150,100);
    px.fill(Qt::transparent);
    
    QPainter paint(&px);
    paint.setPen(QColor("#c56c00"));
    paint.setBrush(QBrush("#c56c00"));
    paint.drawRect(fSecondRect);
    
    QLabel* two = new QLabel;
    two->setPixmap(pm);
    
    QPushButton* cancel = new QPushButton(tr("Cancel"), this);
    cancel->setDefault(false);
    
    fSaveB = new QPushButton(tr("Create Component"), this);
    fSaveB->setDefault(true);
    fSaveB->hide();
    
    connect(cancel, SIGNAL(released()), this, SLOT(cancel()));
    connect(fSaveB, SIGNAL(released()), this, SLOT(createComponent()));
    
    intermediateLayout->addWidget(cancel);
    intermediateLayout->addWidget(new QLabel(tr("")));
    intermediateLayout->addWidget(fSaveB);
    intermediateWidget->setLayout(intermediateLayout);
    
    QWidget* temp = new QWidget;
    QHBoxLayout* tempLayout = new QHBoxLayout;
    fFirstLabel = new QLabel(temp);
    fFirstLabel->setAlignment(Qt::AlignHCenter);
    fSecondLabel = new QLabel(temp);
    fSecondLabel->setAlignment(Qt::AlignHCenter);
    
    QLabel* dospuntos = new QLabel(" : ");
    dospuntos->setAlignment(Qt::AlignHCenter);
    
    tempLayout->addWidget(fFirstLabel);
    tempLayout->addWidget(dospuntos);
    tempLayout->addWidget(fSecondLabel);
    temp->setLayout(tempLayout);
    
    layout->addRow(one, two);
    layout->addRow(temp);
    layout->addRow(new QLabel(""));
    layout->addRow(intermediateWidget);
    
    container->setLayout(layout);
    setCentralWidget(container);
}

void FLComponentWindow::createComponent(){
    
    QString faustToCompile = "process = component(\"";
    faustToCompile += fFirstSource;
    faustToCompile += "\"):component(\"";
    faustToCompile += fSecondSource;
    faustToCompile += "\");";

    
    hide();
    printf("EMIT = %s\n", faustToCompile.toStdString().c_str());
    emit newComponent(faustToCompile);
}

void FLComponentWindow::cancel(){
    emit deleteIt();
}
