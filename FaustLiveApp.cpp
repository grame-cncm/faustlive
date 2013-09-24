//
//  FaustLiveApp.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FaustLiveApp.h"
#include "FLrenameDialog.h"

#include <QFileOpenEvent>
#include <QClipboard>
#include <QMimeData>
#include <QString>
#include <QTextStream>

#include <sstream>

//--------------------GENERAL METHODS-------------------------------------

string FaustLiveApp::pathToContent(string path){
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


bool FaustLiveApp::deleteDirectoryAndContent(string& directory){
    
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

bool FaustLiveApp::rmDir(const QString &dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists())
        return true;
    foreach(const QFileInfo &info, dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        if (info.isDir()) {
            if (!rmDir(info.filePath()))
                return false;
        } else {
            if (!dir.remove(info.fileName()))
                return false;
        }
    }
    QDir parentDir(QFileInfo(dirPath).path());
    return parentDir.rmdir(QFileInfo(dirPath).fileName());
}

bool FaustLiveApp::cpDir(const QString &srcPath, const QString &dstPath)
{
    this->rmDir(dstPath);
    QDir parentDstDir(QFileInfo(dstPath).path());
    if (!parentDstDir.mkdir(QFileInfo(dstPath).fileName()))
        return false;
    
    QDir srcDir(srcPath);
    foreach(const QFileInfo &info, srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        QString srcItemPath = srcPath + "/" + info.fileName();
        QString dstItemPath = dstPath + "/" + info.fileName();
        if (info.isDir()) {
            if (!cpDir(srcItemPath, dstItemPath)) {
                return false;
            }
        } else if (info.isFile()) {
            if (!QFile::copy(srcItemPath, dstItemPath)) {
                return false;
            }
        } else {
            qDebug() << "Unhandled item" << info.filePath() << "in cpDir";
        }
    }
    return true;
}

bool FaustLiveApp::isStringInt(const char* word){
    
    bool returning = true;
    
    for(int i=0; i<strlen(word); i++){
        if(!isdigit(word[i])){
            returning = false;
            break;
        }
    }
    return returning;
}

//----------------------CONSTRUCTOR/DESTRUCTOR---------------------------

FaustLiveApp::FaustLiveApp(int& argc, char** argv) : QApplication(argc, argv){
    
    WindowBaseName = "FLW";
    DefaultName = "DefaultName";
    
    //Initialization of current Session Path  
    
    currentSession = getenv("HOME");
    currentSession += "/CurrentSession";
    if(!QFileInfo(currentSession.c_str()).exists()){
        QDir direct(currentSession.c_str());
        direct.mkdir(currentSession.c_str());
    }
    currentSessionFile = currentSession + "/Description.sffx";
    if(!QFileInfo(currentSessionFile.c_str()).exists()){
        QFile f(currentSessionFile.c_str());
        f.open(QFile::ReadOnly);
        f.close();
        
        printf("création de description File = %s\n", currentSessionFile.c_str());
    }    
    
    currentSourcesFolder = currentSession + "/Sources";
    if(!QFileInfo(currentSourcesFolder.c_str()).exists()){
        QDir direct(currentSourcesFolder.c_str());
        direct.mkdir(currentSourcesFolder.c_str());
    }    
    
    currentSVGFolder = currentSession + "/SVG";
    if(!QFileInfo(currentSVGFolder.c_str()).exists()){
        QDir direct(currentSVGFolder.c_str());
        direct.mkdir(currentSVGFolder.c_str());
    }  
    
    currentIRFolder = currentSession + "/IR";
    if(!QFileInfo(currentIRFolder.c_str()).exists()){
        QDir direct(currentIRFolder.c_str());
        direct.mkdir(currentIRFolder.c_str());
    }  
    
    QDir direc(currentSessionFile.c_str());
    
    //Initializing screen parameters
    QSize screenSize = QApplication::desktop()->geometry().size(); 
    screenWidth = screenSize.width();
    screenHeight = screenSize.height();
    
    //Initializing preference
    homeSettings = currentSession + "/FaustLive_Settings.rf"; 
    bufferSize = 512;
    compressionValue = -1;
    masterIP = "225.3.19.154";
    //    masterIP = "127.0.0.1";
    masterPort = 19000;
    latency = 2;
    opt_level = 3;
    styleChoice = "Default";
    recall_Settings(homeSettings);
    styleClicked(styleChoice);
    
    //If no event opened a window half a second after the application was created, a initialized window is created
    initTimer = new QTimer(this);
    connect(initTimer, SIGNAL(timeout()), this, SLOT(init_Timer_Action()));
    initTimer->start(500);
    
    
    //Initializing menu options 
    MaxRecentFiles = 5;
    recentFileAction = new QAction* [MaxRecentFiles];
    MaxRecentSessions = 3;
    RrecentSessionAction = new QAction* [MaxRecentSessions];
    IrecentSessionAction = new QAction* [MaxRecentSessions];
//    frontWindow = new QAction* [200];
    
    
    //For the application not to quit when the last window is closed
    setQuitOnLastWindowClosed(false);
    
    fMenuBar = new QMenuBar(0);
    fileMenu = new QMenu;
    editMenu = new QMenu;
    windowsMenu = new QMenu;
    sessionMenu = new QMenu;
    viewMenu = new QMenu;
    helpMenu = new QMenu;
    
    setup_Menu();
    
//    homeRecents = getenv("HOME");
    homeRecentFiles = currentSession + "/FaustLive_FileSavings.rf"; 
    recall_Recent_Files(homeRecentFiles);
//    homeRecents = getenv("HOME");
    homeRecentSessions = currentSession + "/FaustLive_SessionSavings.rf"; 
    recall_Recent_Sessions(homeRecentSessions);
    
    //Initializing preference and save dialog
    errorWindow = new FLErrorWindow();
    errorWindow->setWindowTitle("ERROR_WINDOW");
    errorWindow->init_Window();
    connect(errorWindow, SIGNAL(closeAll()), this, SLOT(shut_AllWindows()));
}

FaustLiveApp::~FaustLiveApp(){
    
    save_Recent_Files();
    save_Recent_Sessions();
    
    for(int i=0; i<MaxRecentFiles; i++){
        delete recentFileAction[i];
    }
    for(int i=0; i<MaxRecentSessions; i++){
        delete RrecentSessionAction[i];
        delete IrecentSessionAction[i];
    }
    
    delete newAction;
    delete openAction;
    delete openRecentAction;
    delete shutAction;
    delete shutAllAction;
    delete closeAllAction;
    delete editAction;
    delete pasteAction;
    delete duplicateAction;
    delete takeSnapshotAction;
    delete recallSnapshotAction;
    delete importSnapshotAction;
    delete httpdViewAction;
    delete aboutQtAction;
    delete preferencesAction;
    delete aboutAction; 
    
    delete helpMenu;
    delete viewMenu;
    delete editMenu;
    delete windowsMenu;
    delete fileMenu;
    delete fMenuBar;
    
    delete initTimer;
    
    if(presWin != NULL)
        delete presWin;
    
    delete HelpWindow;
    delete errorWindow;
    
    sessionContent.clear();
}

//---------------------

void FaustLiveApp::setup_Menu(){
    
    //----------------FILE
    
    newAction = new QAction(tr("&New Default Window"), this);
    newAction->setShortcut(tr("Ctrl+N"));
    newAction->setToolTip(tr("Open a new empty file"));
    connect(newAction, SIGNAL(triggered()), this, SLOT(create_Empty_Window()));
    
    openAction = new QAction(tr("&Open..."),this);
    openAction->setShortcut(tr("Ctrl+O"));
    openAction->setToolTip(tr("Open a DSP file"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open_New_Window()));
    
    menuOpen_Example = new QMenu(tr("&Open Example"),fileMenu);
    
    QString examplesPath = QFileInfo(QFileInfo( QCoreApplication::applicationFilePath()).absolutePath()).absolutePath();
    examplesPath += "/Resources/Examples";
    
    QDir examplesDir(examplesPath);
    
    QFileInfoList children = examplesDir.entryInfoList(QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
    
    exampleToOpen = (children.begin())->baseName().toStdString();
    
    QFileInfoList::iterator it;
    int i = 0; 
    
    openExamples = new QAction* [children.size()];
    
    for(it = children.begin(); it != children.end(); it++){
        
        openExamples[i] = new QAction(it->baseName(), menuOpen_Example);
        openExamples[i]->setData(QVariant(it->absoluteFilePath()));
        connect(openExamples[i], SIGNAL(triggered()), this, SLOT(open_Recent_File()));
        
        menuOpen_Example->addAction(openExamples[i]);
        i++;
    }
    
    fileMenu->addAction(menuOpen_Example->menuAction());
    
    openRecentAction = new QAction(tr("&Open Recent..."),this);
    openRecentAction->setEnabled(false);
    openRecentAction->setToolTip(tr("Open a recently opened DSP file"));
    
    for(int i=0; i<MaxRecentFiles; i++){
        recentFileAction[i] = new QAction(this);
        recentFileAction[i]->setVisible(false);
        connect(recentFileAction[i], SIGNAL(triggered()), this, SLOT(open_Recent_File()));
    }
    
    exportAction = new QAction(tr("&Export As..."), this);
    exportAction->setShortcut(tr("Ctrl+P"));
    exportAction->setToolTip(tr("Export the DSP in whatever architecture you choose"));
    connect(exportAction, SIGNAL(triggered()), this, SLOT(export_Action()));
    
    shutAction = new QAction(tr("&Close Window"),this);
    shutAction->setShortcut(tr("Ctrl+W"));
    shutAction->setToolTip(tr("Close the current Window"));
    connect(shutAction, SIGNAL(triggered()), this, SLOT(shut_Window()));
    
    shutAllAction = new QAction(tr("&Close All Windows"),this);
    shutAllAction->setShortcut(tr("Ctrl+Alt+W"));
    shutAllAction->setToolTip(tr("Close all the Windows"));
    connect(shutAllAction, SIGNAL(triggered()), this, SLOT(shut_AllWindows()));
    
    closeAllAction = new QAction(tr("&Closing"),this);
    closeAllAction = new QAction(tr("&Quit FaustLive"),this);
    closeAllAction->setToolTip(tr("Close the application"));   
    connect(closeAllAction, SIGNAL(triggered()), this, SLOT(closeAllWindows()));
    
    fileMenu = fMenuBar->addMenu(tr("&File"));
    fileMenu->addAction(newAction);    
    fileMenu->addSeparator();
    fileMenu->addAction(openAction);
    fileMenu->addAction(menuOpen_Example->menuAction());
    fileMenu->addSeparator();
    fileMenu->addAction(openRecentAction);
    for(int i=0; i<MaxRecentFiles; i++){
        fileMenu->addAction(recentFileAction[i]);
    }
    fileMenu->addSeparator();
    fileMenu->addAction(exportAction);
    fileMenu->addSeparator();
    fileMenu->addAction(shutAction);
    fileMenu->addAction(shutAllAction);
    
    fMenuBar->addSeparator();
    fileMenu->addAction(closeAllAction);
    
    //-----------------EDIT
    
    editAction = new QAction(tr("&Edit Faust Source"), this);
    editAction->setShortcut(tr("Ctrl+E"));
    editAction->setToolTip(tr("Edit the source"));
    connect(editAction, SIGNAL(triggered()), this, SLOT(edit_Action()));
    
    pasteAction = new QAction(tr("&Paste"),this);
    pasteAction->setShortcut(tr("Ctrl+V"));
    pasteAction->setToolTip(tr("Paste a DSP"));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste_Text()));
    
    duplicateAction = new QAction(tr("&Duplicate"),this);
    duplicateAction->setShortcut(tr("Ctrl+D"));
    duplicateAction->setToolTip(tr("Duplicate current DSP"));
    connect(duplicateAction, SIGNAL(triggered()), this, SLOT(duplicate_Window()));
    
    editMenu = fMenuBar->addMenu(tr("&Edit"));
    editMenu->addAction(editAction);
    editMenu->addSeparator();
    editMenu->addAction(pasteAction);
    editMenu->addSeparator();
    editMenu->addAction(duplicateAction);
    fMenuBar->addSeparator();
    
    fMenuBar->addSeparator();
    
    //-----------------WINDOWS
    
    windowsMenu = fMenuBar->addMenu(tr("&Windows"));
    
    fMenuBar->addSeparator();
    //------------------SESSION
    
    takeSnapshotAction = new QAction(tr("&Take Snapshot"),this);
    takeSnapshotAction->setShortcut(tr("Ctrl+S"));
    takeSnapshotAction->setToolTip(tr("Save current state"));
    connect(takeSnapshotAction, SIGNAL(triggered()), this, SLOT(take_Snapshot()));
    
    recallSnapshotAction = new QAction(tr("&Recall Snapshot..."),this);
    recallSnapshotAction->setShortcut(tr("Ctrl+R"));
    recallSnapshotAction->setToolTip(tr("Close all the opened window and open your snapshot"));
    connect(recallSnapshotAction, SIGNAL(triggered()), this, SLOT(recallSnapshotFromMenu()));
    
    recallRecentAction = new QAction(tr("&Recall Recent..."),this);
    recallRecentAction->setEnabled(false);
    
    for(int i=0; i<MaxRecentSessions; i++){
        RrecentSessionAction[i] = new QAction(this);
        RrecentSessionAction[i]->setVisible(false);
        connect(RrecentSessionAction[i], SIGNAL(triggered()), this, SLOT(recall_Recent_Session()));
        
        IrecentSessionAction[i] = new QAction(this);
        IrecentSessionAction[i]->setVisible(false);
        connect(IrecentSessionAction[i], SIGNAL(triggered()), this, SLOT(import_Recent_Session()));
    }
    
    importSnapshotAction = new QAction(tr("&Import Snapshot..."),this);
    importSnapshotAction->setShortcut(tr("Ctrl+I"));
    importSnapshotAction->setToolTip(tr("Import your snapshot in the current session"));
    connect(importSnapshotAction, SIGNAL(triggered()), this, SLOT(importSnapshotFromMenu()));
    
    importRecentAction = new QAction(tr("&Import Recent..."),this);
    importRecentAction->setEnabled(false);
    
    sessionMenu = fMenuBar->addMenu(tr("&Snapshot"));
    sessionMenu->addAction(takeSnapshotAction);
    sessionMenu->addSeparator();
    sessionMenu->addAction(recallSnapshotAction);
    sessionMenu->addAction(recallRecentAction);
    sessionMenu->addAction(recallRecentAction);
    for(int i=0; i<MaxRecentSessions; i++){
        sessionMenu->addAction(RrecentSessionAction[i]);
    }
    sessionMenu->addSeparator();
    sessionMenu->addAction(importSnapshotAction);
    sessionMenu->addAction(importRecentAction);
    for(int i=0; i<MaxRecentSessions; i++){
        sessionMenu->addAction(IrecentSessionAction[i]);
    }
    
    fMenuBar->addSeparator();
    
    //---------------------VIEW
    
    httpdViewAction = new QAction(tr("&View QRcode"),this);
    httpdViewAction->setShortcut(tr("Ctrl+K"));
    httpdViewAction->setToolTip(tr("Print the QRcode of TCP protocol"));
    connect(httpdViewAction, SIGNAL(triggered()), this, SLOT(httpd_View_Window()));
    
    svgViewAction = new QAction(tr("&View SVG Diagram"),this);
    svgViewAction->setShortcut(tr("Ctrl+G"));
    svgViewAction->setToolTip(tr("Open the SVG Diagram in a browser"));
    connect(svgViewAction, SIGNAL(triggered()), this, SLOT(svg_View_Action()));
    
    viewMenu = fMenuBar->addMenu(tr("&View"));
    viewMenu->addAction(httpdViewAction);
    viewMenu->addSeparator();
    viewMenu->addAction(svgViewAction);
    
    fMenuBar->addSeparator();
    
    //---------------------MAIN MENU
    
    aboutQtAction = new QAction(tr("&About Qt"), this);
    aboutQtAction->setToolTip(tr("Show the library's About Box"));
    connect(aboutQtAction, SIGNAL(triggered()), this, SLOT(aboutQt()));
    
    preferencesAction = new QAction(tr("&Preferences"), this);
    preferencesAction->setToolTip(tr("Set the preferences of the application"));
    connect(preferencesAction, SIGNAL(triggered()), this, SLOT(Preferences()));
    
    preference = new QDialog;
    preference->setWindowFlags(Qt::FramelessWindowHint);
    init_PreferenceWindow();
    
    //--------------------HELP
    
    HelpWindow = new QMainWindow;
    HelpWindow->setWindowFlags(Qt::FramelessWindowHint);
    this->init_HelpWindow();
    
    aboutAction = new QAction(tr("&Help..."), this);
    aboutAction->setToolTip(tr("Show the library's About Box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(apropos()));
    
    versionWindow = new QDialog;
    
    versionAction = new QAction(tr("&Version"), this);
    versionAction->setToolTip(tr("Show the version of the libraries used"));
    connect(versionAction, SIGNAL(triggered()), this, SLOT(version_Action()));
    
    presentationAction = new QAction(tr("&About FaustLive"), this);
    presentationAction->setToolTip(tr("Show the presentation Menu"));
    connect(presentationAction, SIGNAL(triggered()), this, SLOT(show_presentation_Action()));
    
    helpMenu = fMenuBar->addMenu(tr("&Help"));
    
    helpMenu->addAction(aboutQtAction);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(versionAction);
    helpMenu->addSeparator();
    helpMenu->addAction(presentationAction);
    helpMenu->addSeparator();
    helpMenu->addAction(preferencesAction);
}

void FaustLiveApp::errorPrinting(const char* msg){
    
    errorWindow->print_Error(msg);
}

void FaustLiveApp::init_Timer_Action(){
    initTimer->stop();
    
    if(FLW_List.size()==0){
        
        if(QFileInfo(currentSessionFile.c_str()).exists()){
            
            QFile f(currentSessionFile.c_str());
            QString text("");
            
            if(f.open(QFile::ReadOnly)){
                
                QTextStream textReading(&f);
                
                textReading>>text;
                f.close();
            }
            
            if(text.toStdString().compare("") == 0){
                
                show_presentation_Action();
            }
            else{
                sessionContent.clear();
                recall_Session(currentSessionFile);
            }
        }
        else{
            show_presentation_Action();
        }
    }
    //    else
    //        presWin = NULL;
}

list<int> FaustLiveApp::get_currentIndexes(){
    list<int> currentIndexes;
    
    list<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++){
        currentIndexes.push_back((*it)->get_indexWindow());
    }
    
    return currentIndexes;
    
}

int FaustLiveApp::find_smallest_index(list<int> currentIndexes){
    
    list<int>::iterator it;
    bool found = true;
    int i = 0;
    
    printf("current index list = %i\n", FLW_List.size());
    
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
        i=1;
    return i;
}

bool FaustLiveApp::isIndexUsed(int index, list<int> currentIndexes){
    
    list<int>::iterator it;
    
    for(it = currentIndexes.begin(); it != currentIndexes.end(); it++){
        if(index == *it ){
            return true;
        }
    }
    return false;
}

void FaustLiveApp::calculate_position(int index, int* x, int* y){
    
    int multiplCoef = index;
    while(multiplCoef > 20){
        multiplCoef-=20;
    }
    
    *x = screenWidth/3 + multiplCoef*10;
    *y = screenHeight/3 + multiplCoef*10;
}

list<string> FaustLiveApp::get_currentDefault(){
    
    list<string> currentDefault;
    
    list<WinInSession*>::iterator it;
    
    for(it = sessionContent.begin(); it != sessionContent.end() ; it++){
        
        if((*it)->name.find(DefaultName)!=string::npos)
            currentDefault.push_back((*it)->name);
    }
    
    return currentDefault;
}

string FaustLiveApp::find_smallest_defaultName(string& sourceToCompare, list<string> currentDefault){
    
    //Conditional jump on currentDefault List...
    
    int index = 1;
    string nomEffet;
    bool found;
    
    do{
        stringstream ss;
        ss << index;
        
        nomEffet = DefaultName + "-" + ss.str();
        
        list<string>::iterator it;
        
        for(it = currentDefault.begin(); it != currentDefault.end(); it++){
            if(nomEffet.compare(*it) == 0){
                found = true;
                break;
            }
            else
                found = false;
        }
        index++;
        
    }while(found);
    
    return nomEffet;
}

FLWindow* FaustLiveApp::getActiveWin(){
    
    list<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++) {
        //The update of Effect is made in Active Window
        
        //        printf("WINDOW = %p // is Active? = %i\n", *it, (*it)->isActiveWindow());
        
        if((*it)->isActiveWindow()){
            return *it;
        }
    }
    
    return NULL;
}

