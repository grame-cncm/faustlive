//
//  FaustLiveApp.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLApp.h"
#include "FLrenameDialog.h"
#if !defined(_WIN32) || defined(__MINGW32__)
#include "FLServerHttp.h"
#endif
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif
#include "FLSessionManager.h"
#include "FLWindow.h"
#include "FLComponentWindow.h"
#include "FLErrorWindow.h"
#include "FLMessageWindow.h"
#include "FLHelpWindow.h"
#include "FLPresentationWindow.h"
#include "utilities.h"

#ifdef REMOTE
#include "faust/remote-dsp.h"
#include "FLRemoteDSPScanner.h"
#include "Server.h"
#endif

#include "FLSettings.h"
#include "FLWinSettings.h"
#include "FLPreferenceWindow.h"

#include "FJUI.h"

//----------------------CONSTRUCTOR/DESTRUCTOR---------------------------

FLApp::FLApp(int& argc, char** argv) : QApplication(argc, argv){

    //Create Current Session Folder
    create_Session_Hierarchy();
    
    FLSettings::createInstance(fSessionFolder);
    FLSessionManager::createInstance(fSessionFolder);
   
#if !defined(_WIN32) || defined(__MINGW32__)
    FLServerHttp::createInstance(fHtmlFolder.toStdString());
    connect(FLServerHttp::_Instance(), SIGNAL(compile(const char*, int)), this, SLOT(compile_HttpData(const char*, int)));
#endif
#ifdef REMOTE
    Server* serv = Server::_Instance();
    serv->start(FLSettings::_Instance()->value("General/Network/RemoteServerPort", 5555).toInt());
#endif
    //Initializing screen parameters
    QSize screenSize = QApplication::desktop()->screen(QApplication::desktop()->primaryScreen())->geometry().size();
    fScreenWidth = screenSize.width();
    fScreenHeight = screenSize.height();
    
    //Base Name of application's windows
    fWindowBaseName = "FLW-";
    
    styleClicked(FLSettings::_Instance()->value("General/Style", "Default").toString());
    
	//Initializing Recalling 
	fRecalling = false;
    
#ifndef __APPLE__
    //For the application not to quit when the last window is closed
	connect(this, SIGNAL(lastWindowClosed()), this, SLOT(closeAllWindows()));
//	setQuitOnLastWindowClosed(true);
#else
    setQuitOnLastWindowClosed(false);
#endif
    
    fMenuBar = new QMenuBar(NULL);
    
    //Initializing menu actions 
    fRecentFileAction = new QAction* [kMAXRECENT];
    for(int i=0; i<kMAXRECENT; i++){
        fRecentFileAction[i] = new QAction(NULL);
        fRecentFileAction[i]->setVisible(false);
        connect(fRecentFileAction[i], SIGNAL(triggered()), this, SLOT(open_Recent_File()));
    }
    
    fRrecentSessionAction = new QAction* [kMAXRECENT];
    fIrecentSessionAction = new QAction* [kMAXRECENT];
    
    for(int i=0; i<kMAXRECENT; i++){
        fRrecentSessionAction[i] = new QAction(NULL);
        fRrecentSessionAction[i]->setVisible(false);
        connect(fRrecentSessionAction[i], SIGNAL(triggered()), this, SLOT(recall_Recent_Session()));

        fIrecentSessionAction[i] = new QAction(NULL);
        fIrecentSessionAction[i]->setVisible(false);
        connect(fIrecentSessionAction[i], SIGNAL(triggered()), this, SLOT(import_Recent_Session()));
    }
    
    setup_Menu();
    
    recall_Recent_Files();
    recall_Recent_Sessions();
    
    //Initializing OutPut Window
    connect(FLErrorWindow::_Instance(), SIGNAL(closeAll()), this, SLOT(shut_AllWindows_FromWindow()));

    // Presentation Window Initialization
    connect(FLPresentationWindow::_Instance(), SIGNAL(newWin()), this, SLOT(create_Empty_Window()));
    connect(FLPresentationWindow::_Instance(), SIGNAL(openWin()), this, SLOT(open_New_Window()));
    connect(FLPresentationWindow::_Instance(), SIGNAL(openSession()), this, SLOT(importSnapshotFromMenu()));
    connect(FLPresentationWindow::_Instance(), SIGNAL(openPref()), this, SLOT(Preferences()));
    connect(FLPresentationWindow::_Instance(), SIGNAL(openHelp()), FLHelpWindow::_Instance(), SLOT(show()));
    connect(FLPresentationWindow::_Instance(), SIGNAL(openExample(const QString&)), this, SLOT(openExampleAction(const QString&)));
    
    //fPresWin->setWindowFlags(*Qt::FramelessWindowHint);
    //Initialiazing Remote Drop Server
#ifdef HTTPCTRL
    launch_Server();
#endif
    
    //If no event opened a window half a second after the application was created, a initialized window is created
    fInitTimer = new QTimer(this);
    connect(fInitTimer, SIGNAL(timeout()), this, SLOT(init_Timer_Action()));
    fInitTimer->start(500);
}

FLApp::~FLApp(){

    QString tempPath = fSessionFolder + "/Temp";
    deleteDirectoryAndContent(tempPath);
    
    save_Recent_Files();
    save_Recent_Sessions();
    
    for(int i=0; i<kMAXRECENT; i++){
        delete fRecentFileAction[i];
        delete fRrecentSessionAction[i];
        delete fIrecentSessionAction[i];
    }
    
    delete fAudioCreator;
    
    delete fRecentFileAction;
    delete fRrecentSessionAction;
    delete fIrecentSessionAction;
    delete fMenuBar;
    
    delete fInitTimer;
    
//    FLHelpWindow::deleteInstance();
    
    FLSettings::deleteInstance();
    FLSessionManager::deleteInstance();
#if !defined(_WIN32) || defined(__MINGW32__)
    FLServerHttp::deleteInstance();
#endif
}

void FLApp::create_Session_Hierarchy(){
    
	QString separationChar;
    
    //Initialization of current Session Path  
#ifdef _WIN32
	char path[512];
	if(!SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path))
        fSessionFolder = path;
	fSessionFolder += "\\FaustLive-CurrentSession-";
	separationChar = "\\";
#else
    fSessionFolder = getenv("HOME");
    fSessionFolder += "/.FaustLive-CurrentSession-";
	separationChar = "/";
