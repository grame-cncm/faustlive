//
//  FLWindow.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLWindow.h"

#include "faust/gui/faustqt.h"
#include "faust/gui/OSCUI.h"

list<GUI*>               GUI::fGuiList;

#include <sstream>
#include "FLToolBar.h"
#include "utilities.h"
#include "Remote_DSP_Factory.h"
#include "Remote_DSP.h"

#include "faust/llvm-dsp.h"

/****************************FaustLiveWindow IMPLEMENTATION***************************/

FLWindow::FLWindow(string& baseName, int index, FLEffect* eff, int x, int y, string& home, int generalPort, int port, string ipRemoteServer){
    
    fShortcut = false;
    fEffect = eff;
    fHttpdWindow = NULL;
    fPortHttp = port;
    fGeneralPort = generalPort;
    
    fIsLocal = true;
    fIpRemoteServer = ipRemoteServer;
    
    setAcceptDrops(true);
    
    //Convert int into string
    fWindowIndex = index;
    stringstream ss;
    ss << fWindowIndex;
    
    fWindowName = baseName + "-" + ss.str();
    
    fHome = home + "/" + fWindowName;
    
    QDir direct;
    direct.mkdir(fHome.c_str());
    
    fRCInterface = NULL;
    fOscInterface = NULL;
    fMenu = NULL;
    
    fSettingsFolder = home + "/Settings";
    
    AudioCreator* creator = AudioCreator::_Instance(fSettingsFolder, NULL);
    
    fAudioManager = creator->createAudioManager(creator->getCurrentSettings());
    fClientOpen = false;
        
    fXPos = x;
    fYPos = y;
    
    setMenu();
    
    setMinimumHeight(QApplication::desktop()->geometry().size().height()/4); 
    set_MenuBar();
}

FLWindow::~FLWindow(){}

//Set up of the Window ToolBar
void FLWindow::setMenu(){
    
    fMenu = new FLToolBar(this);
    
    addToolBar(fMenu);
    
    connect(fMenu, SIGNAL(modified(string, int, int, const string&)), this, SLOT(modifiedOptions(const string&, int, int, const string&)));
    connect(fMenu, SIGNAL(remoteStateChanged(int)), this, SLOT(changeRemoteState(int)));
    connect(fMenu, SIGNAL(sizeGrowth()), this, SLOT(resizingBig()));
    connect(fMenu, SIGNAL(sizeReduction()), this, SLOT(resizingSmall()));
}

//Redirection of a received error
void FLWindow::errorPrint(const char* msg){
    emit error(msg);
}

//Reaction to the modifications of the ToolBar options
void FLWindow::modifiedOptions(string text, int value, int port, const string& ipServer){
    
    if(ipServer.compare(fIpRemoteServer) != 0)
        fIpRemoteServer = ipServer;
    
    if(fPortHttp != port)
        fPortHttp = port;
    
    printf("PORT HTTP = %i\n", fPortHttp);
    
    fEffect->update_compilationOptions(text, value);
}

//Reaction to the resizing the toolbar
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

//If CheckBox of ToolBar is checked or uncheck, the calculation has to be restored in or send out
void FLWindow::changeRemoteState(int state){
    
    string error("");
    
//Checked = send dsp away
    if(state){
        
        if(update_Window(kGetRemote, NULL, error))
            emit errorPrint("Remote Processing Activated");
        else{
            emit errorPrint(error.c_str());
//            emit closeWin();
        }
    }
    else{

        if(update_Window(kGetLocal, fEffect, error))
            emit errorPrint("Remote Processing Desactivated");
        else{
            emit errorPrint(error.c_str());
//            emit closeWin();
        }
    }
}

//Does window contain a default Faust process?
bool FLWindow::is_Default(){
    
    string sourceContent = pathToContent(fEffect->getSource());
    
    if(sourceContent.compare("process = !,!:0,0;") == 0)
        return true;
    else 
        return false;
}