//--------------------------CREATE EFFECT-------------------------------------

void FaustLiveApp::createSourceFile(string& sourceName, string& content){
    
    QFile f(sourceName.c_str());
    
    if(f.open(QFile::WriteOnly)){
        
        QTextStream textWriting(&f);
        
        textWriting<<content.c_str();
        
        f.close();
    }
}

void FaustLiveApp::update_Source(string& oldSource, string& newSource){
    
    if(oldSource.compare(newSource) != 0){
        
        if(QFileInfo(newSource.c_str()).exists()){
            
            string copyNewSource = newSource + ".bak";
            QFile::rename(newSource.c_str(), copyNewSource.c_str());
            
            if(!QFile::copy(oldSource.c_str(), newSource.c_str()))
                QFile::rename(copyNewSource.c_str(), newSource.c_str());
            else
                QFile::remove(copyNewSource.c_str());
        }
        else
            QFile::copy(oldSource.c_str(), newSource.c_str());
    }
}

bool FaustLiveApp::doesEffectNameExists(string nomEffet, list<string> runningEffects){
    
    list<string>::iterator it;
    for(it = runningEffects.begin(); it!= runningEffects.end(); it++){
        
        if(it->compare(nomEffet) == 0)
            return true;
    }
    return false;
}

string FaustLiveApp::getDeclareName(string text, list<string> runningEffects){
    
    string returning = "";
    int pos = text.find("declare name");
    
    if(pos != string::npos){
        text.erase(0, pos);
        
        pos=text.find("\"");
        if(pos != string::npos){
            text.erase(0, pos+1);
        }
        pos = text.find("\"");
        text.erase(pos, text.length()-pos);
        
        returning = renameEffect(text, runningEffects);
        while(returning.find(' ') != string::npos)
            returning.erase(returning.find(' '), 1);
    }
    
    return returning;
}

string FaustLiveApp::renameEffect(string nomEffet, list<string> runningEffects){
    
    while(doesEffectNameExists(nomEffet, runningEffects)){
        
        FLrenameDialog* Msg = new FLrenameDialog(nomEffet, 0);
        Msg->raise();
        Msg->exec();
        nomEffet = Msg->getNewName();
        delete Msg;
    }
    
    return nomEffet;
}

string FaustLiveApp::ifUrlToText(string& source){
    
    //In case the text dropped is a web url
    int pos = source.find("http://");
    
    string UrlText;
    
    if(pos != string::npos){
        UrlText = "process = component(\"";
        UrlText += source;
        UrlText +="\");";
        source = UrlText;
    }
    
    return source;
}

string FaustLiveApp::getNameEffectFromSource(string sourceToCompare){
    
    list<WinInSession*>::iterator it;
    
    for(it = sessionContent.begin() ; it != sessionContent.end() ; it ++){
        
        if((*it)->source.compare(sourceToCompare) == 0)
            return (*it)->name;
    }
    return "";
}

bool FaustLiveApp::isEffectInCurrentSession(string sourceToCompare){
    
    list<WinInSession*>::iterator it;
    
    for(it = sessionContent.begin() ; it != sessionContent.end() ; it ++){
        
        if((*it)->source.compare(sourceToCompare) == 0)
            return true;
    }
    return false;
    
}

list<string> FaustLiveApp::getNameRunningEffects(){
    
    list<string> returning; 
    
    list<WinInSession*>::iterator it;
    
    for(it = sessionContent.begin() ; it != sessionContent.end() ; it ++)
        returning.push_back((*it)->name);   
    
    return returning;
}

bool FaustLiveApp::isEffectNameInCurrentSession(string& sourceToCompare ,string& nom){
    
    list<int> returning;
    
    list<WinInSession*>::iterator it;
    
    for(it = sessionContent.begin() ; it != sessionContent.end() ; it ++){
        
        if((*it)->name.compare(nom)==0 && ((*it)->source.compare(sourceToCompare) != 0 || (*it)->name.find(DefaultName) != string::npos))
            return true;
    }
    return false;
}

Effect* FaustLiveApp::getEffectFromSource(string& source, string& nameEffect, string& sourceFolder, string compilationOptions, int opt_Val, char* error, bool init){
    
    bool fileSourceMark = false;
    string content = "";
    string fichierSource = "";
    
    list<string> currentDefault = get_currentDefault();
    string defaultName = find_smallest_defaultName(source, currentDefault);
    
    source = ifUrlToText(source);
    
    //SOURCE = FILE.DSP    
    if(source.find(".dsp") != string::npos){
        
        list<Effect*>::iterator it;
        for(it = executedEffects.begin(); it!= executedEffects.end(); it++){
            if(source.compare((*it)->getSource()) == 0){
                
                if(isEffectInCurrentSession((*it)->getSource())){
                    return *it;                       
                }
                else{
                    string effetName = (*it)->getName();
                    if(isEffectNameInCurrentSession(source, effetName)){
                        
                        (*it)->setName(renameEffect((*it)->getName(), getNameRunningEffects()));
                        
                        return *it;
                    }
                    else{
                        return *it;
                    }
                }
            }
        }
        
        //OTHERWISE
        
        fileSourceMark = true;
        fichierSource = source;
        
        if(!init){
            //            if(nameEffect.compare("") == 0){
            nameEffect = (QFileInfo(fichierSource.c_str()).baseName()).toStdString();
            
            nameEffect = renameEffect(nameEffect, getNameRunningEffects());
            //            }
            
            printf("NAME EFFECT = %s\n", nameEffect.c_str());
            
            while(nameEffect.find(' ') != string::npos)
                nameEffect.erase(nameEffect.find(' '), 1);
        }
    }
    
    //SOURCE = TEXT
    else{
        
        string name = getDeclareName(source, getNameRunningEffects());
        
        if(name.compare("") == 0)
            name = defaultName;
        
        
        while(name.find(' ') != string::npos)
            name.erase(name.find(' '), 1);
        
        fichierSource = sourceFolder + "/" + name + ".dsp";
        
        string content;
        if(QFileInfo(fichierSource.c_str()).exists()){
            content = pathToContent(fichierSource);
        }
        createSourceFile(fichierSource, source);
        nameEffect = name;
    }
    
    display_CompilingProgress("Compiling your DSP...");
    
    Effect* myNewEffect = new Effect(init, fichierSource, nameEffect);
    //    list<string> runningEffects = getNameRunningEffects();
    
    printf("PARAMETERS = SVG %s// IR %s // Options %s // opt =%i \n", currentSVGFolder.c_str(), currentIRFolder.c_str(), compilationOptions.c_str(), opt_Val);
    
    if(myNewEffect->init(currentSVGFolder, currentIRFolder, compilationOptions, opt_Val, error)){
        
        StopProgressSlot();
        
        connect(myNewEffect, SIGNAL(effectChanged()), this, SLOT(synchronize_Window()));
        
        executedEffects.push_back(myNewEffect);
        
        return myNewEffect;
    }
    else{
        
        if(!fileSourceMark){
            
            QFile f(fichierSource.c_str());
            
            if(f.open(QFile::WriteOnly)){
                
                QTextStream textWriting(&f); 
                
                textWriting<<content.c_str();
                f.close();
            }
        }
        
        StopProgressSlot();
        delete myNewEffect;
        return NULL;
    }
}

list<int> FaustLiveApp::WindowCorrespondingToEffect(Effect* eff){
    
    list<int> returning;
    
    list<WinInSession*>::iterator it;
    
    for(it = sessionContent.begin() ; it != sessionContent.end() ; it ++){
        
        if((*it)->source.compare(eff->getSource()) == 0)
            returning.push_back((*it)->ID);
    }
    return returning;
}

void FaustLiveApp::removeFilesOfWin(string sourceName, string effName){
    
    QFile::remove(sourceName.c_str());
    
    string irFile = currentIRFolder + "/" + effName;
    QFile::remove(irFile.c_str());
    
    string svgFolder = currentSVGFolder + "/" + effName + "-svg";
    deleteDirectoryAndContent(svgFolder);
    
}

void FaustLiveApp::synchronize_Window(){ 
    
    Effect* modifiedEffect = (Effect*)QObject::sender();
    
    string modifiedSource = modifiedEffect->getSource();
    char error[256];
    snprintf(error, 255, "");
    
    QDateTime modifiedLast = QFileInfo(modifiedSource.c_str()).lastModified();
    QDateTime creationDate = modifiedEffect->get_creationDate();
    
    if(QFileInfo(modifiedSource.c_str()).exists() && (modifiedEffect->isSynchroForced() || creationDate<modifiedLast)){
        
        modifiedEffect->setForceSynchro(false);
        
        modifiedEffect->stop_Watcher();
        
        //        display_CompilingProgress("Updating your DSP...");
        
        bool update = modifiedEffect->update_Factory(opt_level, error, currentSVGFolder, currentIRFolder);
        
        if(!update){
            //            StopProgressSlot();
            errorWindow->print_Error(error);
            modifiedEffect->launch_Watcher();
            return;
        }
        else if(strcmp(error, "") != 0){
            errorWindow->print_Error(error);
        }
        
        //        StopProgressSlot();
        
        list<int> indexes = WindowCorrespondingToEffect(modifiedEffect);
        
        list<int>::iterator it;
        for(it=indexes.begin(); it!=indexes.end(); it++){
            list<FLWindow*>::iterator it2;
            
            for (it2 = FLW_List.begin(); it2 != FLW_List.end(); it2++) {
                if((*it2)->get_indexWindow() == *it){
                    if(!(*it2)->update_Window(modifiedEffect, modifiedEffect->getCompilationOptions(), modifiedEffect->getOptValue(),error, bufferSize, compressionValue, masterIP, masterPort, latency)){
                        errorWindow->print_Error(error);
                        break;
                    }
                    else{
                        
                        //                        printf("WINDOW INDEX = %i\n", *it);
                        
                        deleteWinFromSessionFile(*it2);
                        addWinToSessionFile(*it2);
                        
                        string oldSource = modifiedEffect->getSource();
                        string newSource = currentSourcesFolder + "/" + modifiedEffect->getName() + ".dsp";
                        printf("SYNCHRONIZE COPIES = %s TO %s\n", oldSource.c_str(), newSource.c_str());
                        update_Source(oldSource, newSource);
                        break;
                    }
                }
            }
            
        }
        modifiedEffect->erase_OldFactory();
        modifiedEffect->launch_Watcher();
        printf("modifiedEffect OPT = %i\n", modifiedEffect->getOptValue());
    }
    else if(!QFileInfo(modifiedSource.c_str()).exists()){
        
        modifiedEffect->stop_Watcher();
        
        snprintf(error, 255, "WARNING = %s has been deleted or moved\n You are now working on its copy.", modifiedSource.c_str());
        errorWindow->print_Error(error);
        
        string toReplace = currentSourcesFolder + "/" + modifiedEffect->getName() +".dsp";
        //        string oldSource = modifiedEffect->getSource();
        //        list<pair<string,string> > sourceChanges;
        //        sourceChanges.push_back(make_pair(oldSource, toReplace));
        //        
        //        list<WinInSession*>::iterator it;
        //        
        //        for(it = sessionContent.begin(); it != sessionContent.end() ; it++){
        //            
        //            list<pair<string,string> >::iterator it2;
        //            
        //            for(it2 = sourceChanges.begin(); it2 != sourceChanges.end(); it2++){
        //                
        //                if(it2->first.compare((*it)->source) == 0){
        //                    (*it)->source = it2->second;
        //                    break;
        //                }
        //            }
        //        }
        //        
        modifiedEffect->setSource(toReplace);
        modifiedEffect->launch_Watcher();
    }
    
}

//--------------------------------FILE----------------------------------------

//---------------NEW
void FaustLiveApp::create_New_Window(string& source){
    
    bool init = false;
    
    //In case the source is empty, the effect is chosen by default 
    if(source.compare("") == 0){
        source = "process = !,!:0,0;";
        init = true;
    }
    
    //Choice of new Window's index
    list<int> currentIndexes = get_currentIndexes();
    int val = find_smallest_index(currentIndexes);
    stringstream ss;
    ss << val;
    
    //Creation of new effect from the source
    char error[256];
    snprintf(error, 255, "");
    
    string empty("");
    Effect* first = getEffectFromSource(source, empty, currentSourcesFolder, compilationMode, opt_level ,error, false); 
    
    if(first != NULL){
        
        bool optionChanged = (compilationMode.compare(first->getCompilationOptions()) != 0 || opt_level != (first->getOptValue())) && !isEffectInCurrentSession(first->getSource());
        
        //ICI ON VA FAIRE LA COPIE DU FICHIER SOURCE
        string copySource = currentSourcesFolder +"/" + first->getName() + ".dsp";
        string toCopy = first->getSource();
        
        update_Source(toCopy, copySource);
        
        if(strcmp(error, "") != 0){
            errorWindow->print_Error(error);
        }
        
        int x, y;
        calculate_position(val, &x, &y);
        
        FLWindow* win = new FLWindow(WindowBaseName, val, first, x, y, currentSession, indexAudio);
        connect(win, SIGNAL(drop(list<string>)), this, SLOT(drop_Action(list<string>)));
        
        connect(win, SIGNAL(close()), this, SLOT(close_Window_Action()));
        connect(win, SIGNAL(closeAll()), this, SLOT(shut_AllWindows()));
        connect(win, SIGNAL(rightClick(const QPoint &)), this, SLOT(redirect_RCAction(const QPoint &)));
        connect(win, SIGNAL(error(const char*)), this, SLOT(errorPrinting(const char*)));
        
        if(win->init_Window(init, false, error, bufferSize, compressionValue, masterIP, masterPort, latency)){
            FLW_List.push_back(win);
            addWinToSessionFile(win);
            first->launch_Watcher();
            
            //In case the compilation options have changed...
            if(optionChanged)
                first->update_compilationOptions(compilationMode, opt_level);
        }
        else{
            delete win;
            errorWindow->print_Error(error); 
        }
    }
    else
        errorWindow->print_Error(error);
    
}

void FaustLiveApp::create_Empty_Window(){ 
    string empty("");
    create_New_Window(empty);
}

//--------------OPEN

bool FaustLiveApp::event(QEvent *ev){
    
    //In the case of a DSP dropped on the Application's icon, this event is called
    if (ev->type() == QEvent::FileOpen) {
        
        //If the application is opened by a drop action, the current Session has to be reseted
        if(FLW_List.size() == 0)
            reset_CurrentSession();
        
        QString fileName = static_cast<QFileOpenEvent *>(ev)->file();
        string name = fileName.toStdString();
        
        if(name.find(".tar") != string::npos)
            recall_Snapshot(name, true);
        if(name.find(".dsp") != string::npos)
            create_New_Window(name);
        return true;
    } 
    
    return QApplication::event(ev);
}

void FaustLiveApp::open_New_Window(){ 
    
    FLWindow* win = getActiveWin();
    
    QStringList fileNames = QFileDialog::getOpenFileNames(NULL, tr("Open one or several DSPs"), "",tr("Files (*.dsp)"));
    
    QStringList::iterator it;
    
    for(it = fileNames.begin(); it != fileNames.end(); it++){
        
        if(it->toStdString() != ""){
            string inter(it->toStdString());
            
            if(win != NULL && win->is_Default())
                update_SourceInWin(win, inter);
            else
                create_New_Window(inter);
        }
    }
}

//--------------OPEN EXAMPLE

void FaustLiveApp::open_Example_Action(){
    
    string path = QFileInfo(QFileInfo( QCoreApplication::applicationFilePath()).absolutePath()).absolutePath().toStdString() + "/Resources/Examples/" + exampleToOpen + ".dsp";
    
    if(QFileInfo(path.c_str()).exists()){
        
        presWin->hide();
        
        FLWindow* win = getActiveWin();
        
        if(win != NULL && win->is_Default())
            update_SourceInWin(win, path);
        else
            create_New_Window(path);
    }
}

//-------------OPEN RECENT

void FaustLiveApp::save_Recent_Files(){
    
    QFile f(homeRecentFiles.c_str());
    
    if(f.open(QFile::WriteOnly | QFile::Truncate)){
        
        QTextStream text(&f);
        
        list<pair<string, string> >::iterator it;
        
        for (it = recentFiles.begin(); it != recentFiles.end(); it++) {
            QString toto = it->first.c_str();
            QString tata = it->second.c_str();
            text << toto <<' '<<tata<< endl;
        }
    }
    f.close();
}

void FaustLiveApp::recall_Recent_Files(string& filename){
    
    QFile f(filename.c_str());
    QString toto, titi;
    
    if(f.open(QFile::ReadOnly)){
        
        QTextStream textReading(&f);
        
        while(!textReading.atEnd()){
            textReading >> toto >> titi;
            string tata = toto.toStdString();
            string tutu = titi.toStdString();
            
            if(tata.compare("") != 0)
                set_Current_File(tata, tutu);
        }
        f.close();
    }
}

void FaustLiveApp::set_Current_File(string& pathName, string& effName){
    
    pair<string,string> myPair = make_pair(pathName, effName);
    
    recentFiles.remove(myPair);
    recentFiles.push_front(myPair);
    
    update_Recent_File();
}

void FaustLiveApp::update_Recent_File(){
    
    int j = 0;
    
    list<pair<string, string> >::iterator it;
    
    for (it = recentFiles.begin(); it != recentFiles.end(); it++) {
        
        if(j<MaxRecentFiles){
            
            QString text;
            text += it->second.c_str();
            recentFileAction[j]->setText(text);
            recentFileAction[j]->setData(it->first.c_str());
            recentFileAction[j]->setVisible(true);
            
            j++;
        }
    }
}

void FaustLiveApp::open_Recent_File(){
    
    QAction* action = qobject_cast<QAction*>(sender());
    string toto(action->data().toString().toStdString());
    
    FLWindow* win = getActiveWin();
    
    if(win != NULL && win->is_Default())
        update_SourceInWin(win, toto);
    else
        create_New_Window(toto);
}

//---------------Export

void FaustLiveApp::export_Win(FLWindow* win){

    exportDialog = new FLExportManager(QUrl("http://localhost:8888"), win->get_Effect()->getSource(), win->get_Effect()->getName());
    
    connect(exportDialog, SIGNAL(error(const char*)), this, SLOT(errorPrinting(const char*)));
    connect(exportDialog, SIGNAL(start_progressing(const char*)), this, SLOT(display_CompilingProgress(const char*)));
    connect(exportDialog, SIGNAL(stop_progressing()), this, SLOT(StopProgressSlot()));
    connect(exportDialog, SIGNAL(processEnded()), this, SLOT(destroyExportDialog()));

    exportDialog->init();
//    exportDialog->exec();
}

void FaustLiveApp::export_Action(){ 
    
    FLWindow* win = getActiveWin();
    
    if(win != NULL)
        export_Win(win);
}

void FaustLiveApp::destroyExportDialog(){
    delete exportDialog;
}

//--------------CLOSE

