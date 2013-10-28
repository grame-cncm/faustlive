//
//  FLWindow.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLWindow.h"

#include "faust/gui/faustqt.h"

list<GUI*>               GUI::fGuiList;

#include <sstream>
#include <QDir>
#include <QApplication>

/****************************FaustLiveWindow IMPLEMENTATION***************************/

FLWindow::FLWindow(string& baseName, int index, FLEffect* eff, int x, int y, string& home){
    
    fShortcut = false;
    fEffect = eff;
    fPortHttp = 5510;
    
    setAcceptDrops(true);
    
    //Convert int into string
    fWindowIndex = index;
    stringstream ss;
    ss << fWindowIndex;
    
    fWindowName = baseName + "-" + ss.str();
    
    fHome = home + "/" + fWindowName;
    
    QDir direct;
    direct.mkdir(fHome.c_str());
    
    fHttpdWindow = new HTTPWindow();
    connect(fHttpdWindow, SIGNAL(closeAll()), this, SLOT(emit_closeAll()));
    
    //Allocating interfaces
    fRCInterface = new FUI();
    
    string folder = home + "/Settings";
    
    AudioCreator* creator = AudioCreator::_Instance(folder, NULL);
    
    fAudioManager = creator->createAudioManager(creator->getCurrentSettings());
    fClientOpen = false;
        
    fXPos = x;
    fYPos = y;
    
    setMenuBar();
    
    //    setAttribute(Qt::WA_MacNoClickThrough);
    
    setMinimumHeight(QApplication::desktop()->geometry().size().height()/4);
    
    //    setAttribute(Qt::WA_DeleteOnClose);
}

FLWindow::~FLWindow(){
    
    delete fAudioManager;
    delete fMenu;
}

string FLWindow::pathToContent(string path){
    QFile file(path.c_str());
    string Content;
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        Content += "";
        return Content;
    }
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        Content += line.data();
    }
    return Content;
}

bool FLWindow::deleteDirectoryAndContent(string& directory){
    
    QDir srcDir(directory.c_str());
    
    QFileInfoList children = srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
    
    if(children.size() == 0){
        QString toRemove(directory.c_str());
        srcDir.rmdir(srcDir.absolutePath());
        return true;
    }
    else{
        foreach(const QFileInfo &info, children) {
            
            QString path(directory.c_str());
            
            QString item = path + "/" + info.fileName();
            
            if (info.isDir()) {
                string file = item.toStdString();
                if (!deleteDirectoryAndContent(file)) {
                    return false;
                }
            } else if (info.isFile()) {
                if (!QFile::remove(item)) {
                    return false;
                }
            } else {
                qDebug() << "Unhandled item" << info.filePath() << "in cpDir";
            }
        }
        return true;
    }
}

void FLWindow::setMenuBar(){
    
    fMenu = new FLToolBar(this);
    
    addToolBar(fMenu);
    
    connect(fMenu, SIGNAL(modified(string, int, int)), this, SLOT(modifiedOptions(string, int, int)));
    connect(fMenu, SIGNAL(sizeGrowth()), this, SLOT(resizingBig()));
    connect(fMenu, SIGNAL(sizeReduction()), this, SLOT(resizingSmall()));
}

void FLWindow::errorPrint(const char* msg){
    emit error(msg);
}

void FLWindow::modifiedOptions(string text, int value, int port){
    
    if(fPortHttp != port)
        fPortHttp = port;
    
    fEffect->update_compilationOptions(text, value);
}

void FLWindow::resizingSmall(){

    setMinimumSize(QSize(0,0));
    adjustSize();
}

void FLWindow::resizingBig(){
    
//    QSize winSize = fMenu->geometry().size();
//    winSize += fMenu->minimumSize();
//    
//    printf("SIZE BEFORE RESIZE = %i || %i\n", winSize.width(), winSize.height());
//    
    QSize winMinSize = minimumSize();
    winMinSize += fMenu->geometry().size();
    
//    setGeometry(0,0,winSize.width(), winSize.height());
    setMinimumSize(winMinSize);
//
//    printf("SIZE AFTER RESIZE = %i || %i\n", winSize.width(), winSize.height());
    adjustSize();
}

void FLWindow::emit_closeAll(){
    emit closeAll();
}

bool FLWindow::is_Default(){
    
    string sourceContent = pathToContent(fEffect->getSource());
    
    if(sourceContent.compare("process = !,!:0,0;") == 0)
        return true;
    else 
        return false;
}

//------------------------CLOSING ACTIONS