//------------------------CLOSING ACTIONS

//Reaction to click an x button
void FLWindow::closeEvent(QCloseEvent* /*event*/){
    
    if(!fShortcut)
        emit closeWin();
    else
        emit shut_AllWindows();
}

//A way to know if user is trying shortcut ALT + click on x button of a window
//= Tracking the event of ALT pressed
void FLWindow::keyPressEvent(QKeyEvent* event){ 
    
    if(event->key() == Qt::Key_Alt)
        fShortcut = true;
}

void FLWindow::keyReleaseEvent(QKeyEvent* event){
    
    if(event->key() == Qt::Key_Alt)
        fShortcut = false;
}

//During the execution, when a window is shut, its associate folder has to be removed
void FLWindow::shut_Window(){

    deleteDirectoryAndContent(fHome);    
    close_Window();
}

//Closing the window without removing its property for example when the application is quit
void FLWindow::close_Window(){
    
    hide();
    
    if(fClientOpen)
        fAudioManager->stop();
    
    deleteInterfaces();
    
    if(fHttpdWindow){
        delete fHttpdWindow;
        fHttpdWindow = NULL;
    }
//     printf("deleting instance = %p\n", current_DSP);   
    deleteDSPInstance((llvm_dsp*)fCurrent_DSP);
    
    printf("DELETE AUDIO MANAGER FROM CLOSE WIN\n");
    
    delete fAudioManager;
    delete fMenu;
}

//Delete of QTinterface and of saving graphical interface
void FLWindow::deleteInterfaces(){
    delete fInterface;
    delete fRCInterface;
    delete fOscInterface;
    fInterface = NULL;
    fRCInterface = NULL;
    fOscInterface = NULL;
}

//------------------------DRAG AND DROP ACTIONS

//Reaction to drop on the window
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

void FLWindow::dragLeaveEvent ( QDragLeaveEvent * /*event*/ ){
    //    setWindowFlags();
    centralWidget()->show();
}

//------------------------WINDOW ACTIONS

//Artificial content of a default window
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

//Show Window on front end
void FLWindow::frontShow(){
    show();
    raise();
}

//Initialization of User Interface + StartUp of Audio Client
bool FLWindow::init_Window(bool init, bool /*recall*/, string& errorMsg){
    
    fCurrent_DSP = createDSPInstance(fEffect->getFactory());

    string textOptions = fEffect->getCompilationOptions();
    if(textOptions.compare(" ") == 0)
        textOptions = "";
    
    fMenu->setOptions(textOptions.c_str());
    fMenu->setVal(fEffect->getOptValue());
    fMenu->setPort(fPortHttp);
    fMenu->setIP(fIpRemoteServer);
    
    if (fCurrent_DSP == NULL){
        errorMsg = "Impossible to create a DSP instance"; 
        return false;
    }
    
    //Window tittle is build with the window Name + effect Name
    string inter = fWindowName + " : " + fEffect->getName();
    
    fInterface = new QTGUI(this, inter.c_str());
    fRCInterface = new FUI;
    
    char* argv[1];
    argv[0] = (char*)fWindowName.c_str();
    
    fOscInterface = new OSCUI(argv[0], 1, argv);
    
    printf("OSCINTERFACE = %p\n", fOscInterface);
    
    if(fRCInterface && fInterface && fOscInterface){
        
        //Building interface and Audio parameters
        fCurrent_DSP->buildUserInterface(fRCInterface);
        fCurrent_DSP->buildUserInterface(fInterface);
        fCurrent_DSP->buildUserInterface(fOscInterface);
        
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
            fOscInterface->run();
            fInterface->run();
            return true;
        } 
        else {
            deleteInterfaces();
            return false;
        }
    } 
    
    errorMsg = "Interface could not be allocated";
    return false;
}