void FaustLiveApp::display_Progress(){
    
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
        PBar = new QProgressBar(savingMessage);
        PBar->setAlignment(Qt::AlignCenter);
        PBar->setRange(0,100);
        PBar->setValue(0);
        
        QVBoxLayout* layoutSave = new QVBoxLayout;
        
        layoutSave->addWidget(tittle);
        layoutSave->addWidget(new QLabel(tr("")));
        layoutSave->addWidget(text);
        layoutSave->addWidget(new QLabel(tr("")));
        layoutSave->addWidget(PBar);
        savingMessage->setLayout(layoutSave);
        
        savingMessage->adjustSize();
        savingMessage->show();
        
        endTimer = new QTimer(this);
        connect(endTimer, SIGNAL(timeout()), this, SLOT(update_ProgressBar()));
        endTimer->start(25);
    }
    else
        quit();
}

void FaustLiveApp::update_ProgressBar(){
    
    int value = PBar->value();
    int maximum = PBar->maximum();
    if (value < maximum){
        value ++;
        PBar->setValue(value);
    }
    if(value >= maximum){
        this->quit();
    }
}

void FaustLiveApp::closeAllWindows(){
    
    //    printf("CLOSE ALL WINDOWS\n");
    
    display_Progress();
    
    update_CurrentSession();
    sessionContentToFile(currentSessionFile);
    
    list<FLWindow*>::iterator it;
    
    for(it = FLW_List.begin(); it != FLW_List.end(); it++){
        
        string toto = (*it)->get_Effect()->getSource();
        string tutu = (*it)->get_Effect()->getName();
        
        if(toto.find(currentSourcesFolder) == string::npos)
            set_Current_File(toto, tutu);
        
        (*it)->close_Window();
        delete (*it);
    }
    FLW_List.clear();
    
    list<Effect*>::iterator it2;
    for(it2 = executedEffects.begin() ;it2 != executedEffects.end(); it2++)
        delete (*it2);
    
    executedEffects.clear();
    
}

void FaustLiveApp::common_shutAction(FLWindow* win){
    
    Effect* toDelete = NULL;
    
    string toto = win->get_Effect()->getSource();
    string tutu = win->get_Effect()->getName();
    if(toto.find(currentSourcesFolder) == string::npos)
        set_Current_File(toto, tutu);
    
    deleteWinFromSessionFile(win);
    
    win->shut_Window();
    
    QFileInfo ff((win)->get_Effect()->getSource().c_str());
    string toCompare = ff.absolutePath().toStdString();
    
    if(toCompare.compare(currentSourcesFolder) == 0 && !isEffectInCurrentSession((win)->get_Effect()->getSource())){
        executedEffects.remove((win)->get_Effect());
        //        QFile::remove((win)->get_Effect()->getSource().c_str());
        removeFilesOfWin((win)->get_Effect()->getSource().c_str(), (win)->get_Effect()->getName().c_str());
        toDelete = (win)->get_Effect();
    }
    else if(!isEffectInCurrentSession((win)->get_Effect()->getSource())){
        (win)->get_Effect()->stop_Watcher();
        string toErase = currentSourcesFolder + "/" + (win)->get_Effect()->getName() + ".dsp";
        //        QFile::remove(toErase.c_str());
        removeFilesOfWin(toErase, (win)->get_Effect()->getName());
    }
    
    FLW_List.remove(win);
    delete (win);
    
    if(toDelete != NULL){
        delete toDelete;
        toDelete = NULL;
    }
}

void FaustLiveApp::shut_AllWindows(){
    
    while(FLW_List.size() != 0 ){
        
        FLWindow* win = *(FLW_List.begin());
        
        common_shutAction(win);
    }
}

void FaustLiveApp::shut_Window(){
    
    if(errorWindow->isActiveWindow())
        errorWindow->hideWin();
    //    else if(preference->isActive())
    //        hide_preferenceWindow();
    
    else{
        
        list<FLWindow*>::iterator it = FLW_List.begin();
        
        while(it != FLW_List.end()) {
            
            if((*it)->isActiveWindow()){
                
                common_shutAction(*it);
                
                break;
            }
            else if((*it)->is_httpdWindow_active()){
                (*it)->hide_httpdWindow();
                break;
            }
            else
                it++;
        }
    }
}

void FaustLiveApp::close_Window_Action(){
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    common_shutAction(win);
}

//--------------------------------EDITION----------------------------------------

void FaustLiveApp::edit(FLWindow* win){
    
    string source = win->get_Effect()->getSource();
    
    string cmd("open ");
    
    cmd += source;
    
    QString pgm("TextEdit");
    QStringList args;
    args<<source.c_str();
    //    args.push_back(source.c_str());
    
    string error = source + " could not be opened!";
    
    if(system(cmd.c_str()))
        errorWindow->print_Error(error.c_str());
}

void FaustLiveApp::edit_Action(){
    
    FLWindow* win = getActiveWin();
    if(win != NULL)
        edit(win);
}

void FaustLiveApp::duplicate(FLWindow* window){
    
    Effect* commonEffect = window->get_Effect();
    //To avoid flicker of the original window, the watcher is stopped during operation
    commonEffect->stop_Watcher();
    
    string source = commonEffect->getSource();
    
    list<int> currentIndexes = get_currentIndexes();
    int val = find_smallest_index(currentIndexes);
    stringstream ss;
    ss << val;
    
    int x = window->get_x() + 10;
    int y = window->get_y() + 10;
    
    FLWindow* win = new FLWindow(WindowBaseName, val, commonEffect, x, y, currentSession, indexAudio);
    
    connect(win, SIGNAL(drop(list<string>)), this, SLOT(drop_Action(list<string>)));
    connect(win, SIGNAL(close()), this, SLOT(close_Window_Action()));
    connect(win, SIGNAL(closeAll()), this, SLOT(shut_AllWindows()));
    connect(win, SIGNAL(rightClick(const QPoint &)), this, SLOT(redirect_RCAction(const QPoint &)));
    connect(win, SIGNAL(error(const char*)), this, SLOT(errorPrinting(const char*)));
    
    //Save then Copy of duplicated window's parameters
    window->save_Window(indexAudio);
    
    string toFind = window->get_nameWindow();
    string toReplace = win->get_nameWindow();
    
    string savingsPath = currentSession + "/" + toFind + "/" + toFind + ".rc";
    QFile toCpy(savingsPath.c_str());
    string path = currentSession + "/" + toReplace + "/" + toReplace + ".rc";
    toCpy.copy(path.c_str());
    
    savingsPath = currentSession + "/" + toFind + "/" + toFind + ".jc";
    QFile toCy(savingsPath.c_str());
    path = currentSession + "/" + toReplace + "/" + toReplace + ".jc";
    toCy.copy(path.c_str());
    
    //Replacement of WindowName in the audio Connections File to reconnect the new window as the duplicated one.
    list<pair<string, string> > changeTable;
    changeTable.push_back(make_pair(toFind, toReplace));
    win->update_ConnectionFile(changeTable);
    
    char error[256];
    snprintf(error, 255, "");
    
    if(win->init_Window(false, true, error, bufferSize, compressionValue, masterIP, masterPort, latency)){
        FLW_List.push_back(win);
        addWinToSessionFile(win);
    }
    else{
        string toDelete = currentSession + "/" + win->get_nameWindow(); 
        deleteDirectoryAndContent(toDelete);
        delete win;
        errorWindow->print_Error(error); 
    }
    
    //Whatever happens, the watcher has to be started (at least for the duplicated window that needs it)
    commonEffect->launch_Watcher();
}

void FaustLiveApp::duplicate_Window(){ 
    
    //copy parameters of previous window
    FLWindow* win = getActiveWin();
    if(win != NULL)
        duplicate(win);
}

void FaustLiveApp::update_SourceInWin(FLWindow* win, string source){
    
    char error[256];
    snprintf(error, 255, "");
    string empty("");
    
    //Deletion of reemplaced effect from session
    Effect* leavingEffect = win->get_Effect();
    leavingEffect->stop_Watcher();
    deleteWinFromSessionFile(win);
    
    Effect* newEffect = getEffectFromSource(source, empty, currentSourcesFolder, compilationMode, opt_level, error, false);
    
    bool optionChanged;
    
    if(newEffect != NULL)
        optionChanged = (compilationMode.compare(newEffect->getCompilationOptions()) != 0 || opt_level != (newEffect->getOptValue())) && !isEffectInCurrentSession(newEffect->getSource());
    
    
    if(newEffect == NULL || (!(win)->update_Window(newEffect, compilationMode, opt_level,error, bufferSize, compressionValue, masterIP, masterPort, latency))){
        //If the change fails, the leaving effect has to be reimplanted
        leavingEffect->launch_Watcher();
        addWinToSessionFile(win);
        errorWindow->print_Error(error);
        return;
    }
    else{
        
        //ICI ON VA FAIRE LA COPIE DU FICHIER SOURCE
        string copySource = currentSourcesFolder +"/" + newEffect->getName() + ".dsp";
        string toCopy = newEffect->getSource();
        
        update_Source(toCopy, copySource);
        
        if(strcmp(error, "") != 0){
            errorWindow->print_Error(error);
        }
        
        //If the change is successfull : 
        QFileInfo ff(leavingEffect->getSource().c_str());
        string toCompare = ff.absolutePath().toStdString();
        
        string inter = leavingEffect->getSource();
        
        //If leaving effect is not used elsewhere : 
        if(!isEffectInCurrentSession(inter)){                        
            if(toCompare.compare(currentSourcesFolder) == 0){
                
                //The effects pointing in the Sources Folder are not kept (nor in the list of exectued Effects, nor the source file)
                //If newEffect source = oldEffect source the file is kept because it has been modified and is needed
                if(newEffect->getSource().compare(leavingEffect->getSource())!=0)
                    removeFilesOfWin(leavingEffect->getSource(), leavingEffect->getName().c_str());
                //                        QFile::remove(leavingEffect->getSource().c_str());
                
                executedEffects.remove(leavingEffect);
                delete leavingEffect;
            }
            else{
                //The copy made of the source is erased
                string toErase = currentSourcesFolder + "/" + leavingEffect->getName() + ".dsp";
                //                    QFile::remove(toErase.c_str());
                removeFilesOfWin(toErase, leavingEffect->getName());
            }
        }
        //If the effect is used in an other window, its watcher is reactivated
        else
            leavingEffect->launch_Watcher();
        
        //The new effect is added in the session and watched
        addWinToSessionFile(win);
        newEffect->launch_Watcher();
        
        //In case the compilation options have changed...
        if(optionChanged)
            newEffect->update_compilationOptions(compilationMode, opt_level);
    }
    
}

void FaustLiveApp::paste(FLWindow* win){
    
    //Recuperation of Clipboard Content
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    
    if (mimeData->hasText()) {
        QString clipText = clipboard->text();
        string text = clipText.toStdString();
        
        update_SourceInWin(win, text);
        
    }
}

void FaustLiveApp::paste_Text(){
    
    FLWindow* win = getActiveWin();
    if(win != NULL)
        paste(win);
} 

void FaustLiveApp::drop_Action(list<string> sources){
    
    list<string>::iterator it = sources.begin();
    FLWindow* win = (FLWindow*)QObject::sender();
    
    //The first source dropped is updated in the dropped Window
    
    update_SourceInWin(win, *it);
    
    //The other source are opened in new Windows
    it++;
    while(it!=sources.end()){
        
        create_New_Window(*it);
        it++;
        
    }
}

//-----------------------------RIGHT CLICK EVENT

void FaustLiveApp::redirect_RCAction(const QPoint & p){
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    QMenu* rcMenu = new QMenu();
    
    rcMenu->addAction(editAction);
    rcMenu->addAction(pasteAction);
    rcMenu->addAction(duplicateAction);
    rcMenu->addSeparator();
    rcMenu->addAction(httpdViewAction);
    rcMenu->addAction(svgViewAction);
    rcMenu->addSeparator();
    rcMenu->addAction(exportAction);
    
    rcMenu->exec(p);
}

//--------------------------------SESSION----------------------------------------

void FaustLiveApp::sessionContentToFile(string filename){
    
    //    printf("SIZE OF CONTENT SESSION = %i\n", session->size());
    
    QFile f(filename.c_str());
    
    if(f.open(QFile::WriteOnly | QIODevice::Truncate)){
        
        list<WinInSession*>::iterator it;
        
        QTextStream textWriting(&f);
        
        for(it = sessionContent.begin() ; it != sessionContent.end() ; it ++){
            
            //            printf("ID = %i// Source = %s // Name = %s // X = %f // Y = %f // Option = %s\n", (*it)->ID, (*it)->source.c_str(), (*it)->name.c_str(), (*it)->x, (*it)->y, (*it)->compilationOptions.c_str());
            
            
            textWriting<<(*it)->ID<<' '<<QString((*it)->source.c_str())<<' '<<QString((*it)->name.c_str())<<' '<<(*it)->x<<' '<<(*it)->y<<' '<<QString((*it)->compilationOptions.c_str())<<' '<<(*it)->opt_level<<endl;
        }
        f.close();
    }
}

void FaustLiveApp::fileToSessionContent(string filename, list<WinInSession*>* session){
    
    QFile f(filename.c_str());
    
    if(f.open(QFile::ReadOnly)){
        
        list<WinInSession*>::iterator it;
        
        QTextStream textReading(&f);
        
        while(!textReading.atEnd()){
            
            int id = 0;
            int opt;
            QString Source, Nom, CompilationOptions;
            float x,y;
            
            textReading>>id>>Source>>Nom>>x>>y>>CompilationOptions>>opt;
            
            if(id != 0){
                
                WinInSession* intermediate = new WinInSession();
                intermediate->ID = id;
                intermediate->source = Source.toStdString();
                intermediate->name = Nom.toStdString();
                intermediate->x = x;
                intermediate->y = y;
                intermediate->compilationOptions = CompilationOptions.toStdString();
                intermediate->opt_level = opt;
                
                
                //                printf("FILLING ID = %i// Source = %s // Name = %s // X = %f // Y = %f // Option = %s\n", intermediate->ID, intermediate->source.c_str(), intermediate->name.c_str(), intermediate->x, intermediate->y, intermediate->compilationOptions.c_str());
                
                session->push_back(intermediate);
                //                printf("SNAPSHOT SIZE = %i\n", session->size());
            }
        }
        f.close();
    }
}

string FaustLiveApp::restore_compilationOptions(string compilationOptions){
    
    int pos = compilationOptions.find("/");
    
    while(pos != string::npos){
        
        compilationOptions.erase(pos, 1);
        compilationOptions.insert(pos, " ");
        
        pos = compilationOptions.find("/");
    }
    
    return compilationOptions;
}

//Recall for any type of session (current or snapshot)
void FaustLiveApp::recall_Session(string filename){
    
    //    printf("FILENAME TO RECALL = %s\n", filename.c_str());
    
    //Temporary copies of the description files in which all the modifications due to conflicts will be saved
    list<WinInSession*>  snapshotContent;
    
    fileToSessionContent(filename, &snapshotContent);
    
    if(filename.compare(currentSessionFile) == 0){
        
        //Reset current Session File to avoid the return true in all the isInCurrentSession? when recalling currentSession
        
        //In case of a disappearance of a source file
        currentSessionRestoration(&snapshotContent);
    }
    else{
        //Different resolution of disappearance for a snapshot 
        snapshotRestoration(filename, &snapshotContent);
    }
    
    //------------Resolution of the name conflicts (Window Names & Effect Names)
    list<std::pair<int,int> > indexChanges = establish_indexChanges(&snapshotContent);
    
    list<std::pair<string, string> > windowNameChanges;
    
    list<std::pair<int, int> >::iterator it2;
    for(it2 = indexChanges.begin(); it2 != indexChanges.end(); it2++){
        
        stringstream s1;
        s1<<it2->first;
        
        string nameW = WindowBaseName + "-" + s1.str();
        
        stringstream s2;
        s2<<it2->second;
        string newNameW = WindowBaseName + "-" + s2.str();
        
        windowNameChanges.push_back(make_pair(nameW, newNameW));
    }
    
    list<std::pair<string,string> > nameChanges = establish_nameChanges(&snapshotContent);
    
    //In case of Snapshot Restoration, sources and window parameters Folders have to be copied in currentSession
    if((QFileInfo(filename.c_str()).absolutePath().toStdString()).compare(currentSession) != 0){
        establish_sourceChanges(nameChanges, &snapshotContent);
        
        QFileInfo sourceDir(filename.c_str());
        string snapshotSourcesFolder = sourceDir.absolutePath().toStdString() + "/Sources"; 
        copy_AllSources(snapshotSourcesFolder, currentSourcesFolder, nameChanges,".dsp");
        string snapshotIRFolder = sourceDir.absolutePath().toStdString() + "/IR"; 
        copy_AllSources(snapshotIRFolder, currentIRFolder, nameChanges, "");
        string snapshotSVGFolder = sourceDir.absolutePath().toStdString() + "/SVG";         
        copy_SVGFolders(snapshotSVGFolder, currentSVGFolder, nameChanges);
        
        string snapshotFolder = sourceDir.absolutePath().toStdString();
        
        copy_WindowsFolders(snapshotFolder, currentSession, windowNameChanges);
        
        
    }//Otherwise, one particular case has to be taken into account. If the content was modified and the Effect NOT recharged from source. The original has to be recopied!
    else{
        
    }
    
    //--------------Recalling without conflict the session
    
    list<WinInSession*>::iterator it;
    
    for(it = snapshotContent.begin() ; it != snapshotContent.end() ; it ++){
        
        char error[256];
        snprintf(error, 255, "");
        
        (*it)->compilationOptions = restore_compilationOptions((*it)->compilationOptions);
        
        Effect* newEffect = getEffectFromSource((*it)->source, (*it)->name, currentSourcesFolder, (*it)->compilationOptions, (*it)->opt_level, error, true);
        //            printf("THE new Factory = %p\n", newEffect->getFactory());
        
        //ICI ON NE VA PAS FAIRE LA COPIE DU FICHIER SOURCE!!!
        
        if(newEffect != NULL){
            
            if(strcmp(error, "") != 0){
                errorWindow->print_Error(error);
            }
            
            FLWindow* win = new FLWindow(WindowBaseName, (*it)->ID, newEffect, (*it)->x*screenWidth, (*it)->y*screenHeight, currentSession, indexAudio);
            
            connect(win, SIGNAL(drop(list<string>)), this, SLOT(drop_Action(list<string>)));
            connect(win, SIGNAL(close()), this, SLOT(close_Window_Action()));
            connect(win, SIGNAL(closeAll()), this, SLOT(shut_AllWindows()));
            connect(win, SIGNAL(rightClick(const QPoint &)), this, SLOT(redirect_RCAction(const QPoint &)));
            connect(win, SIGNAL(error(const char*)), this, SLOT(errorPrinting(const char*)));
            
            //Modification of connection files with the new window & effect names
            
            win->update_ConnectionFile(windowNameChanges);
            win->update_ConnectionFile(nameChanges);
            
            if(win->init_Window(false, true, error, bufferSize, compressionValue, masterIP, masterPort, latency)){
                //                    printf("The new DSP INstance = %p\n", win->current_DSP);
                FLW_List.push_back(win);
                newEffect->launch_Watcher();
                addWinToSessionFile(win);
                win->save_Window(indexAudio);
                
                //In case the compilation options have changed...
                if((*it)->compilationOptions.compare(newEffect->getCompilationOptions()) != 0)
                    newEffect->update_compilationOptions((*it)->compilationOptions, (*it)->opt_level);
            }
            else{
                //                    printf("deleting DSP... : %p\n", win->current_DSP);
                delete win;
                errorWindow->print_Error(error);    
            }
        }
        else{
            errorWindow->print_Error(error);
        }
    }
    //    sessionContent.merge(snapshotContent);
}

//--------------RECENTLY OPENED

void FaustLiveApp::save_Recent_Sessions(){
    
    QFile f(homeRecentSessions.c_str());
    
    if(f.open(QFile::WriteOnly | QFile::Truncate)){
        
        QTextStream text(&f);
        
        for (int i = min(MaxRecentSessions, recentSessions.size()) - 1; i>=0; i--) {
            QString toto = recentSessions[i];
            text << toto << endl;
        }
    }
    f.close();
}