#endif
    
    fSessionFolder += APP_VERSION;
    if(!QFileInfo(fSessionFolder).exists()){
        QDir direct(fSessionFolder);
        direct.mkdir(fSessionFolder);
    } 
    
    //    To copy QT resources that where loaded at compilation with application.qrc
    fExamplesFolder = fSessionFolder + separationChar  + "Examples";
    if(!QFileInfo(fExamplesFolder).exists()){
        QDir direct(fExamplesFolder);
        direct.mkdir(fExamplesFolder);
    }  
    
    QDir exDir(":/");
    
    if(exDir.cd("Examples")){
        
        QFileInfoList children = exDir.entryInfoList(QDir::Files);
        
        QFileInfoList::iterator it;
        
        for(it = children.begin(); it != children.end(); it++){
            
			QString pathInSession = fExamplesFolder + separationChar + it->baseName() + "." + it->completeSuffix();
            
            if(!QFileInfo(pathInSession).exists()){
                
                QFile file(it->absoluteFilePath());
                file.copy(pathInSession);
                
                QFile newFile(pathInSession);
                newFile.setPermissions(QFile::ReadOwner);
            }
        }
        
    }
    
    
    fLibsFolder = fSessionFolder + separationChar  + "Libs";
    if(!QFileInfo(fLibsFolder).exists()){
        QDir direct(fLibsFolder);
        direct.mkdir(fLibsFolder);
    }  

    QDir libsDir(":/");
    
    if(libsDir.cd("Libs")){
        
        QFileInfoList children = libsDir.entryInfoList(QDir::Files);
        
        QFileInfoList::iterator it;
        
        for(it = children.begin(); it != children.end(); it++){
            
			QString pathInSession = fLibsFolder + separationChar + it->baseName() + "." + it->completeSuffix();
            
            if(!QFileInfo(pathInSession).exists()){
                
                QFile file(it->absoluteFilePath());
                file.copy(pathInSession);
            }
        }
    }
        
    fHtmlFolder = fSessionFolder + separationChar  + "Html";
    if(!QFileInfo(fHtmlFolder).exists()){
        QDir direct(fHtmlFolder);
        direct.mkdir(fHtmlFolder);
    }  
    
    QDir htmlDir(":/");
    
    if(htmlDir.cd("Html")){
        QFileInfoList children = htmlDir.entryInfoList(QDir::Files);
        
        QFileInfoList::iterator it;
        
        for(it = children.begin(); it != children.end(); it++){
            
			QString pathInSession = fHtmlFolder + separationChar + it->baseName() + "." + it->completeSuffix();
            
            if(!QFileInfo(pathInSession).exists()){
                
                QFile file(it->absoluteFilePath());
                file.copy(pathInSession);
            }
        }
    }
    
    fDocFolder = fSessionFolder + separationChar  + "Documentation";
    if(!QFileInfo(fDocFolder).exists()){
        QDir direct(fDocFolder);
        direct.mkdir(fDocFolder);
    }  
    
    
    QDir docDir(":/");
    
    if(docDir.cd("Documentation")){
        QFileInfoList children = docDir.entryInfoList(QDir::Files);
        
        QFileInfoList::iterator it;
        
        for(it = children.begin(); it != children.end(); it++){
            
			QString pathInSession = fDocFolder + separationChar + it->baseName() + "." + it->completeSuffix();
            
            if(!QFileInfo(pathInSession).exists()){
                
                QFile file(it->absoluteFilePath());
                file.copy(pathInSession);
            }
        }
    }
    
    QString factoryFolder = fSessionFolder + "/SHAFolder";
    QDir shaFolder(factoryFolder);
    shaFolder.mkdir(factoryFolder);
    
    QString tempPath = fSessionFolder + "/Temp";
    QDir tempFolder(tempPath);
    tempFolder.mkdir(tempPath);
    
    QString pathWindows = fSessionFolder + "/Windows";
    
    QDir windowsDir(pathWindows);
    if(!windowsDir.exists())
        windowsDir.mkdir(pathWindows);
    
    QString sched(":/usr/local/lib/faust/scheduler.ll");
    if(QFileInfo(sched).exists()){
        QString newScheduler = fLibsFolder + "/scheduler.ll";
        
        QFile f(sched);
        f.copy(newScheduler);
    }
}

//-------- Build FaustLive Menu
QMenu* FLApp::create_FileMenu(){

    QMenu* fileMenu = new QMenu(tr("File"), 0);
    
    QAction* newAction = new QAction(tr("&New Default Window"), NULL);
    newAction->setShortcut(tr("Ctrl+N"));
    newAction->setToolTip(tr("Open a new empty file"));
    connect(newAction, SIGNAL(triggered()), this, SLOT(create_Empty_Window()));
    
    QAction* openAction = new QAction(tr("&Open..."),NULL);
    openAction->setShortcut(tr("Ctrl+O"));
    openAction->setToolTip(tr("Open a DSP file"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open_New_Window()));
    
    QAction* openRemoteAction = new QAction(tr("&Open remote DSP..."),NULL);
//    openAction->setShortcut(tr("Ctrl+O"));
    openRemoteAction->setToolTip(tr("Open a DSP file"));
    connect(openRemoteAction, SIGNAL(triggered()), this, SLOT(open_Remote_Window())); 
    
    //SESSION
    
    QAction* componentAction = new QAction(tr("New Component Creator"), fileMenu);
    //    componentAction->setShortcut(tr("Ctrl"));
    componentAction->setToolTip(tr("Open a new window to create a 2 dsp component"));
    connect(componentAction, SIGNAL(triggered()), this, SLOT(create_Component_Window()));
    
    QAction* takeSnapshotAction = new QAction(tr("&Take Snapshot"),NULL);
    takeSnapshotAction->setShortcut(tr("Ctrl+S"));
    takeSnapshotAction->setToolTip(tr("Save current state"));
    connect(takeSnapshotAction, SIGNAL(triggered()), this, SLOT(take_Snapshot()));
    
    QAction* recallSnapshotAction = new QAction(tr("&Recall Snapshot..."),NULL);
    recallSnapshotAction->setShortcut(tr("Ctrl+R"));
    recallSnapshotAction->setToolTip(tr("Close all the opened window and open your snapshot"));
    connect(recallSnapshotAction, SIGNAL(triggered()), this, SLOT(recallSnapshotFromMenu()));
    
    QAction* importSnapshotAction = new QAction(tr("&Import Snapshot..."),NULL);
    importSnapshotAction->setShortcut(tr("Ctrl+I"));
    importSnapshotAction->setToolTip(tr("Import your snapshot in the current session"));
    connect(importSnapshotAction, SIGNAL(triggered()), this, SLOT(importSnapshotFromMenu()));
    
    //SHUT
    
    QAction* shutAllAction = new QAction(tr("&Close All Windows"),NULL);
    shutAllAction->setShortcut(tr("Ctrl+Alt+W"));
    shutAllAction->setToolTip(tr("Close all the Windows"));
    connect(shutAllAction, SIGNAL(triggered()), this, SLOT(shut_AllWindows_FromMenu()));
    
    QAction* closeAllAction = new QAction(tr("&Quit FaustLive"),NULL);
    closeAllAction->setShortcut(tr("Ctrl+Q"));
    closeAllAction->setToolTip(tr("Close the application"));   
    connect(closeAllAction, SIGNAL(triggered()), this, SLOT(closeAllWindows()));
    
    fileMenu->addAction(newAction);    
    fileMenu->addSeparator();
    fileMenu->addAction(openAction);
    fileMenu->addAction(create_ExampleMenu()->menuAction());
    fileMenu->addAction(create_RecentFileMenu()->menuAction());
#ifdef REMOTE
    fileMenu->addAction(openRemoteAction);
#endif
    fileMenu->addSeparator();
    fileMenu->addAction(componentAction);
    fileMenu->addSeparator();
    fileMenu->addAction(takeSnapshotAction);
    fileMenu->addSeparator();
    fileMenu->addAction(recallSnapshotAction);
    fileMenu->addAction(create_LoadSessionMenu(true)->menuAction());
    fileMenu->addSeparator();
    fileMenu->addAction(importSnapshotAction);
    fileMenu->addAction(create_LoadSessionMenu(false)->menuAction());
    fileMenu->addSeparator();
    fileMenu->addAction(shutAllAction);
    fileMenu->addSeparator();
    fileMenu->addAction(closeAllAction);
    
    
    return fileMenu;
}

