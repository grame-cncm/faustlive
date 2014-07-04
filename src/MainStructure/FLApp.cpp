//
//  FaustLiveApp.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLApp.h"
#include "FLrenameDialog.h"
#ifndef _WIN32
#include "FLServerHttp.h"
#include "faust/remote-dsp.h"
#else
#include <windows.h>
#include <shlobj.h>
#endif
#include "FLSessionManager.h"
#include "FLWindow.h"
#include "FLErrorWindow.h"
#include "FLHelpWindow.h"
#include "FLPresentationWindow.h"
#include "FLExportManager.h"
#include "utilities.h"

#ifdef __APPLE__
#include "faust/remote-dsp.h"
#endif

#include "FLSettings.h"
#include "FLWinSettings.h"
#include "FLPreferenceWindow.h"

#include "FJUI.h"

#include <sstream>

//----------------------CONSTRUCTOR/DESTRUCTOR---------------------------

FLApp::FLApp(int& argc, char** argv) : QApplication(argc, argv){
    
    //Create Current Session Folder
    create_Session_Hierarchy();
    
    FLSettings::initInstance(fSessionFolder);
    
    //Initializing screen parameters
    QSize screenSize = QApplication::desktop()->geometry().size(); 
    fScreenWidth = screenSize.width();
    fScreenHeight = screenSize.height();
    
    //Base Name of application's windows
    fWindowBaseName = "FLW-";
    
    styleClicked(FLSettings::getInstance()->value("General/Style", "Default").toString());
    
	//Initializing Recalling 
	fRecalling = false;
    
    // Presentation Window Initialization
    fPresWin = new FLPresentationWindow;
    connect(fPresWin, SIGNAL(newWin()), this, SLOT(create_Empty_Window()));
    connect(fPresWin, SIGNAL(openWin()), this, SLOT(open_New_Window()));
    connect(fPresWin, SIGNAL(openSession()), this, SLOT(recallSnapshotFromMenu()));
    connect(fPresWin, SIGNAL(openPref()), this, SLOT(Preference()));
    connect(fPresWin, SIGNAL(openHelp()), this, SLOT(apropos()));
    connect(fPresWin, SIGNAL(openExample(const QString&)), this, SLOT(openExampleAction(const QString&)));
    
    fPresWin->setWindowFlags(Qt::FramelessWindowHint);  
    fPresWin->move((fScreenWidth-fPresWin->width())/2, 20);
    
#ifndef __APPLE__
    //For the application not to quit when the last window is closed
	connect(this, SIGNAL(lastWindowClosed()), this, SLOT(closeAllWindows()));
#else
    setQuitOnLastWindowClosed(false);
#endif
    
    fMenuBar = new QMenuBar(NULL);
    
    //Initializing menu actions 
    fRecentFileAction = new QAction* [kMAXRECENTFILES];
    for(int i=0; i<kMAXRECENTFILES; i++){
        fRecentFileAction[i] = new QAction(NULL);
        fRecentFileAction[i]->setVisible(false);
        connect(fRecentFileAction[i], SIGNAL(triggered()), this, SLOT(open_Recent_File()));
    }
    
    fRrecentSessionAction = new QAction* [kMAXRECENTSESSIONS];
    for(int i=0; i<kMAXRECENTSESSIONS; i++){
        fRrecentSessionAction[i] = new QAction(NULL);
        fRrecentSessionAction[i]->setVisible(false);
        connect(fRrecentSessionAction[i], SIGNAL(triggered()), this, SLOT(recall_Recent_Session()));
    }
    
    fIrecentSessionAction = new QAction* [kMAXRECENTSESSIONS];
    for(int i=0; i<kMAXRECENTSESSIONS; i++){
        fIrecentSessionAction[i] = new QAction(NULL);
        fIrecentSessionAction[i]->setVisible(false);
        connect(fIrecentSessionAction[i], SIGNAL(triggered()), this, SLOT(import_Recent_Session()));
    }
    
    setup_Menu();
    
    recall_Recent_Files();
    recall_Recent_Sessions();
    
    //Initializing OutPut Window
    fErrorWindow = new FLErrorWindow();
    fErrorWindow->setWindowTitle("MESSAGE_WINDOW");
    fErrorWindow->init_Window();
    connect(fErrorWindow, SIGNAL(closeAll()), this, SLOT(shut_AllWindows()));

    
    //Initialiazing Remote Drop Server
#ifndef _WIN32
    fServerHttp = NULL;
    launch_Server();
#endif
    fCompilingMessage = NULL;
    
    //If no event opened a window half a second after the application was created, a initialized window is created
    fInitTimer = new QTimer(this);
    connect(fInitTimer, SIGNAL(timeout()), this, SLOT(init_Timer_Action()));
    fInitTimer->start(500);
}