void FaustLiveApp::recall_Recent_Sessions(string& filename){
    
    QFile f(filename.c_str());
    QString toto;
    
    if(f.open(QFile::ReadOnly)){
        
        QTextStream textReading(&f);
        
        while(!textReading.atEnd()){
            textReading >> toto;
            string tata = toto.toStdString();
            set_Current_Session(tata);
        }
        f.close();
    }
}

void FaustLiveApp::set_Current_Session(string& pathName){
    QString currentSess = pathName.c_str();
    recentSessions.removeAll(currentSess);
    recentSessions.prepend(currentSess);
    update_Recent_Session();
}

void FaustLiveApp::update_Recent_Session(){
    
    QMutableStringListIterator i(recentSessions);
    while(i.hasNext()){
        if(!QFile::exists(i.next()))
            i.remove();
    }
    
    for(int j=0; j<MaxRecentSessions; j++){
        if(j<recentSessions.count()){
            
            QString path = QFileInfo(recentSessions[j]).baseName();
            
            QString text = tr("&%1 %2").arg(j+1).arg(path);
            RrecentSessionAction[j]->setText(text);
            RrecentSessionAction[j]->setData(recentSessions[j]);
            RrecentSessionAction[j]->setVisible(true);
            
            IrecentSessionAction[j]->setText(text);
            IrecentSessionAction[j]->setData(recentSessions[j]);
            IrecentSessionAction[j]->setVisible(true);
        }
        else{
            RrecentSessionAction[j]->setVisible(false);
            IrecentSessionAction[j]->setVisible(false);
        }
    }
}

void FaustLiveApp::recall_Recent_Session(){
    
    QAction* action = qobject_cast<QAction*>(sender());
    string toto(action->data().toString().toStdString());
    recall_Snapshot(toto, false);
}

void FaustLiveApp::import_Recent_Session(){
    
    QAction* action = qobject_cast<QAction*>(sender());
    string toto(action->data().toString().toStdString());
    recall_Snapshot(toto, true);
}

//---------------CURRENT SESSION FUNCTIONS
string FaustLiveApp::convert_compilationOptions(string compilationOptions){
    int pos = compilationOptions.find(" ");
    
    while(pos != string::npos){
        
        compilationOptions.erase(pos, 1);
        compilationOptions.insert(pos, "/");
        
        pos = compilationOptions.find(" ");
    }
    
    return compilationOptions;
}

void FaustLiveApp::addWinToSessionFile(FLWindow* win){
    
    string compilationOptions = convert_compilationOptions(win->get_Effect()->getCompilationOptions());
    
    if(compilationOptions.compare("") == 0)
        compilationOptions = "/";
    
    WinInSession* intermediate = new WinInSession;
    intermediate->ID = win->get_indexWindow();
    intermediate->source = win->get_Effect()->getSource().c_str();
    intermediate->name = win->get_Effect()->getName().c_str();
    intermediate->x = (float)win->get_x()/(float)screenWidth;
    intermediate->y = (float)win->get_y()/(float)screenHeight;
    intermediate->compilationOptions = compilationOptions.c_str();
    intermediate->opt_level = win->get_Effect()->getOptValue();
    
    
    int i = frontWindow.size();
    
    QString name = win->get_nameWindow().c_str();
    name+=" : ";
    name+= win->get_Effect()->getName().c_str();
    
    QAction* fifiWindow = new QAction(name, windowsMenu);
    frontWindow.push_back(fifiWindow);
    
    fifiWindow->setData(QVariant(win->get_nameWindow().c_str()));
    connect(fifiWindow, SIGNAL(triggered()), win, SLOT(frontShow()));
    
    windowsMenu->addAction(fifiWindow);
    
    //    printf("ADDING ID = %i// Source = %s // Name = %s // X = %f // Y = %f // Option = %s\n", intermediate->ID, intermediate->source.c_str(), intermediate->name.c_str(), intermediate->x, intermediate->y, intermediate->compilationOptions.c_str());
    
    sessionContent.push_back(intermediate);
}

void FaustLiveApp::deleteWinFromSessionFile(FLWindow* win){
    
    list<WinInSession*>::iterator it;
    
    for(it = sessionContent.begin() ; it != sessionContent.end() ; it++){
        
        if((*it)->ID == win->get_indexWindow()){
            //            printf("REMOVING = %i\n", win->get_indexWindow());
            sessionContent.remove(*it);
            
            QAction* toRemove = NULL;
            
            QList<QAction*>::iterator it;
            for(it = frontWindow.begin(); it != frontWindow.end() ; it++){
                if((*it)->data().toString().toStdString().compare(win->get_nameWindow()) == 0){
                    windowsMenu->removeAction(*it);
                    frontWindow.removeOne(*it);
                    //                    toRemove = *it;
                    break;
                }
            }
            //            delete toRemove;
            break;
        }
    }
}

void FaustLiveApp::reset_CurrentSession(){
    
    QDir srcDir(currentSession.c_str());
    
    QFileInfoList children = srcDir.entryInfoList(QDir::Dirs | QDir::Drives | QDir::NoDotAndDotDot);
    
    QFileInfoList::iterator it;
    
    for(it = children.begin(); it != children.end(); it++){
        
        string toRemove(it->absoluteFilePath().toStdString());
        //        printf("WATH IS BEING REMOVED??? = %s\n", toRemove.c_str());
        deleteDirectoryAndContent(toRemove);
    }
    
    QDir nv(currentSession.c_str());
    QString ss(currentSession.c_str());
    ss += "/Sources";
    nv.mkdir(ss);
    
    QString svg(currentSession.c_str());
    svg += "/SVG";
    nv.mkdir(svg);
    
    QString ir(currentSession.c_str());
    ir += "/IR";
    nv.mkdir(ir);
    
    sessionContent.clear();
    
    recall_Settings(homeSettings);
}

void FaustLiveApp::update_CurrentSession(){
    
    list<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++){
        
        deleteWinFromSessionFile(*it);
        addWinToSessionFile(*it);
        (*it)->save_Window(indexAudio);
    }
}

void FaustLiveApp::currentSessionRestoration(list<WinInSession*>* session){
    
    //If 2 windows are pointing on the same lost source, the Dialog has not to appear twice
    std::map<string,bool> updated;
    
    //List of the sources to updated in Session File
    list<pair<string, string> > sourceChanges;
    
    list<WinInSession*>::iterator it;
    
    for(it = session->begin() ; it != session->end() ; it++){
        
        string contentOrigin = pathToContent((*it)->source);
        string sourceSaved = currentSourcesFolder + "/" + (*it)->name + ".dsp";
        string contentSaved = pathToContent(sourceSaved);
        
        QFileInfo infoSource((*it)->source.c_str());
        //If one source (not in the Source folder) couldn't be found, the User is asked to decide whether to reload it from the copied file
        if(updated.find((*it)->source) == updated.end() && infoSource.absolutePath().toStdString().compare(currentSourcesFolder) != 0 && (!infoSource.exists() || contentSaved.compare(contentOrigin) != 0) ){
            
            string mesg;
            bool contentModif = false;
            
            if(!infoSource.exists())
                mesg = (*it)->source + " cannot be found! Do you want to reload it from a copied file?";
            else{
                mesg = "The content of " + (*it)->source + " has been modified, do you want to reload it from a copied file?";
                contentModif = true;
            }
            
            QPushButton* yes_Button;
            QPushButton*  cancel_Button; 
            
            QString msg(mesg.c_str());
            
            QMessageBox* existingNameMessage = new QMessageBox(QMessageBox::Warning, tr("Notification"), msg);
            
            yes_Button = existingNameMessage->addButton(tr("Copied File"), QMessageBox::AcceptRole);
            cancel_Button = existingNameMessage->addButton(tr("Original File"), QMessageBox::RejectRole);
            
            existingNameMessage->exec();
            updated[(*it)->source] = true;
            
            if (existingNameMessage->clickedButton() == cancel_Button) {
                delete existingNameMessage;
                
                string toErase = currentSourcesFolder + "/" + (*it)->name + ".dsp";
                removeFilesOfWin(toErase, (*it)->name);
                if(!contentModif){
                    deleteLineIndexed((*it)->ID);
                    //                    string toErase = currentSourcesFolder + "/" + (*it)->name + ".dsp";
                    //                    removeFilesOfWin(toErase, (*it)->name);
                }
                else{
                    string newSource = currentSourcesFolder + "/" + (*it)->name + ".dsp";
                    QFile file((*it)->source.c_str());
                    file.copy(newSource.c_str());
                }
            }    
            else{
                delete existingNameMessage;
                string newSource = currentSourcesFolder + "/" + (*it)->name + ".dsp";
                sourceChanges.push_back(make_pair((*it)->source, newSource));
                (*it)->source = newSource;
            }
        }
        //If the source was in the Source Folder and couldn't be found, it can't be reloaded.
        else if(updated.find((*it)->source) == updated.end() && infoSource.absolutePath().toStdString().compare(currentSourcesFolder) == 0 && !infoSource.exists()){
            
            deleteLineIndexed((*it)->ID);
            string msg = (*it)->name + " could not be reload. The File is lost!"; 
            errorWindow->print_Error(msg.c_str());
        }
        else if(updated.find((*it)->source) != updated.end()){
            
            list<pair<string, string> >::iterator itS;
            
            for(itS = sourceChanges.begin(); itS != sourceChanges.end() ; itS++){
                if(itS->first.compare((*it)->source) == 0)
                    (*it)->source = itS->second;
                
            }
        }
    }
}

//---------------SAVE SNAPSHOT FUNCTIONS

void FaustLiveApp::take_Snapshot(){
    
    QFileDialog* fileDialog = new QFileDialog;
    //    QWidget* parent = this;
    
    string filename = fileDialog->getSaveFileName().toStdString();
    
    //If no name is placed, nothing happens
    if(filename.compare("") != 0){
        
        update_CurrentSession();
        sessionContentToFile(currentSessionFile);
        
        //Copy of current Session under a new name, at a different location
        cpDir(currentSession.c_str(), filename.c_str());
        //
        //        string descriptionFile = filename + "/Description.sffx";
        
        QProcess myCmd;
        QByteArray error;
        
        string systemInstruct("tar cfv ");
        systemInstruct += filename + ".tar " + filename;
        
        myCmd.start(systemInstruct.c_str());
        myCmd.waitForFinished();
        
        error = myCmd.readAllStandardError();
        
        if(strcmp(error.data(), "") != 0)
            errorWindow->print_Error(error.data());
        
        QProcess myCmd2;
        QByteArray error2;
        
        string rmInstruct("rm -r ");
        rmInstruct += filename;
        
        myCmd2.start(rmInstruct.c_str());
        myCmd2.waitForFinished();
        
        error2 = myCmd2.readAllStandardError();
        
        if(strcmp(error2.data(), "") != 0)
            errorWindow->print_Error(error2.data());
        
        string sessionName =  filename + ".tar ";
        set_Current_Session(sessionName);
        
        //        deleteDirectoryAndContent(filename);
    }
}

//---------------RESTORE SNAPSHOT FUNCTIONS

void FaustLiveApp::snapshotRestoration(string& file, list<WinInSession*>* session){
    
    //If 2 windows are pointing on the same lost source, the Dialog has not to appear twice
    std::map<string,bool> updated;
    
    list<pair<string,string> > sourceChanges;
    
    list<WinInSession*>::iterator it;
    for(it = session->begin() ; it != session->end() ; it++){
        
        
        QFileInfo infoSource((*it)->source.c_str());
        
        string contentOrigin = pathToContent((*it)->source);
        
        string sourceSaved = QFileInfo(file.c_str()).absolutePath().toStdString() + "/Sources/" + (*it)->name + ".dsp";
        string contentSaved = pathToContent(sourceSaved);
        
        //If one source (not in the Source folder) couldn't be found, the User is informed that we are now working on the copy
        if(updated.find((*it)->source) == updated.end() && infoSource.absolutePath().toStdString().compare(currentSourcesFolder) != 0  && (!infoSource.exists() || contentSaved.compare(contentOrigin) != 0)){
            
            char error[256];
            
            if(!infoSource.exists())
                snprintf(error, 255, "WARNING = %s cannot be found! It is reloaded from a copied file.", (*it)->source.c_str());
            
            else if(contentSaved.compare(contentOrigin) != 0)
                snprintf(error, 255, "WARNING = The content of %s has been modified! It is reloaded from a copied file.", (*it)->source.c_str());
            
            errorWindow->print_Error(error);
            string newSource = currentSourcesFolder + "/" + (*it)->name + ".dsp";
            updated[(*it)->source] = true;       
            sourceChanges.push_back(make_pair((*it)->source, newSource));
            
            (*it)->source = newSource;
        }
        else if(updated.find((*it)->source) != updated.end()){
            
            list<pair<string, string> >::iterator itS;
            
            for(itS = sourceChanges.begin(); itS != sourceChanges.end() ; itS++){
                if(itS->first.compare((*it)->source) == 0)
                    (*it)->source = itS->second;
            }
        }
    }
}

void FaustLiveApp::recallSnapshotFromMenu(){
    
    QString fileName = QFileDialog::getOpenFileName(NULL, tr("Recall a Session"), "",tr("Files (*.tar)"));
    
    string filename = fileName.toStdString();
    
    if(filename != ""){
        
        recall_Snapshot(filename, false);
    }
}

void FaustLiveApp::importSnapshotFromMenu(){
    
    QString fileName = QFileDialog::getOpenFileName(NULL, tr("Recall a Session"), "",tr("Files (*.tar)"));
    
    string filename = fileName.toStdString();
    
    if(filename != ""){
        
        recall_Snapshot(filename, true);
    }
}

void FaustLiveApp::recall_Snapshot(string filename, bool importOption){ 
    
    set_Current_Session(filename);
    
    QProcess myCmd;
    QByteArray error;
    
    string systemInstruct("tar xfv ");
    systemInstruct += filename +" -C /";
    
    myCmd.start(systemInstruct.c_str());
    myCmd.waitForFinished();
    
    error = myCmd.readAllStandardError();
    
    if(strcmp(error.data(), "") != 0)
        errorWindow->print_Error(error.data());
    
    if(!importOption){
        shut_AllWindows(); 
        reset_CurrentSession();
    }
    
    string finalFile = QFileInfo(filename.c_str()).canonicalPath().toStdString() + "/" + QFileInfo(filename.c_str()).baseName().toStdString();
    
    string finalFilename = finalFile + "/Description.sffx";
    
    recall_Session(finalFilename);
    
    QProcess myCmd2;
    QByteArray error2;
    
    string rmInstruct("rm -r ");
    rmInstruct += finalFile;
    
    myCmd2.start(rmInstruct.c_str());
    myCmd2.waitForFinished();
    
    error = myCmd.readAllStandardError();
    
    if(strcmp(error2.data(), "") != 0)
        errorWindow->print_Error(error2.data());
}

//---------------RENAMING AND ALL FUNCTIONS TO IMPORT

void FaustLiveApp::deleteLineIndexed(int index){
    
    list<WinInSession*>::iterator it;
    
    for(it = sessionContent.begin(); it != sessionContent.end() ; it++){
        
        //Check if line wasn't empty
        if((*it)->ID == index){
            sessionContent.remove(*it);
            break;    
        }
    }
}

list<std::pair<int, int> > FaustLiveApp::establish_indexChanges(list<WinInSession*>* session){
    
    //For each window of the session File, if it's index is used (in current session or by the previous windows re-indexed), it is associated to a new one (the smallest not used)
    
    list<std::pair<int,int> > returning;
    
    list<WinInSession*>::iterator it;
    
    list<int> currentIndexes = get_currentIndexes();
    
    for(it = session->begin(); it != session->end() ; it++){
        
        int newID = (*it)->ID;
        printf("oldIndex = %i\n", newID);
        
        if(isIndexUsed((*it)->ID, currentIndexes)){
            
            newID = find_smallest_index(currentIndexes);
        }
        printf("newIndex = %i\n", newID);
        currentIndexes.push_back(newID);
        returning.push_back(make_pair((*it)->ID, newID));
        (*it)->ID = newID;
    }
    return returning;
}

list<std::pair<string,string> > FaustLiveApp::establish_nameChanges(list<WinInSession*>* session){
    
    //For each window of the session File, if it's index is used (in current session or by the previous windows re-indexed), it is associated to a new one (the smallest not used)
    
    list<std::pair<string,string> > nameChanges;
    list<string> currentDefault = get_currentDefault();
    
    //If 2 windows are holding the same name & source, to avoid renaming them twice
    std::map<string,bool> updated;
    
    list<WinInSession*>::iterator it;
    
    for(it = session->begin(); it != session->end() ; it++){
        
        string newName = (*it)->name;
        
        //1- If the source is already is current Session (& not pointing in Sources Folder) ==> getName already given
        if(isEffectInCurrentSession((*it)->source) && QFileInfo((*it)->source.c_str()).absolutePath().toStdString().compare(currentSourcesFolder) != 0){
            
            newName = getNameEffectFromSource((*it)->source);
            nameChanges.push_front(make_pair((*it)->name, newName));
            (*it)->name = newName;
        }
        
        //2- If the name was already updated ==> don't do nothing
        else if(updated.find(newName) != updated.end()){
            
            list<pair<string,string> >::iterator it2;
            
            for(it2 = nameChanges.begin(); it2 != nameChanges.end(); it2++){
                
                if(it2->first.compare((*it)->name) == 0){
                    (*it)->name = it2->second;
                    break;
                }
            }
            
        }
        
        //3- If the name is a DefaultName
        //==> if the Name is not used in current Session, we keep it and inform of its use
        //==>if it is used, it is renamed as the smallest available and we inform of its use
        
        else if(newName.find(DefaultName) != string::npos){
            bool found = false;
            list<string>::iterator it2;
            for(it2 = currentDefault.begin(); it2 != currentDefault.end(); it2++){
                if(newName.compare(*it2) == 0){
                    found = true;
                    break;
                }
            }
            if(!found){
                
                //                printf("NO FOUND thE DEFAULT NAME\n");
                currentDefault.push_back(newName);
                nameChanges.push_front(make_pair((*it)->name, newName));
                updated[(*it)->name] = true;
                (*it)->name = newName;
            }
            else{
                //                printf("Found the default name\n");
                newName = find_smallest_defaultName((*it)->source, currentDefault);
                currentDefault.push_back(newName);
                nameChanges.push_front(make_pair((*it)->name, newName));
                updated[(*it)->name] = true;
                (*it)->name = newName;
            }
        }
        //4- If the source is in current Folder and its name is already used, it has to be renamed
        else if(QFileInfo((*it)->source.c_str()).absolutePath().toStdString().compare(currentSourcesFolder) == 0){
            
            string intermediateSource = "";
            while(isEffectNameInCurrentSession(intermediateSource, newName)){
                
                FLrenameDialog* Msg = new FLrenameDialog((*it)->name, 0);
                Msg->raise();
                Msg->exec();
                newName = Msg->getNewName();
                delete Msg;
            }
            nameChanges.push_front(make_pair((*it)->name, newName));
            
            updated[(*it)->name] = true;
            (*it)->name = newName;
        }
        //5- If the Name is in current session, is has to be renamed
        else if(isEffectNameInCurrentSession((*it)->source, newName)){
            while(isEffectNameInCurrentSession((*it)->source, newName)){
                
                FLrenameDialog* Msg = new FLrenameDialog((*it)->name, 0);
                Msg->raise();
                Msg->exec();
                newName = Msg->getNewName();
                delete Msg;
            }
            nameChanges.push_front(make_pair((*it)->name, newName));
            updated[(*it)->name] = true;
            (*it)->name = newName;
        }
        else{
            nameChanges.push_front(make_pair((*it)->name, newName));
            updated[(*it)->name] = true;
            (*it)->name = newName;
        }
    }
    
    return nameChanges;
}