//Change of the effect in a Window
bool FLWindow::update_Window(int becomeRemote, FLEffect* newEffect, string& error){
    
    printf("FLWindow::update_Win\n");
    
    //Step 1 : Save the parameters of the actual interface
    fXPos = this->geometry().x();
    fYPos = this->geometry().y();
    save_Window(); 
    
    //Step 2 : Delete the actual interface
    hide(); 
    deleteInterfaces();
    
    //Step 3 : creating the user interface
    fRCInterface = new FUI();
    
    char* argv[1];
    argv[0] = (char*)fWindowName.c_str();
    
    fOscInterface = new OSCUI(argv[0], 1, argv);
    
    string newName = fEffect->getName();
    
    dsp* charging_DSP = NULL;
    Remote_DSP_Factory* charging_Factory = NULL;
    
    if(becomeRemote == kGetLocal || (becomeRemote == kCrossFade && fIsLocal)){
    
        //Step 4 : creating the new DSP instance
        charging_DSP = createDSPInstance(newEffect->getFactory());
        newName = newEffect->getName();
    }
    else if(becomeRemote == kGetRemote || (becomeRemote == kCrossFade && !fIsLocal)){
        
        //Step 4 : creating the new DSP instance
        
        int nArg = 2;
        char* argu[2];
        
        argu[0] = "--NJ_ip";
        argu[1] = (char*)(searchLocalIP().toStdString().c_str());
        
        charging_Factory = createRemoteFactory(fIpRemoteServer, pathToContent(fEffect->getSource()).c_str(), nArg, argu, fEffect->getOptValue(), error);
        
        if(charging_Factory == NULL)
            return false;
        
//        printf("SR & BS = %i | %i\n", fAudioManager->sample_rate(), fAudioManager->get_buffer_size());
        
        charging_DSP = createRemoteInstance(charging_Factory, fAudioManager->get_sample_rate(), fAudioManager->get_buffer_size(), error);
        
        if (charging_DSP == NULL)
            deleteRemoteFactory(charging_Factory);
        
        //    printf("charging DSP = %p\n", charging_DSP);
    }
    
    if (charging_DSP == NULL)
        return false;
    
    //Step 5 : get the new compilation parameters
    string textOptions = fEffect->getCompilationOptions();
    if(textOptions.compare(" ") == 0)
        textOptions = "";
    fMenu->setOptions(textOptions.c_str());
    fMenu->setVal(fEffect->getOptValue());
    fMenu->setPort(fPortHttp);
    fMenu->setIP(fIpRemoteServer);
    
    if(fRCInterface && fOscInterface){
        
        if(!fAudioManager->init_FadeAudio(error, newName.c_str(), charging_DSP)){
            
//            printf("INIT FADE AUDIO FAILED\n");
            
            //Step 6 : Recall the parameters of the window
            string intermediate = fWindowName + " : " + fEffect->getName();
            
            fInterface = new QTGUI(this, intermediate.c_str());
            
            fCurrent_DSP->buildUserInterface(fRCInterface);
            fCurrent_DSP->buildUserInterface(fInterface);
            fCurrent_DSP->buildUserInterface(fOscInterface);
            
            recall_Window();
            setGeometry(fXPos, fYPos, 0, 0);
            adjustSize();
            show();
            //Step 9 : Launch User Interface
            fInterface->run();
            fOscInterface->run();
            
            return false;
        }
        
        //Step 6 : Set the new interface
        string inter = fWindowName + " : " + newName;
        
        fInterface = new QTGUI(this, inter.c_str());
        
        charging_DSP->buildUserInterface(fRCInterface);
        charging_DSP->buildUserInterface(fInterface);
        charging_DSP->buildUserInterface(fOscInterface);
        
        //Step 7 : Recall the parameters of the window
        
        recall_Window();
        
        fAudioManager->start_Fade();
        
        setGeometry(fXPos, fYPos, 0, 0);
        adjustSize();
        show();
        
        fAudioManager->wait_EndFade();
        
        //Step 8 : Change the current DSP as the dropped one
        dsp* VecInt;
        
//        printf("CURRENT DSP = %p\n", fCurrent_DSP);
//        printf("CHARGING DSP = %p\n", charging_DSP);
        
        VecInt = fCurrent_DSP;
        fCurrent_DSP = charging_DSP; 
        charging_DSP = VecInt;
        
//        printf("deleting DSP leaving = %p\n", charging_DSP);
        
        if(becomeRemote == kGetRemote){
            deleteDSPInstance((llvm_dsp*)charging_DSP);
            fIsLocal = true;
            fRemoteFactory = charging_Factory;
        }
        else if(becomeRemote == kGetLocal){
            deleteRemoteInstance((Remote_DSP*)charging_DSP);
            deleteRemoteFactory(fRemoteFactory);
            fIsLocal = false;
        }
        else{
            if(fIsLocal){
                deleteDSPInstance((llvm_dsp*)charging_DSP);
                fEffect = newEffect;
            }
            else{
                
                Remote_DSP_Factory* factoryInt;
                
                factoryInt = fRemoteFactory;
                fRemoteFactory = charging_Factory; 
                charging_Factory = factoryInt;
                
                deleteRemoteInstance((Remote_DSP*)charging_DSP);
                deleteRemoteFactory(charging_Factory);
            }
        }
        
        //Step 9 : Launch User Interface
        fInterface->run();
        fOscInterface->run();
        return true;
        
    }
    else{
        error = "Impossible to allocate DSP interface";
        return false;
    }
}