QMenu* FLApp::create_ExampleMenu(){
    
    QMenu* menuOpen_Example = new QMenu(tr("&Open Example"), NULL);
    
    QDir examplesDir(":/");
    
    if(examplesDir.cd("Examples")){
        
        QFileInfoList children = examplesDir.entryInfoList(QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
        
        QFileInfoList::iterator it;
        int i = 0; 
        
        QAction** openExamples = new QAction* [children.size()];
        
        for(it = children.begin(); it != children.end(); it++){
            
            openExamples[i] = new QAction(it->baseName(), NULL);
            openExamples[i]->setData(QVariant(it->absoluteFilePath()));
            connect(openExamples[i], SIGNAL(triggered()), this, SLOT(open_Example_From_FileMenu()));
            
            menuOpen_Example->addAction(openExamples[i]);
            i++;
        }
    }
    
    return menuOpen_Example;
}

QMenu* FLApp::create_RecentFileMenu(){
    
    QMenu* openRecentAction = new QMenu(tr("&Open Recent File"), NULL);
    
    for(int i=0; i<kMAXRECENT; i++)
        openRecentAction->addAction(fRecentFileAction[i]);
    
    return openRecentAction;
}

//@param recallOrImport : true = Recall ||| false = Import
QMenu* FLApp::create_LoadSessionMenu(bool recallOrImport){
    
    QString menuName("");
    
    if(recallOrImport)
        menuName = "Recall Recent Snapshot";
    else
        menuName = "Import Recent Snapshot";
    
    QMenu* loadRecentMenu = new QMenu( menuName, NULL);
    
    for(int i=0; i<kMAXRECENT; i++){
    
        if(recallOrImport)
            loadRecentMenu->addAction(fRrecentSessionAction[i]);
        else
            loadRecentMenu->addAction(fIrecentSessionAction[i]);
    }
    
    return loadRecentMenu;
}

QMenu* FLApp::create_NavigateMenu(){
    
    QMenu* navigateMenu = new QMenu(tr("Navigate"), 0);

    return navigateMenu;
    
}

QMenu* FLApp::create_HelpMenu(){
    
    QMenu* helpMenu = new QMenu(tr("Help"), 0);
    
    QAction* aboutQtAction = new QAction(tr("&About Qt"), NULL);
    aboutQtAction->setToolTip(tr("Show the library's About Box"));
    connect(aboutQtAction, SIGNAL(triggered()), this, SLOT(aboutQt()));
    
    QAction* preferencesAction = new QAction(tr("&Preferences"), NULL);
    preferencesAction->setToolTip(tr("Set the preferences of the application"));
    connect(preferencesAction, SIGNAL(triggered()), this, SLOT(Preferences()));
    
    QAction* aboutAction = new QAction(tr("&Help..."), NULL);
    aboutAction->setToolTip(tr("Show the library's About Box"));
    connect(aboutAction, SIGNAL(triggered()), FLHelpWindow::_Instance(), SLOT(show()));
    
    
        
    QAction* versionAction = new QAction(tr("&Version"), this);
    versionAction->setToolTip(tr("Show the version of the libraries used"));
    connect(versionAction, SIGNAL(triggered()), this, SLOT(version_Action()));
    
    QAction* presentationAction = new QAction(tr("&About FaustLive"), NULL);
    presentationAction->setToolTip(tr("Show the presentation Menu"));
    connect(presentationAction, SIGNAL(triggered()), this, SLOT(show_presentation_Action()));

    QAction* openFLDoc = new QAction(tr("&Open FaustLive Documentation"), NULL);
    openFLDoc->setToolTip(tr("Open FaustLive Documentation in appropriate application"));
    connect(openFLDoc, SIGNAL(triggered()), this, SLOT(open_FL_doc()));

    QAction* openFDoc = new QAction(tr("&Open Faust Documentation"), NULL);
    openFDoc->setToolTip(tr("Open Faust Documentation in appropriate application"));
    connect(openFDoc, SIGNAL(triggered()), this, SLOT(open_F_doc()));
    
    helpMenu->addAction(presentationAction);
    helpMenu->addSeparator();
    helpMenu->addAction(preferencesAction);
    helpMenu->addSeparator();
    helpMenu->addAction(openFLDoc);
    helpMenu->addAction(openFDoc);  
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(versionAction);
    helpMenu->addAction(aboutQtAction);


    return helpMenu;
}

void FLApp::setup_Menu(){
    
    //---------------------Presentation MENU
    
    connect(FLPreferenceWindow::_Instance(), SIGNAL(newStyle(const QString&)), this, SLOT(styleClicked(const QString&)));
#if !defined(_WIN32) || defined(__MINGW32__)
    connect(FLPreferenceWindow::_Instance(), SIGNAL(dropPortChange()), this, SLOT(changeDropPort()));
#endif
#ifdef REMOTE
    connect(FLPreferenceWindow::_Instance(), SIGNAL(remoteServerPortChanged()), this, SLOT(changeRemoteServerPort()));
#endif
    fAudioCreator = AudioCreator::_Instance(NULL);
    
    //--------------------HELP Menu
    FLHelpWindow::createInstance(fLibsFolder);
    
    //----------------MenuBar setups
    
    fMenuBar->addMenu(create_FileMenu());
    fMenuBar->addSeparator();
    
    fNavigateMenu = create_NavigateMenu();
    fMenuBar->addMenu(fNavigateMenu);
    fMenuBar->addSeparator();
    
    fMenuBar->addMenu(create_HelpMenu());
}

//--Starts the presentation menu if no windows are opened (session restoration or drop on icon that opens the application)
void FLApp::init_Timer_Action(){
    fInitTimer->stop();
    
    if(FLW_List.size()==0){
        if(!recall_CurrentSession())
            show_presentation_Action();
    }
}

//--Print errors in errorWindow
void FLApp::errorPrinting(const QString& msg){
    FLErrorWindow::_Instance()->print_Error(msg);
}

//--------- OPERATIONS ON WINDOWS INDEXES

QList<int> FLApp::get_currentIndexes(){
    
    QList<int> currentIndexes;
    
    for( QList<FLWindow*>::iterator it = FLW_List.begin(); it != FLW_List.end(); it++){
        currentIndexes.push_back((*it)->get_indexWindow());
    }
    return currentIndexes;
    
}

int FLApp::find_smallest_index(QList<int> currentIndexes){
    
    QList<int>::iterator it;
    bool found = true;
    int i = 0;
    
    while(found && currentIndexes.size() != 0){
        i++;
        for (it = currentIndexes.begin(); it != currentIndexes.end(); it++){
            if(*it == i){
                found = true;
                break;
            }
            else
                found = false;
        }        
    }
    
    if(i == 0)
        i = 1;
    return i;
}

//Calculates the position of a new window to avoid overlapping
void FLApp::calculate_position(int index, int* x, int* y){
    
    int multiplCoef = index;
    while(multiplCoef > 20){
        multiplCoef-=20;
    }
    
    *x = fScreenWidth/3 + multiplCoef*10;
    *y = fScreenHeight/3 + multiplCoef*10;
}

//---------GET WINDOW FROM ONE OF IT'S PARAMETER 

FLWindow* FLApp::getActiveWin(){
    
    QList<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++) {
        
        if((*it)->isActiveWindow())
            return *it;
    }
    
    return NULL;
}