void FaustLiveApp::establish_sourceChanges(list<std::pair<string,string> > nameChanges, list<WinInSession*>* session){
    
    //For all the effects which source is in the current source Folder, the path in the description file has to be changed with the new name
    
    list<pair<string, string> > sourceChanges;
    
    list<WinInSession*>::iterator it;
    
    list<std::pair<string,string> >::iterator it2;
    for(it2 = nameChanges.begin(); it2 != nameChanges.end() ; it2++){
        
        for(it = session->begin(); it != session->end() ; it++){
            
            if(QFileInfo((*it)->source.c_str()).absolutePath().toStdString().compare(currentSourcesFolder)== 0){
                
                string inter = it2->first;
                QFileInfo sourceInfo((*it)->source.c_str());
                string inter2 = sourceInfo.baseName().toStdString();
                
                if(inter2.compare(inter) == 0){
                    string inter = currentSourcesFolder + "/" + it2->second + ".dsp";
                    (*it)->source = inter;
                }
            }
            
        }
    }
}

void FaustLiveApp::copy_AllSources(string& srcDir, string& dstDir, list<std::pair<string,string> > nameChanges, string extension){
    
    //Following the renaming table, the sources are copied from snapshot Folder to current Session Folder
    
    QDir src(srcDir.c_str());
    
    QFileInfoList chilDirs = src.entryInfoList(QDir::Files);
    
    QFileInfoList::iterator it;
    for(it = chilDirs.begin(); it!=chilDirs.end(); it++){
        
        string name = it->baseName().toStdString();
        
        list<std::pair<string,string> >::iterator it2;
        
        QFile source(it->filePath());
        
        for(it2 = nameChanges.begin(); it2 != nameChanges.end() ; it2++){
            if(name.compare(it2->first) == 0)
                name = it2->second;
        }
        
        string newDir = dstDir + "/" + name + extension;
        
        //        printf("COPY OF = %s TO %s\n",it->filePath().toStdString().c_str(), newDir.c_str());
        
        source.copy(newDir.c_str());
    }
    
}

void FaustLiveApp::copy_WindowsFolders(string& srcDir, string& dstDir, list<std::pair<string,string> > windowNameChanges){
    
    //Following the renaming table, the Folders containing the parameters of the window are copied from snapshot Folder to current Session Folder
    
    //    printf("WINDOW FOLDER SIZE = %i\n", windowNameChanges.size());
    //    
    //    list< pair<string, string > >::iterator itw;
    //    for(itw = windowNameChanges.begin(); itw != windowNameChanges.end(); itw++){
    //        
    //        printf("WINDOW NAME CHANGES = %s TO %s\n", itw->first.c_str(), itw->second.c_str());
    //    }
    
    QDir src(srcDir.c_str());
    
    QFileInfoList chilDirs = src.entryInfoList(QDir::AllDirs);
    
    QFileInfoList::iterator it;
    
    for(it = chilDirs.begin(); it!=chilDirs.end(); it++){
        
        list<std::pair<string,string> >::iterator it2;
        
        string name = it->baseName().toStdString();
        
        for(it2 = windowNameChanges.begin(); it2 !=windowNameChanges.end() ; it2++){
            
            //            printf("comparing basename = %s TO MY %s\n", name.c_str(), it2->first.c_str());
            
            if(name.compare(it2->first) == 0){
                string newDir(dstDir.c_str());
                newDir += "/" + it2->second;
                
                QDir dir(newDir.c_str());
                dir.mkdir(newDir.c_str());
                
                string fileRC = it->filePath().toStdString() + "/" + it2->first + ".rc";
                string newFileRC = newDir + "/" + it2->second +".rc";
                QFile toCopy(fileRC.c_str());
                toCopy.copy(newFileRC.c_str());
                
                string fileJC = it->filePath().toStdString() + "/" + it2->first + ".jc";
                string newFileJC = newDir + "/"+ it2->second + ".jc";
                QFile toCpy(fileJC.c_str());
                toCpy.copy(newFileJC.c_str());
                
                //                printf("NEW FILE = %s\n", newFileJC.c_str());
            }
        }
    }
    
}

void FaustLiveApp::copy_SVGFolders(string& srcDir, string& dstDir, list<std::pair<string,string> > nameChanges){
    
    QDir src(srcDir.c_str());
    
    QFileInfoList chilDirs = src.entryInfoList(QDir::AllDirs);
    
    QFileInfoList::iterator it;
    
    for(it = chilDirs.begin(); it!=chilDirs.end(); it++){
        
        list<std::pair<string,string> >::iterator it2;
        
        string name = it->baseName().toStdString();
        
        for(it2 = nameChanges.begin(); it2 !=nameChanges.end() ; it2++){
            
            string toCompare = it2->first +"-svg";
            
            if(name.compare(toCompare) == 0){
                string newDir = dstDir + "/" +it2->second + "-svg";
                printf("COPYING = %s TO %s\n", it->absoluteFilePath().toStdString().c_str(), newDir.c_str());
                //                if(!QFileInfo(newDir.c_str()).exists())
                cpDir(it->absoluteFilePath(), newDir.c_str());
            }
            
            
        }
    }
    
}

//--------------------------------VIEW----------------------------------------

void FaustLiveApp::viewHttpd(FLWindow* win){
    
    char error[256];
    
    if(!win->init_Httpd(error))
        errorWindow->print_Error(error);
}

void FaustLiveApp::httpd_View_Window(){
    
    //Searching the active Window to show its QRcode
    FLWindow* win = getActiveWin();
    if(win != NULL)
        viewHttpd(win);
    else
        errorWindow->print_Error("No active Window");
}

void FaustLiveApp::viewSvg(FLWindow* win){
    
    string source = win->get_Effect()->getSource();
    string pathToOpen = currentSVGFolder + "/" + win->get_Effect()->getName() + "-svg/process.svg";
    
    string cmd = "open " + pathToOpen;
    string error = pathToOpen + " could not be opened!";
    
    if(system(cmd.c_str()))
        errorWindow->print_Error(error.c_str());
}

void FaustLiveApp::svg_View_Action(){
    
    //Searching the active Window to show its SVG Diagramm
    FLWindow* win = getActiveWin();
    if(win != NULL)
        viewSvg(win);
    
}

//--------------------------------HELP----------------------------------------
void FaustLiveApp::init_HelpWindow(){
    
    QTabWidget *MyTabWidget;
    QWidget *tab_1;
    QWidget *tab_2;
    QWidget *tab_3;
    QWidget *tab_4;
    
    QGroupBox*  groupBox;
    QGroupBox *groupBox_1;
    QPlainTextEdit *plainTextEdit_7;
    QPlainTextEdit *lineEdit;
    
    QGroupBox *groupBox_2;
    QTextBrowser *plainTextEdit_4;
    QTextBrowser *plainTextEdit_5;
    QTextBrowser *plainTextEdit_6;
    QTextBrowser *plainTextEdit_8;
    QTextBrowser *plainTextEdit_14;
    
    QGroupBox *groupBox_3;
    QPlainTextEdit *plainTextEdit_9;
    QPlainTextEdit *plainTextEdit_10;
    QPlainTextEdit *plainTextEdit_11;
    QPlainTextEdit *plainTextEdit_17;
    QPlainTextEdit *plainTextEdit_13;
    
    QGroupBox *groupBox_4;
    QPlainTextEdit *plainTextEdit;
    QPlainTextEdit *plainTextEdit_2;
    QPlainTextEdit *plainTextEdit_3;
    QPlainTextEdit *plainTextEdit_12;
    QPlainTextEdit *plainTextEdit_15;
    QPlainTextEdit *plainTextEdit_16;
    QPushButton *pushButton;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    
    QVBoxLayout* layout = new QVBoxLayout;
    QVBoxLayout* layout1 = new QVBoxLayout;
    QVBoxLayout* layout2 = new QVBoxLayout;
    QVBoxLayout* layout3 = new QVBoxLayout;
    QVBoxLayout* layout4 =  new QVBoxLayout;
    QVBoxLayout* layout5 = new QVBoxLayout;
    QVBoxLayout* layout6 =  new QVBoxLayout;
    QVBoxLayout* layout7 = new QVBoxLayout;
    QVBoxLayout* layout8 =  new QVBoxLayout;
    
    //----------------------Global
    
    HelpWindow->setWindowTitle(QString::fromUtf8("FAUST LIVE HELP"));
    
    groupBox = new QGroupBox(HelpWindow);
    
    MyTabWidget = new QTabWidget(groupBox);
    layout->addWidget(MyTabWidget);
    
    QMetaObject::connectSlotsByName(HelpWindow);
    
    HelpWindow->setGeometry(QRect(screenWidth/4, 0, screenWidth/2, screenHeight*3/4));
    
    pushButton = new QPushButton("OK", HelpWindow);
    pushButton->connect(pushButton, SIGNAL(clicked()), this, SLOT(end_apropos()));
    
    //---------------------General
    
    tab_1 = new QWidget();
    groupBox_1 = new QGroupBox(tab_1);
    groupBox_1->setGeometry(QRect(0,0, 100, 100));
    
    plainTextEdit_7 = new QPlainTextEdit(tr("\nFaustLive is a dynamical compiler for processors coded with Faust language.\nThanks to its embedded Faust & LLVM compiler, this application allows dynamical compilation of your faust objects.\n""\n""Every window of the application corresponds to an audio application, which parameters you can adjust, that you can auditorely connect with Jack to other audio applications!\n"),groupBox_1);
    
    lineEdit = new QPlainTextEdit(tr("\n\n\n\nDISTRIBUTED by GRAME - Centre de Creation Musicale"),groupBox_1);
    
    MyTabWidget->addTab(tab_1, QString());
    MyTabWidget->setTabText(MyTabWidget->indexOf(tab_1), QApplication::translate("HelpMenu", "General", 0, QApplication::UnicodeUTF8));
    
    MyTabWidget->setMaximumSize(screenWidth/2, screenHeight*3/4);
    
    plainTextEdit_7->setReadOnly(true);
    lineEdit->setReadOnly(true);
    
    QLabel* tittle = new QLabel(tr("FAUST LIVE"));
    //    tittle->setStyleSheet("*{color:white;}");
    tittle->setAlignment(Qt::AlignCenter);
    layout1->addWidget(tittle);
    layout1->addWidget(plainTextEdit_7);
    layout1->addWidget(new QLabel(tr("")));
    layout1->addWidget(lineEdit);
    
    groupBox_1->setLayout(layout1);
    
    layout2->addWidget(groupBox_1);
    tab_1->setLayout(layout2);
    
    //----------------------Tools
    
    tab_2 = new QWidget();
    groupBox_2 = new QGroupBox(tab_2);
    groupBox_2->setGeometry(QRect(0,0, 100, 100));
    
    QString sheet = QString::fromLatin1("a{ text-decoration: underline; color: white;}");
    
    plainTextEdit_4 = new QTextBrowser(groupBox_2);
    plainTextEdit_4->setOpenExternalLinks(true);
    plainTextEdit_4->document()->setDefaultStyleSheet(sheet);
    plainTextEdit_5 = new QTextBrowser(groupBox_2);
    plainTextEdit_5->setOpenExternalLinks(true);
    plainTextEdit_5->document()->setDefaultStyleSheet(sheet);
    plainTextEdit_6 = new QTextBrowser(groupBox_2);
    plainTextEdit_6->setOpenExternalLinks(true);
    plainTextEdit_6->document()->setDefaultStyleSheet(sheet);
    plainTextEdit_8 = new QTextBrowser(groupBox_2);
    plainTextEdit_8->setOpenExternalLinks(true); 
    plainTextEdit_8->document()->setDefaultStyleSheet(sheet);
    plainTextEdit_14 = new QTextBrowser(groupBox_2);
    plainTextEdit_14->setOpenExternalLinks(true); 
    plainTextEdit_14->document()->setDefaultStyleSheet(sheet);
    MyTabWidget->addTab(tab_2, QString());
    
    plainTextEdit_4->setHtml("<br>Jack (the Jack Audio Connection Kit) is a low-latency audio server. It can connect any number of different applications to a single hardware audio device.""<br>""YOU CAN DOWNLOAD IT HERE : <a href =http://www.jackosx.com> www.jackosx.com</a>\n");
    plainTextEdit_5->setHtml("<br>To develop your own effects, you will need to handle Faust language.\n""<br>""LEARN MORE ABOUT FAUST : <a href = http://faust.grame.fr>faust.grame.fr</a>");
    plainTextEdit_6->setHtml("<br>Thanks to its embedded LLVM compiler, this application allows dynamical compilation of your faust objects.""<br>""LEARN MORE ABOUT LLVM : <a href = http://llvm.org>llvm.org</a>");
    plainTextEdit_8->setHtml("<br>Core Audio is the digital audio infrastructure of MAC OS X.""<br>""LEARN MORE ABOUT COREAUDIO : <a href = http://developer.apple.com/library/ios/#documentation/MusicAudio/Conceptual/CoreAudioOverview/WhatisCoreAudio/WhatisCoreAudio.html>developer.apple.com </a> ");
    plainTextEdit_14->setHtml("<br>NetJack (fully integrated in Jack) is a Realtime Audio Transport over a generic IP Network. That way you can send your audio signals through the network to a server.""<br>""LEARN MORE ABOUT NETJACK : <a href = http://netjack.sourceforge.net> netjack.sourceforge.net</a>\n");
    
    MyTabWidget->setTabText(MyTabWidget->indexOf(tab_2), QApplication::translate("HelpMenu", "Tools", 0, QApplication::UnicodeUTF8));
    
    QLabel* tittle20 = new QLabel(tr("COREAUDIO"));
    //    tittle20->setStyleSheet("*{color:white;}");
    tittle20->setAlignment(Qt::AlignCenter);
    QLabel* tittle2 = new QLabel(tr("JACK"));
    //    tittle2->setStyleSheet("*{color:white;}");
    tittle2->setAlignment(Qt::AlignCenter);
    QLabel* tittle30 = new QLabel(tr("NETJACK"));
    //    tittle30->setStyleSheet("*{color:white;}");
    tittle30->setAlignment(Qt::AlignCenter);
    QLabel* tittle3 = new QLabel(tr("FAUST"));
    //    tittle3->setStyleSheet("*{color:white;}");
    tittle3->setAlignment(Qt::AlignCenter);
    QLabel* tittle4 = new QLabel(tr("LLVM COMPILER"));
    //    tittle4->setStyleSheet("*{color:white;}");
    tittle4->setAlignment(Qt::AlignCenter);
    
    layout3->addWidget(tittle20);
    layout3->addWidget(plainTextEdit_8);
    layout3->addWidget(tittle2);
    layout3->addWidget(plainTextEdit_4);
    layout3->addWidget(tittle30);
    layout3->addWidget(plainTextEdit_14);
    layout3->addWidget(tittle3);
    layout3->addWidget(plainTextEdit_5);
    layout3->addWidget(tittle4);
    layout3->addWidget(plainTextEdit_6);
    
    groupBox_2->setLayout(layout3);
    
    layout4->addWidget(groupBox_2);
    tab_2->setLayout(layout4);
    
    //-----------------------Window Properties
    
    tab_3 = new QWidget();
    groupBox_3 = new QGroupBox(tab_3);
    groupBox_3->setGeometry(QRect(0,0, 100, 100));
    
    plainTextEdit_9 = new QPlainTextEdit(groupBox_3);
    plainTextEdit_9->setReadOnly(true);
    plainTextEdit_10 = new QPlainTextEdit(groupBox_3);
    plainTextEdit_10->setReadOnly(true);
    plainTextEdit_11 = new QPlainTextEdit(groupBox_3);
    plainTextEdit_11->setReadOnly(true);
    plainTextEdit_17 = new QPlainTextEdit(groupBox_3);
    plainTextEdit_17->setReadOnly(true);
    plainTextEdit_13 = new QPlainTextEdit(groupBox_3);
    plainTextEdit_13->setReadOnly(true);
    MyTabWidget->addTab(tab_3, QString());
    
    plainTextEdit_9->setPlainText(QApplication::translate("HelpMenu", "\nWith Jack router, you can connect a window to another one or to an external application like I-Tunes, VLC or directly to the computer input/output.\nYou can choose Jack as the audio architecture in the preferences.", 0, QApplication::UnicodeUTF8));
    plainTextEdit_10->setPlainText(QApplication::translate("HelpMenu", "\nYou can open in a text editor the code Faust corresponding to the active window. When you save your modification, the window(s) corresponding to this source will be updated. The graphical parameters and the audio connexions that can be kept will stay unmodified.", 0, QApplication::UnicodeUTF8));
    plainTextEdit_11->setPlainText(QApplication::translate("HelpMenu", "\nThose actions correspond to a modification of the audio application running in the active window. An audio crossfade will be calculated between the outcoming and the incoming audio application. The new application will be connected as the outcoming one. This way, if the incoming application has more ports than the outcoming one, they'll have to be connected by the user.\n""It is possible to drop/paste a source file.dsp, some Faust code, a url leading to Faust code.", 0, QApplication::UnicodeUTF8));
    plainTextEdit_17->setPlainText(QApplication::translate("HelpMenu", "\nYou can add compilation options in the edit line of the window. If several windows correspond to the same audio application, they will load the chosen options.\n""You can set the default compilation options in the preferences.\n""	EXAMPLE = -vec -fun", 0, QApplication::UnicodeUTF8));
    plainTextEdit_13->setPlainText(QApplication::translate("HelpMenu", "\nYou can display a new window with a QRcode so that you can control the userInterface of the audio application remotely.", 0, QApplication::UnicodeUTF8));
    MyTabWidget->setTabText(MyTabWidget->indexOf(tab_3), QApplication::translate("HelpMenu", "Properties of a Window", 0, QApplication::UnicodeUTF8));
    
    
    QLabel* tittle5 = new QLabel(tr("AUDIO CONNECTIONS/DECONNECTIONS"));
    //    tittle5->setStyleSheet("*{color:white;}");
    tittle5->setAlignment(Qt::AlignCenter);
    QLabel* tittle6 = new QLabel(tr("EDIT FAUST SOURCE"));
    //    tittle6->setStyleSheet("*{color:white;}");
    tittle6->setAlignment(Qt::AlignCenter);
    QLabel* tittle7 = new QLabel(tr("DRAG & DROP/PASTE"));
    //    tittle7->setStyleSheet("*{color:white;}");
    tittle7->setAlignment(Qt::AlignCenter);
    QLabel* tittle8 = new QLabel(tr("VIEW QRCODE"));
    //    tittle8->setStyleSheet("*{color:white;}");
    tittle8->setAlignment(Qt::AlignCenter);
    QLabel* tittle9 = new QLabel(tr("COMPILATION OPTIONS"));
    //    tittle9->setStyleSheet("*{color:white;}");
    tittle9->setAlignment(Qt::AlignCenter);
    
    layout5->addWidget(tittle5);
    layout5->addWidget(plainTextEdit_9);
    layout5->addWidget(tittle6);
    layout5->addWidget(plainTextEdit_10);
    layout5->addWidget(tittle7);
    layout5->addWidget(plainTextEdit_11);
    layout5->addWidget(tittle9); 
    layout5->addWidget(plainTextEdit_17);
    layout5->addWidget(tittle8);
    layout5->addWidget(plainTextEdit_13);
    
    groupBox_3->setLayout(layout5);
    
    layout6->addWidget(groupBox_3);
    tab_3->setLayout(layout6);
    
    //-----------------------Faust Live Menu
    
    tab_4 = new QWidget();
    groupBox_4 = new QGroupBox(tab_4);
    groupBox_4->setGeometry(QRect(0,0, 100, 100));
    
    plainTextEdit = new QPlainTextEdit(groupBox_4);
    plainTextEdit->setReadOnly(true);
    plainTextEdit_2 = new QPlainTextEdit(groupBox_4);
    plainTextEdit_2->setReadOnly(true);
    plainTextEdit_3 = new QPlainTextEdit(groupBox_4);
    plainTextEdit_3->setReadOnly(true);
    plainTextEdit_12 = new QPlainTextEdit(groupBox_4);
    plainTextEdit_12->setReadOnly(true);
    MyTabWidget->addTab(tab_4, QString());
    
    plainTextEdit->setPlainText(QApplication::translate("HelpMenu", "\nOpen a new empty Window  ==> you can create a new effect from zero\n""\n""Open a dsp already coded ==> You can test it and keep editing a DSP", 0, QApplication::UnicodeUTF8));
    plainTextEdit_2->setPlainText(QApplication::translate("HelpMenu", "\nDuplicate Active Window ==> The new window has the same characteristics : same Faust code, same graphical parameters, same audio connections.\n", 0, QApplication::UnicodeUTF8));
    plainTextEdit_3->setPlainText(QApplication::translate("HelpMenu", "\nTake a snapshot ==> Saving the actual state of the application (all the windows, their graphical parameters, their audio connections, their position on the screen).\n""\n""Recall Session ==> Opening of a snapshot and closing every window opened. All the windows of the snapshot are re-opened with their graphical parameters and the connections are re-established. If one of the source file can't be found, the user is warned and the application is re-open from the saved file corresponding\n""\n""Import Snapshot ==> The same principle than recalling except that the current windows are not closed. That way, some audio application/windows may have to be renamed during the importation.", 0, QApplication::UnicodeUTF8));
    plainTextEdit_12->setPlainText(QApplication::translate("HelpMenu", "\nA window will be displayed every time the program catches an error : whether it's a error in the edited code, a compilation problem, a lack of memory during saving action, ...", 0, QApplication::UnicodeUTF8));
    MyTabWidget->setTabText(MyTabWidget->indexOf(tab_4), QApplication::translate("HelpMenu", "Use FaustLive Menu", 0, QApplication::UnicodeUTF8));
    
    QLabel* tittle10 = new QLabel(tr("NEW/OPEN WINDOW"));
    //    tittle10->setStyleSheet("*{color:white;}");
    tittle10->setAlignment(Qt::AlignCenter);
    QLabel* tittle11 = new QLabel(tr("DUPLICATE WINDOW"));
    //    tittle11->setStyleSheet("*{color:white;}");
    tittle11->setAlignment(Qt::AlignCenter);
    QLabel* tittle12 = new QLabel(tr("SNAPSHOTS"));
    //    tittle12->setStyleSheet("*{color:white;}");
    tittle12->setAlignment(Qt::AlignCenter);
    QLabel* tittle13 = new QLabel(tr("ERROR WINDOW"));
    //    tittle13->setStyleSheet("*{color:white;}");
    tittle13->setAlignment(Qt::AlignCenter);
    
    layout7->addWidget(tittle10);
    layout7->addWidget(plainTextEdit);
    layout7->addWidget(tittle11);
    layout7->addWidget(plainTextEdit_2);
    layout7->addWidget(tittle12);
    layout7->addWidget(plainTextEdit_3);
    layout7->addWidget(tittle13);
    layout7->addWidget(plainTextEdit_12);    
    groupBox_4->setLayout(layout7);
    
    layout8->addWidget(groupBox_4);
    tab_4->setLayout(layout8);
    
    layout->addWidget(pushButton);
    layout->setSizeConstraint(QLayout::SetNoConstraint);
    groupBox->setLayout(layout);
    groupBox->setGeometry(QRect(screenWidth/4, 0, screenWidth/4, screenHeight/4));
    HelpWindow->setCentralWidget(groupBox);
    
    MyTabWidget->setCurrentIndex(0);
}

