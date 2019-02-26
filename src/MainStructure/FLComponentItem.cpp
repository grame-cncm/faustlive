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

#include "FLComponentItem.h"
#include "FLSessionManager.h"
#include "FLErrorWindow.h"

/****************************COMPONENT ITEM***************************/
FLComponentItem::FLComponentItem(const QString& index, QWidget* parent) : QWidget(parent)
{
    fIndex = index;
    setAcceptDrops(true);
    
    fSource = "";
    fLayout = new QVBoxLayout;
    
    QLabel *image = new QLabel("<h2>DROP YOUR FAUST DSP</h2>");
    image->setAlignment(Qt::AlignCenter);
    
    fLayout->addWidget(image);
    fCurrentWidget = image;
    setLayout(fLayout);
    fCompiledDSP = NULL;
}

FLComponentItem::FLComponentItem(const QString& source, QRect rect, QWidget* parent) : QWidget(parent)
{
    fSource = source;
    setGeometry(rect);
}

FLComponentItem::~FLComponentItem()
{
    FLSessionManager* sessionManager = FLSessionManager::_Instance();
    
    if(fCompiledDSP)
        sessionManager->deleteDSPandFactory(fCompiledDSP);
    
    delete fLayout;
}

QString FLComponentItem::source()
{
    if(fSource == "")
        fSource = "process = _;";
    return fSource;
}

void FLComponentItem::dragEnterEvent(QDragEnterEvent* event)
{
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

QString FLComponentItem::handleDrop(QDropEvent* event)
{
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

void FLComponentItem::createInterfaceInRect(const QString& source)
{
    QString errorMsg("");
    
    FLSessionManager* sessionManager = FLSessionManager::_Instance();
    
    QPair<QString, void*> factorySetts = sessionManager->createFactory(source, NULL, errorMsg);
    
    if(factorySetts.second == NULL){
        FLErrorWindow::_Instance()->print_Error(errorMsg);
        return;
    }
    
    fCompiledDSP = sessionManager->createDSP(factorySetts, source, NULL, NULL, NULL, errorMsg);
    
    if (fCompiledDSP == NULL)
        return;
    
    fSource = source;
//    setTitle(QFileInfo(fSource).baseName());
    QWidget* parent = (QWidget*) this;
    QTGUI* inter = new QTGUI(parent);
    fCompiledDSP->buildUserInterface(inter);
    
//    interface->setMinimumSize(300,300);
    
//    printf("RECT HEIGHT || WIDTH= %i || %i\n", interface->rect().height(), interface->rect().width());
    
//    QPixmap pixmap(interface->minimumSize()); 
//    interface->render(&pixmap);
//    pixmap.save("example.png");
    
//    pixmap (QSize(15,15), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    
//    QPixmap pix(interface->size());
//    
//    interface->render(&pix);
//    
//    QLabel* inter = new QLabel;
//    inter->setPixmap(pixmap);
//    inter->setSize(interface->size());
    
    inter->setEnabled(false);
//    inter->resize(150,150);
//    interface->run();
    
    delete fLayout;
    delete fCurrentWidget;
//    delete interface;
    fLayout = new QVBoxLayout;
    fLayout->addWidget(inter);
    setLayout(fLayout);
    
    fCurrentWidget = inter;
}

void FLComponentItem::dropEvent(QDropEvent* event)
{
    QString source = handleDrop(event);
        
    if(source != ""){
        event->acceptProposedAction();
        createInterfaceInRect(source);
    }
}

QString FLComponentItem::faustComponent(const QString& layoutIndex)
{
    // Il faut pouvoir discerner les cas où on veut rendre le component (file ou URL web), où on veut rendre la source (ensemble de components ou "")
    if(QFileInfo(fSource).exists() || fSource.indexOf("http://") == 0){
        QString faustCode = "vgroup(\"[" + layoutIndex + "]component" + fIndex + "\", component(\"" + fSource + "\"))";
        return faustCode;
    }
    else
        return fSource;
}