//--------------------------------FILE-----------------------------

QString FLApp::createWindowFolder(const QString& sessionFolder, int index){
    
    QString path = sessionFolder + "/Windows/" + fWindowBaseName + QString::number(index);
    
    QDir dir(path);
    if(!dir.exists())
        dir.mkdir(path);
    
    return path;
}

QString FLApp::copyWindowFolder(const QString& sessionNewFolder, int newIndex, const QString& sessionFolder, int index, map<int, int> indexChanges){
    
    QString newPath = sessionNewFolder + "/Windows/" + fWindowBaseName + QString::number(newIndex);
    QString oldPath = sessionFolder + "/Windows/" + fWindowBaseName + QString::number(index);
    
    cpDir(oldPath, newPath);
    
    QString jcPath = newPath + "/Connections.jc";
    
    map<string, string> indexStringChanges;
    
    for(map<int, int>::iterator it = indexChanges.begin(); it!= indexChanges.end(); it++){
        string oldN = fWindowBaseName.toStdString() + QString::number(it->first).toStdString();
        string newN = fWindowBaseName.toStdString() + QString::number(it->second).toStdString();
        indexStringChanges[oldN] = newN;
    }
    
    FJUI::update(jcPath.toStdString().c_str(), indexStringChanges);
    
    return newPath;
}

void FLApp::connectWindowSignals(FLWindow* win){
    
    connect(win, SIGNAL(drop(QList<QString>)), this, SLOT(drop_Action(QList<QString>)));
    connect(win, SIGNAL(closeWin()), this, SLOT(close_Window_Action()));
    connect(win, SIGNAL(shut_AllWindows()), this, SLOT(shut_AllWindows_FromWindow()));
    connect(win, SIGNAL(duplicate_Action()), this, SLOT(duplicate_Window()));
    connect(win, SIGNAL(windowNameChanged()), this, SLOT(updateNavigateText()));
}

//---------------NEW WINDOW

FLWindow* FLApp::createWindow(int index, const QString& mySource, FLWinSettings* windowSettings, QString& error){
    
    if(FLW_List.size() >= numberWindows){
        error = "You cannot open more windows. If you are not happy with this limit, feel free to contact us : research.grame@gmail.com ^^";
        return NULL;
    }
    
    int init = kNoInit;
    
    QString source(mySource);
    
    //In case the source is empty, the effect is chosen by default 
    if(source.compare("") == 0){
        
        source = "process = !,!:0,0;";
        
        if(QString::compare(FLSettings::_Instance()->value("General/Style", "Blue").toString(), "Blue") == 0 || QString::compare(FLSettings::_Instance()->value("General/Style", "Grey").toString(), "Grey") == 0)
            init = kInitWhite;
        else
            init = kInitBlue;
    }
    
    QList<QMenu*> appMenus;
    appMenus.push_back(create_FileMenu());
    
    QMenu* navigateMenu = create_NavigateMenu();
    appMenus.push_back(navigateMenu);
    
    appMenus.push_back(create_HelpMenu());
    
    FLWindow* win = new FLWindow(fWindowBaseName, index, fSessionFolder, windowSettings,appMenus);
    
    if(win->init_Window(init, source, error)){
    
        FLW_List.push_back(win);
        
        connectWindowSignals(win);
        
        fNavigateMenus[win] = navigateMenu;
        
        QString name = win->get_nameWindow();
        name+=" : ";
        name+= win->getName();
        
        QAction* navigate = new QAction( name, NULL);
        connect(navigate, SIGNAL(triggered()), this, SLOT(frontShow()));
        
        fFrontWindow[navigate] = win;
        updateNavigateMenus();
        
        return win;
    }
    else{        
        delete win;
        delete windowSettings;
        return NULL;
    }
}

//--Creation accessed from Menu
void FLApp::create_New_Window(const QString& source){
    
    QString error("");
    //Choice of new Window's index
    int val = find_smallest_index(get_currentIndexes());
    
    int x, y;
    calculate_position(val, &x, &y);
    
    QString windowPath = createWindowFolder(fSessionFolder, val);
    
    QString settingPath = windowPath + "/Settings.ini";
    FLWinSettings* windowSettings = new FLWinSettings(val, settingPath, QSettings::IniFormat);
    windowSettings->setValue("Position/x", x);
    windowSettings->setValue("Position/y", y);
    
    FLWindow* win = createWindow(val, source, windowSettings, error);
    
    if(!win)
        errorPrinting(error);
}

//--Creation of Default Window from Menu
void FLApp::create_Empty_Window(){ 
    QString empty("");
    create_New_Window(empty);
}

//--------------NEW COMPONENT

void FLApp::create_Component_Window(){
    
    FLComponentWindow* componentWindow = new FLComponentWindow();
    
    connect(componentWindow, SIGNAL(newComponent(const QString&)), this, SLOT(create_New_Window(const QString&)));
    connect(componentWindow, SIGNAL(deleteIt()), this, SLOT(deleteComponent()));
    componentWindow->show();
}

void FLApp::deleteComponent(){
    FLComponentWindow* senderWindow = qobject_cast<FLComponentWindow*>(sender());
    senderWindow->hide();
    senderWindow->deleteLater();
}

//--------------OPEN