void FLWindow::closeEvent(QCloseEvent* event){ 
    
    if(!fShortcut)
        emit close();
    else
        emit closeAll();
}

void FLWindow::keyPressEvent(QKeyEvent* event){ 
    
    if(event->key() == Qt::Key_Alt)
        fShortcut = true;
}

void FLWindow::keyReleaseEvent(QKeyEvent* event){
    
    if(event->key() == Qt::Key_Alt)
        fShortcut = false;
}

void FLWindow::shut_Window(){
    
    //During the execution, when a window is shut, its associate folder has to be removed
    deleteDirectoryAndContent(fHome);
    
    close_Window();
}

void FLWindow::close_Window(){
    
    if(fClientOpen)
        fAudioManager->stop();
    
    deleteInterfaces();
    
    if(fHttpdWindow){
        delete fHttpdWindow;
        fHttpdWindow = NULL;
    }
    //     printf("deleting instance = %p\n", current_DSP);   
    deleteDSPInstance(fCurrent_DSP);
}

void FLWindow::deleteInterfaces(){
    delete fInterface;
    delete fRCInterface;
    fInterface = NULL;
    fRCInterface = NULL;
}

//------------------------DRAG AND DROP ACTIONS

void FLWindow::dropEvent ( QDropEvent * event ){
    
    //The widget was hidden from crossing of an object through the window
    this->centralWidget()->show();
    
    list<string>    sourceList;    
    
    //The event is not entirely handled by the window, it is redirected to the application through the drop signal
    if (event->mimeData()->hasUrls()) {
        list<string>    sourceList;
        QList<QUrl> urls = event->mimeData()->urls();
        QList<QUrl>::iterator i;
        
        for (i = urls.begin(); i != urls.end(); i++) {
            
            QString fileName = i->toLocalFile();
            string dsp = fileName.toStdString();
            
            event->accept();
            
            sourceList.push_back(dsp);
        }   
        emit drop(sourceList);
    }
    if (event->mimeData()->hasText()){
        event->accept();
        
        string TextContent = event->mimeData()->text().toStdString();
        sourceList.push_back(TextContent);
        
        emit drop(sourceList);
    }
}

//That way the drag movement is more visible : the central widget is hidden when an object is crossing the window and reset visible when the object leaves the window
void FLWindow::dragEnterEvent ( QDragEnterEvent * event ){
    
    if (event->mimeData()->hasFormat("text/uri-list") || event->mimeData()->hasFormat("text/plain")){
        
        if (event->mimeData()->hasUrls()) {
            list<string>    sourceList;
            QList<QUrl> urls = event->mimeData()->urls();
            QList<QUrl>::iterator i;
            
            for (i = urls.begin(); i != urls.end(); i++) {
                
                QString fileName = i->toLocalFile();
                string dsp = fileName.toStdString();
                
                if(QFileInfo(dsp.c_str()).completeSuffix().toStdString().compare("dsp") == 0){
                    
                    centralWidget()->hide();
                    event->acceptProposedAction();   
                }
            }
            
        }
        else if(event->mimeData()->hasFormat("text/plain")){
            centralWidget()->hide();
            //            setWindowFlags(Qt::FramelessWindowHint);
            event->acceptProposedAction();      
        }
    }
}

void FLWindow::dragLeaveEvent ( QDragLeaveEvent * event ){
    //    setWindowFlags();
    centralWidget()->show();
}

//------------------------WINDOW ACTIONS

int FLWindow::calculate_Coef(){
    
    //Calculation of screen position of the window, depending on its index
    
    int multiplCoef = fWindowIndex;
    while(multiplCoef > 20){
        multiplCoef-=20;
    }
    return multiplCoef;
}

void FLWindow::print_initWindow(){
    
    //To help the user, a new empty window is filled with a TextEdit
    QPlainTextEdit* plainTextEdit = new QPlainTextEdit(this);
    if(plainTextEdit){
        plainTextEdit->setContextMenuPolicy(Qt::NoContextMenu);
        plainTextEdit->setFixedSize (QApplication::desktop()->geometry().size().height()/3, QApplication::desktop()->geometry().size().height()/3);
        
        plainTextEdit->setPlainText(tr("\n      INIT WINDOW\n\n\n      DROP A DSP \n      OR EDIT ME.\n\n      !! ^^ !!"));
        
        QFont font;
        font.setFamily(QString::fromUtf8("Menlo"));
        font.setPointSize(26);
        plainTextEdit->setFont(font);
        
        plainTextEdit->setReadOnly(true);
        QSize size = plainTextEdit->frameSize();
        resize(size);
        setCentralWidget(plainTextEdit);
        //        plainTextEdit->setAlignment(Qt::AlignCenter);
    }
}