void FaustLiveApp::apropos(){
    HelpWindow->show();
}

void FaustLiveApp::end_apropos(){
    HelpWindow->hide(); 
}

void FaustLiveApp::version_Action(){
    
    QVBoxLayout* layoutGeneral = new QVBoxLayout;
    
    string text = "This application is using ""\n""- Jack 2";
//    text += jack_get_version_string();
    text += "\n""- NetJack ";
    text += "2.1";
    text += "\n""- CoreAudio API ";
    text += "4.0";
    text += "\n""- LLVM Compiler ";
    text += "3.1";
    
    QPlainTextEdit* versionText = new QPlainTextEdit(tr(text.c_str()), versionWindow);
    
    layoutGeneral->addWidget(versionText);
    versionWindow->setLayout(layoutGeneral);
    
    versionWindow->exec();
    
    delete versionText;
    delete layoutGeneral;
}

//-------------------------------PRESENTATION WINDOW-----------------------------

void FaustLiveApp::itemClick(QListWidgetItem *item){
    exampleToOpen = item->text().toStdString();
}

void FaustLiveApp::itemDblClick(QListWidgetItem* item){
    exampleToOpen = item->text().toStdString();
    open_Example_Action();
}

void FaustLiveApp::init_presentationWindow(){
    
    string ImagesPath = QFileInfo(QFileInfo( QCoreApplication::applicationFilePath()).absolutePath()).absolutePath().toStdString() + "/Resources/Images/";
    
    QGroupBox*      iconeBox = new QGroupBox;
    QGroupBox*      buttonBox = new QGroupBox;
    QGroupBox*      gridBox = new QGroupBox;
    QGroupBox*      textBox = new QGroupBox;
    QGroupBox*      openExamples = new QGroupBox;
    
    QPushButton*    new_Window;
    QPushButton*    open_Window;
    QPushButton*    open_Session;
    QPushButton*    help;
    
    QPushButton*    ok;
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(iconeBox);
    mainLayout->addWidget(gridBox);
    mainLayout->addWidget(textBox);
    
    presWin->setLayout(mainLayout);
    
    //------------------------ICON
    
    QHBoxLayout *layout = new QHBoxLayout;
    
    QLabel *image = new QLabel();
    //Path of the executable + the presentation Image
    string pathImage = ImagesPath + "Presentation_Image.png";
    QPixmap presImg(pathImage.c_str());
    presImg.scaledToWidth(100, Qt::SmoothTransformation);
    presImg.scaledToHeight(100, Qt::SmoothTransformation);
    image->setPixmap(presImg);
    image->setAlignment(Qt::AlignCenter);
    layout->addWidget(image);
    iconeBox->setLayout(layout);
    
    QLabel* text = new QLabel("<h2>WELCOME TO FAUST LIVE</h2>\nDynamical Faust Compiler");  
    text->setAlignment(Qt::AlignCenter);
    text->setStyleSheet("*{color: black}");
    
    QLabel* subTitle = new QLabel("");
    
    layout->addWidget(text);
    layout->addWidget(subTitle);
    
    QHBoxLayout* layout3 = new QHBoxLayout;
    QGridLayout* layout2 = new QGridLayout;
    
    
    //------------------------GRID BUTTONS
    
    QLabel *new_Image = new QLabel(gridBox);
    //Path of the executable + the presentation Image
    string pathNew = ImagesPath + "InitWin.png";
    QPixmap newPix(pathNew.c_str());
    newPix = newPix.scaledToWidth(60, Qt::SmoothTransformation);
    new_Image->setPixmap(newPix);
    layout2->addWidget(new_Image, 0, 0);
    
    new_Window = new QPushButton("New Default Window\nStart coding a Faust Application from a default process.\nOr drop a dsp on the init window.");
    new_Window->setToolTip("Open a window containing a default process.");
    new_Window->setFlat(true);
    new_Window->setDefault(false);
    
    layout2->addWidget(new_Window, 0, 1);
    connect(new_Window, SIGNAL(clicked()), this, SLOT(new_Window_pres()));
    
    QLabel *open_Image = new QLabel(gridBox);
    string pathOpen = ImagesPath + "OpenWin.png";
    QPixmap openPix(pathOpen.c_str());
    openPix = openPix.scaledToWidth(60, Qt::SmoothTransformation);
    open_Image->setPixmap(openPix);
    layout2->addWidget(open_Image, 1, 0);
    
    open_Window = new QPushButton("Open Your Effect\nIf you already coded a DSP, you can open it directly.");
    open_Window->setToolTip("Open the DSP you choose.");
    open_Window->setFlat(true);
    open_Window->setDefault(false);
    
    layout2->addWidget(open_Window, 1, 1);
    connect(open_Window, SIGNAL(clicked()), this, SLOT(open_Window_pres()));
    
    QLabel *snap_Image = new QLabel(gridBox);
    string pathSnap = ImagesPath + "RecallMenu.png";
    QPixmap snapPix(pathSnap.c_str());
    snapPix = snapPix.scaledToWidth(60, Qt::SmoothTransformation);
    snap_Image->setPixmap(snapPix);
    layout2->addWidget(snap_Image, 2, 0);
    
    open_Session = new QPushButton("Recall Snapshot\nIf you already took a snapshot of your application,\n you can open it directly.");
    open_Session->setToolTip("Open a saved snapshot.");
    open_Session->setFlat(true);
    open_Session->setDefault(false);
    
    layout2->addWidget(open_Session, 2, 1);
    connect(open_Session, SIGNAL(clicked()), this, SLOT(open_Session_pres()));
    
    QLabel *help_Image = new QLabel(gridBox);
    string pathHelp = ImagesPath + "HelpMenu.png";
    QPixmap helpPix(pathHelp.c_str());
    helpPix = helpPix.scaledToWidth(60, Qt::SmoothTransformation);
    help_Image->setPixmap(helpPix);
    layout2->addWidget(help_Image, 3, 0);
    
    help = new QPushButton("About Faust Live\n Learn all about FaustLive charateristics.");
    help->setToolTip("Help Menu.");
    
    layout2->addWidget(help, 3, 1);
    connect(help, SIGNAL(clicked()), this, SLOT(apropos()));
    help->setFlat(true);
    help->setDefault(false);
    
    
    //-------------------------OPEN EXAMPLES
    
    
    QVBoxLayout *layout5 = new QVBoxLayout;
    
    QListWidget *vue = new QListWidget(openExamples);
    
    QString examplesPath = QFileInfo(QFileInfo( QCoreApplication::applicationFilePath()).absolutePath()).absolutePath();
    examplesPath += "/Resources/Examples";
    
    QDir examplesDir(examplesPath);
    
    QFileInfoList children = examplesDir.entryInfoList(QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
    
    exampleToOpen = (children.begin())->baseName().toStdString();
    
    QFileInfoList::iterator it;
    
    for(it = children.begin(); it != children.end(); it++)
        vue->addItem(QString(it->baseName()));
    
    connect(vue, SIGNAL(itemDoubleClicked( QListWidgetItem *)), this, SLOT(itemDblClick(QListWidgetItem *)));
    connect(vue, SIGNAL(itemClicked( QListWidgetItem *)), this, SLOT(itemClick(QListWidgetItem *)));
    
    
    layout5->addWidget(new QLabel(tr("<h2>Try Out an Example</h2>")));
    layout5->addWidget(vue);
    
    ok = new QPushButton("Open");
    connect(ok, SIGNAL(clicked()), this, SLOT(open_Example_Action()));
    ok->setDefault(true);
    ok->setStyleSheet("*{background-color: transparent;}");
    layout5->addWidget(ok);
    
    buttonBox->setLayout(layout2);
    openExamples->setLayout(layout5);
    
    layout3->addWidget(buttonBox);
    layout3->addWidget(openExamples); 
    
    gridBox->setLayout(layout3);
    
    QHBoxLayout *layout4 = new QHBoxLayout;
    
    QLabel* lab = new QLabel("");
    
    QPushButton* cancel = new QPushButton("Cancel");
    cancel->setStyleSheet("*{background-color: transparent;}");
    connect(cancel, SIGNAL(clicked()), this, SLOT(hide_presentationWin()));
    layout4->addWidget(new QLabel(""));
    layout4->addWidget(cancel);
    layout4->addWidget(new QLabel(""));    
    QLabel* more = new QLabel("<p>by GRAME, Centre de Creation Musicale");  
    more->setAlignment(Qt::AlignRight);
    
    layout4->addWidget(more);
    textBox->setLayout(layout4);
    
    
    new_Window->setStyleSheet("QPushButton:flat{"
                              "background-color: lightGray;"
                              "color: black;"
                              "border: 2px solid gray;"
                              "border-radius: 6px;"
                              "}"
                              "QPushButton:flat:hover{"
                              "background-color: darkGray;"                         
                              "}" );
    open_Window->setStyleSheet("QPushButton:flat{"
                               "background-color: lightGray;"
                               "color: black;"
                               "border: 2px solid gray;"
                               "border-radius: 6px;"
                               "}"
                               "QPushButton:flat:hover{"
                               "background-color: darkGray;"                         
                               "}" );
    open_Session->setStyleSheet("QPushButton:flat{"
                                "background-color: lightGray;"
                                "color: black;"
                                "border: 2px solid gray;"
                                "border-radius: 6px;"
                                "}"
                                "QPushButton:flat:hover{"
                                "background-color: darkGray;"                         
                                "}" );
    
    help->setStyleSheet("QPushButton:flat{"
                        "background-color: lightGray;"
                        "color: black;"
                        "border: 2px solid gray;"
                        "border-radius: 6px;"
                        "}"
                        "QPushButton:flat:hover{"
                        "background-color: darkGray;"                         
                        "}" );
}

void FaustLiveApp::new_Window_pres(){
    presWin->hide();
    //    reset_CurrentSession();
    create_Empty_Window();
}

void FaustLiveApp::open_Window_pres(){
    presWin->hide();   
    //    reset_CurrentSession();
    open_New_Window();
}

void FaustLiveApp::open_Session_pres(){
    presWin->hide();
    //    reset_CurrentSession();
    recallSnapshotFromMenu();
}

void FaustLiveApp::show_presentation_Action(){
    
    presWin = new QDialog;
    presWin->setWindowFlags(Qt::FramelessWindowHint);
    init_presentationWindow();
    
    presWin->show();
    presWin->raise();
}

void FaustLiveApp::hide_presentationWin(){
    presWin->hide();
}

//--------------------------------PREFERENCES---------------------------------------

void FaustLiveApp::linkClicked(const QUrl& link){
    
    string myLink = link.path().toStdString();
    
    int pos = 0;
    pos = myLink.find("\\");
    
    while(pos != string::npos){
        myLink.insert(pos + 1, 1, ' ');
        pos = myLink.find("\\", pos+2);
    }
    
    string myCmd = "open -a " + myLink;
    system(myCmd.c_str());
    
    splRate->reload();
}

void FaustLiveApp::styleClicked(){
    
    QPushButton* item = (QPushButton*)QObject::sender();
    styleClicked(item->text().toStdString());
}

void FaustLiveApp::styleClicked(string style){
    
    if(style.compare("Default") == 0){
        
        styleChoice = "Default";
        
        setStyleSheet(
                      
                      // BUTTONS
                      "QPushButton{"
                      //                           "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
                      //                           "stop: 0 #E67E30, stop : 1 #AD4F09);"
                      "min-width : 80px;"
                      "border: 2px solid grey;"
                      "border-radius: 6px;"
                      "margin-top: 1ex;"
                      "border-color: #811453;"
                      "}"
                      
                      "QPushButton:hover{"
                      "border: 2px #811453;"
                      "}"
                      
                      "QPushButton:pressed{"
                      "background-color: #6A455D;"
                      "border-color: #811453;"
                      "}"
                      
                      // GROUPS
                      "QGroupBox{"
                      "subcontrol: .QGroupBox"
                      "border: 2px solid gray;"
                      "border-radius: 5px;"
                      "margin-top: 3ex;"
                      "font-size:10pt;"
                      "font-weight:bold;"
                      "color: black;"
                      "}"
                      
                      "QGroupBox::title {"
                      "subcontrol-origin: margin;"
                      "subcontrol-position: top center;"
                      "padding: 0 5px;"
                      "color: black;"
                      "}"
                      
                      // MAINWINDOW
                      "QMainWindow {"
                      "margin-top: 3ex;"
                      "font-size:10pt;"
                      "font-weight:bold;"
                      "color: black;"
                      "}"
                      
                      // PLAINTEXTEDIT
                      "QPlainTextEdit, QTextEdit{"
                      "background-color: transparent;"
                      "border: 2px solid gray;"
                      "border-radius: 5px;"
                      "top: 3px;"
                      "margin-top: 3ex;"
                      "font-size:12pt;"
                      "font-weight:bold;"
                      "color: black;"
                      "}"
                      
                      //TextBrowser
                      "QTextBrowser {"
                      "color: black;"
                      "}"
                      "QTextBrowser:document{"
                      "text-decoration: underline;"
                      "color: black;"
                      "font: Menlo;"
                      "font-size: 14px"
                      "}"
                      
                      //LABEL
                      "QLabel{"
                      "color : black;"
                      "}"
                      
                      // SLIDERS
                      // horizontal sliders
                      "QSlider::groove:vertical {"
                      "background: red;"
                      "position: absolute;" /* absolutely position 4px from the left and right of the widget. setting margins on the widget should work too... */
                      "left: 13px; right: 13px;"
                      "}"
                      
                      "QSlider::handle:vertical {"
                      "height: 40px;"
                      "width: 30px;"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #E67E30, stop : 0.05 #AD4F09, stop: 0.3 #E67E30, stop : 0.90 #AD4F09, stop: 0.91 #AD4F09);"
                      "margin: 0 -5px; /* expand outside the groove */"
                      "border-radius: 5px;"
                      "}"
                      
                      "QSlider::add-page:vertical {"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                      "stop: 0 #6A455D, stop : 0.5 #6A455D);"
                      "}"
                      
                      "QSlider::sub-page:vertical {"
                      "background: grey;"
                      "}"
                      
                      // horizontal sliders
                      
                      "QSlider::groove:horizontal {"
                      "background: red;"
                      "position: absolute;" /* absolutely position 4px from the left and right of the widget. setting margins on the widget should work too... */
                      "top: 14px; bottom: 14px;"
                      "}"
                      
                      "QSlider::handle:horizontal {"
                      "width: 40px;"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                      "stop: 0 #E67E30, stop : 0.05 #AD4F09, stop: 0.3 #E67E30, stop : 0.90 #AD4F09, stop: 0.91 #AD4F09);"
                      "margin: -5px 0; /* expand outside the groove */"
                      "border-radius: 5px;"
                      "}"
                      
                      "QSlider::sub-page:horizontal {"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #6A455D, stop : 0.5 #6A455D);"
                      "}"
                      
                      "QSlider::add-page:horizontal {"
                      "background: grey;"
                      "}"
                      );
    }
    
    if(style.compare("Blue") == 0){    
        
        styleChoice = "Blue";
        setStyleSheet(
                      
                      // BUTTONS
                      "QPushButton{"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
                      "stop: 0 #811453, stop: 1 #702963);"
                      "min-width : 80px;"
                      "border: 2px solid grey;"
                      "border-radius: 6px;"
                      "margin-top: 1ex;"
                      "}"
                      
                      "QPushButton:hover{"
                      "border: 2px solid orange;"
                      "}"
                      
                      "QPushButton:pressed{"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #811453, stop: 1 #702963);"
                      "}"
                      
                      // GROUPS
                      "QGroupBox{"
                      //Subcontrol differenciates the instances of QGroupBox and the subclasses of QGroupBox
                      "subcontrol: .QGroupBox"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
                      "stop: 0 #003366, stop: 1 #22427C);"
                      "border: 2px solid gray;"
                      "border-radius: 5px;"
                      "margin-top: 3ex;"
                      "font-size:10pt;"
                      "font-weight:bold;"
                      "color: dark grey;"
                      "color: white;"
                      "}"
                      
                      "QGroupBox::title {"
                      "subcontrol-origin: margin;"
                      "subcontrol-position: top center;"
                      "padding: 0 5px;"
                      "color: white;"
                      "}"
                      
                      // MAINWINDOW
                      "QMainWindow {"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
                      "stop: 0 #003366, stop: 1 #22427C);"
                      "border: 2px solid gray;"
                      "border-radius: 5px;"
                      "margin-top: 3ex;"
                      "font-size:10pt;"
                      "font-weight:bold;"
                      "color: dark grey;"
                      "color: white;"
                      "}"
                      
                      // PLAINTEXTEDIT
                      "QPlainTextEdit, QTextEdit{"
                      "background-color: transparent;"
                      "border: 2px #702963;"
                      "border-radius: 5px;"
                      "top: 3px;"
                      "margin-top: 3ex;"
                      "font-size:12pt;"
                      "font-weight:bold;"
                      "color: white;"
                      "}"
                      
                      
                      "QLabel{"
                      "color : white;"
                      "}"
                      //TextBrowser
                      "QTextBrowser {"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
                      "stop: 0 #003366, stop: 1 #22427C);"
                      "min-height:30px;"
                      "color: white;"
                      "}"
                      
                      "QTextBrowser:link{"
                      "text-decoration: underline;"
                      "color: white;"
                      "font: Menlo;"
                      "font-size: 14px"
                      "}"
                      
                      
                      "QString:link {"
                      "color: rgb(215, 208, 100);"
                      "}"
                      "QString:visited {"
                      "color: rgb(215, 208, 100);"
                      "}"
                      
                      // SLIDERS
                      // horizontal sliders
                      "QSlider::groove:vertical {"
                      "background: red;"
                      "position: absolute;" /* absolutely position 4px from the left and right of the widget. setting margins on the widget should work too... */
                      "left: 13px; right: 13px;"
                      "}"
                      
                      "QSlider::handle:vertical {"
                      "height: 40px;"
                      "width: 30px;"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #AAAAAA, stop : 0.05 #0A0A0A, stop: 0.3 #101010, stop : 0.90 #AAAAAA, stop: 0.91 #000000);"
                      "margin: 0 -5px; /* expand outside the groove */"
                      "border-radius: 5px;"
                      "}"
                      
                      "QSlider::add-page:vertical {"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                      "stop: 0 yellow, stop : 0.5 orange);"
                      "}"
                      
                      "QSlider::sub-page:vertical {"
                      "background: grey;"
                      "}"
                      
                      // horizontal sliders
                      
                      "QSlider::groove:horizontal {"
                      "background: red;"
                      "position: absolute;" /* absolutely position 4px from the left and right of the widget. setting margins on the widget should work too... */
                      "top: 14px; bottom: 14px;"
                      "}"
                      
                      "QSlider::handle:horizontal {"
                      "width: 40px;"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                      "stop: 0 #6A455D, stop : 0.05 #811453, stop: 0.3 #811453, stop : 0.90 #6A455D, stop: 0.91 #702963);"
                      "margin: -5px 0; /* expand outside the groove */"
                      "border-radius: 5px;"
                      "}"
                      
                      "QSlider::sub-page:horizontal {"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 yellow, stop : 0.5 orange);"
                      "}"
                      
                      "QSlider::add-page:horizontal {"
                      "background: grey;"
                      "}"
                      
                      // TABS
                      //TabWidget and TabBar
                      "QTabWidget::pane {" /* The tab widget frame */
                      "color : white;"
                      "border-top: 2px #702963;"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #003366, stop: 1.0 #22427C);"
                      "}"
                      
                      "QTabWidget::tab-bar {"
                      "left: 5px;" /* move to the right by 5px */
                      "}"
                      
                      /* Style the tab using the tab sub-control. Note that
                       it reads QTabBar _not_ QTabWidget */
                      "QTabBar::tab {"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #003366, stop: 0.4 #22427C,"
                      "stop: 0.5 #003366, stop: 1.0 #22427C);"
                      "border: 2px solid #808080;"
                      "color : white;"
                      //"border-bottom-color: #C2C7CB;" /* same as the pane color */
                      "border-bottom-color: #702963;" /* same as the pane color */
                      "border-top-left-radius: 4px;"
                      "border-top-right-radius: 4px;"
                      "min-width: 8ex;"
                      "padding: 2px;"
                      "}"
                      
                      "QTabBar::tab:selected, QTabBar::tab:hover {"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #003366, stop: 0.4 #22427C,"
                      "stop: 0.5 #003366, stop: 1.0 #22427C);"
                      "color : white;"
                      "}"
                      
                      "QTabBar::tab:selected {"
                      "color : white;"
                      "border-color: #702963;"
                      "border-bottom-color: #22427C;" /* same as pane color */
                      "}"
                      
                      "QTabBar::tab:!selected {"
                      "    margin-top: 2px;" /* make non-selected tabs look smaller */
                      "}"
                      );
    }
    
    if(style.compare("Grey") == 0){
        
        styleChoice = "Grey";
        setStyleSheet(
                      
                      // BUTTONS
                      "QPushButton {"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
                      "stop: 0 #B0B0B0, stop: 1 #404040);"
                      "min-width : 80px;"
                      "border: 2px solid grey;"
                      "border-radius: 6px;"
                      "margin-top: 1ex;"
                      "}"
                      
                      "QPushButton:hover {"
                      "border: 2px solid orange;"
                      "}"
                      
                      "QPushButton:pressed {"
                      //"border: 1px solid orange;"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #404040, stop: 1 #B0B0B0);"
                      "}"
                      // GROUPS
                      "QGroupBox {"
                      "subcontrol: .QGroupBox"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
                      "stop: 0 #A0A0A0, stop: 1 #202020);"
                      "border: 2px solid gray;"
                      "border-radius: 5px;"
                      "margin-top: 3ex;"
                      "font-size:10pt;"
                      "font-weight:bold;"
                      "color: white;"
                      "}"
                      
                      "QGroupBox::title {"
                      "subcontrol-origin: margin;"
                      "subcontrol-position: top center;" /* position at the top center */
                      "padding: 0 5px;"
                      "color : white;"
                      "}"
                      "QMainWindow {"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
                      "stop: 0 #A0A0A0, stop: 1 #202020);"
                      "border: 2px solid gray;"
                      "border-radius: 5px;"
                      "margin-top: 3ex;"
                      "font-size:10pt;"
                      "font-weight:bold;"
                      "color: white;"
                      "}"    
                      
                      // PLAINTEXTEDIT
                      "QPlainTextEdit, QTextEdit{"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
                      "stop: 0 #A0A0A0, stop: 1 #202020);"
                      "border-color: yellow;"
                      "border-radius: 5px;"
                      "top: 3px;"
                      "margin-top: 3ex;"
                      "font-size:12pt;"
                      "font-weight:bold;"
                      "color: white;"
                      "}"
                      
                      
                      //TextBrowser
                      "QTextBrowser {"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
                      "stop: 0 #A0A0A0, stop: 1 #202020);"
                      //                           "border: 2px solid gray;"
                      //                           "border-radius: 5px;"
                      //                           "top: 3px;"
                      //                           "margin-top: 3ex;"
                      //                           "font-size:10pt;"
                      //                           "font-weight:bold;"
                      //                           "color: dark grey;"
                      "color: white;"
                      "}"
                      
                      "QTextDocument{"
                      "text-decoration: underline;"
                      "color: white;"
                      "font: Menlo;"
                      "font-size: 14px"
                      "}"
                      
                      // SLIDERS
                      // horizontal sliders
                      "QSlider::groove:vertical {"
                      "background: red;"
                      "position: absolute;" /* absolutely position 4px from the left and right of the widget. setting margins on the widget should work too... */
                      "left: 13px; right: 13px;"
                      "}"
                      
                      "QSlider::handle:vertical {"
                      "height: 40px;"
                      "width: 30px;"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #AAAAAA, stop : 0.05 #0A0A0A, stop: 0.3 #101010, stop : 0.90 #AAAAAA, stop: 0.91 #000000);"
                      "margin: 0 -5px; /* expand outside the groove */"
                      "border-radius: 5px;"
                      "}"
                      
                      "QSlider::add-page:vertical {"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                      "stop: 0 yellow, stop : 0.5 orange);"
                      "}"
                      
                      "QSlider::sub-page:vertical {"
                      "background: grey;"
                      "}"
                      
                      // horizontal sliders
                      
                      "QSlider::groove:horizontal {"
                      "background: red;"
                      "position: absolute;" /* absolutely position 4px from the left and right of the widget. setting margins on the widget should work too... */
                      "top: 14px; bottom: 14px;"
                      "}"
                      
                      "QSlider::handle:horizontal {"
                      "width: 40px;"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                      "stop: 0 #AAAAAA, stop : 0.05 #0A0A0A, stop: 0.3 #101010, stop : 0.90 #AAAAAA, stop: 0.91 #000000);"
                      "margin: -5px 0; /* expand outside the groove */"
                      "border-radius: 5px;"
                      "}"
                      
                      "QSlider::sub-page:horizontal {"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 yellow, stop : 0.5 orange);"
                      "}"
                      
                      "QSlider::add-page:horizontal {"
                      "background: grey;"
                      "}"
                      
                      // TABS
                      //TabWidget and TabBar
                      "QTabWidget::pane {" /* The tab widget frame */
                      //"border-top: 2px solid #C2C7CB;"
                      "border-top: 2px solid orange;"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #A0A0A0, stop: 1 #202020);"
                      "}"
                      
                      "QTabWidget::tab-bar {"
                      "left: 5px;" /* move to the right by 5px */
                      "}"
                      
                      /* Style the tab using the tab sub-control. Note that
                       it reads QTabBar _not_ QTabWidget */
                      "QTabBar::tab {"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #909090, stop: 0.4 #888888,"
                      "stop: 0.5 #808080, stop: 1.0 #909090);"
                      "border: 2px solid #808080;"
                      //"border-bottom-color: #C2C7CB;" /* same as the pane color */
                      "border-bottom-color: orange;" /* same as the pane color */
                      "border-top-left-radius: 4px;"
                      "border-top-right-radius: 4px;"
                      "min-width: 8ex;"
                      "padding: 2px;"
                      "}"
                      
                      "QTabBar::tab:selected, QTabBar::tab:hover {"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #D0D0D0, stop: 0.4 #A0A0A0,"
                      "stop: 0.5 #808080, stop: 1.0 #A0A0A0);"
                      //"stop: 0.5 #A0A0A0, stop: 1.0 #C0C0C0);"
                      //"stop: 0 #fafafa, stop: 0.4 #f4f4f4,"
                      //"stop: 0.5 #e7e7e7, stop: 1.0 #fafafa);"
                      //"border-bottom-color: orange;" /* same as the pane color */
                      "}"
                      
                      "QTabBar::tab:selected {"
                      "border-color: orange;"
                      "border-bottom-color: #A0A0A0;" /* same as pane color */
                      "}"
                      
                      "QTabBar::tab:!selected {"
                      "    margin-top: 2px;" /* make non-selected tabs look smaller */
                      "}"
                      );
    }
    
    if(style.compare("Salmon") == 0){
        
        styleChoice = "Salmon";
        setStyleSheet(
                      
                      // BUTTONS
                      "QPushButton{"
                      "background-color:#FF5E4D;"
                      "min-width : 80px;"
                      "border: 2px solid grey;"
                      "border-radius: 6px;"
                      "margin-top: 1ex;"
                      "}"
                      
                      "QPushButton:hover{"
                      "border: 2px ;"
                      "}"
                      
                      "QPushButton:pressed{"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #FFE4C4, stop: 1 #FEC3AC);"
                      "}"
                      
                      // GROUPS
                      "QGroupBox{"
                      //Subcontrol differenciates the instances of QGroupBox and the subclasses of QGroupBox
                      "subcontrol: .QGroupBox"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
                      "stop: 0 #FFE4C4, stop: 1 #FEC3AC);"
                      "border: 2px solid gray;"
                      "border-radius: 5px;"
                      "margin-top: 3ex;"
                      "font-size:10pt;"
                      "font-weight:bold;"
                      "color: dark grey;"
                      "color: white;"
                      "}"
                      
                      "QGroupBox::title {"
                      "subcontrol-origin: margin;"
                      "subcontrol-position: top center;"
                      "padding: 0 5px;"
                      "color: black;"
                      "}"
                      
                      // MAINWINDOW
                      "QMainWindow {"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
                      "stop: 0 #FFE4C4,stop: 1 #FEC3AC);"
                      "border: 2px solid gray;"
                      "border-radius: 5px;"
                      "margin-top: 3ex;"
                      "font-size:10pt;"
                      "font-weight:bold;"
                      "color: dark grey;"
                      "color: white;"
                      "}"
                      
                      // PLAINTEXTEDIT
                      "QPlainTextEdit, QTextEdit{"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
                      "stop: 0 #FFE4C4,stop: 1 #FEC3AC);"
                      "border: 2px solid gray;"
                      "border-radius: 5px;"
                      "top: 3px;"
                      "margin-top: 3ex;"
                      "font-size:12pt;"
                      "font-weight:bold;"
                      "color: black;"
                      "}"
                      
                      "QLabel{"
                      "color : black;"
                      "}"
                      //TextBrowser
                      "QTextBrowser {"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
                      "stop: 0 #FFE4C4,stop: 1 #FEC3AC);"
                      "color: black;"
                      "}"
                      
                      "QTextBrowser:document{"
                      "text-decoration: underline;"
                      "color: white;"
                      "font: Menlo;"
                      "font-size: 14px"
                      "}"
                      
                      // SLIDERS
                      // horizontal sliders
                      "QSlider::groove:vertical {"
                      "background: red;"
                      "position: absolute;" /* absolutely position 4px from the left and right of the widget. setting margins on the widget should work too... */
                      "left: 13px; right: 13px;"
                      "}"
                      
                      "QSlider::handle:vertical {"
                      "height: 40px;"
                      "width: 30px;"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                      "stop: 0 #4E3D28, stop : 0.05 #856D4D, stop: 0.1 #4E3D28, stop : 0.15 #856D4D, stop: 0.2 #4E3D28, stop : 0.25 #856D4D, stop: 0.3 #4E3D28, stop : 0.35 #856D4D, stop: 0.4 #4E3D28, stop : 0.45 #856D4D, stop: 0.5 #4E3D28, stop : 0.55 #856D4D, stop: 0.6 #4E3D28, stop : 0.65 #856D4D, stop: 0.7 #4E3D28, stop : 0.75 #856D4D, stop: 0.8 #4E3D28, stop : 0.85 #856D4D, stop: 0.95 #4E3D28);"
                      "margin: 0 -5px; /* expand outside the groove */"
                      "border-radius: 5px;"
                      "}"
                      
                      "QSlider::add-page:vertical {"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                      "stop: 0 #FF5E4D, stop : 0.5 #FF5E4D);"
                      "}"
                      
                      "QSlider::sub-page:vertical {"
                      "background: #CECECE;"
                      "}"
                      
                      // horizontal sliders
                      
                      "QSlider::groove:horizontal {"
                      "background: red;"
                      "position: absolute;" /* absolutely position 4px from the left and right of the widget. setting margins on the widget should work too... */
                      "top: 14px; bottom: 14px;"
                      "}"
                      
                      "QSlider::handle:horizontal {"
                      "width: 40px;"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                      "stop: 0 #4E3D28, stop : 0.05 #856D4D, stop: 0.1 #4E3D28, stop : 0.15 #856D4D, stop: 0.2 #4E3D28, stop : 0.25 #856D4D, stop: 0.3 #4E3D28, stop : 0.35 #856D4D, stop: 0.4 #4E3D28, stop : 0.45 #856D4D, stop: 0.5 #4E3D28, stop : 0.55 #856D4D, stop: 0.6 #4E3D28, stop : 0.65 #856D4D, stop: 0.7 #4E3D28, stop : 0.75 #856D4D, stop: 0.8 #4E3D28, stop : 0.85 #856D4D, stop: 0.95 #4E3D28);"
                      "margin: -5px 0; /* expand outside the groove */"
                      "border-radius: 5px;"
                      "}"
                      
                      "QSlider::sub-page:horizontal {"
                      "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #FF5E4D stop : 0.5 #FF5E4D);"
                      "}"
                      
                      "QSlider::add-page:horizontal {"
                      "background: #CECECE;"
                      "}"
                      
                      //                      "uiKnob{"
                      //                      "background-color : #FF5E4D;"
                      //                      "}"
                      //                      
                      // TABS
                      //TabWidget and TabBar
                      "QTabWidget::pane {" /* The tab widget frame */
                      "color : black;"
                      "border-top: 2px #FF5E4D;"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                      "stop: 0 #FFE4C4,stop: 1 #FEC3AC);"
                      "}"
                      
                      "QTabWidget::tab-bar {"
                      "left: 5px;" /* move to the right by 5px */
                      "}"
                      
                      /* Style the tab using the tab sub-control. Note that
                       it reads QTabBar _not_ QTabWidget */
                      "QTabBar::tab {"
                      "background: #FFE4C4;"
                      "border: 2px solid #FF5E4D;"
                      "color : black;"
                      "border-bottom-color: #FF5E4D;" 
                      "border-top-left-radius: 4px;"
                      "border-top-right-radius: 4px;"
                      "min-width: 8ex;"
                      "padding: 2px;"
                      "}"
                      
                      "QTabBar::tab:selected, QTabBar::tab:hover {"
                      "background: #FEC3AC;"
                      "color : white;"
                      "}"
                      
                      "QTabBar::tab:!selected {"
                      "    margin-top: 2px;" /* make non-selected tabs look smaller */
                      "}"
                      );
    }
}