//--In the case of a DSP dropped on the Application's icon, this event is called
bool FLApp::event(QEvent *ev){
    
    if (ev->type() == QEvent::FileOpen) {
        
        QString fileName = static_cast<QFileOpenEvent *>(ev)->file();
        
        if(fileName.indexOf(".tar") != -1)
            recall_Snapshot(fileName, true);
        if(fileName.indexOf(".dsp") != -1)
            create_New_Window(fileName);
        return true;
    } 
    
    return QApplication::event(ev);
}

//--Open a dsp from disk
void FLApp::open_New_Window(){ 
    
    //    In case we are opening a new Window from the presentation Menu --> the application must not close
    fRecalling = true;
    
    FLWindow* win = getActiveWin();
    
    QStringList fileNames = QFileDialog::getOpenFileNames(NULL, tr("Open one or several DSPs"), "",tr("Files (*.dsp)"));
    
    QStringList::iterator it;
    
    for(it = fileNames.begin(); it != fileNames.end(); it++){
        
        if((*it) != ""){
            QString inter(*it);
            
            if(win != NULL && win->is_Default())
                win->update_Window(inter);
            else
                create_New_Window(inter);
        }
    }
    
    fRecalling = false;
}

//--------------OPEN EXAMPLE

void FLApp::open_Example_From_FileMenu(){
    
    QAction* action = qobject_cast<QAction*>(sender());
    QString toto(action->data().toString());
    
    open_Example_Action(toto);
}

void FLApp::open_Example_Action(QString pathInQResource){
    
    QFileInfo toOpen(pathInQResource);
    openExampleAction(toOpen.baseName());
}

void FLApp::openExampleAction(const QString& exampleName){
    
    QString pathInSession = fExamplesFolder + "/" + exampleName + ".dsp";
    
    QString codeToCompile = pathToContent(pathInSession);
    
    FLWindow* win = getActiveWin();
    
    if(win != NULL && win->is_Default())
        win->update_Window(codeToCompile);
    else
        create_New_Window(codeToCompile);    
}

//-------------RECENT FILES MANAGEMENT

void FLApp::save_Recent(QList<QString>& recents, const QString& pathToSettings){
    
    int index = 1;
    
    for (QList<QString>::iterator it=recents.begin(); it!=recents.end(); it++) {
        
        if(index<kMAXRECENT+1){
            
            QString settingPath = pathToSettings + QString::number(index);
            
            FLSettings::_Instance()->setValue(settingPath, *it);
            
            index++;
        }
        else
            break;
    }
}

void FLApp::recall_Recent(QList<QString>& recents, const QString& pathToSettings){
    
    for(int i=1; i<=kMAXRECENT; i++){
        
        QString settingPath = pathToSettings + QString::number(i);
        
        QString path = FLSettings::_Instance()->value(settingPath, "").toString();
        recents.push_back(path);
    }
    
}

//--Save/Recall from file 
void FLApp::save_Recent_Files(){
    
    QString settingPath = "General/RecentFiles/";
    
    save_Recent(fRecentFiles, settingPath);
}

void FLApp::recall_Recent_Files(){
    
    QString settingPath = "General/RecentFiles/";
    recall_Recent(fRecentFiles, settingPath);
    
    update_Recent_File();
}

//--Add new recent file
void FLApp::set_Current_File(const QString& path){
    
    fRecentFiles.removeOne(path);
    fRecentFiles.push_front(path);
    
    update_Recent_File();
    
}

//--Visual Update
void FLApp::update_Recent_File(){
    
    int index = 0;
    
    for(QList<QString>::iterator it=fRecentFiles.begin(); it!=fRecentFiles.end(); it++){
        
        if(index<kMAXRECENT){
            
            QString path = *it;
            
            if(path != ""){
                QString text = tr("&%1 %2").arg(index+1).arg(QFileInfo(path).baseName());
                
                fRecentFileAction[index]->setText(text);
                fRecentFileAction[index]->setData(path);
                fRecentFileAction[index]->setVisible(true);
                index++;
            }
        }
        else
            break;
    }
}

void FLApp::open_Recent_File(){
    QAction* action = qobject_cast<QAction*>(sender());
    QString toto(action->data().toString());
    
    open_Recent_File(toto);
}

//--Open a recent file
void FLApp::open_Recent_File(const QString& toto){
    
    FLWindow* win = getActiveWin();
    
    if(win != NULL && win->is_Default())
        win->update_Window(toto);
    else
        create_New_Window(toto);
}


//--------------OPEN Remote DSP

void FLApp::open_Remote_Window(){
//    
//    FLRemoteDSPScanner* dspScanner = new FLRemoteDSPScanner();
//    
//    if(dspScanner->exec()){
//        QString sha = dspScanner->shaKey();
//        
//        printf("SHA KEY TO RECALL = %s\n", sha.toStdString().c_str());
////        ---> ICI IL FAUT CONSTRUIRE LES SETTINGS DE LA W POUR POUVOIR LA CRÉER SEULEMENT À PARTIR DE LA CLÉ SHA DE SA REMOTE FACTORY
//    }
//    
//    delete dspScanner;
}

//--------------------------------SESSION

//--------------RECENTLY OPENED

//--Save/Recall from file
void FLApp::save_Recent_Sessions(){
   
    QString settingPath = "General/RecentSessions/";
        
    save_Recent(fRecentSessions, settingPath);
}

void FLApp::recall_Recent_Sessions(){
    
    QString settingPath = "General/RecentSessions/";
    recall_Recent(fRecentSessions, settingPath);
    
    update_Recent_Session();
}

//Add new recent session
void FLApp::set_Current_Session(const QString& path){
    
    fRecentSessions.removeOne(path);
    fRecentSessions.push_front(path);
    
    update_Recent_Session();
}

//Visual Update
void FLApp::update_Recent_Session(){
    
    int index = 0;
    
    for(QList<QString>::iterator it=fRecentSessions.begin(); it!=fRecentSessions.end(); it++){
        
        printf("Index = %i || Size = %i || Session = %s\n", index, fRecentSessions.size(), it->toStdString().c_str());
        
        if(index<kMAXRECENT){
        
            QString path = *it;
            
            if(path != ""){
                QString text = tr("&%1 %2").arg(index+1).arg(QFileInfo(path).baseName());
                
                fRrecentSessionAction[index]->setText(text);
                fRrecentSessionAction[index]->setData(path);
                fRrecentSessionAction[index]->setVisible(true);
                
                fIrecentSessionAction[index]->setText(text);
                fIrecentSessionAction[index]->setData(path);
                fIrecentSessionAction[index]->setVisible(true);
                
                index++;
            }
        }
        else
            break;
    }
}

//Recall or Import recent session
void FLApp::recall_Recent_Session(){
    
    QAction* action = qobject_cast<QAction*>(sender());
    QString toto(action->data().toString());
    recall_Snapshot(toto, false);
}