FLApp::~FLApp(){
    
    save_Recent_Files();
    save_Recent_Sessions();
    
    for(int i=0; i<kMAXRECENTFILES; i++){
        delete fRecentFileAction[i];
    }
    for(int i=0; i<kMAXRECENTSESSIONS; i++){
        delete fRrecentSessionAction[i];
        delete fIrecentSessionAction[i];
    }
    
    delete fAudioCreator;
    
    delete fRecentFileAction;
    delete fRrecentSessionAction;
    delete fIrecentSessionAction;
    delete fMenuBar;
    
    delete fInitTimer;
    
    delete fPresWin;
    delete fHelpWindow;
    delete fErrorWindow;
    delete fExportDialog;
    
    delete FLSettings::getInstance();
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
    
    fSessionFolder += FLVERSION;
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
    
    QString factoryFolder = fSessionFolder + "/SHAFolder";
    QDir shaFolder(factoryFolder);
    shaFolder.mkdir(factoryFolder);
    
    
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
    
    //SESSION
    
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
    connect(shutAllAction, SIGNAL(triggered()), this, SLOT(shut_AllWindows()));
    
    QAction* closeAllAction = new QAction(tr("&Quit FaustLive"),NULL);
    closeAllAction->setShortcut(tr("Ctrl+Q"));
    closeAllAction->setToolTip(tr("Close the application"));   
    connect(closeAllAction, SIGNAL(triggered()), this, SLOT(closeAllWindows()));
    
    fileMenu->addAction(newAction);    
    fileMenu->addSeparator();
    fileMenu->addAction(openAction);
    fileMenu->addAction(create_ExampleMenu()->menuAction());
    fileMenu->addAction(create_RecentFileMenu()->menuAction());
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
    
    for(int i=0; i<kMAXRECENTFILES; i++)
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
    
    for(int i=0; i<kMAXRECENTSESSIONS; i++){
    
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
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(apropos()));
    
    //    fVersionWindow = new QDialog;
    //    
    //    fVersionAction = new QAction(tr("&Version"), this);
    //    fVersionAction->setToolTip(tr("Show the version of the libraries used"));
    //    connect(fVersionAction, SIGNAL(triggered()), this, SLOT(version_Action()));
    
    QAction* presentationAction = new QAction(tr("&About FaustLive"), NULL);
    presentationAction->setToolTip(tr("Show the presentation Menu"));
    connect(presentationAction, SIGNAL(triggered()), this, SLOT(show_presentation_Action()));
    
    
    helpMenu->addAction(aboutQtAction);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAction);
    //    fHelpMenu->addAction(fVersionAction);
    //    fHelpMenu->addSeparator();
    helpMenu->addAction(presentationAction);
    helpMenu->addSeparator();
    helpMenu->addAction(preferencesAction);

    return helpMenu;
}