void FaustLiveApp::Preferences(){
    
    audioArchitecture->setCurrentIndex(indexAudio);
    currentIndexChange(indexAudio);
    
    int val = preference->exec();
}

void FaustLiveApp::init_PreferenceWindow(){
    
    preference->setWindowTitle("PREFERENCES");
    
    QTabWidget* myTab = new QTabWidget(preference);
    myTab->setStyleSheet("*{}""*::tab-bar{}");
    QTabBar* myTabBar = new QTabBar(myTab);
    
    QGroupBox* menu1 = new QGroupBox(myTab);
    QGroupBox* menu2 = new QGroupBox(myTab);
    QGroupBox* menu3 = new QGroupBox(myTab);
    
    myTab->addTab(menu1, tr("Window Preferences"));
    myTab->addTab(menu2, tr("Audio Preferences"));
    myTab->addTab(menu3, tr("Style Preferences"));
    
    compilModes = new QLineEdit(menu1);
    optVal = new QLineEdit(menu1);
    
    audioArchitecture = new QComboBox(menu2);
    
    audioArchitecture->addItem("Core Audio");
    //    audioArchitecture->setItemData(0, 0, Qt::UserRole-1);
    audioArchitecture->addItem("Jack");
    audioArchitecture->addItem("NetJack");
    
    QWidget* intermediateWidget = new QWidget(preference);
    
    cancelB = new QPushButton(tr("Cancel"), intermediateWidget);
    cancelB->setDefault(false);;
    
    saveB = new QPushButton(tr("Save"), intermediateWidget);
    saveB->setDefault(true);
    
    connect(saveB, SIGNAL(released()), this, SLOT(save_Mode()));
    connect(cancelB, SIGNAL(released()), this, SLOT(hide_preferenceWindow()));
    connect(audioArchitecture, SIGNAL(activated(int)), this, SLOT(currentIndexChange(int)));
    
    recall_Settings(homeSettings);
    
    compilModes->setText(compilationMode.c_str());
    stringstream oV;
    
    oV << opt_level;
    optVal->setText(oV.str().c_str());
    
    layout = new QFormLayout;
    layout2 = new QFormLayout;
    layout3 = new QFormLayout;
    layout4 = new QGridLayout;
    layout5 = new QVBoxLayout;
    QHBoxLayout* intermediateLayout = new QHBoxLayout;
    intermediateLayout->setAlignment(Qt::AlignCenter);
    
    layout->addRow(new QLabel(tr("")));
    QLabel* la1 = new QLabel(tr("Default Compilation Options"));
    layout->addRow(la1, compilModes);
    QLabel* la2 = new QLabel(tr("Optimization Value of compilation"));
    layout->addRow(la2, optVal);
    layout->addRow(new QLabel(tr("")));
    layout2->addRow(new QLabel(tr("")));
    QLabel* la3 = new QLabel(tr("Audio Architecture"));
    layout2->addRow(la3, audioArchitecture);
    
    QPlainTextEdit* container = new QPlainTextEdit(menu3);
    container->setReadOnly(true);
    container->setStyleSheet("*{background-color : transparent;}");
    
    QPushButton* grey = new QPushButton(tr("Grey"));
    grey->setFlat(true);
    grey->setStyleSheet("QPushButton:flat{"
                        "background-color: #A0A0A0;"
                        "color: white;"
                        "min-width:100px;"
                        "border: 2px solid gray;"
                        "border-radius: 6px;"
                        "}"
                        "QPushButton:flat:hover{"
                        "background-color: #202020;"                         
                        "}" );
    
    QPushButton* blue = new QPushButton(tr("Blue"));
    blue->setFlat(true);
    blue->setStyleSheet("QPushButton:flat{"
                        "background-color: #22427C;"
                        "color: white;"
                        "min-width:100px;"
                        "border: 2px solid gray;"
                        "border-radius: 6px;"
                        "}"
                        "QPushButton:flat:hover{"
                        "background-color: #702963;"                         
                        "}" );
    
    QPushButton* defaultColor = new QPushButton(tr("Default"));
    defaultColor->setFlat(true);
    defaultColor->setStyleSheet("QPushButton:flat{"
                                "background-color: lightGray;"
                                "color: black;"
                                "min-width:100px;"
                                "border: 2px solid gray;"
                                "border-radius: 6px;"
                                "}"
                                "QPushButton:flat:hover{"
                                "background-color: darkGray;"                         
                                "}" );
    
    QPushButton* pastel = new QPushButton(tr("Salmon"));
    pastel->setFlat(true);
    pastel->setStyleSheet("QPushButton:flat{"
                          "background-color: #FFE4C4;"
                          "color: black;"
                          "min-width:100px;"
                          "border: 2px solid gray;"
                          "border-radius: 6px;"
                          "}"
                          "QPushButton:flat:hover{"
                          "background-color: #FF5E4D;"                         
                          "}" );
    
    connect(grey, SIGNAL(clicked()), this, SLOT(styleClicked()));
    connect(blue, SIGNAL(clicked()), this, SLOT(styleClicked()));
    connect(defaultColor, SIGNAL(clicked()), this, SLOT(styleClicked()));
    connect(pastel, SIGNAL(clicked()), this, SLOT(styleClicked()));
    
    layout4->addWidget(defaultColor, 0, 0);    
    layout4->addWidget(blue, 1, 0);
    layout4->addWidget(grey, 1, 1);
    layout4->addWidget(pastel, 0, 1);
    
    container->setLayout(layout4);
    
    layout5->addWidget(container);
    menu3->setLayout(layout5);
    
    stringstream ll, bf, ss;
    string urlText("");
    QString sheet("");
    
    switch (indexAudio) {
        case kCoreaudio:
            
            bf << bufferSize;  
            printf("buf size = %i\n", bufferSize);
            bufSize = new QLineEdit(bf.str().c_str());
            bufSize->setText(bf.str().c_str());
            
            splRate = new QTextBrowser;
            
            urlText = "To modify the machine sample rate, go to <a href = /Applications/Utilities/Audio\\MIDI\\Setup.app>Audio Configuration</a>";
            sheet = QString::fromLatin1("a{ text-decoration: underline; color: black; font: Menlo; font-size: 14px }");
            splRate->document()->setDefaultStyleSheet(sheet);
            splRate->setStyleSheet("*{color: black; font: Menlo; font-size: 14px; background-color : white; }");
            
            
            splRate->setOpenExternalLinks(false);
            splRate->setHtml(urlText.c_str());
            splRate->setFixedHeight(50);
            connect(splRate, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(linkClicked(const QUrl&)));
            
            ASR = new QLabel(tr("Audio Sample Rate"));
            ABS = new QLabel(tr("Audio Buffer Size"));
            
            layout2->addRow(ABS, bufSize);
            layout2->addRow(splRate);
            break;
            
        case kNetjackaudio:
            
            ss << compressionValue;
            bf << masterPort;
            ll << latency;            
            
            cprValue = new QLineEdit(ss.str().c_str());
            cprValue->setText(ss.str().c_str());
            CV = new QLabel(tr("Compression Value"));
            layout2->addRow(CV, cprValue);
            
            mIP = new QLineEdit(masterIP.c_str());
            mIP->setText(masterIP.c_str());
            MIA = new QLabel(tr("Master IP adress"));
            layout2->addRow(MIA, mIP);
            
            mPort = new QLineEdit(bf.str().c_str());
            mPort->setText(bf.str().c_str());
            MP = new QLabel(tr("Master Port"));
            layout2->addRow(MP, mPort);
            
            lat = new QLineEdit(ll.str().c_str());
            lat->setText(ll.str().c_str());
            LAT = new QLabel(tr("Latency"));
            layout2->addRow(LAT, lat);
            
            break;
        default:
            break;
    }
    
    layout3->addRow(myTab);
    
    intermediateLayout->addWidget(cancelB);
    intermediateLayout->addWidget(new QLabel(tr("")));
    intermediateLayout->addWidget(saveB);
    
    intermediateWidget->setLayout(intermediateLayout);
    layout3->addRow(intermediateWidget);
    
    menu1->setLayout(layout);
    menu2->setLayout(layout2);
    preference->setLayout(layout3);
    
    //    audioArchitecture->setCurrentIndex(indexAudio);
}