void FLApp::import_Recent_Session(){
    
    QAction* action = qobject_cast<QAction*>(sender());
    QString toto(action->data().toString());
    recall_Snapshot(toto, true);
}

//---------------CURRENT SESSION FUNCTIONS

//Update Current Session Structure with current parameters of the windows
void FLApp::update_CurrentSession(){
    
    QList<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++)
        (*it)->save_Window();
}

//---------------SAVE SNAPSHOT FUNCTIONS

//Save the current State in SnapshotFolder.tar
//It copies the hidden Session Folder and compresses it
void FLApp::take_Snapshot(){
    
    QFileDialog* fileDialog = new QFileDialog;
    fileDialog->setConfirmOverwrite(true);
    
	QString filename;
    
#ifndef _WIN32
    filename = fileDialog->getSaveFileName(NULL, "Take Snapshot", tr(""), tr("(*.tar)"));
#else
	filename = fileDialog->getSaveFileName(NULL, "Take Snapshot", tr(""));
#endif
    
    
    //If no name is placed, nothing happens
    if(filename.compare("") != 0){
        
        display_CompilingProgress("Saving your snapshot...");
        
        int pos = filename.indexOf(".tar");
        
        set_Current_Session(filename);
        
        if(pos != -1)
            filename = filename.mid(0, pos);
        
        update_CurrentSession();

        FLSessionManager::_Instance()->createSnapshot(filename);
        
        StopProgressSlot();
    }
}

//---------------RESTORE SNAPSHOT FUNCTIONS

void FLApp::recallSnapshotFromMenu(){
	
	QString fileName;
#ifndef _WIN32
    fileName = QFileDialog::getOpenFileName(NULL, tr("Recall a Snapshot"), "",tr("Files (*.tar)"));
#else
	fileName = QFileDialog::getExistingDirectory(NULL, tr("Recall a Snapshot"), "", QFileDialog::ShowDirsOnly);
#endif
    
    if(fileName != "")
        recall_Snapshot(fileName, false);
}

void FLApp::importSnapshotFromMenu(){
    
	QString fileName;
#ifndef _WIN32
    fileName = QFileDialog::getOpenFileName(NULL, tr("Import a Snapshot"), "",tr("Files (*.tar)"));
#else
	fileName = QFileDialog::getExistingDirectory(NULL, tr("Import a Snapshot"), "", QFileDialog::ShowDirsOnly);
#endif
    
    if(fileName != "")
        recall_Snapshot(fileName, true);
    
}

//----Recall or Import a Snapshot
//@param : filename = snapshot that is loaded
//@param : importOption = false for recalling | true for importing
void FLApp::recall_Snapshot(const QString& name, bool importOption){ 

    QString filename(name);
    
    display_CompilingProgress("Uploading your snapshot...");
    
	fRecalling = true;
    
    set_Current_Session(filename);

    if(!importOption)
        shut_AllWindows_FromWindow();
    
    QString folderName = QFileInfo(filename).canonicalPath() + "/" + QFileInfo(filename).baseName();
    
    map<int, QString> restoredSources = FLSessionManager::_Instance()->snapshotRestoration(filename);

    map<int, int> indexChanges;
    
    QList<int> currentIndexes = get_currentIndexes();
    
//    Creating mapping between saved index and new index
    for(map<int, QString>::iterator it = restoredSources.begin(); it != restoredSources.end(); it++){
        
        int indexValue;
        
        if(importOption)
            indexValue = find_smallest_index(currentIndexes);
        else
            indexValue = it->first;
        
        indexChanges[it->first] = indexValue;
        currentIndexes.push_back(indexValue);
    }
    
//    Restore windows with new index
    for(map<int, int>::iterator it = indexChanges.begin(); it != indexChanges.end(); it++){
        
        QString windowPath = copyWindowFolder(fSessionFolder, it->second, folderName, it->first, indexChanges);
    
        QString settingPath = windowPath + "/Settings.ini";
        FLWinSettings* windowSettings = new FLWinSettings(it->second, settingPath, QSettings::IniFormat);
        
        QString error;
        if(!createWindow(it->second, restoredSources[it->first], windowSettings, error))
            errorPrinting(error);
    }
    
#ifndef _WIN32
    deleteDirectoryAndContent(folderName);
#endif
    
	fRecalling = false;
    
    StopProgressSlot();
}

//----Recall saved current session
bool FLApp::recall_CurrentSession(){
    
    map<int, QString> restoredSources = FLSessionManager::_Instance()->currentSessionRestoration();
    
    if(restoredSources.size() == 0)
        return false;
    
    map<int, QString>::iterator it;
    for(it = restoredSources.begin(); it != restoredSources.end(); it++){
        
        QString windowPath = createWindowFolder(fSessionFolder, it->first);
        
        QString settingPath = windowPath + "/Settings.ini";
        FLWinSettings* windowSettings = new FLWinSettings(it->first, settingPath, QSettings::IniFormat);
        
        QString error;
        if(!createWindow(it->first, it->second, windowSettings, error))
            errorPrinting(error);
    }
    
    return true;
    
}

//--------------CLOSE

//Display progress Bar on FaustLive quit
void FLApp::display_Progress(){
    
//    Quit dialog is shown only if there are windows to save
    if(FLW_List.size() != 0){
        
//        Stop showing the menu bar 
        fMenuBar->setEnabled(false);
        
        QDialog* savingMessage = new QDialog();
        savingMessage->setWindowFlags(Qt::FramelessWindowHint);
        
        QLabel* tittle = new QLabel(tr("<h2>FAUSTLIVE</h2>"));
        tittle->setAlignment(Qt::AlignCenter);
        QLabel* text = new QLabel(tr("Saving your session..."), savingMessage);
        text->setAlignment(Qt::AlignCenter);
        fPBar = new QProgressBar(savingMessage);
        fPBar->setAlignment(Qt::AlignCenter);
        fPBar->setRange(0,100);
        fPBar->setValue(0);
        
        QVBoxLayout* layoutSave = new QVBoxLayout;
        
        layoutSave->addWidget(tittle);
        layoutSave->addWidget(new QLabel(tr("")));
        layoutSave->addWidget(text);
        layoutSave->addWidget(new QLabel(tr("")));
        layoutSave->addWidget(fPBar);
        savingMessage->setLayout(layoutSave);
        
        //savingMessage->move((fScreenWidth-savingMessage->width())/2, (fScreenHeight-savingMessage->height())/2);
        savingMessage->adjustSize();
        centerOnPrimaryScreen(savingMessage);
        savingMessage->show();
        
        fEndTimer = new QTimer(this);
        connect(fEndTimer, SIGNAL(timeout()), this, SLOT(update_ProgressBar()));
        fEndTimer->start(7);
    }
    else
        quit();
}

