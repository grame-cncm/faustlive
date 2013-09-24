//
//  FLWindow.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLWindow.h"
#include <sstream>
#include <QDir>
#include <QApplication>

/****************************FaustLiveWindow IMPLEMENTATION***************************/

FLWindow::FLWindow(string& baseName, int index, Effect* eff, int x, int y, string& home, int IDAudio){
    
    shortcut = false;
    effect = eff;
    
    setAcceptDrops(true);
    
    //Convert int into string
    windowIndex = index;
    stringstream ss;
    ss << windowIndex;
    
    nameWindow = baseName + "-" + ss.str();
    
    appHome = home + "/" + nameWindow;
    
    QDir direct;
    direct.mkdir(appHome.c_str());
    
    httpdWindow = new HTTPWindow();
    connect(httpdWindow, SIGNAL(closeAll()), this, SLOT(emit_closeAll()));
    
    //Allocating interfaces
    finterface = new FUI();
    jinterface = new FJUI();
    
    indexAudio = IDAudio;
    clientOpen = false;
    audioSwitched = false;
        
    xPos = x;
    yPos = y;
    
    setMenuBar();
    
    //    setAttribute(Qt::WA_MacNoClickThrough);
    
    setMinimumHeight(QApplication::desktop()->geometry().size().height()/4);
    
    //    setAttribute(Qt::WA_DeleteOnClose);
}

FLWindow::~FLWindow(){
    
    delete audioClient;
    
    delete menu;
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
    
    menu = new FLToolBar(this);
    
    addToolBar(menu);
    
    connect(menu, SIGNAL(modified(string, int)), this, SLOT(modifiedOptions(string, int)));
    connect(menu, SIGNAL(sizeChanged()), this, SLOT(resizing()));
}

void FLWindow::errorPrint(const char* msg){
    emit error(msg);
}

void FLWindow::modifiedOptions(string text, int value){
    
    
    effect->update_compilationOptions(text, value);
}

void FLWindow::resizing(){
    adjustSize();
}

void FLWindow::emit_closeAll(){
    emit closeAll();
}

bool FLWindow::is_Default(){
    
    string sourceContent = pathToContent(effect->getSource());
    
    if(sourceContent.compare("process = !,!:0,0;") == 0)
        return true;
    else 
        return false;
}

//------------------------CLOSING ACTIONS

void FLWindow::closeEvent(QCloseEvent* event){ 
    
    if(!shortcut)
        emit close();
    else
        emit closeAll();
}

void FLWindow::keyPressEvent(QKeyEvent* event){ 
    
    if(event->key() == Qt::Key_Alt)
        shortcut = true;
}

void FLWindow::keyReleaseEvent(QKeyEvent* event){
    
    if(event->key() == Qt::Key_Alt)
        shortcut = false;
}

void FLWindow::shut_Window(){
    
    //During the execution, when a window is shut, its associate folder has to be removed
    deleteDirectoryAndContent(appHome);
    
    close_Window();
}

void FLWindow::close_Window(){
    
    if(clientOpen)
        audioClient->stop();
    
    deleteInterfaces();
    
    if(httpdWindow){
        delete httpdWindow;
        httpdWindow = NULL;
    }
    //     printf("deleting instance = %p\n", current_DSP);   
    deleteDSPInstance(current_DSP);
}