//Start/Stop of audio
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

//Switch of Audio architecture
bool FLWindow::update_AudioArchitecture(string& error){
    
    AudioCreator* creator = AudioCreator::_Instance(fSettingsFolder, NULL);
    delete fAudioManager;
    
    fAudioManager = creator->createAudioManager(creator->getNewSettings());
    
    printf("NEW MANAGER\n");
    
    if(init_audioClient(error))
        return true;
    else
        return false;
}

//Initialization of audio Client
bool FLWindow::init_audioClient(string& error){
    
    if(fAudioManager->initAudio(error, fEffect->getName().c_str(), fCurrent_DSP, fWindowName.c_str())){ 
            
        recall_Window();
        return true;
    }
    else{
        return false;
    }
}

//------------------------SAVING WINDOW ACTIONS

//When the window is imported in a current session, its properties may have to be updated
void FLWindow::update_ConnectionFile(list<pair<string,string> > changeTable){
    
    string connectFile = fHome + "/" + fWindowName + ".jc";
    fAudioManager->change_Connections(connectFile.c_str(), changeTable);
}

//Read/Write window properties in saving file
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

int FLWindow::get_Port(){
    
    if(fHttpdWindow != NULL)
        return fHttpdWindow->get_Port();
    else
        return fPortHttp;
}

string FLWindow::get_RemoteServerIP(){
    return fIpRemoteServer;
}

//------------------------HTTPD

//Calculation of screen position of the HTTP window, depending on its index
int FLWindow::calculate_Coef(){
    
    int multiplCoef = fWindowIndex;
    while(multiplCoef > 20){
        multiplCoef-=20;
    }
    return multiplCoef;
}

//Initalization of QrCode Window
bool FLWindow::init_Httpd(string& error){
    
    if(fHttpdWindow == NULL){
        fHttpdWindow = new HTTPWindow();
        connect(fHttpdWindow, SIGNAL(closeAll()), this, SLOT(shut_All()));
    }
    
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
            fHttpdWindow->display_HttpdWindow(calculate_Coef()*10, 0, fGeneralPort);
            
            return true;
        }
        else
            return false;
    }
    else{
        error = "Httpd Window could not be allocated!"; 
        return false;
    }
}

bool FLWindow::is_httpdWindow_active() {
    return fHttpdWindow->isActiveWindow();
}