//Update of this progress bar
void FLApp::update_ProgressBar(){
    
    int value = fPBar->value();
    int maximum = fPBar->maximum();
    
    if (value < maximum){
        value ++;
        fPBar->setValue(value);
    }
    
    if(value >= maximum){
        this->quit();
    }
}

//Quit FaustLive
void FLApp::closeAllWindows(){
    
	printf("FLApp::closeAllWindows()\n");

//This function is called when there are no more windows. In case of session recallin, the application can not be closed !!
    if(fRecalling)
    	   return;
    
    display_Progress();
        
    update_CurrentSession();
        
// Windows have to be saved before being closed, otherwise the connections are not well saved
    QList<FLWindow*>::iterator it;
    
    for(it = FLW_List.begin(); it != FLW_List.end(); it++)
        (*it)->save_Window();
    
    for(it = FLW_List.begin(); it != FLW_List.end(); it++){
            
        (*it)->close_Window();
        (*it)->deleteLater();
    }
    
    FLW_List.clear();
    
    FLSessionManager::_Instance()->saveCurrentSources(fSessionFolder);
}

//Shut all Windows forcing to pass into closeEvent of the window
void FLApp::shut_AllWindows_FromMenu(){

    while(FLW_List.size() != 0 ){
        FLWindow* win = *(FLW_List.begin());
		win->shut();
    } 
}

//Shut all Windows already coming from closeEvent
void FLApp::shut_AllWindows_FromWindow(){

    while(FLW_List.size() != 0 ){
        FLWindow* win = *(FLW_List.begin());
		common_shutAction(win);
    } 
}

//Close from Window Action
void FLApp::close_Window_Action(){
    
    FLWindow* win = (FLWindow*)QObject::sender();
    common_shutAction(win);
}

//Shut a specific window 
void FLApp::common_shutAction(FLWindow* win){
    
    QString path = win->getPath();
    if(path != "")
        set_Current_File(path);
    
    win->shut_Window();
        
    QAction* action = fFrontWindow.key(win);
    fFrontWindow.remove(action);
    fNavigateMenus.remove(win);
    updateNavigateMenus();
    
    FLW_List.removeOne(win);
    win->deleteLater();
}

//--------------------------------Navigate---------------------------------

void FLApp::updateNavigateText(){
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    QString name = win->get_nameWindow();
    name+=" : ";
    name+= win->getName();
    
    QAction* action = fFrontWindow.key(win);
    action->setText(name);
    
    updateNavigateMenus();
}

void FLApp::updateNavigateMenus(){
    
    fNavigateMenu->clear();
    
    for(QMap<FLWindow*, QMenu*>::iterator it = fNavigateMenus.begin(); it != fNavigateMenus.end(); it++){
        
        it.value()->clear();
        
        for(QMap<QAction*, FLWindow*>::iterator ite = fFrontWindow.begin(); ite != fFrontWindow.end() ; ite++){
            it.value()->addAction(ite.key());
            fNavigateMenu->addAction(ite.key());
        }
    }
    
}

void FLApp::frontShow(){
    
    QAction* action = (QAction*)QObject::sender();
    fFrontWindow[action]->raise();
}

//--------------------------------Window----------------------------------------

//Duplicate a specific window
void FLApp::duplicate(FLWindow* window){
    
    int val = find_smallest_index(get_currentIndexes());

    //Save then Copy the duplicated window's parameters
    window->save_Window();
    
    map<int, int> indexChanges;
    indexChanges[window->get_indexWindow()] = val;
    
    QString windowPath = copyWindowFolder(fSessionFolder, val, fSessionFolder, window->get_indexWindow(), indexChanges);
    
    QString settingPath = windowPath + "/Settings.ini";
    FLWinSettings* windowSettings = new FLWinSettings(val, settingPath, QSettings::IniFormat);
    
    windowSettings->setValue("Position/x", windowSettings->value("Position/x", 0).toInt()+10);
    windowSettings->setValue("Position/y", windowSettings->value("Position/y", 0).toInt()+10);
    
    QString error("");
    if(!createWindow(val, window->get_source(), windowSettings, error))
        errorPrinting(error);
}

//Duplication window from Menu
void FLApp::duplicate_Window(){ 
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    if(win != NULL)
        duplicate(win);
}

//---------------Drop

//Drop of sources on a window
void FLApp::drop_Action(QList<QString> sources){
    
    QList<QString>::iterator it = sources.begin();
    
    while(it!=sources.end()){
        create_New_Window(*it);
        it++;
    }
}

//--------------------------------HELP----------------------------------------

//Open Faust and FaustLive documentation
void FLApp::open_FL_doc(){
    
    QString pathToOpen = fDocFolder + "/UserManual.pdf";
    
    QUrl url = QUrl::fromLocalFile(pathToOpen);
    bool b = QDesktopServices::openUrl(url);
    
    if(!b)
        errorPrinting("Impossible to open FaustLive documentation ! Make sure a file association is set up for .pdf.");
}

void FLApp::open_F_doc(){
    
    QString pathToOpen = fDocFolder + "/faust-quick-reference.pdf";
    
    QUrl url = QUrl::fromLocalFile(pathToOpen);
    bool b = QDesktopServices::openUrl(url);
    
    if(!b)
        errorPrinting("Impossible to open Faust documentation ! Make sure a file association is set up for .pdf.");
}

//Not Active Window containing version of all the librairies
void FLApp::version_Action(){
    
    QDialog* versionWindow = new QDialog;
    
    QVBoxLayout* layoutGeneral = new QVBoxLayout;
    
    QString text = "FAUSTLIVE \n- Dist version ";
    text += readFile(":/distVersion.txt");
    text += "- Build version ";
    text += readFile(":/buildVersion.txt");
    text += "\nThis application is using ""\n""- Jack 2";
//    text += jack_get_version_string();
    text += "\n""- NetJack ";
    text += "2.1";
    text += "\n""- CoreAudio API ";
    text += "4.0";
    text += "\n""- LLVM Compiler ";
    text += "3.1";
        
    QPlainTextEdit* versionText = new QPlainTextEdit(text, versionWindow);
        
    layoutGeneral->addWidget(versionText);
    versionWindow->setLayout(layoutGeneral);
    centerOnPrimaryScreen(versionWindow);
    versionWindow->exec();

    delete versionText;
    delete layoutGeneral;
    delete versionWindow;
}

//-------------------------------PRESENTATION WINDOW-----------------------------

void FLApp::show_presentation_Action(){
    
    FLPresentationWindow::_Instance()->show();
    FLPresentationWindow::_Instance()->raise();
}

//--------------------------------PREFERENCES---------------------------------------

//Modification of application style
void FLApp::styleClicked(const QString& style){
    
    QFile file;
    FLSettings::_Instance()->setValue("General/Style", style);
    
    QString styleFile = ":/Styles/" + style + ".qss";    
    file.setFileName(styleFile);
    
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString styleSheet = QLatin1String(file.readAll());
        
        setStyleSheet(styleSheet);
        file.close();
    }
}