void FLWindow::frontShow(){
    show();
    raise();
}

//Initialization of User Interface + StartUp of Audio Client
bool FLWindow::init_Window(bool init, bool recall, char* errorMsg){
    
    fCurrent_DSP = createDSPInstance(fEffect->getFactory());

    string textOptions = fEffect->getCompilationOptions();
    if(textOptions.compare(" ") == 0)
        textOptions = "";
    
    fMenu->setOptions(textOptions.c_str());
    fMenu->setVal(fEffect->getOptValue());
    fMenu->setPort(fPortHttp);
    
    if (fCurrent_DSP == NULL){
        snprintf(errorMsg, 255, "Impossible to create a DSP instance"); 
        return false;
    }
    
    //Window tittle is build with the window Name + effect Name
    string inter = fWindowName + " : " + fEffect->getName();
    
    fInterface = new QTGUI(this, inter.c_str());
    
    if(fRCInterface && fInterface){
        
        //Building interface and Audio parameters
        fCurrent_DSP->buildUserInterface(fRCInterface);
        fCurrent_DSP->buildUserInterface(fInterface);
        
        if(init)
            print_initWindow();        
        
//        this->adjustSize();  
        
        if(init_audioClient(errorMsg)){
            
            start_Audio();
            
            setGeometry(fXPos, fYPos, 0, 0);
            adjustSize();
            
//            QSize s_max = maximumSize();
            frontShow();               
            setMinimumSize(QSize(0, 0));
            setMaximumSize(QSize(QApplication::desktop()->geometry().size().width(), QApplication::desktop()->geometry().size().height()));
//            adjustSize();
            fInterface->run();
            return true;
        }
        else{        
            deleteInterfaces();
            return false;
        }
    }
    else
        return false;
}

//Change of the effect in a Window
bool FLWindow::update_Window(FLEffect* newEffect, string compilationOptions, int optVal, char* error){
    
    //Step 1 : Save the parameters of the actual interface
    fXPos = this->geometry().x();
    fYPos = this->geometry().y();
    save_Window(); 
    
    //Step 2 : Delete the actual interface
    hide(); 
    deleteInterfaces();
    
    //Step 3 : creating the user interface
    fRCInterface = new FUI();
    
    //Step 4 : creating the new DSP instance
    llvm_dsp* charging_DSP = createDSPInstance(newEffect->getFactory());
    //    printf("charging DSP = %p\n", charging_DSP);
    if (fCurrent_DSP == NULL)
        return false;
    
    //Step 5 : get the new compilation parameters
    string textOptions = fEffect->getCompilationOptions();
    if(textOptions.compare(" ") == 0)
        textOptions = "";
    fMenu->setOptions(textOptions.c_str());
    fMenu->setVal(optVal);
    fMenu->setPort(fPortHttp);
    
    if(fRCInterface){
        
        if(!fAudioManager->init_FadeAudio(error, newEffect->getName().c_str(), charging_DSP)){
            
//            printf("INIT FADE AUDIO FAILED\n");
            
            //Step 6 : Recall the parameters of the window
            string intermediate = fWindowName + " : " + fEffect->getName();
            
            fInterface = new QTGUI(this, intermediate.c_str());
            
            fCurrent_DSP->buildUserInterface(fRCInterface);
            fCurrent_DSP->buildUserInterface(fInterface);
            
            recall_Window();
            setGeometry(fXPos, fYPos, 0, 0);
            adjustSize();
            show();
            //Step 9 : Launch User Interface
            fInterface->run();
            
            return false;
        }
        
        //Step 6 : Set the new interface
        string inter = fWindowName + " : " + newEffect->getName();
        
        fInterface = new QTGUI(this, inter.c_str());
        
        charging_DSP->buildUserInterface(fRCInterface);
        charging_DSP->buildUserInterface(fInterface);
        
        //Step 7 : Recall the parameters of the window
        
        recall_Window();
        
        fAudioManager->start_Fade();
        
        setGeometry(fXPos, fYPos, 0, 0);
        adjustSize();
        show();
        
        fAudioManager->wait_EndFade();
        
        //Step 8 : Change the current DSP as the dropped one
        llvm_dsp* VecInt;
        
//        printf("CURRENT DSP = %p\n", fCurrent_DSP);
//        printf("CHARGING DSP = %p\n", charging_DSP);
        
        VecInt = fCurrent_DSP;
        fCurrent_DSP = charging_DSP; 
        charging_DSP = VecInt;
        
//        printf("deleting DSP leaving = %p\n", charging_DSP);
        deleteDSPInstance(charging_DSP);
        fEffect = newEffect;
        
        //Step 9 : Launch User Interface
        fInterface->run();
        return true;
        
    }
    else{
        snprintf(error, 255, "Impossible to allocate DSP interface");
        return false;
    }
}