void FLApp::setup_Menu(){
    
    //----------------FILE
    fMenuBar->addMenu(create_FileMenu());
    fMenuBar->addSeparator();
    
    fNavigateMenu = create_NavigateMenu();
    fMenuBar->addMenu(fNavigateMenu);
    fMenuBar->addSeparator();
    
    fMenuBar->addMenu(create_HelpMenu());
    
    //---------------------Presentation MENU
    
    fPrefDialog = new FLPreferenceWindow;
    
    connect(fPrefDialog, SIGNAL(newStyle(const QString&)), this, SLOT(styleClicked(const QString&)));
    connect(fPrefDialog, SIGNAL(dropPortChange()), this, SLOT(changeDropPort()));
    fAudioCreator = AudioCreator::_Instance(NULL);
    
    //--------------------HELP Menu
    
    fHelpWindow = new FLHelpWindow(fLibsFolder);
    fHelpWindow->setWindowFlags(Qt::FramelessWindowHint);
    
    fHelpWindow->move((fScreenWidth-fHelpWindow->width())/2, (fScreenHeight-fHelpWindow->height())/2);
    
    //    EXPORT MANAGER
    
    fExportDialog = new FLExportManager(fSessionFolder);
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
void FLApp::errorPrinting(const char* msg){
    fErrorWindow->print_Error(msg);
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

FLWindow* FLApp::getWinFromHttp(int port){
    
    QList<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++) {
        
        if(port == (*it)->get_Port())
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
    
    //    ICI IL FAUDRAIT EN PROFITER POUR CHANGER LE .jc ?????????
    
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

void FLApp::redirectMenuToWindow(FLWindow* win){
    
    connect(win, SIGNAL(drop(QList<QString>)), this, SLOT(drop_Action(QList<QString>)));
    connect(win, SIGNAL(migrate(const QString&, int)), this, SLOT(migrate_ProcessingInWin(const QString&, int)));
    connect(win, SIGNAL(error(const char*)), this, SLOT(errorPrinting(const char*)));
    connect(win, SIGNAL(closeWin()), this, SLOT(close_Window_Action()));
    connect(win, SIGNAL(duplicate_Action()), this, SLOT(duplicate_Window()));
    connect(win, SIGNAL(export_Win()), this, SLOT(export_Action()));
}

//---------------NEW

bool FLApp::createWindow(int index, const QString& mySource, FLWinSettings* windowSettings, QString& error){
    
    if(FLW_List.size() >= numberWindows){
        error = "You cannot open more windows. If you are not happy with this limit, feel free to contact us : research.grame@gmail.com ^^";
        return false;
    }
    
    int init = kNoInit;
    
    QString source(mySource);
    
    //In case the source is empty, the effect is chosen by default 
    if(source.compare("") == 0){
        
        source = "process = !,!:0,0;";
        
        if(QString::compare(FLSettings::getInstance()->value("General/Style", "Blue").toString(), "Blue") == 0 || QString::compare(FLSettings::getInstance()->value("General/Style", "Grey").toString(), "Blue") == 0)
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
        
        redirectMenuToWindow(win);
        
        fNavigateMenus[win] = navigateMenu;
        
        QString name = win->get_nameWindow();
        name+=" : ";
        name+= win->getName();
        
        QAction* navigate = new QAction( name, NULL);
        connect(navigate, SIGNAL(triggered()), this, SLOT(frontShow()));
        
        fFrontWindow[navigate] = win;
        updateNavigateMenus();
        
        return true;
    }
    else{
        delete windowSettings;
        delete win;
        return false;
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
    windowSettings->setValue("FaustOptions", FLSettings::getInstance()->value("General/Compilation/FaustOptions", "").toString());
    windowSettings->setValue("OptValue", FLSettings::getInstance()->value("General/Compilation/OptValue", "").toString());
    
    if(!createWindow(val, source, windowSettings, error))
        fErrorWindow->print_Error(error);
}

//--Creation of Default Window from Menu
void FLApp::create_Empty_Window(){ 
    QString empty("");
    create_New_Window(empty);
}

//--------------OPEN

//--In the case of a DSP dropped on the Application's icon, this event is called
bool FLApp::event(QEvent *ev){
    
    if (ev->type() == QEvent::FileOpen) {
        
        //If the application is opened by a drop action, the current Session has to be reseted
        if(FLW_List.size() == 0)
            reset_CurrentSession();
        
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
    
    FLWindow* win = getActiveWin();
    
    if(win != NULL && win->is_Default())
        win->update_Window(pathInSession);
    else
        create_New_Window(pathInSession);    
}

//-------------OPEN RECENT

//--Save/Recall from file 
void FLApp::save_Recent_Files(){
    
    QList<std::pair<QString, QString> >::iterator it;
    int index = 1;
    
    for (it = fRecentFiles.begin(); it != fRecentFiles.end(); it++) {
        
        QString settingPath = "General/RecentFiles/" + QString::number(index) + "/path";
        QString path = it->first;
        FLSettings::getInstance()->setValue(settingPath, path);
        
        QString settingName = "General/RecentFiles/" + QString::number(index) + "/name";
        QString name = it->second;
        FLSettings::getInstance()->setValue(settingName, name);
        
        index ++;
    }
}

void FLApp::recall_Recent_Files(){
    
    for(int i=1; i<=kMAXRECENTFILES; i++){
        
        QString settingName = "General/RecentFiles/" + QString::number(i) + "/name";
        QString settingPath = "General/RecentFiles/" + QString::number(i) + "/path";
        
        QString name = FLSettings::getInstance()->value(settingName, "").toString();
        QString path = FLSettings::getInstance()->value(settingPath, "").toString();
        
        if(name != "" && path != "")
            set_Current_File( path, name);
    }
}

//--Add new recent file
void FLApp::set_Current_File(const QString& pathName, const QString& effName){
    
    std::pair<QString,QString> myPair;
	myPair = make_pair(pathName, effName);
    
    fRecentFiles.removeOne(myPair);
    fRecentFiles.push_front(myPair);
    
    update_Recent_File();
    
}

//--Visual Update
void FLApp::update_Recent_File(){
    
    int j = 0;
    
    QList<std::pair<QString, QString> >::iterator it;
    
    for (it = fRecentFiles.begin(); it != fRecentFiles.end(); it++) {
        
        if(j<kMAXRECENTFILES){
            
            QString text;
            text += it->second;
            fRecentFileAction[j]->setText(text);
            fRecentFileAction[j]->setData(it->first);
            fRecentFileAction[j]->setVisible(true);
            
            j++;
        }
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

//--------------------------------SESSION

//Parses the fileSessionContent one option at a time
QString FLApp::parseNextOption(QString& optionsCompilation){
    
    int pos = optionsCompilation.indexOf(" ");
    
    if(pos != -1){
        
        //        The next option is from the beginning to the space
        QString option = optionsCompilation.mid(0, pos);
        
        //        The rest of the options are from after the space to the end of the string
        optionsCompilation = optionsCompilation.mid(pos+1);
        
        return option;
    }
    //    When the string is all parsed
    else{
        QString option = optionsCompilation;
        optionsCompilation = "";
        return option;
    }
}

//--------------RECENTLY OPENED

//--Save/Recall from file
void FLApp::save_Recent_Sessions(){
    
    QList<QString>::iterator it;
    int index = 1;
    
    for (it = fRecentSessions.begin(); it != fRecentSessions.end(); it++) {
        
        QString settingPath = "General/RecentSessions/" + QString::number(index);
        FLSettings::getInstance()->setValue(settingPath, *it);
        
        index++;
    }
}

void FLApp::recall_Recent_Sessions(){
    
    for(int i=1; i<=kMAXRECENTSESSIONS; i++){
        
        QString settingPath = "General/RecentSessions/" + QString::number(i) ;
        QString path = FLSettings::getInstance()->value(settingPath, "").toString();
        
        if(path != "")
            set_Current_Session(path);
    }
    
}

//Add new recent session
void FLApp::set_Current_Session(const QString& pathName){
    
    QString currentSess = pathName;
    fRecentSessions.removeAll(currentSess);
    fRecentSessions.prepend(currentSess);
    update_Recent_Session();

}

//Visual Update
void FLApp::update_Recent_Session(){
    
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
        }
        else{
            fRrecentSessionAction[j]->setVisible(false);
            fIrecentSessionAction[j]->setVisible(false);
        }
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

//Reset Current Session Folder
void FLApp::reset_CurrentSession(){
    
    QDir srcDir(fSessionFolder);    
    
    QFileInfoList children = srcDir.entryInfoList(QDir::Dirs | QDir::Drives | QDir::NoDotAndDotDot);
    
    QFileInfoList::iterator it;
    
    for(it = children.begin(); it != children.end(); it++){
        
        QString toRemove(it->absoluteFilePath());
        
        if(toRemove.indexOf(fWindowBaseName) != -1)
            deleteDirectoryAndContent(toRemove);
    }
    
    //    recall_Settings(fSettingsFolder);
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
        
        int pos = filename.indexOf(".tar");
        
        if(pos != -1)
            filename = filename.mid(0, pos);
        
        update_CurrentSession();
        
        FLSessionManager::_Instance()->createSnapshot(filename);
        
        set_Current_Session(filename);
#ifndef _WIN32
        tarFolder(filename);
#endif
    }
}

//---------------RESTORE SNAPSHOT FUNCTIONS

void FLApp::recallSnapshotFromMenu(){
	
    fRecalling = true;
    
	QString fileName;
#ifndef _WIN32
    fileName = QFileDialog::getOpenFileName(NULL, tr("Recall a Snapshot"), "",tr("Files (*.tar)"));
#else
	fileName = QFileDialog::getExistingDirectory(NULL, tr("Recall a Snapshot"), "/home", QFileDialog::ShowDirsOnly);
#endif
    
    if(fileName != ""){
        
        recall_Snapshot(fileName, false);
    }
}

void FLApp::importSnapshotFromMenu(){
    
	QString fileName;
#ifndef _WIN32
    fileName = QFileDialog::getOpenFileName(NULL, tr("Import a Snapshot"), "",tr("Files (*.tar)"));
#else
	fileName = QFileDialog::getExistingDirectory(NULL, tr("Import a Snapshot"), "/home", QFileDialog::ShowDirsOnly);
#endif
    
    if(fileName != ""){
        
        recall_Snapshot(fileName, true);
    }
}

#ifndef _WIN32
void FLApp::tarFolder(const QString& folder){

    
    display_CompilingProgress("Saving your session...");
    
    QProcess myCmd;
    QByteArray error;
    
    QString systemInstruct("tar cfv ");
    systemInstruct += folder + ".tar " + folder;
    
    myCmd.start(systemInstruct);
    myCmd.waitForFinished();
    
    error = myCmd.readAllStandardError();
    
    if(myCmd.readChannel() == QProcess::StandardError )
        fErrorWindow->print_Error(error.data());
    
    deleteDirectoryAndContent(folder);
    StopProgressSlot();
}

void FLApp::untarFolder(const QString& folder){
    
    display_CompilingProgress("Uploading your snapshot...");
    
    QProcess myCmd;
	QByteArray error;
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    
    QString systemInstruct("tar xfv ");
    systemInstruct += folder +" -C /";
    
	myCmd.setProcessEnvironment(env);
    myCmd.start(systemInstruct);
    myCmd.waitForFinished();
    
    error = myCmd.readAllStandardError();
    
    if(myCmd.readChannel() == QProcess::StandardError )
        fErrorWindow->print_Error(error.data());
    
    StopProgressSlot();

}
#endif

//@param : filename = snapshot that is loaded
//@param : importOption = false for recalling | true for importing
void FLApp::recall_Snapshot(const QString& filename, bool importOption){ 
    
	fRecalling = true;
    
    set_Current_Session(filename);
    
    untarFolder(filename);
    
    if(!importOption)
        shut_AllWindows();
    
    QString finalFile = QFileInfo(filename).canonicalPath() + "/" + QFileInfo(filename).baseName();

    map<int, QString> restoredSources = FLSessionManager::_Instance()->snapshotRestoration(finalFile);
    
    map<int, int> indexChanges;
    
    QList<int> currentIndexes = get_currentIndexes();
    
    for(map<int, QString>::iterator it = restoredSources.begin(); it != restoredSources.end(); it++){
        
        int indexValue;
        
        if(importOption)
            indexValue = find_smallest_index(currentIndexes);
        else
            indexValue = it->first;
        
        indexChanges[it->first] = indexValue;
        currentIndexes.push_back(indexValue);
    }
    
    for(map<int, int>::iterator it = indexChanges.begin(); it != indexChanges.end(); it++){
        
        QString windowPath = copyWindowFolder(fSessionFolder, it->second, finalFile, it->first, indexChanges);
    
        QString settingPath = windowPath + "/Settings.ini";
        FLWinSettings* windowSettings = new FLWinSettings(it->second, settingPath, QSettings::IniFormat);
        
        QString error;
        if(!createWindow(it->second, restoredSources[it->first], windowSettings, error))
            fErrorWindow->print_Error(error);
    }
    
//    ICI ON FAIT LE TOUR DES RESTORED WIN POUR MODIFIER LEURS .JC
//    --> LE PROBLEME C'EST QUE ÇA DEVRAIT ÊTRE FAIT AVANT QUE LA W SOIT INITIALISÉE POUR QUE LES CONNEXIONS SOIENT FAIT CORRECTEMENT !!!!
    
#ifndef _WIN32
    
    display_CompilingProgress("Ending upload...");

    deleteDirectoryAndContent(finalFile);
    
//    QProcess myCmd2;
//    QByteArray error2;
//    
//    QString rmInstruct("rm -r ");
//    rmInstruct += finalFile;
//    
//    myCmd2.start(rmInstruct);
//    myCmd2.waitForFinished();
//    
//    error2 = myCmd2.readAllStandardError();
//    
//    if(myCmd2.readChannel() == QProcess::StandardError )
//        fErrorWindow->print_Error(error2.data());
    
    StopProgressSlot();
#endif
    
	fRecalling = false;
}

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
            fErrorWindow->print_Error(error);
    }
    
    return true;
    
}

//--------------CLOSE

//Display progress Bar on FaustLive quit
void FLApp::display_Progress(){
    
    if(FLW_List.size() != 0){
        
        fMenuBar->setEnabled(false);
        
        QDialog* savingMessage = new QDialog();
        savingMessage->setWindowFlags(Qt::FramelessWindowHint);
        
        QLabel* tittle = new QLabel(tr("<h2>FAUSTLIVE</h2>"));
        tittle->setStyleSheet("*{color : black}");
        tittle->setAlignment(Qt::AlignCenter);
        QLabel* text = new QLabel(tr("Saving your session..."), savingMessage);
        text->setStyleSheet("*{color : black}");
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
        
        savingMessage->move((fScreenWidth-savingMessage->width())/2, (fScreenHeight-savingMessage->height())/2);
        savingMessage->adjustSize();
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
            
            QString path = (*it)->getPath();
            QString name = (*it)->getName();
            
            if(path != "")
                set_Current_File(path, name);
            
        (*it)->close_Window();
        (*it)->deleteLater();
    }
    FLW_List.clear();
    
    FLSessionManager* sessionManager = FLSessionManager::_Instance();
    sessionManager->saveCurrentSources(fSessionFolder);
}

//Shut all Windows from Menu
void FLApp::shut_AllWindows(){
    
    while(FLW_List.size() != 0 ){
        FLWindow* win = *(FLW_List.begin());
        
        common_shutAction(win);
    }
#ifndef __APPLE__
    closeAllWindows();
#endif   
}

//Close Window from Menu
void FLApp::shut_Window(){
    
    if(fErrorWindow->isActiveWindow())
        fErrorWindow->hideWin();
    
    else{
        
        QList<FLWindow*>::iterator it = FLW_List.begin();
        
        while(it != FLW_List.end()) {
            
            if((*it)->isActiveWindow()){
                
                common_shutAction(*it);
                
                break;
            }
#ifdef _WIN32
            else if((*it)->is_httpdWindow_active()){
                (*it)->hide_httpdWindow();
                break;
            }
#endif
            else
                it++;
        }
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
    QString name = win->getName();
    
    if(path != "")
        set_Current_File(path, name);
    
    win->shut_Window();
        
    QAction* action = fFrontWindow.key(win);
    fFrontWindow.remove(action);
    fNavigateMenus.remove(win);
    updateNavigateMenus();
    
    FLW_List.removeOne(win);
    win->deleteLater();
    
//#ifndef __APPLE__
//    if(FLW_List.size() == 0 && fExportDialog->isDialogVisible() && !fPresWin->isVisible()){
//    printf("FLAPP::CLOSE ALL WINDOWS FROM COMMON SHUT WIN \n");
//    	closeAllWindows();
//    }
//
//#endif
}

//--------------------------------Navigate---------------------------------

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

    //Save then Copy of duplicated window's parameters
    window->save_Window();
    
    map<int, int> indexChanges;
    indexChanges[window->get_indexWindow()] = val;
    
    QString windowPath = copyWindowFolder(fSessionFolder, val, fSessionFolder, window->get_indexWindow(), indexChanges);
    
    QString settingPath = windowPath + "/Settings.ini";
    FLWinSettings* windowSettings = new FLWinSettings(val, settingPath, QSettings::IniFormat);
    
    windowSettings->setValue("Position/x", windowSettings->value("Position/x", 0).toInt()+10);
    windowSettings->setValue("Position/y", windowSettings->value("Position/y", 0).toInt()+10);
    
    QString error;
    createWindow(val, window->get_source(), windowSettings, error);
}

//Duplication window from Menu
void FLApp::duplicate_Window(){ 
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    if(win != NULL)
        duplicate(win);
}

//---------------Export

//Open ExportManager for a specific Window
void FLApp::export_Win(FLWindow* win){
    //    
    //    QString expanded_code = win->get_Effect()->get_expandedVersion().c_str();
    //    
    //    fExportDialog->exportFile(win->get_Effect()->getSource(), expanded_code);
}

//Export From Menu
void FLApp::export_Action(){ 
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    if(win != NULL)
        export_Win(win);
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
void FLApp::apropos(){
    fHelpWindow->show();
}

//Not Active Window containing version of all the librairies
void FLApp::version_Action(){
    
    //    QVBoxLayout* layoutGeneral = new QVBoxLayout;
    //    
    //    string text = "This application is using ""\n""- Jack 2";
    ////    text += jack_get_version_string();
    //    text += "\n""- NetJack ";
    //    text += "2.1";
    //    text += "\n""- CoreAudio API ";
    //    text += "4.0";
    //    text += "\n""- LLVM Compiler ";
    //    text += "3.1";
    //    
    //    QPlainTextEdit* versionText = new QPlainTextEdit(tr(text), fVersionWindow);
    //    
    //    layoutGeneral->addWidget(versionText);
    //    fVersionWindow->setLayout(layoutGeneral);
    //    
    //    fVersionWindow->exec();
    //    
    //    delete versionText;
    //    delete layoutGeneral;
}

//-------------------------------PRESENTATION WINDOW-----------------------------

void FLApp::show_presentation_Action(){
    
    fPresWin->show();
    fPresWin->raise();
}

//--------------------------------PREFERENCES---------------------------------------

//Modification of application style
void FLApp::styleClicked(const QString& style){
    
    QFile file;
    
    if(style.compare("Default") == 0){
        
        FLSettings::getInstance()->setValue("General/Style", "Default");
        
        file.setFileName(":/Styles/Default.qss");
    }
    
    if(style.compare("Blue") == 0){    
        
        FLSettings::getInstance()->setValue("General/Style", "Blue");
        
        file.setFileName(":/Styles/Blue.qss");
    }
    
    if(style.compare("Grey") == 0){
        
        FLSettings::getInstance()->setValue("General/Style", "Grey");
        
        file.setFileName(":/Styles/Grey.qss");
    }
    
    if(style.compare("Salmon") == 0){
        
        FLSettings::getInstance()->setValue("General/Style", "Salmon");
        
        file.setFileName(":/Styles/Salmon.qss");
    }
    
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString styleSheet = QLatin1String(file.readAll());
        
        setStyleSheet(styleSheet);
        file.close();
    }
}

//Preference triggered from Menu
void FLApp::Preferences(){
    
    fPrefDialog->exec();
    
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
        
        fErrorWindow->print_Error(errorToPrint);
        fErrorWindow->print_Error(error);
        
        for(it = FLW_List.begin() ; it != updateFailPointer; it++)
            (*it)->stop_Audio();
        
        fAudioCreator->restoreSavedSettings();
        
        for(it = FLW_List.begin() ; it != FLW_List.end(); it++){
            
            if(!(*it)->update_AudioArchitecture(error))
            {
                reinitSuccess = false;
                break;
            }
        }
        
        if(!reinitSuccess){
            
            shut_AllWindows();
            
            errorToPrint += fAudioCreator->get_ArchiName();
            errorToPrint = " could not be reinitialize";
            
            fErrorWindow->print_Error(errorToPrint);
            fErrorWindow->print_Error(error);
        }
        else{
            
            for(it = FLW_List.begin() ; it != FLW_List.end(); it++)
                (*it)->start_Audio();
            
            errorToPrint = fAudioCreator->get_ArchiName();
            errorToPrint += " was reinitialized";
            fErrorWindow->print_Error(errorToPrint);
            
        }
        
    }
    else{
        
        for(it = FLW_List.begin() ; it != FLW_List.end(); it++)
            (*it)->start_Audio();
        //        fAudioCreator->saveCurrentSettings();
        fAudioCreator->tempSettingsToSavedSettings();
        //        If there is no current window, it is strange to show that msg
        if(FLW_List.size() != 0){
            errorToPrint = "Update successfull";
            fErrorWindow->print_Error(errorToPrint);
        }
    }
    
    StopProgressSlot();
}

//--------------------------LONG WAITING PROCESSES------------------------------

//Display Messages like "Compiling..." / "Connection with server..."
void FLApp::display_CompilingProgress(const QString& msg){
    fCompilingMessage = new QDialog();
    fCompilingMessage->setWindowFlags(Qt::FramelessWindowHint);
    
    QLabel* tittle = new QLabel(tr("<h2>FAUSTLIVE</h2>"));
    tittle->setAlignment(Qt::AlignCenter);
    QLabel* text = new QLabel(msg, fCompilingMessage);
    text->setAlignment(Qt::AlignCenter);
    text->setStyleSheet("*{color: black}");
    
    QVBoxLayout* layoutSave = new QVBoxLayout;
    
    layoutSave->addWidget(tittle);
    layoutSave->addWidget(new QLabel(tr("")));
    layoutSave->addWidget(text);
    layoutSave->addWidget(new QLabel(tr("")));
    fCompilingMessage->setLayout(layoutSave);
    
    fCompilingMessage->move((fScreenWidth-fCompilingMessage->width())/2, (fScreenHeight-fCompilingMessage->height())/2);
    fCompilingMessage->adjustSize();
    fCompilingMessage->show();
    fCompilingMessage->raise();
}

//Stop displaying the message
void FLApp::StopProgressSlot(){
    
    fCompilingMessage->hide();
    delete fCompilingMessage;
}

//--------------------------FAUSTLIVE SERVER ------------------------------

#ifndef _WIN32
//Start FaustLive Server that wraps HTTP interface in droppable environnement 
void FLApp::launch_Server(){
    
    bool returning = true;
    
    if(fServerHttp == NULL){
        
        fServerHttp = new FLServerHttp();
        
        int i = 0;
        
        while(!fServerHttp->start()){
            
            QString s("Server Could Not Start On Port ");
            s += QString::number(FLSettings::getInstance()->value("General/Network/HttpDropPort", 7777).toInt());
            
            fErrorWindow->print_Error(s);
            
            FLSettings::getInstance()->setValue("General/Network/HttpDropPort", FLSettings::getInstance()->value("General/Network/HttpDropPort", 7777).toInt()+1);
            
            if(i > 15){
                returning = false;
                break;
            }
            else{
                i++;
            }
        }
        
        connect(fServerHttp, SIGNAL(compile_Data(const char*, int)), this, SLOT(compile_HttpData(const char*, int)));
    }
    else
        returning = false;
    
    if(!returning)
        fErrorWindow->print_Error("Server Did Not Start.\n Please Choose another port.");
    //    That way, it doesn't say it when the application is started
    else if(FLW_List.size() != 0){
        QString s("Server Started On Port ");
        s += QString::number(FLSettings::getInstance()->value("General/Network/HttpDropPort", 7777).toInt());
        fErrorWindow->print_Error(s);
    }
}

//Stop FaustLive Server
void FLApp::stop_Server(){
    if(fServerHttp != NULL){
        fServerHttp->stop();
        delete fServerHttp;
        fServerHttp = NULL;
    }
}

//Update when a file is dropped on HTTP interface (= drop in FaustLive window)
void FLApp::compile_HttpData(const char* data, int port){
    
    string error("");
    
	QString source(data);
    
    FLWindow* win = getWinFromHttp(port);
    
    if(win != NULL){
        
        win->update_Window(source);
        
        win->resetHttpInterface();
        
        string url = win->get_HttpUrl().toStdString();
        
        fServerHttp->compile_Successfull(url);
    }
    else{
        fServerHttp->compile_Failed(error);
    }  
}

void FLApp::changeDropPort(){
    
    stop_Server();
    launch_Server();
    
}
#endif