//Preference triggered from Menu
void FLApp::Preferences(){
    
    FLPreferenceWindow::_Instance()->exec();
    
    if(fAudioCreator->didSettingChanged()){
        
        fAudioCreator->visualSettingsToTempSettings();
        update_AudioArchitecture();
    }
}

//Update Audio Architecture of all opened windows
void FLApp::update_AudioArchitecture(){
    
    QList<FLWindow*>::iterator it;
    QList<FLWindow*>::iterator updateFailPointer;
    
    bool updateSuccess = true;
    QString errorToPrint;
    QString error;
    
    display_CompilingProgress("Updating Audio Architecture...");
    
    //Save all audio clients
    for(it = FLW_List.begin() ; it != FLW_List.end(); it++)    
        (*it)->save_Window();
    
    //Stop all audio clients
    for(it = FLW_List.begin() ; it != FLW_List.end(); it++)
        (*it)->stop_Audio();
    
    //Try to init new audio architecture
    for(it = FLW_List.begin() ; it != FLW_List.end(); it++){
        
        if(!(*it)->update_AudioArchitecture(error)){
            updateSuccess = false;
            updateFailPointer = it;
            break;
        }
    }
    
    bool reinitSuccess = true;
    
    //If init failed, reinit old audio Architecture
    if(!updateSuccess){
        
        errorToPrint = "Update not successfull";
        
        errorPrinting(errorToPrint);
        errorPrinting(error);
        
//        If some audio did start before the failure, they have to be stopped again
        for(it = FLW_List.begin() ; it != updateFailPointer; it++)
            (*it)->stop_Audio();
        
//        Switch back to previous architecture
        fAudioCreator->restoreSavedSettings();
        
        for(it = FLW_List.begin() ; it != FLW_List.end(); it++){
            
            if(!(*it)->update_AudioArchitecture(error))
            {
                reinitSuccess = false;
                break;
            }
        }
        
//        In case switch back fails, every window is closed
        if(!reinitSuccess){
            
            shut_AllWindows_FromMenu();
            
            errorToPrint += fAudioCreator->get_ArchiName();
            errorToPrint = " could not be reinitialize";
            
            errorPrinting(errorToPrint);
            errorPrinting(error);
        }
        else{
            
            for(it = FLW_List.begin() ; it != FLW_List.end(); it++)
                (*it)->start_Audio();
            
            errorToPrint = fAudioCreator->get_ArchiName();
            errorToPrint += " was reinitialized";
            errorPrinting(errorToPrint);
            
        }
        
    }
    else{
        
        for(it = FLW_List.begin() ; it != FLW_List.end(); it++)
            (*it)->start_Audio();
        
            fAudioCreator->tempSettingsToSavedSettings();

        //If there is no current window, it is strange to show that msg
        if(FLW_List.size() != 0){
            errorToPrint = "Update successfull";
            errorPrinting(errorToPrint);
        }
    }
    
    StopProgressSlot();
}

//--------------------------LONG WAITING PROCESSES------------------------------

//Display Messages like "Compiling..." / "Connection with server..."
void FLApp::display_CompilingProgress(const QString& msg){
    FLMessageWindow::_Instance()->displayMessage(msg);
    FLMessageWindow::_Instance()->show();
    FLMessageWindow::_Instance()->raise();
}

//Stop displaying the message
void FLApp::StopProgressSlot(){
    FLMessageWindow::_Instance()->hide();
}

//--------------------------FAUSTLIVE SERVER ------------------------------

FLWindow* FLApp::getWinFromHttp(int port){
    
    QList<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++) {
        
        if(port == (*it)->get_Port())
            return *it;
    }
    
    return NULL;
}

#ifdef HTTPCTRL
FLWindow* FLApp::httpPortToWin(int port){
    
    for(QList<FLWindow*>::iterator it = FLW_List.begin(); it != FLW_List.end(); it++){
        if((*it)->get_Port() == port)
            return *it;
    }
    return NULL;
}

//Start FaustLive Server that wraps HTTP interface in droppable environnement 
void FLApp::launch_Server(){
    
    bool returning = true;
    
    int i = 0;
        
    bool started = true;
    
    while(!FLServerHttp::_Instance()->start()){
        
        started = false;
        
        QString s("Server Could Not Start On Port ");
        s += QString::number(FLSettings::_Instance()->value("General/Network/HttpDropPort", 7777).toInt());
            
        errorPrinting(s);
            
        FLSettings::_Instance()->setValue("General/Network/HttpDropPort", FLSettings::_Instance()->value("General/Network/HttpDropPort", 7777).toInt()+1);
        
        if(i > 15){
            returning = false;
            break;
        }
        else
            i++;
    } 
    
    if(!returning)
        errorPrinting("Server Did Not Start.\n Please Choose another port.");
    
//    That way, it doesn't say it when it starts normally
    else if(!started){
        QString s("Server Started On Port ");
        s += QString::number(FLSettings::_Instance()->value("General/Network/HttpDropPort", 7777).toInt());
        errorPrinting(s);
    }
}

//Stop FaustLive Server
void FLApp::stop_Server(){
        FLServerHttp::_Instance()->stop();
}

//Update when a file is dropped on HTTP interface (= drop in FaustLive window)
void FLApp::compile_HttpData(const char* data, int port){
        
    FLWindow* win;
    
    QString source(data);
    QString error("");
    
    bool success = false;
    
    if(port == 0){
        
        int val = find_smallest_index(get_currentIndexes());
        
        int x, y;
        calculate_position(val, &x, &y);
        
        QString windowPath = createWindowFolder(fSessionFolder, val);
        
        QString settingPath = windowPath + "/Settings.ini";
        FLWinSettings* windowSettings = new FLWinSettings(val, settingPath, QSettings::IniFormat);
        windowSettings->setValue("Position/x", x);
        windowSettings->setValue("Position/y", y);
        windowSettings->setValue("Http/Enabled", true);
        
        win = createWindow(val, source, windowSettings, error);
            
        if(win != NULL)
            success = true;
    }
    else{
        win = httpPortToWin(port);
        
        if(win->update_Window(source))
            success = true;
    }
    
//The server has to know whether the compilation is successfull, to stop blocking the answer to its client
    if(success){
        string url = win->get_HttpUrl().toStdString();
        FLServerHttp::_Instance()->compile_Successfull(url);
    }
    else
        FLServerHttp::_Instance()->compile_Failed(error.toStdString());
}

void FLApp::changeDropPort(){
    
    stop_Server();
    launch_Server();
    
}
#endif

#ifdef REMOTE
void FLApp::changeRemoteServerPort(){

    Server* serv = Server::_Instance();
    serv->stop();
    serv->start(FLSettings::_Instance()->value("General/Network/RemoteServerPort", 5555).toInt());
}
#endif