void FLWindow::stop_Audio(){
    
    fAudioManager->stop();
    printf("STOP AUDIO\n");
    fClientOpen = false;
}

void FLWindow::start_Audio(){
    
    recall_Window();
    
    fAudioManager->start();
    
    string connectFile = fHome + "/" + fWindowName + ".jc";

    fAudioManager->connect_Audio(connectFile);
    printf("CONNECT = %s\n", connectFile.c_str());
    
    fClientOpen = true;
}

bool FLWindow::update_AudioArchitecture(char* error){
    
    AudioCreator* creator = AudioCreator::_Instance("/Users/denoux/CurrentSession/Settings", NULL);
    delete fAudioManager;
    
    fAudioManager = creator->createAudioManager(creator->getNewSettings());
    
    if(init_audioClient(error))
        return true;
    else
        return false;
}

bool FLWindow::init_audioClient(char* error){
    
    if(fAudioManager->initAudio(error, fEffect->getName().c_str(), fCurrent_DSP, fWindowName.c_str())){ 
            
        recall_Window();
        return true;
    }
    else{
        return false;
    }
}

//------------------------SAVING WINDOW ACTIONS

void FLWindow::update_ConnectionFile(list<pair<string,string> > changeTable){
    
    string connectFile = fHome + "/" + fWindowName + ".jc";
    fAudioManager->change_Connections(connectFile.c_str(), changeTable);
}

void FLWindow::save_Window(){
    
    //Graphical parameters//
    string rcfilename = fHome + "/" + fWindowName + ".rc";
    fRCInterface->saveState(rcfilename.c_str());
    
    printf("WINDOW SAVED\n");
    
    //Audio Connections parameters
    string connectFile = fHome + "/" + fWindowName + ".jc";
        
    fAudioManager->save_Connections(connectFile);
}

void FLWindow::recall_Window(){
    
    //Graphical parameters//
    string rcfilename = fHome + "/" + fWindowName + ".rc";
    QString toto(rcfilename.c_str());
    
    if(QFileInfo(toto).exists()){
        fRCInterface->recallState(rcfilename.c_str());	
        printf("state recalled for %p\n", fRCInterface);
    }
}

//------------------------ACCESSORS

string FLWindow::get_nameWindow(){
    return fWindowName;
}

int FLWindow::get_indexWindow(){
    return fWindowIndex;
}

FLEffect* FLWindow::get_Effect(){
    return fEffect;
}

int FLWindow::get_x(){
    
    fXPos = this->geometry().x();
    return fXPos;
}

int FLWindow::get_y(){
    fYPos = this->geometry().y();
    return fYPos;
}

//------------------------HTTPD

bool FLWindow::init_Httpd(char* error){
    
    if(fHttpdWindow != NULL){
        fHttpdWindow->search_IPadress();
        
        //HttpdInterface reset the parameters when build. So we have to save the parameters before
        
        save_Window();
        
        string windowTitle = fWindowName + ":" + fEffect->getName();
        if(fHttpdWindow->build_httpdInterface(error, windowTitle, fCurrent_DSP, fPortHttp)){
            
            //recall parameters to run them properly
            //For a second, the initial parameters are reinitialize : it can sound weird
            recall_Window();
            
            fHttpdWindow->launch_httpdInterface();
            fHttpdWindow->display_HttpdWindow(calculate_Coef()*10, 0);
            
            return true;
        }
        else
            return false;
    }
    else{
        snprintf(error, 255, "%s", "Httpd Window could not be allocated!"); 
        return false;
    }
}

bool FLWindow::is_httpdWindow_active(){
    return fHttpdWindow->isActiveWindow();
}

void FLWindow::hide_httpdWindow(){
    fHttpdWindow->hide();
}

string& FLWindow::get_HttpUrl(){
    return fHttpdWindow->getUrl();
}

//------------------------Right Click Reaction

void FLWindow::contextMenuEvent(QContextMenuEvent* ev){
    
    emit rightClick(ev->globalPos());
}