void FaustLiveApp::currentIndexChange(int index){
    
    //    audioArchitecture->setCurrentIndex(index);
    
    //    printf("TEXT = %s\n", audioArchitecture->currentText().toStdString().c_str());
    
    delete splRate;
    splRate = NULL;
    delete bufSize;
    bufSize = NULL;
    delete ASR;
    ASR= NULL;
    delete ABS;
    ABS = NULL;
    delete cprValue;
    cprValue = NULL;
    delete mIP;
    mIP = NULL;
    delete mPort;
    mPort = NULL;
    delete lat;
    lat = NULL;
    delete CV;
    CV = NULL;
    delete MIA;
    MIA = NULL;
    delete MP;
    MP = NULL;
    delete LAT;
    LAT = NULL;
    
    string urlText;
    QString sheet;
    
    if(index == 2){
        
        cprValue = new QLineEdit;
        mIP = new QLineEdit;
        mPort = new QLineEdit;
        lat = new QLineEdit;
        CV = new QLabel(tr("Compression Value"));
        //        CV->setStyleSheet("*{color:white;}");
        MIA = new QLabel(tr("Master IP adress"));
        //        MIA->setStyleSheet("*{color:white;}");
        MP = new QLabel(tr("Master Port"));
        //        MP->setStyleSheet("*{color:white;}");
        LAT = new QLabel(tr("Latency"));
        //        LAT->setStyleSheet("*{color:white;}");
        
        stringstream ss;
        ss << compressionValue;
        
        cprValue->setText(ss.str().c_str());
        
        mIP->setText(masterIP.c_str());
        
        stringstream bf;
        bf << masterPort;
        
        mPort->setText(bf.str().c_str());
        
        stringstream ll;
        ll << latency;
        
        lat->setText(ll.str().c_str());
        
        layout2->insertRow(4, CV, cprValue);
        layout2->insertRow(5, MIA, mIP);
        layout2->insertRow(6, MP, mPort);
        layout2->insertRow(7, LAT, lat);
    }
    else if(index == 0){
        
        splRate = new QTextBrowser;
        bufSize = new QLineEdit;
        ASR = new QLabel(tr("Audio Sample Rate"));
        ABS = new QLabel(tr("Audio Buffer Size"));
        
        sheet = QString::fromLatin1("a{ text-decoration: underline; color: black; font: Menlo; font-size: 14px }");
        splRate->document()->setDefaultStyleSheet(sheet);
        
        urlText = "To modify the sample rate, go to <a href = /Applications/Utilities/Audio\\MIDI\\Setup.app>Audio Configuration</a>";
        
        splRate->setOpenExternalLinks(false);
        splRate->setHtml(urlText.c_str());
        splRate->setFixedHeight(50);
        connect(splRate, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(linkClicked(const QUrl&)));
        
        stringstream bf;
        bf << bufferSize;
        
        if(bf.str().compare("") == 0 || bf.str().compare("0") == 0)
            bf.str() = "512";
        
        bufSize->setText(bf.str().c_str());
        
        layout2->insertRow(4, ABS, bufSize);
        layout2->insertRow(5, splRate);
    }
}

void FaustLiveApp::hide_preferenceWindow(){
    preference->hide();
}

void FaustLiveApp::save_Mode(){
    
    compilationMode = compilModes->text().toStdString();
    
    const char* intermediate = optVal->text().toStdString().c_str();
    if(isStringInt(intermediate))
        opt_level = atoi(intermediate);
    else
        opt_level = 3;
    
    int newIndex = audioArchitecture->currentIndex();
    //    printf("NEW INDEX = %i\n", newIndex);
    
    int newBS = bufferSize;
    int newCV = compressionValue;
    int newMP = masterPort;
    int newLAT = latency; 
    string newMIA = masterIP;
    
    if(newIndex == 0){
        const char* inter2 = bufSize->text().toStdString().c_str();
        if(isStringInt(inter2)){
            newBS = atoi(inter2);
            if(newBS == 0)
                newBS = 512;
        }
        else
            newBS = 512;
    }
    else if(newIndex == 2){
        
        const char* inter = cprValue->text().toStdString().c_str();
        if(isStringInt(inter)){
            newCV = atoi(inter);
            if(newCV == 0)
                newCV = -1;
        }
        else
            newCV = -1;
        
        const char* inter2 = mPort->text().toStdString().c_str();
        if(isStringInt(inter2)){
            newMP = atoi(inter2);
            if(newMP == 0)
                newMP = 19000;
        }
        else
            newMP = 19000;   
        
        if(mIP->text().toStdString().compare("") == 0)
            newMIA = "225.3.19.154";
        else
            newMIA = mIP->text().toStdString();
        
        const char* inter3 = lat->text().toStdString().c_str();
        if(isStringInt(inter3)){
            newLAT = atoi(inter3);
            if(newLAT == 0)
                newLAT = 2;
        }
        else
            newLAT = 2;   
    }
    
    hide_preferenceWindow();
    
    if(indexAudio != newIndex){
        
        //        printf("newBS = %i\n", newBS);
        update_AudioArchitecture(newIndex, newBS, newCV, newMIA, newMP, newLAT);
    }
    else if( (indexAudio == 0 && (bufferSize != newBS)) || ( indexAudio == 2 && (compressionValue != newCV || newMIA.compare(masterIP) != 0 || masterPort != newMP || latency != newLAT)) ){
        
        update_AudioParameters(newIndex, newBS, newCV, newMIA, newMP, newLAT);
    }
    
    save_Settings(homeSettings);
}

void FaustLiveApp::save_Settings(string& home){
    
    string modeText = compilationMode;
    
    int pos = 0;
    
    if(modeText.compare("") == 0){
        modeText = " ";
    }
    
    while(modeText.find(" ", pos) != string::npos){
        
        if(pos != string::npos && modeText[pos] != '-'){
            modeText.replace(pos, 1, "/");
        }
        pos = modeText.find(" ", pos+1);
    }
    
    QFile f(home.c_str()); 
    
    if(f.open(QFile::WriteOnly | QIODevice::Truncate)){
        
        QTextStream textWriting(&f);
        QString toSave = masterIP.c_str();
        if(toSave == "")
            toSave = '/';
        
        textWriting<<modeText.c_str()<<' '<<opt_level<<' '<<indexAudio <<' '<< bufferSize <<' '<<compressionValue <<' '<< toSave <<' '<< masterPort <<' '<<latency<<' '<<styleChoice.c_str()<<endl;
        
        f.close();
    }
}

void FaustLiveApp::recall_Settings(string& home){
    
    QString ModeText;
    
    QFile f(home.c_str()); 
    
    if(f.open(QFile::ReadOnly)){
        
        QTextStream textReading(&f);
        QString intermediate;
        QString styleIntermediate;
        
        textReading>>ModeText>>opt_level>>indexAudio>>bufferSize>>compressionValue >> intermediate >> masterPort>> latency>>styleIntermediate;
        
        masterIP = intermediate.toStdString();
        styleChoice = styleIntermediate.toStdString();
        
        if(masterIP.compare("/") == 0)
            masterIP = "";
        
        f.close();
    }
    
    string modeText = ModeText.toStdString();
    
    int pos = 0;
    
    while(modeText.find("/", pos) != string::npos){
        
        if(pos != string::npos && modeText[pos] != '-')
            modeText.replace(pos, 1, " ");
        
        pos = modeText.find("/", pos+1);
    }
    
    if(modeText.compare(" ") == 0)
        modeText = "";
    
    compilationMode = modeText;
}

void FaustLiveApp::update_AudioArchitecture(int newIndex, int newBS, int newCV, string newMIA, int newMP, int newLAT){
    
    //    printf("Update_Audio FaustLive : newBS = %i\n", newBS);
    
    list<FLWindow*>::iterator it;
    
    bool updateSuccess = true;
    char error[256];
    snprintf(error, 255, "");
    
    display_CompilingProgress("Updating Audio Architecture...");
    
    //    list<FLWindow*> toDelete;
    
    //Save all audio clients
    for(it = FLW_List.begin() ; it != FLW_List.end(); it++)    
        (*it)->save_Window(indexAudio);
    
    //Stop all audio clients
    for(it = FLW_List.begin() ; it != FLW_List.end(); it++)
        (*it)->stop_Audio();
    
    for(it = FLW_List.begin() ; it != FLW_List.end(); it++)    
        (*it)->reset_audioSwitch();   
    
    //Try to init new audio architecture
    for(it = FLW_List.begin() ; it != FLW_List.end(); it++){
        
        if(!(*it)->update_AudioArchitecture(newIndex, newBS, newCV, newMIA, newMP, newLAT)){
            updateSuccess = false;
            break;
        }
    }
    
    bool reinitSuccess = true;
    
    //If init failed, reinit old audio Architecture
    if(!updateSuccess){
        
        //        toDelete.clear();
        
        switch(newIndex){
            case kCoreaudio:
                snprintf(error, 255, "%s", "Impossible to init Core Audio Client");
                break;
            case kJackaudio:
                snprintf(error, 255, "%s", "Impossible to init Jack Client");
                break;
            case kNetjackaudio:
                snprintf(error, 255, "%s", "Impossible to init remote Net Jack Client.\nMake sure the server is running!");
            default:
                break;
        }
        
        errorWindow->print_Error(error);
        
        for(it = FLW_List.begin() ; it != FLW_List.end(); it++){
            
            if(!(*it)->init_audioClient(indexAudio, bufferSize, compressionValue, masterIP, masterPort, latency))
            {
                reinitSuccess = false;
                break;
            }
        }
        
        snprintf(error, 255, "");
        
        if(!reinitSuccess){
            
            shut_AllWindows();
            switch(indexAudio){
                case kCoreaudio:
                    snprintf(error, 255, "%s", "\nImpossible to restart CoreAudio.");
                    break;
                case kJackaudio:
                    snprintf(error, 255, "%s", "\nImpossible to restart Jack.");
                    break;
                case kNetjackaudio:
                    snprintf(error, 255, "%s", "\nImpossible to restart NetJack.");
                default:
                    break;
            }
        }
        else{
            
            switch(indexAudio){
                case kCoreaudio:
                    snprintf(error, 255, "%s", "\nStill running with CoreAudio.");
                    break;
                case kJackaudio:
                    snprintf(error, 255, "%s", "\nStill running with Jack.");
                    break;
                case kNetjackaudio:
                    snprintf(error, 255, "%s", "\nStill running with NetJack.");
                default:
                    break;
            }
        }
        
        errorWindow->print_Error(error);   
    }
    else{
        indexAudio = newIndex;
        
        if(indexAudio == 0)
            bufferSize = newBS;
        
        else if(indexAudio == 2){
            compressionValue = newCV;
            masterIP = newMIA;
            masterPort = newMP;
            latency = newLAT;
        }
    }
    
    StopProgressSlot();
}

void FaustLiveApp::update_AudioParameters(int index, int newBS, int newCV, string newMIA, int newMP, int newLAT){
    
    bool updateSuccess = true;
    char error[256];
    snprintf(error, 255, "");
    
    list<FLWindow*>::iterator it;
    
    display_CompilingProgress("Updating Audio Parameters...");
    
    //Save all audio clients
    for(it = FLW_List.begin() ; it != FLW_List.end(); it++)    
        (*it)->save_Window(indexAudio);
    
    //Stop all audio clients
    for(it = FLW_List.begin() ; it != FLW_List.end(); it++){
        
        printf("LAUNCH STOP AUDIO\n");
        (*it)->stop_Audio();
    }
    
    //Reset switch indicator
    for(it = FLW_List.begin() ; it != FLW_List.end(); it++)    
        (*it)->reset_audioSwitch();    
    
    //Try to init new audio architecture
    for(it = FLW_List.begin() ; it != FLW_List.end(); it++){
        
        if(!(*it)->update_AudioParameters(error, index, newBS, newCV, newMIA, newMP, newLAT)){
            updateSuccess = false;
            break;
        }
    }
    
    bool reinitSuccess = true;
    
    //If init failed, reinit old audio Architecture
    if(!updateSuccess){
        
        switch(index){
            case kCoreaudio:
                snprintf(error, 255, "%s", "Impossible to init Core Audio Client with new parameters.");
                break;
            case kNetjackaudio:
                snprintf(error, 255, "%s", "Impossible to init remote Net Jack Client with new parameters.");
            default:
                break;
        }
        
        errorWindow->print_Error(error);
        
        for(it = FLW_List.begin() ; it != FLW_List.end(); it++){
            
            if(!(*it)->init_audioClient(index, bufferSize, compressionValue, masterIP, masterPort, latency))
            {
                reinitSuccess = false;
                break;
            }
        }
        
        snprintf(error, 255, "");
        
        if(!reinitSuccess){
            
            shut_AllWindows();
            switch(index){
                case kCoreaudio:
                    snprintf(error, 255, "%s", "\nImpossible to restart CoreAudio with previous parameters.");
                    break;
                case kNetjackaudio:
                    snprintf(error, 255, "%s", "\nImpossible to restart NetJack with previous parameters.");
                default:
                    break;
            }
            errorWindow->print_Error(error);   
        }
    }
    else{
        
        if(indexAudio == 0)
            bufferSize = newBS;
        
        else if(indexAudio == 2){
            compressionValue = newCV;
            masterIP = newMIA;
            masterPort = newMP;
            latency = newLAT;
        }
    }
    
    StopProgressSlot();
}

//--------------------------LONG WAITING PROCESSES------------------------------

void FaustLiveApp::display_CompilingProgress(const char* msg){
    compilingMessage = new QDialog();
    compilingMessage->setWindowFlags(Qt::FramelessWindowHint);
    
    QLabel* tittle = new QLabel(tr("<h2>FAUSTLIVE</h2>"));
    tittle->setAlignment(Qt::AlignCenter);
    QLabel* text = new QLabel(tr(msg), compilingMessage);
    text->setAlignment(Qt::AlignCenter);
    text->setStyleSheet("*{color: black}");
    
    QVBoxLayout* layoutSave = new QVBoxLayout;
    
    layoutSave->addWidget(tittle);
    layoutSave->addWidget(new QLabel(tr("")));
    layoutSave->addWidget(text);
    layoutSave->addWidget(new QLabel(tr("")));
    compilingMessage->setLayout(layoutSave);
    
    compilingMessage->adjustSize();
    compilingMessage->show();
    compilingMessage->raise();
}

void FaustLiveApp::StopProgressSlot()
{
    compilingMessage->hide();
    delete compilingMessage;
}