void FLWindow::deleteInterfaces(){
    delete interface;
    delete finterface;
    delete jinterface;
    interface = NULL;
    finterface = NULL;
    jinterface = NULL;
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
    
    int multiplCoef = windowIndex;
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
bool FLWindow::init_Window(bool init, bool recall, char* errorMsg, int bufferSize,int compressionValue, string masterIP, int masterPort, int latency){
    
    current_DSP = createDSPInstance(effect->getFactory());

    string textOptions = effect->getCompilationOptions();
    if(textOptions.compare(" ") == 0)
        textOptions = "";
    
    menu->setOptions(textOptions.c_str());
    menu->setVal(effect->getOptValue());
    
    if (current_DSP == NULL){
        snprintf(errorMsg, 255, "Impossible to create a DSP instance"); 
        return false;
    }
    
    //Window tittle is build with the window Name + effect Name
    string inter = nameWindow + " : " + effect->getName();
    
    interface = new QTGUI(this, inter.c_str());
    
    crossfade_netjackaudio* njaudio;
    
    if(finterface && jinterface && interface){
        
        //Building interface and Audio parameters
        current_DSP->buildUserInterface(finterface);
        current_DSP->buildUserInterface(interface);
        
        if(init)
            print_initWindow();        
        
        this->adjustSize();  
        
        if(init_audioClient(indexAudio, bufferSize, compressionValue, masterIP, masterPort, latency)){
            
            setGeometry(xPos, yPos, 0, 0);
            adjustSize();
            
            frontShow();               
            interface->run();
            return true;
        }
        else{        
            deleteInterfaces();
            switch (indexAudio) {
                case kCoreaudio:
                    snprintf(errorMsg, 255, "Impossible to init Core Audio Client");
                    break;
                    
                case kJackaudio:
                    snprintf(errorMsg, 255, "Impossible to init Jack Client");        
                    break;
                    
                case kNetjackaudio:
                    snprintf(errorMsg, 255, "Impossible to init remote Net Jack Client.\nMake sure the server is running!"); 
                    break;
                    
                default:
                    break;
            }
            return false;
        }
    }
    else
        return false;
}

//Change of the effect in a Window
bool FLWindow::update_Window(Effect* newEffect, string compilationOptions, int optVal, char* error, int bufferSize, int compressionValue, string masterIP, int masterPort, int latency){
    
    audioFader_Interface* audioFade = dynamic_cast<audioFader_Interface*>(audioClient);
    
    //    printf("yPos = %i\n", yPos);
    
    //Step 1 : Save the parameters of the actual interface
    xPos = this->geometry().x();
    yPos = this->geometry().y();
    //    printf("yPos = %i\n", yPos);
    
    save_Window(indexAudio); 
    
    //Step 2 : Delete the actual interface
    hide(); 
    deleteInterfaces();
    
    //Step 3 : creating the user interface
    finterface = new FUI();        
    jinterface = new FJUI();
    
    //Step 4 : creating the new DSP instance
    llvm_dsp* charging_DSP = createDSPInstance(newEffect->getFactory());
    //    printf("charging DSP = %p\n", charging_DSP);
    string textOptions = effect->getCompilationOptions();
    if(textOptions.compare(" ") == 0)
        textOptions = "";
    
    menu->setOptions(textOptions.c_str());
    menu->setVal(optVal);
    
    if (current_DSP == NULL)
        return false;
    
    if(finterface && jinterface){
        
        audio* newAudio;
        
        //Step 5 : Init the audio of incoming DSP
        if(indexAudio != 1){
            
            if(indexAudio == 0){
                newAudio = new crossfade_coreaudio(bufferSize);
                
                printf("New crossfade coreaudio\n");
            }
            else if(indexAudio == 2){
                
                crossfade_netjackaudio* njAudio = new crossfade_netjackaudio(compressionValue, masterIP, masterPort, latency, 0);
                connect(njAudio, SIGNAL(error(const char*)), this, SLOT(errorPrint(const char*)));
                
                newAudio = (audio*)njAudio;
            }
            
            audioFader_Interface* newAudioFade = dynamic_cast<audioFader_Interface*> (newAudio);
            
            printf("NEW AUDIO FADE = %p\n", newAudioFade);
            
            newAudioFade->launch_fadeIn();
            
            if(!newAudio->init(newEffect->getName().c_str(), charging_DSP)){
                //Step 6 : Recall the parameters of the window
                string intermediate = nameWindow + " : " + effect->getName();
                
                interface = new QTGUI(this, intermediate.c_str());
                
                current_DSP->buildUserInterface(finterface);
                current_DSP->buildUserInterface(interface);
                
                bool recalled = recall_Window();
                setGeometry(xPos, yPos, 0, 0);
                adjustSize();
                show();
                //Step 9 : Launch User Interface
                interface->run();
                
                if(indexAudio == 0)
                    snprintf(error, 255, "Impossible to init new Core Audio Client");
                else
                    snprintf(error, 255, "Impossible to init new Net Jack Client");                    
                
                return false;
            }
            
        } 
        else
            ((crossfade_jackaudio*)audioClient)->init_FadeIn_Audio(charging_DSP, newEffect->getName().c_str());
        
        
        //Step 7: Crossfade audio between outcoming and incoming DSP   
        
//        caudio->launch_crossfade(charging_DSP);
        
        audioFade->launch_fadeOut();
        
        //Step 6 : Recall the parameters of the window
        string inter = nameWindow + " : " + newEffect->getName();
        
        interface = new QTGUI(this, inter.c_str());
        
        charging_DSP->buildUserInterface(finterface);
        charging_DSP->buildUserInterface(interface);
        
        if(indexAudio != 1)
            newAudio->start();
        //Step 6 : Recall the parameters of the window
        
        bool recalled = recall_Window();
        setGeometry(xPos, yPos, 0, 0);
        adjustSize();
        show();
        
        while(audioFade->get_FadeOut()==1){}
        
        if(indexAudio != 1){
            audioClient->stop();
            audio* intermediate = (audio*)audioClient;
            audioClient = newAudio;
            delete intermediate;
        }
        else
            ((crossfade_jackaudio*)audioClient)->upDate_DSP();
        
        //Step 8 : Change the current DSP as the dropped one
        llvm_dsp* VecInt;
        
        printf("CURRENT DSP = %p\n", current_DSP);
        printf("CHARGING DSP = %p\n", charging_DSP);
        
        VecInt = current_DSP;
        current_DSP = charging_DSP; 
        charging_DSP = VecInt;
        
        printf("deleting DSP leaving = %p\n", charging_DSP);
        deleteDSPInstance(charging_DSP);
        effect = newEffect;
        //Step 9 : Launch User Interface
        interface->run();
        return true;
        
    }
    else{
        snprintf(error, 255, "Impossible to allocate DSP interface");
        return false;
    }
}

void FLWindow::stop_Audio(){
    
    audioClient->stop();
    printf("STOP AUDIO\n");
    clientOpen = false;
}

void FLWindow::reset_audioSwitch(){
    audioSwitched = false;
}

bool FLWindow::update_AudioArchitecture(int index, int bufferSize, int compressionValue, string masterIP, int masterPort, int latency){
    
    delete audioClient;
    
    if(windowIndex <3){
    
    if(init_audioClient(index, bufferSize, compressionValue, masterIP, masterPort, latency)){
        audioSwitched = true;
//        printf("audio has switched = %i\n", audioSwitched);
        return true;
    }
        else
            return false;
    }
    else
        return false;
}

bool FLWindow::init_audioClient(int index, int bufferSize, int compressionValue, string masterIP, int masterPort, int latency){
    
//    printf("AUDIOSWITCHED = %i\n", audioSwitched);
    
    // In case the audio switch did work for not all the windows, this one has to switch back
    if(audioSwitched && clientOpen){
        stop_Audio();
        delete audioClient;
        indexAudio = index;
//        printf("Window %i has stopped\n", windowIndex);
    }
    
    bool success = false;
    
    crossfade_netjackaudio* njaudio;
    
    switch(index){
        case kCoreaudio:
            audioClient = new crossfade_coreaudio(bufferSize);
            break;
        case kJackaudio:
            audioClient = new crossfade_jackaudio(0,0);
            break;
        case kNetjackaudio:
            njaudio = new crossfade_netjackaudio(compressionValue, masterIP, masterPort, latency, 0);
            connect(njaudio, SIGNAL(error(const char*)), this, SLOT(errorPrint(const char*)));
            audioClient = (audio*)njaudio;
        default:
            break;
    }
    
    if((index == 1 && ((crossfade_jackaudio*)audioClient)->init(nameWindow.c_str(), current_DSP, effect->getName().c_str())) || (index != 1 && audioClient->init(nameWindow.c_str(), current_DSP))){ 
            
        recall_Window();
        audioClient->start();
        
        if(index == 1){
            
            crossfade_jackaudio* jaudio = (crossfade_jackaudio*)audioClient;
            
            string jackfilename = appHome + "/" + nameWindow + ".jc";
            
            if(QFileInfo(jackfilename.c_str()).exists()){
                
                list<pair<string, string> > connection = jinterface->recallConnections(jackfilename.c_str());
                
                jaudio->reconnect(connection);
            }
            else
                jaudio->default_connections();  
        }
        success = true;
        clientOpen = true;
    }
    return success;
}

bool FLWindow::update_AudioParameters(char* error, int index, int bufferSize, int compressionValue, string masterIP, int masterPort, int latency){
    
    audioClient->stop();
    delete audioClient;
    
    if(index == 0){
        
        audioClient = new crossfade_coreaudio(bufferSize);
        
        if(audioClient->init(effect->getName().c_str(), current_DSP)){
            
            recall_Window();       
            audioClient->start();
            audioSwitched = true;
            return true;
        }
        else{
            snprintf(error, 255, "Impossible to init Core Audio Client with new parameters!");
            return false;
        }
    }
    else if(index == 2){
        
        crossfade_netjackaudio* newNetJack = new crossfade_netjackaudio(compressionValue, masterIP, masterPort, latency, 0);
        connect(newNetJack, SIGNAL(error(const char*)), this, SLOT(errorPrint(const char*)));
        
        audioClient = (audio*)newNetJack;
        
        
        if(audioClient->init(effect->getName().c_str(), current_DSP)){
            
            recall_Window();
            audioClient->start();
            audioSwitched = true;
            return true;
        }
        else{
            snprintf(error, 255, "Impossible to init remote Net Jack Client with new parameters!");
            return false;
        }
    }
}


//------------------------SAVING WINDOW ACTIONS

void FLWindow::update_ConnectionFile(list<pair<string,string> > changeTable){
    
    string jackfilename = appHome + "/" + nameWindow + ".jc";
    jinterface->update(jackfilename.c_str(), changeTable);
}

void FLWindow::save_Window(int index){
    
    //Graphical parameters//
    string rcfilename = appHome + "/" + nameWindow + ".rc";
    finterface->saveState(rcfilename.c_str());
    
    //Jack connections//
    if(index == 1){
        string jackfilename = appHome + "/" + nameWindow + ".jc";
        
        crossfade_jackaudio* jaudio = (crossfade_jackaudio*)audioClient;
        
        jinterface->saveConnections(jackfilename.c_str(), jaudio->get_audio_connections());
    }
}

bool FLWindow::recall_Window(){
    
    //Graphical parameters//
    string rcfilename = appHome + "/" + nameWindow + ".rc";
    QString toto(rcfilename.c_str());
    
    if(QFileInfo(toto).exists()){
        finterface->recallState(rcfilename.c_str());	
    }
}

//------------------------ACCESSORS

string FLWindow::get_nameWindow(){
    return nameWindow;
}

int FLWindow::get_indexWindow(){
    return windowIndex;
}

Effect* FLWindow::get_Effect(){
    return effect;
}

int FLWindow::get_x(){
    
    xPos = this->geometry().x();
    return xPos;
}

int FLWindow::get_y(){
    yPos = this->geometry().y();
    return yPos;
}

//------------------------HTTPD

bool FLWindow::init_Httpd(char* error){
    
    if(httpdWindow != NULL){
        httpdWindow->search_IPadress();
        
        //HttpdInterface reset the parameters when build. So we have to save the parameters before
        
        save_Window(indexAudio);
        
        string windowTitle = nameWindow + ":" + effect->getName();
        if(httpdWindow->build_httpdInterface(error, windowTitle, current_DSP)){
            
            //recall parameters to run them properly
            //For a second, the initial parameters are reinitialize : it can sound weird
            recall_Window();
            
            httpdWindow->launch_httpdInterface();
            httpdWindow->display_HttpdWindow(calculate_Coef()*10, 0);
            
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
    return httpdWindow->isActiveWindow();
}

void FLWindow::hide_httpdWindow(){
    httpdWindow->hide();
}

//------------------------Right Click Reaction

void FLWindow::contextMenuEvent(QContextMenuEvent* ev){
    
    emit rightClick(ev->globalPos());
}