void FLWindow::hide_httpdWindow() {
    fHttpdWindow->hide();
}

string FLWindow::get_HttpUrl() {
    return fHttpdWindow->getUrl();
}

void FLWindow::set_generalPort(int port){
    fGeneralPort = port;
}

//------------------------Right Click Reaction

void FLWindow::contextMenuEvent(QContextMenuEvent* ev) {
    
    fWindowMenu->exec(ev->globalPos());
}

void FLWindow::set_MenuBar(){
    
    //----------------FILE
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    
    QAction* newAction = new QAction(tr("&New Default Window"), this);
    newAction->setShortcut(tr("Ctrl+N"));
    newAction->setToolTip(tr("Open a new empty file"));
    connect(newAction, SIGNAL(triggered()), this, SLOT(create_Empty()));
    
    QAction* openAction = new QAction(tr("&Open..."),this);
    openAction->setShortcut(tr("Ctrl+O"));
    openAction->setToolTip(tr("Open a DSP file"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open_New()));
    
    QMenu* menuOpen_Example = new QMenu(tr("&Open Example"), fileMenu);
    
    QDir examplesDir(":/");
    
    if(examplesDir.cd("Examples")){
        
        QFileInfoList children = examplesDir.entryInfoList(QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
        
        QFileInfoList::iterator it;
        int i = 0; 
        
        QAction** openExamples = new QAction* [children.size()];
        
        for(it = children.begin(); it != children.end(); it++){
            
            openExamples[i] = new QAction(it->baseName(), menuOpen_Example);
            openExamples[i]->setData(QVariant(it->absoluteFilePath()));
            connect(openExamples[i], SIGNAL(triggered()), this, SLOT(open_Example()));
            
            menuOpen_Example->addAction(openExamples[i]);
            i++;
        }
    }
    
    QMenu* openRecentAction = new QMenu(tr("&Open Recent File"), fileMenu);
    
    fRecentFileAction = new QAction* [kMAXRECENTFILES];
    
    for(int i=0; i<kMAXRECENTFILES; i++){
        fRecentFileAction[i] = new QAction(this);
        fRecentFileAction[i]->setVisible(false);
        connect(fRecentFileAction[i], SIGNAL(triggered()), this, SLOT(open_Recent_File()));
        
        openRecentAction->addAction(fRecentFileAction[i]);
    }
        
//SESSION
    
    QAction* takeSnapshotAction = new QAction(tr("&Take Snapshot"),this);
    takeSnapshotAction->setShortcut(tr("Ctrl+S"));
    takeSnapshotAction->setToolTip(tr("Save current state"));
    connect(takeSnapshotAction, SIGNAL(triggered()), this, SLOT(take_Snapshot()));
    
    QAction* recallSnapshotAction = new QAction(tr("&Recall Snapshot..."),this);
    recallSnapshotAction->setShortcut(tr("Ctrl+R"));
    recallSnapshotAction->setToolTip(tr("Close all the opened window and open your snapshot"));
    connect(recallSnapshotAction, SIGNAL(triggered()), this, SLOT(recallSnapshot()));
    
    QMenu* recallRecentAction = new QMenu(tr("&Recall Recent Snapshot"), fileMenu);
    QMenu* importRecentAction = new QMenu(tr("&Import Recent Snapshot"), fileMenu);
    
    fRrecentSessionAction = new QAction* [kMAXRECENTSESSIONS];
    fIrecentSessionAction = new QAction* [kMAXRECENTSESSIONS];
    
    for(int i=0; i<kMAXRECENTSESSIONS; i++){
        fRrecentSessionAction[i] = new QAction(this);
        fRrecentSessionAction[i]->setVisible(false);
        connect(fRrecentSessionAction[i], SIGNAL(triggered()), this, SLOT(recall_Recent_Session()));
        
        recallRecentAction->addAction(fRrecentSessionAction[i]);
        
        fIrecentSessionAction[i] = new QAction(this);
        fIrecentSessionAction[i]->setVisible(false);
        connect(fIrecentSessionAction[i], SIGNAL(triggered()), this, SLOT(import_Recent_Session()));
        
        importRecentAction->addAction(fIrecentSessionAction[i]);
    }
    
    QAction* importSnapshotAction = new QAction(tr("&Import Snapshot..."),this);
    importSnapshotAction->setShortcut(tr("Ctrl+I"));
    importSnapshotAction->setToolTip(tr("Import your snapshot in the current session"));
    connect(importSnapshotAction, SIGNAL(triggered()), this, SLOT(importSnapshot()));
    
    QAction* shutAction = new QAction(tr("&Close Window"),this);
    shutAction->setShortcut(tr("Ctrl+W"));
    shutAction->setToolTip(tr("Close the current Window"));
    connect(shutAction, SIGNAL(triggered()), this, SLOT(shut()));
    
    QAction* shutAllAction = new QAction(tr("&Close All Windows"),this);
    shutAllAction->setShortcut(tr("Ctrl+Alt+W"));
    shutAllAction->setToolTip(tr("Close all the Windows"));
    connect(shutAllAction, SIGNAL(triggered()), this, SLOT(shut_All()));
    
    QAction* closeAllAction = new QAction(tr("&Closing"),this);
    closeAllAction->setShortcut(tr("Ctrl+Q"));
    closeAllAction = new QAction(tr("&Quit FaustLive"),this);
    closeAllAction->setToolTip(tr("Close the application"));   
    connect(closeAllAction, SIGNAL(triggered()), this, SLOT(closeAll()));
    
    fileMenu->addAction(newAction);    
    fileMenu->addSeparator();
    fileMenu->addAction(openAction);
    fileMenu->addAction(menuOpen_Example->menuAction());
    fileMenu->addAction(openRecentAction->menuAction());
    fileMenu->addSeparator();
    fileMenu->addAction(takeSnapshotAction);
    fileMenu->addSeparator();
    fileMenu->addAction(recallSnapshotAction);
    fileMenu->addAction(recallRecentAction->menuAction());
    fileMenu->addSeparator();
    fileMenu->addAction(importSnapshotAction);
    fileMenu->addAction(importRecentAction->menuAction());
    fileMenu->addSeparator();
    fileMenu->addAction(shutAction);
    fileMenu->addAction(shutAllAction);
    fileMenu->addSeparator();
    fileMenu->addAction(closeAllAction);
    
    menuBar()->addSeparator();

    //-----------------Window
    
    QAction* editAction = new QAction(tr("&Edit Faust Source"), this);
    editAction->setShortcut(tr("Ctrl+E"));
    editAction->setToolTip(tr("Edit the source"));
    connect(editAction, SIGNAL(triggered()), this, SLOT(edit()));
    
    QAction* pasteAction = new QAction(tr("&Paste"),this);
    pasteAction->setShortcut(tr("Ctrl+V"));
    pasteAction->setToolTip(tr("Paste a DSP"));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
    
    QAction* duplicateAction = new QAction(tr("&Duplicate"),this);
    duplicateAction->setShortcut(tr("Ctrl+D"));
    duplicateAction->setToolTip(tr("Duplicate current DSP"));
    connect(duplicateAction, SIGNAL(triggered()), this, SLOT(duplicate()));
    
    QAction* httpdViewAction = new QAction(tr("&View QRcode"),this);
    httpdViewAction->setShortcut(tr("Ctrl+K"));
    httpdViewAction->setToolTip(tr("Print the QRcode of TCP protocol"));
    connect(httpdViewAction, SIGNAL(triggered()), this, SLOT(httpd_View()));
    
    QAction* svgViewAction = new QAction(tr("&View SVG Diagram"),this);
    svgViewAction->setShortcut(tr("Ctrl+G"));
    svgViewAction->setToolTip(tr("Open the SVG Diagram in a browser"));
    connect(svgViewAction, SIGNAL(triggered()), this, SLOT(svg_View()));
    
    QAction* exportAction = new QAction(tr("&Export As..."), this);
    exportAction->setShortcut(tr("Ctrl+P"));
    exportAction->setToolTip(tr("Export the DSP in whatever architecture you choose"));
    connect(exportAction, SIGNAL(triggered()), this, SLOT(exportManage()));
    
    fWindowMenu = menuBar()->addMenu(tr("&Window"));
    fWindowMenu->addAction(editAction);
    fWindowMenu->addAction(pasteAction);
    fWindowMenu->addAction(duplicateAction);
    fWindowMenu->addSeparator();
    fWindowMenu->addAction(httpdViewAction);
    fWindowMenu->addAction(svgViewAction);
    fWindowMenu->addSeparator();
    fWindowMenu->addAction(exportAction);
    
    menuBar()->addSeparator();
    
    //-----------------NAVIGATE
    
    fNavigateMenu = new QMenu;
    fNavigateMenu = menuBar()->addMenu(tr("&Navigate"));    
    menuBar()->addSeparator();
    
    //---------------------MAIN MENU
    
    QAction* aboutQtAction = new QAction(tr("&About Qt"), this);
    aboutQtAction->setToolTip(tr("Show the library's About Box"));
    connect(aboutQtAction, SIGNAL(triggered()), this, SLOT(aboutQt()));
    
    QAction* preferencesAction = new QAction(tr("&Preferences"), this);
    preferencesAction->setToolTip(tr("Set the preferences of the application"));
    connect(preferencesAction, SIGNAL(triggered()), this, SLOT(preferences()));
    
    //--------------------HELP
    
    QAction* aboutAction = new QAction(tr("&Help..."), this);
    aboutAction->setToolTip(tr("Show the library's About Box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutFaustLive()));

    
    QAction* presentationAction = new QAction(tr("&About FaustLive"), this);
    presentationAction->setToolTip(tr("Show the presentation Menu"));
    connect(presentationAction, SIGNAL(triggered()), this, SLOT(show_presentation()));
    
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    
    helpMenu->addAction(aboutQtAction);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(presentationAction);
    helpMenu->addSeparator();
    helpMenu->addAction(preferencesAction);
}

//------SLOTS FROM MENU ACTIONS THAT REDIRECT TO FLAPP
void FLWindow::create_Empty(){
    emit create_Empty_Window();
}

void FLWindow::open_New(){
    emit open_New_Window();
}

void FLWindow::open_Example(){
    
    QAction* action = qobject_cast<QAction*>(sender());
    QString toto(action->data().toString());
    
    emit open_Ex(toto);
}

void FLWindow::take_Snapshot(){
    emit takeSnapshot();
}

void FLWindow::recallSnapshot(){
    emit recallSnapshotFromMenu();
}

void FLWindow::importSnapshot(){
    emit importSnapshotFromMenu();
}

void FLWindow::shut(){
    
    emit closeWin();
}

void FLWindow::shut_All(){
    emit shut_AllWindows();
}

void FLWindow::closeAll(){
    emit closeAllWindows();
}

void FLWindow::edit(){
    emit edit_Action();
}

void FLWindow::paste(){
    emit paste_Action();
}

void FLWindow::duplicate(){
    emit duplicate_Action();
}

void FLWindow::httpd_View(){
    emit httpd_View_Window();
}

void FLWindow::svg_View(){
    emit svg_View_Action();
}

void FLWindow::exportManage(){
    emit export_Win();
}

void FLWindow::aboutQt(){
    emit show_aboutQt();
}

void FLWindow::preferences(){
    emit show_preferences();
}

void FLWindow::aboutFaustLive(){
    emit apropos();
}

void FLWindow::show_presentation(){
    emit show_presentation_Action();
}

void FLWindow::set_RecentFile(list<pair<string, string> > recents){
    fRecentFiles = recents;
}

void FLWindow::update_RecentFileMenu(){
    int j = 0;
    
    list<pair<string, string> >::iterator it;
    
    for (it = fRecentFiles.begin(); it != fRecentFiles.end(); it++) {
        
        if(j<kMAXRECENTFILES){
            
            QString text;
            text += it->second.c_str();
            fRecentFileAction[j]->setText(text);
            fRecentFileAction[j]->setData(it->first.c_str());
            fRecentFileAction[j]->setVisible(true);
            
            j++;
        }
    }
}

void FLWindow::open_Recent_File(){
    
    QAction* action = qobject_cast<QAction*>(sender());
    string toto(action->data().toString().toStdString());
    
    emit open_File(toto);
}

void FLWindow::set_RecentSession(QStringList recents){
    fRecentSessions = recents;
}

void FLWindow::update_RecentSessionMenu(){

    QMutableStringListIterator i(fRecentSessions);
    while(i.hasNext()){
        if(!QFile::exists(i.next()))
            i.remove();
    }
    
    for(int j=0; j<kMAXRECENTSESSIONS; j++){
        if(j<fRecentSessions.count()){
            
            QString path = QFileInfo(fRecentSessions[j]).baseName();
            
            QString text = tr("&%1 %2").arg(j+1).arg(path);
            fRrecentSessionAction[j]->setText(text);
            fRrecentSessionAction[j]->setData(fRecentSessions[j]);
            fRrecentSessionAction[j]->setVisible(true);
            
            fIrecentSessionAction[j]->setText(text);
            fIrecentSessionAction[j]->setData(fRecentSessions[j]);
            fIrecentSessionAction[j]->setVisible(true);
            
            //            printf("TEXT = %s\n", text.toStdString().c_str());
        }
        else{
            fRrecentSessionAction[j]->setVisible(false);
            fIrecentSessionAction[j]->setVisible(false);
        }
    }
}

void FLWindow::recall_Recent_Session(){

    QAction* action = qobject_cast<QAction*>(sender());
    string toto(action->data().toString().toStdString());
    
    emit recall_Snapshot(toto, false);
}

void FLWindow::import_Recent_Session(){
    
    QAction* action = qobject_cast<QAction*>(sender());
    string toto(action->data().toString().toStdString());
    
    emit recall_Snapshot(toto, true);
}

void FLWindow::initNavigateMenu(QList<QString> wins){
        
    QList<QString>::iterator it;
    for(it = wins.begin(); it != wins.end() ; it++){

        QAction* fifiWindow = new QAction(*it, fNavigateMenu);
        fFrontWindow.push_back(fifiWindow);
        
        fifiWindow->setData(QVariant(*it));
        connect(fifiWindow, SIGNAL(triggered()), this, SLOT(frontShowFromMenu()));
        
        fNavigateMenu->addAction(fifiWindow);
        
        printf("NAME = %s\n", (*it).toStdString().c_str());
    }
}

void FLWindow::addWinInMenu(QString name){
    
    QAction* fifiWindow = new QAction(name, fNavigateMenu);
    fFrontWindow.push_back(fifiWindow);
    
    fifiWindow->setData(QVariant(name));
    connect(fifiWindow, SIGNAL(triggered()), this, SLOT(frontShowFromMenu()));
    
    fNavigateMenu->addAction(fifiWindow);
}

void FLWindow::deleteWinInMenu(QString name){
    
    QList<QAction*>::iterator it;
    for(it = fFrontWindow.begin(); it != fFrontWindow.end() ; it++){
        if((*it)->data().toString().compare(name) == 0){
            fNavigateMenu->removeAction(*it);
            fFrontWindow.removeOne(*it);
            break;
        }
    }
}

void FLWindow::frontShowFromMenu(){
    
    QAction* action = qobject_cast<QAction*>(sender());

    emit front(action->data().toString());
}

