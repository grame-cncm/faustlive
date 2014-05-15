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
#else
#include <windows.h>
#include <shlobj.h>
#endif
#include "FLWindow.h"
#include "FLErrorWindow.h"
#include "FLExportManager.h"
#include "utilities.h"

#include "faust/remote-dsp.h"

#include <sstream>

//----------------------CONSTRUCTOR/DESTRUCTOR---------------------------

FLApp::FLApp(int& argc, char** argv) : QApplication(argc, argv){

    //Create Current Session Folder
    create_Session_Hierarchy();
    
    //Initializing screen parameters
    QSize screenSize = QApplication::desktop()->geometry().size(); 
    fScreenWidth = screenSize.width();
    fScreenHeight = screenSize.height();
    
    //Base Name of application's windows
    fWindowBaseName = "FLW";
    
    //Initializing preference
    fOpt_level = 3;
    fServerUrl = "http://faustservice.grame.fr";
    fPort = 7777;
    fStyleChoice = "Default";
    recall_Settings(fSettingsFolder);
    styleClicked(fStyleChoice);
    
    // Presentation Window Initialization
    fPresWin = new QDialog;
    fPresWin->setWindowFlags(Qt::FramelessWindowHint);
    init_presentationWindow();   
    
    //Initializing menu options 
    fRecentFileAction = new QAction* [kMAXRECENTFILES];
    fRrecentSessionAction = new QAction* [kMAXRECENTSESSIONS];
    fIrecentSessionAction = new QAction* [kMAXRECENTSESSIONS];
    
#ifndef __APPLE__
    //For the application not to quit when the last window is closed
    setQuitOnLastWindowClosed(true);
#else
    setQuitOnLastWindowClosed(false);
#endif
    
    fMenuBar = new QMenuBar;
    setup_Menu();
    
    recall_Recent_Files(fRecentsFile);
    recall_Recent_Sessions(fHomeRecentSessions);
    
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

    fPresWin->move((fScreenWidth-fPresWin->width())/2, 20);
    
    //If no event opened a window half a second after the application was created, a initialized window is created
    fInitTimer = new QTimer(this);
    connect(fInitTimer, SIGNAL(timeout()), this, SLOT(init_Timer_Action()));
    fInitTimer->start(500);
}

FLApp::~FLApp(){
    
    save_Recent_Files();
    save_Recent_Sessions();
    save_Settings(fSettingsFolder);
    
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
    
    fSessionContent.clear();
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

	printf("HOME = %s\n", fSessionFolder.toStdString().c_str());

    fSessionFolder += FLVERSION;
    if(!QFileInfo(fSessionFolder).exists()){
        QDir direct(fSessionFolder);
        direct.mkdir(fSessionFolder);
    }
    
	fSessionFile = fSessionFolder + separationChar + "Description.txt";
    if(!QFileInfo(fSessionFile).exists()){
        QFile f(fSessionFile);
        f.open(QFile::ReadOnly);
        f.close();
    }    
    
	fSourcesFolder = fSessionFolder + separationChar + "Sources";
    if(!QFileInfo(fSourcesFolder).exists()){
        QDir direct(fSourcesFolder);
        direct.mkdir(fSourcesFolder);
    }    
    
    fSettingsFolder = fSessionFolder + separationChar  + "Settings";
    if(!QFileInfo(fSettingsFolder).exists()){
        QDir direct(fSettingsFolder);
        direct.mkdir(fSettingsFolder);
    }
    
    fRecentsFile = fSettingsFolder + separationChar  + "FaustLive_FileSavings.rf"; 
    fHomeRecentSessions = fSettingsFolder + separationChar  + "FaustLive_SessionSavings.rf"; 
    
    fSVGFolder = fSessionFolder + separationChar  + "SVG";
    if(!QFileInfo(fSVGFolder).exists()){
        QDir direct(fSVGFolder);
        direct.mkdir(fSVGFolder);
    }  
    
    fIRFolder = fSessionFolder + separationChar  + "IR";
    if(!QFileInfo(fIRFolder).exists()){
        QDir direct(fIRFolder);
        direct.mkdir(fIRFolder);
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
    
    
    QDir direc(fSessionFile);
}

//--Build FaustLive Menu
void FLApp::setup_Menu(){
    
    //----------------FILE
    
    QMenu* fileMenu = new QMenu;
    fNavigateMenu = new QMenu;
    QMenu* helpMenu = new QMenu;
    
    QAction* newAction = new QAction(tr("&New Default Window"), this);
    newAction->setShortcut(tr("Ctrl+N"));
    newAction->setToolTip(tr("Open a new empty file"));
    connect(newAction, SIGNAL(triggered()), this, SLOT(create_Empty_Window()));
    
    QAction* openAction = new QAction(tr("&Open..."),this);
    openAction->setShortcut(tr("Ctrl+O"));
    openAction->setToolTip(tr("Open a DSP file"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open_New_Window()));
    
    QMenu* menuOpen_Example = new QMenu(tr("&Open Example"), fileMenu);
    
    QDir examplesDir(":/");
    
    if(examplesDir.cd("Examples")){
    
        QFileInfoList children = examplesDir.entryInfoList(QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
    
        fExampleToOpen = (children.begin())->baseName();
        
        QFileInfoList::iterator it;
        int i = 0; 
        
        QAction** openExamples = new QAction* [children.size()];
        
        for(it = children.begin(); it != children.end(); it++){
            
            openExamples[i] = new QAction(it->baseName(), menuOpen_Example);
            openExamples[i]->setData(QVariant(it->absoluteFilePath()));
            connect(openExamples[i], SIGNAL(triggered()), this, SLOT(open_Example_From_FileMenu()));
            
            menuOpen_Example->addAction(openExamples[i]);
            i++;
        }
    }
    
    QMenu* openRecentAction = new QMenu(tr("&Open Recent File"), fileMenu);
    
    for(int i=0; i<kMAXRECENTFILES; i++){
        fRecentFileAction[i] = new QAction(this);
        fRecentFileAction[i]->setVisible(false);
        connect(fRecentFileAction[i], SIGNAL(triggered()), this, SLOT(open_Recent_File()));
        
        openRecentAction->addAction(fRecentFileAction[i]);
    }
    
    //SESSION
    
    QAction* recallSnapshotAction = new QAction(tr("&Recall Snapshot..."),this);
    recallSnapshotAction->setShortcut(tr("Ctrl+R"));
    recallSnapshotAction->setToolTip(tr("Close all the opened window and open your snapshot"));
    connect(recallSnapshotAction, SIGNAL(triggered()), this, SLOT(recallSnapshotFromMenu()));
    
    QMenu* recallRecentAction = new QMenu(tr("&Recall Recent Snapshot"), fileMenu);
    QMenu* importRecentAction = new QMenu(tr("&Import Recent Snapshot"), fileMenu);
    
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
    connect(importSnapshotAction, SIGNAL(triggered()), this, SLOT(importSnapshotFromMenu()));
    
    //SHUT
    
    QAction* shutAction = new QAction(tr("&Close Window"),this);
    shutAction->setShortcut(tr("Ctrl+W"));
    shutAction->setToolTip(tr("Close the current Window"));
    connect(shutAction, SIGNAL(triggered()), this, SLOT(shut_Window()));
    
    QAction* shutAllAction = new QAction(tr("&Close All Windows"),this);
    shutAllAction->setShortcut(tr("Ctrl+Alt+W"));
    shutAllAction->setToolTip(tr("Close all the Windows"));
    connect(shutAllAction, SIGNAL(triggered()), this, SLOT(shut_AllWindows()));
    
    QAction* closeAllAction = new QAction(tr("&Closing"),this);
    closeAllAction->setShortcut(tr("Ctrl+Q"));
    closeAllAction = new QAction(tr("&Quit FaustLive"),this);
    closeAllAction->setToolTip(tr("Close the application"));   
    connect(closeAllAction, SIGNAL(triggered()), this, SLOT(closeAllWindows()));
    
    fileMenu = fMenuBar->addMenu(tr("&File"));
    fileMenu->addAction(newAction);    
    fileMenu->addSeparator();
    fileMenu->addAction(openAction);
    fileMenu->addAction(menuOpen_Example->menuAction());
    fileMenu->addAction(openRecentAction->menuAction());
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
    
    fMenuBar->addSeparator();
    
    //---------------------NAVIGATE MENU
    
    fNavigateMenu = fMenuBar->addMenu(tr("&Navigate"));    
    fMenuBar->addSeparator();
    
    //---------------------MAIN MENU
    
    QAction* aboutQtAction = new QAction(tr("&About Qt"), this);
    aboutQtAction->setToolTip(tr("Show the library's About Box"));
    connect(aboutQtAction, SIGNAL(triggered()), this, SLOT(aboutQt()));
    
    QAction* preferencesAction = new QAction(tr("&Preferences"), this);
    preferencesAction->setToolTip(tr("Set the preferences of the application"));
    connect(preferencesAction, SIGNAL(triggered()), this, SLOT(Preferences()));
    
    fPrefDialog = new QDialog;
    fPrefDialog->setWindowFlags(Qt::FramelessWindowHint);
    init_PreferenceWindow();
    fPrefDialog->move((fScreenWidth-fPrefDialog->width())/2, (fScreenHeight-fPrefDialog->height())/2);
    
    //--------------------HELP
    
    fHelpWindow = new QMainWindow;
    fHelpWindow->setWindowFlags(Qt::FramelessWindowHint);
    
    init_HelpWindow();
    fHelpWindow->move((fScreenWidth-fHelpWindow->width())/2, (fScreenHeight-fHelpWindow->height())/2);
    
    QAction* aboutAction = new QAction(tr("&Help..."), this);
    aboutAction->setToolTip(tr("Show the library's About Box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(apropos()));
    
//    fVersionWindow = new QDialog;
//    
//    fVersionAction = new QAction(tr("&Version"), this);
//    fVersionAction->setToolTip(tr("Show the version of the libraries used"));
//    connect(fVersionAction, SIGNAL(triggered()), this, SLOT(version_Action()));
    
    QAction* presentationAction = new QAction(tr("&About FaustLive"), this);
    presentationAction->setToolTip(tr("Show the presentation Menu"));
    connect(presentationAction, SIGNAL(triggered()), this, SLOT(show_presentation_Action()));
    
    helpMenu = fMenuBar->addMenu(tr("&Help"));
    
    helpMenu->addAction(aboutQtAction);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAction);
//    fHelpMenu->addAction(fVersionAction);
//    fHelpMenu->addSeparator();
    helpMenu->addAction(presentationAction);
    helpMenu->addSeparator();
    helpMenu->addAction(preferencesAction);
    
    
//    EXPORT MANAGER
    
    fExportDialog = new FLExportManager(fServerUrl, fSessionFolder);
}

//--Starts the presentation menu if no windows are opened (session restoration or drop on icon that opens the application)
void FLApp::init_Timer_Action(){
    fInitTimer->stop();
    
    if(FLW_List.size()==0){
        
        if(QFileInfo(fSessionFile).exists()){
            
            QFile f(fSessionFile);
            QString text("");
            
            if(f.open(QFile::ReadOnly)){
        
                QTextStream textReading(&f);
            
                textReading>>text;
                f.close();
            }
//            string text = ReadInFile(fSessionFile);
            
            if(text.compare("") == 0){
                
                show_presentation_Action();
            }
            else{
                fSessionContent.clear();
                recall_Session(fSessionFile);
            }
        }
        else{
            show_presentation_Action();
        }
    }
}

//--Print errors in errorWindow
void FLApp::errorPrinting(const char* msg){
    fErrorWindow->print_Error(msg);
}

//--------- OPERATIONS ON WINDOWS INDEXES

QList<int> FLApp::get_currentIndexes(){
    QList<int> currentIndexes;
    
    QList<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++){
        currentIndexes.push_back((*it)->get_indexWindow());
    }
    
    return currentIndexes;
    
}

int FLApp::find_smallest_index(QList<int> currentIndexes){
    
    QList<int>::iterator it;
    bool found = true;
    int i = 0;
    
//    printf("current index list = %i\n", FLW_List.size());
    
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

bool FLApp::isIndexUsed(int index, QList<int> currentIndexes){
    
    QList<int>::iterator it;
    
    for(it = currentIndexes.begin(); it != currentIndexes.end(); it++){
        if(index == *it ){
            return true;
        }
    }
    return false;
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


//----------NAMING EFFECTS

//Default Names
QList<QString> FLApp::get_currentDefault(){
    
    QList<QString> currentDefault;
    
    QList<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin(); it != fSessionContent.end() ; it++){
        
        if((*it)->name.indexOf(DEFAULTNAME)!=-1)
            currentDefault.push_back((*it)->name);
    }
    
    
//    printf("WIN CONTENT = %i\n", currentDefault.size());
    return currentDefault;
}

QString FLApp::find_smallest_defaultName(QList<QString> currentDefault){
    
    //Conditional jump on currentDefault List...
    
    int index = 1;
    QString nomEffet("");
    bool found = false;
    
    QString ss;
    
    do{
		ss = "";
        ss = QString::number(index);
        
        nomEffet = DEFAULTNAME;
        nomEffet += "_";
		nomEffet += ss;
        
        QList<QString>::iterator it;
        
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

//Return declare name if there is one.
//If the name exists in the session, the user is asked to choose another one
QString FLApp::getDeclareName(QString text){
    
    QString returning = "";
    int pos = text.indexOf("declare name");
    
    if(pos != -1){
        text.remove(0, pos);
        
        pos=text.indexOf("\"");
        if(pos != -1){
            text.remove(0, pos+1);
        }
        pos = text.indexOf("\"");
        text.remove(pos, text.length()-pos);
        
        text = renameEffect("", text, false);
        
        returning = text;
    }
    
    return returning;
}

//Dialog with the user to modify the name of a faust application
QString FLApp::renameEffect(const QString& source, const QString& nomEffet, bool isRecalledEffect){
    
    QString newName(nomEffet);
    
    while(isEffectNameInCurrentSession(source , newName, isRecalledEffect)){
        
        FLrenameDialog* Msg = new FLrenameDialog(newName, 0);
        Msg->raise();
        Msg->exec();
        newName = Msg->getNewName();
        
        while(newName.indexOf(' ') != -1)
            newName.remove(newName.indexOf(' '), 1);
        
        delete Msg;
    }
    return newName;
}

//Removing spaces in name. They cause problems
QString FLApp::nameWithoutSpaces(QString name){
    
    while(name.indexOf(' ') != -1)
        name.replace(name.indexOf(' '), 1, "_");
    
    return name;
}

//Returns the list of the names of all running Effects 
QList<QString> FLApp::getNameRunningEffects(){
    
    QList<QString> returning; 
    
    QList<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++)
        returning.push_back((*it)->name);   
    
    return returning;
}

//Find name of an Effect in CurrentSession, depending on its source file
QString FLApp::getNameEffectFromSource(const QString& sourceToCompare){
    
    QList<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
        
        if((*it)->source.compare(sourceToCompare) == 0)
            return (*it)->name;
    }
    return "";
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

//Lists all the windows containing the same Effect
QList<int> FLApp::WindowCorrespondingToEffect(FLEffect* effect){
    
    QList<int> returning;
    
    QList<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
        
        if((*it)->source.compare(effect->getSource()) == 0 && effect->isLocal() == (*it)->isLocal){
            returning.push_back((*it)->ID);
            }
    }
    return returning;
}

//--------------------------CREATE EFFECT

// GET COMPILED EFFECT
// @param : isLocal = is Effect local or remote
// @param : source = file source of wanted Effect
// @param : ip/port = ip/port of wanted processing machine (Remote Case)
//
// @return : Effect already compiled | NULL if not compiled
FLEffect* FLApp::getCompiledEffect(bool isLocal, QString source, const QString& ip, int port){
    
    QList<FLEffect*>::iterator beginList, endList, it;
    
    if(isLocal){
        beginList = fExecutedEffects.begin();
        endList = fExecutedEffects.end();
    }
    else{
        beginList = fRemoteEffects.begin();
        endList = fRemoteEffects.end();
    }
    
    for(it = beginList; it != endList; it++){
        
        const QString comparedSource = (*it)->getSource();
        const QString comparedIp = (*it)->getRemoteIP();
        int comparedPort = (*it)->getRemotePort();
        
        if(source.compare(comparedSource) == 0 && ip.compare(comparedIp) == 0 && port == comparedPort){

//Effect in current session = directly returned
            if(isLocal && isLocalEffectInCurrentSession(comparedSource))
                return *it;                       
            
            else if(!isLocal && isRemoteEffectInCurrentSession(comparedSource, comparedIp, comparedPort))
                return *it;
            
//Effect kept but potentially its name is used by another effect in the current session
//            Potentially the code has to be recompiled (code has been modified in remote or before being reused)
            else{
                (*it)->setName(renameEffect(source, (*it)->getName(), false));
                
                QString copyFile = fSourcesFolder + "/" + (*it)->getName() + ".dsp";
                
                QString contentSaved = pathToContent(source);
                QString contentInSource = pathToContent(copyFile);
                
                if((*it)->hasToBeRecompiled() || contentSaved.compare(contentInSource) != 0)
                    synchronize_Window(*it);
//                else
//                    IL FAUT METTRE A JOUR LE NOM DE LA COPIE, LE NOM DU IR, DU SVG, ...
                
                return *it;
            }
            
        }
    }
    
    return NULL;
}

//--Creates an Effect depending on it's source
//@param : source = faust code as string, file or url
//@param : nameEffect = name of faust application
//@param : sourceFolder = session folder to save the source
//@param : compilataion options
//@param : error = filled if an error occures
//@param : recall = true when recalling a session
//@param : isLocal = is Effect local or remote
//@param : ip/port = in case of Remote Effect, wanted processing machine
//
// @return : Effect | NULL if not compiled
FLEffect* FLApp::getEffectFromSource(QString source, QString nameEffect, const QString& sourceFolder, QString& compilationOptions, int opt_Val, QString& error, bool recall, bool isLocal, const QString& ip, int port){
    
//Marker of Source type : if true = file || if false = source
    bool fileSourceMark = false;
    QString content = "";
    QString fileSource = "";
    
    //SOURCE = URL --> source becomes text
    source = ifUrlToText(source);
    
    //SOURCE = FILE.DSP    
    if(QFileInfo(source).completeSuffix().compare("dsp") == 0){
        
//        If the effect was already compiled, it is recycled
        FLEffect* effectRecycled = getCompiledEffect(isLocal, source, ip, port);
        
        if(effectRecycled)
            return effectRecycled;
        
// If the effect is compiled for the first time, it has to be renamed if its name is already used
        
        fileSourceMark = true;
        fileSource = source;
        
//        NOT RECALLING
        if(!recall && nameEffect.compare("") == 0){

            nameEffect = QFileInfo(fileSource).baseName();
            nameEffect = renameEffect(fileSource, nameEffect, false);
            nameEffect = nameWithoutSpaces(nameEffect);
        }
    }
    
    //SOURCE = TEXT
    else{
//        Naming Effect from its declare name or a default name
        QString name = getDeclareName(source);
        
        if(name.compare("") == 0){
            QString defaultName = find_smallest_defaultName(get_currentDefault());
            name = defaultName;
        }
            
        nameEffect = nameWithoutSpaces(name);
        
//        Creating source file
        fileSource = sourceFolder + "/" + name + ".dsp";
        
        if(QFileInfo(fileSource).exists())
            content = pathToContent(fileSource);
        
        createSourceFile(fileSource, source);
    }
    
    display_CompilingProgress("Compiling your DSP...");
    
    FLEffect* myNewEffect = new FLEffect(recall, fileSource, nameEffect, isLocal);
    
    if(myNewEffect && myNewEffect->init(fSVGFolder, fIRFolder, fLibsFolder, compilationOptions, opt_Val, error, ip, port)){
        
        StopProgressSlot();
        
        connect(myNewEffect, SIGNAL(effectChanged()), this, SLOT(synchronize_Window()));
        
        if(isLocal)
            fExecutedEffects.push_back(myNewEffect);
        else
            fRemoteEffects.push_back(myNewEffect);
        
        return myNewEffect;
    }
    else{
        
        // If init failed  
        if(!fileSourceMark){
            
            // If file did not exist, it is removed
            if(content.compare("") == 0)
                QFile(fileSource).remove();
            // Otherwise, its previous content is restored
            else
                createSourceFile(fileSource, content);
        }
        
        StopProgressSlot();
        
        if(myNewEffect)
            delete myNewEffect;

        return NULL;
    }
}

//-------------------------ACTIONS ON FILE SOURCE OF EFFECTS

//--Creates an new file in the Source Folder of the Current Session
void FLApp::createSourceFile(const QString& sourceName, const QString& content){
    
    QFile f(sourceName);
    
    if(f.open(QFile::WriteOnly)){
        
        QTextStream textWriting(&f);
        
        textWriting<<content;
        
        f.close();
    }
}

//--Update FileName
void FLApp::update_Source(const QString& oldSource, const QString& newSource){
    
    if(oldSource.compare(newSource) != 0){
        
        if(QFileInfo(newSource).exists()){
            
            QString copyNewSource = newSource + ".bak";
            QFile::rename(newSource, copyNewSource);
            
            if(!QFile::copy(oldSource, newSource))
                QFile::rename(copyNewSource, newSource);
            else
                QFile::remove(copyNewSource);
        }
        else
            QFile::copy(oldSource, newSource);
    }
}

//--Transforms an Url into Text if it is one.
QString FLApp::ifUrlToText(const QString& source){
    
    //In case the text dropped is a web url
    int pos = source.indexOf("http://");
    
    QString UrlText(source);
    
    if(pos != -1){
        UrlText = "process = component(\"";
        UrlText += source;
        UrlText +="\");";
//        source = UrlText;
    }
    
    return UrlText;
}

//-------------------QUESTIONS ABOUT EFFECTS IN CURRENT SESSIONS

//--Finds out if a Local Effect exist in CurrentSession, depending on its source file
bool FLApp::isLocalEffectInCurrentSession(const QString& sourceToCompare){
    
    QList<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
        
        if((*it)->isLocal && (*it)->source.compare(sourceToCompare) == 0)
            return true;
    }
    return false;
    
}

//--Finds out if a Remote Effect exist in CurrentSession, depending on its source file
bool FLApp::isRemoteEffectInCurrentSession(const QString& sourceToCompare, const QString& ip, int port){
    
    QList<WinInSession*>::iterator it;
        
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
        if(!(*it)->isLocal && (*it)->source.compare(sourceToCompare) == 0 && (*it)->ipServer.compare(ip) == 0 && (*it)->portServer == port)
            return true;
    }
    
    return false;
    
}

//--Finds out if a source is used in any kind of effect
bool FLApp::isSourceInCurrentSession(const QString& sourceToCompare){
    
    QList<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
        
        if((*it)->source.compare(sourceToCompare) == 0)
            return true;
    }
    return false;
    
}

//Finds out if an Effect Name exist in CurrentSession
//DefaultName are not taken into account
bool FLApp::isEffectNameInCurrentSession(const QString& sourceToCompare , const QString& nom, bool isRecalledEffect){

    QList<int> returning;
    
    QList<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
        
        if((*it)->name.compare(nom)==0 && 
           (((*it)->source.compare(sourceToCompare) != 0 || 
            ((*it)->name.contains(DEFAULTNAME) != string::npos && isRecalledEffect)))
           )
            return true;
    }
    return false;
}

//----------------------UPDATES UPDATES

//In case source is modified from a remote/local effect and has to be recompiled when reused
void FLApp::setRecompileEffects(FLEffect* modifiedEffect){
    
    QList<FLEffect*>::iterator it;
    
    for(it = fExecutedEffects.begin(); it != fExecutedEffects.end(); it++){
        if(modifiedEffect->getSource().compare((*it)->getSource()) == 0)
            (*it)->forceRecompilation(true);
    }
    
    for(it = fRemoteEffects.begin(); it != fRemoteEffects.end(); it++){
        if(modifiedEffect->getSource().compare((*it)->getSource()) == 0)
            (*it)->forceRecompilation(true);
    }
}

void FLApp::synchronize_Window(FLEffect* modifiedEffect){
    QString modifiedSource = modifiedEffect->getSource();
    QString error("");
    
    QDateTime modifiedLast = QFileInfo(modifiedSource).lastModified();
    QDateTime creationDate = modifiedEffect->get_creationDate();
    
    //Avoiding the flicker when the source is saved
    if(QFileInfo(modifiedSource).exists() && (modifiedEffect->isSynchroForced() || creationDate<modifiedLast)){
        
        modifiedEffect->setForceSynchro(false);
        
        modifiedEffect->stop_Watcher();
        
        display_CompilingProgress("Updating your DSP...");
        
        if(!modifiedEffect->update_Factory(error)){
            
            StopProgressSlot();
            fErrorWindow->print_Error(error);
            
            modifiedEffect->launch_Watcher();
            return;
        }
        else if(error.compare("") != 0){
            fErrorWindow->print_Error(error);
        }
        
        StopProgressSlot();

        QList<int> indexes = WindowCorrespondingToEffect(modifiedEffect);
        
        QList<int>::iterator it;
        for(it=indexes.begin(); it!=indexes.end(); it++){
            QList<FLWindow*>::iterator it2;
            
            for (it2 = FLW_List.begin(); it2 != FLW_List.end(); it2++) {
                if((*it2)->get_indexWindow() == *it){
                    
                    printf("UPDATE_FACTORY\n");
                    
                    if(!(*it2)->update_Window(modifiedEffect, error)){
                        
                        fErrorWindow->print_Error(error);
                        break;
                    }
                    else{
                        
                        deleteWinFromSessionFile(*it2);
                        addWinToSessionFile(*it2);
                        
                        QString oldSource = modifiedEffect->getSource();
                        QString newSource = fSourcesFolder + "/" + modifiedEffect->getName() + ".dsp";
						printf("SYNCHRONIZE COPIES = %s TO %s\n", oldSource.toStdString().c_str(), newSource.toStdString().c_str());
                        update_Source(oldSource, newSource);
                        break;
                    }
                }
            }
            
        }
        
        setRecompileEffects(modifiedEffect);
        
        modifiedEffect->erase_OldFactory();
        modifiedEffect->launch_Watcher();
        printf("modifiedEffect OPT = %i\n", modifiedEffect->getOptValue());
    }
    //In case the file is erased during the execution
    else if(!QFileInfo(modifiedSource).exists()){
        
        modifiedEffect->stop_Watcher();
        
        error = "\nWARNING = ";
        error += modifiedSource; 
        error += " has been deleted or moved\n You are now working on its copy.";
        fErrorWindow->print_Error(error);
        
        QString toReplace = fSourcesFolder + "/" + modifiedEffect->getName() +".dsp";      
        modifiedEffect->setSource(toReplace);
        modifiedEffect->launch_Watcher();
    }
}

//Refresh a window. In case the source/options have been modified.
void FLApp::synchronize_Window(){ 

    printf("FLApp::synchronize_Window\n");
    
    FLEffect* modifiedEffect = (FLEffect*)QObject::sender();
    
    synchronize_Window(modifiedEffect);
}

//Modify the content of a specific window with a new source
void FLApp::update_SourceInWin(FLWindow* win, const QString& source){
    
    QString error;
    QString empty("");
    
    //Deletion of reemplaced effect from session
    FLEffect* leavingEffect = win->get_Effect();
    leavingEffect->stop_Watcher();
    deleteWinFromSessionFile(win);

    QString winOptions = win->get_Effect()->getCompilationOptions();
    int winOptValue = win->get_Effect()->getOptValue();
    
    FLEffect* newEffect = getEffectFromSource(source, empty, fSourcesFolder, winOptions, winOptValue, error, false, win->get_Effect()->isLocal(), win->get_Effect()->getRemoteIP(), win->get_Effect()->getRemotePort());

//We will force the update of compilation parameters ?
//    for a recycled effect
    bool optionChanged;
    
    if(newEffect != NULL)
        optionChanged = (winOptions.compare(newEffect->getCompilationOptions()) != 0 || winOptValue != (newEffect->getOptValue())) &&
        !isLocalEffectInCurrentSession(newEffect->getSource()) &&
        !isRemoteEffectInCurrentSession(newEffect->getSource(), newEffect->getRemoteIP(), newEffect->getRemotePort());
    
//  If the change fails, the leaving effect has to be reimplanted
    if(newEffect == NULL || (!(win)->update_Window(newEffect, error))){
        leavingEffect->launch_Watcher();
        addWinToSessionFile(win);
        fErrorWindow->print_Error(error);
        return;
    }
    else{

        //ICI ON VA FAIRE LA COPIE DU FICHIER SOURCE
        QString copySource = fSourcesFolder +"/" + newEffect->getName() + ".dsp";
        QString toCopy = newEffect->getSource();

        update_Source(toCopy, copySource);
        
        if(error.compare("") != 0){
            fErrorWindow->print_Error(error);
        }
        
        //If the change is successfull : 
        deleteEffect(leavingEffect, newEffect);
        
        //The new effect is added in the session and watched
        addWinToSessionFile(win);
        newEffect->launch_Watcher();
        
        //Forcing the update of compilation parameters for a recycled effect
        if(optionChanged){
            newEffect->update_compilationOptions(winOptions,  winOptValue);
            printf("update_compilationOptions\n");
        }
    }
    
}

//Migrate from one processing machine to another
bool FLApp::migrate_ProcessingInWin(const QString& ip, int port){
    
    FLWindow* migratingWin = (FLWindow*)QObject::sender();
    
    //Deletion of reemplaced effect from session
    FLEffect* leavingEffect = migratingWin->get_Effect();
    leavingEffect->stop_Watcher();
    deleteWinFromSessionFile(migratingWin);
    
    QString error("");
    
    bool isEffectLocal = false;
    bool isMigrationSuccessfull = true;
    
    if(ip.compare("localhost") == 0){
        isEffectLocal = true;
        printf("Back to Local host\n");
    }
    
    QString compilationOptions(migratingWin->get_Effect()->getCompilationOptions());
    
    FLEffect* newEffect = getEffectFromSource(migratingWin->get_Effect()->getSource(), migratingWin->get_Effect()->getName(), fSourcesFolder, compilationOptions, migratingWin->get_Effect()->getOptValue(), error, false, isEffectLocal, ip, port);
    
    if(newEffect == NULL){
        error += "\nImpossible to switch processing machine";
        isMigrationSuccessfull = false;
    }
        
    if(isMigrationSuccessfull && migratingWin->update_Window(newEffect, error)){
        newEffect->launch_Watcher();
        migratingWin->migrationSuccessfull();
        addWinToSessionFile(migratingWin);
        
        deleteEffect(migratingWin->get_Effect(), newEffect);
        
//        
//        vector<pair<string, string> > factories_list;
//        getRemoteFactoriesAvailable(ip.toStdString(), port, &factories_list);
//        
//        for(int i=0; i<factories_list.size(); i++)
//            printf("FACTORIES = %s\n", factories_list[i].first.c_str(), factories_list[i].second.c_str());
        
        return true;
    }
    else{
        leavingEffect->launch_Watcher();
        migratingWin->migrationFailed();
        addWinToSessionFile(migratingWin);        
        
        fErrorWindow->print_Error(error);
        return false;
    }
}

//--------------------------------FILE-----------------------------

//---------------NEW

void FLApp::redirectMenuToWindow(FLWindow* win){
    
    win->set_GeneralPort(fPort);
    
    win->set_RecentFile(fRecentFiles);
    win->update_RecentFileMenu();
    
    win->set_RecentSession(fRecentSessions);
    win->update_RecentSessionMenu();
    win->initNavigateMenu(fFrontWindow);
    
    connect(win, SIGNAL(drop(QList<QString>)), this, SLOT(drop_Action(QList<QString>)));
    connect(win, SIGNAL(migrate(const QString&, int)), this, SLOT(migrate_ProcessingInWin(const QString&, int)));
    connect(win, SIGNAL(error(const char*)), this, SLOT(errorPrinting(const char*)));
    connect(win, SIGNAL(create_Empty_Window()), this, SLOT(create_Empty_Window()));
    connect(win, SIGNAL(open_New_Window()), this, SLOT(open_New_Window()));
    connect(win, SIGNAL(open_Ex(QString)), this, SLOT(open_Example_Action(QString)));
    connect(win, SIGNAL(open_File(QString)), this, SLOT(open_Recent_File(QString)));
    connect(win, SIGNAL(takeSnapshot()), this, SLOT(take_Snapshot()));
    connect(win, SIGNAL(recallSnapshotFromMenu()), this, SLOT(recallSnapshotFromMenu()));
    connect(win, SIGNAL(importSnapshotFromMenu()), this, SLOT(importSnapshotFromMenu()));
    connect(win, SIGNAL(closeWin()), this, SLOT(close_Window_Action()));
    connect(win, SIGNAL(shut_AllWindows()), this, SLOT(shut_AllWindows()));
    connect(win, SIGNAL(closeAllWindows()), this, SLOT(closeAllWindows()));
    connect(win, SIGNAL(edit_Action()), this, SLOT(edit_Action()));
    connect(win, SIGNAL(paste_Action()), this, SLOT(paste_Text()));
    connect(win, SIGNAL(duplicate_Action()), this, SLOT(duplicate_Window()));
    connect(win, SIGNAL(httpd_View_Window()), this, SLOT(httpd_View_Window()));
    connect(win, SIGNAL(svg_View_Action()), this, SLOT(svg_View_Action()));
    connect(win, SIGNAL(export_Win()), this, SLOT(export_Action()));
    
    connect(win, SIGNAL(show_aboutQt()), this, SLOT(aboutQt()));
    connect(win, SIGNAL(show_preferences()), this, SLOT(Preferences()));
    connect(win, SIGNAL(apropos()), this, SLOT(apropos()));
    connect(win, SIGNAL(show_presentation_Action()), this, SLOT(show_presentation_Action()));
    connect(win, SIGNAL(recall_Snapshot(QString, bool)), this, SLOT(recall_Snapshot(QString, bool)));
    connect(win, SIGNAL(front(QString)), this, SLOT(frontShow(QString)));
    connect(win, SIGNAL(savePrefs()), this, SLOT(save_Mode()));
}

//--Creation of a new window
FLWindow* FLApp::new_Window(const QString& mySource, QString& error){
    
    if(FLW_List.size() >= numberWindows){
        error = "You cannot open more windows. If you are not happy with this limit, feel free to contact us : research.grame@gmail.com ^^";
        return NULL;
    }
    
    int init = kNoInit;
    
    QString source(mySource);
    
    //In case the source is empty, the effect is chosen by default 
    if(source.compare("") == 0){
        
        source = "process = !,!:0,0;";
        
        if(fStyleChoice == "Blue" || fStyleChoice == "Grey")
            init = kInitWhite;
        else
            init = kInitBlue;
    }
    
    //Choice of new Window's index
    QList<int> currentIndexes = get_currentIndexes();
    int val = find_smallest_index(currentIndexes);
    QString ss;
    ss = QString::number(val);
    
    //Creation of new effect from the source    
    QString empty("");
    FLEffect* first = getEffectFromSource(source, empty, fSourcesFolder, fCompilationMode, fOpt_level ,error, false, true); 

    if(first != NULL){
//        To force the update in case compilation option changed ??? 
//        bool optionChanged = (fCompilationMode.compare(first->getCompilationOptions()) != 0 || fOpt_level != (first->getOptValue())) && !isLocalEffectInCurrentSession(first->getSource()) && !isRemoteEffectInCurrentSession(first->getSource(), first->getRemoteIP(), first->getRemotePort());
        
        //Copy of the source File in the CurrentSession Source Folder
        QString copySource = fSourcesFolder +"/" + first->getName() + ".dsp";
        QString toCopy = first->getSource();
        
        update_Source(toCopy, copySource);
        
        printf("Update Source\n");
        
        if(error.compare("") != 0){
            fErrorWindow->print_Error(error);
        }
        
        int x, y;
        calculate_position(val, &x, &y);
        
        FLWindow* win = new FLWindow(fWindowBaseName, val, first, x, y, fSessionFolder);
        
        printf("WIN = %p\n", win);
        
        redirectMenuToWindow(win);
        
        if(win->init_Window(init, error)){
            
            printf("INIT\n");
            
            FLW_List.push_back(win);
            addWinToSessionFile(win);
            
            first->launch_Watcher();
            
// ?????        
//            if(optionChanged){
//                printf("update_compilationOptions\n");
//                first->update_compilationOptions(fCompilationMode, fOpt_level);
//            }
            return win;
        }
        else{
            delete win;
            return NULL;
        }
    }
    else
        return NULL;
    
}

//--Creation accessed from Menu
void FLApp::create_New_Window(const QString& source){
    
    QString error("");
    
    if(new_Window(source, error) == NULL){
        fErrorWindow->print_Error(error);
        printf("NULLLLLLLLLL\n");
    }
    
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
        QString name = fileName;
        
        if(name.indexOf(".tar") != -1)
            recall_Snapshot(name, true);
        if(name.indexOf(".dsp") != -1)
            create_New_Window(name);
        return true;
    } 
    
    return QApplication::event(ev);
}

//--Open a dsp from disk
void FLApp::open_New_Window(){ 
    
    FLWindow* win = getActiveWin();
    
    QStringList fileNames = QFileDialog::getOpenFileNames(NULL, tr("Open one or several DSPs"), "",tr("Files (*.dsp)"));
    
    QStringList::iterator it;
    
    for(it = fileNames.begin(); it != fileNames.end(); it++){
        
        if((*it) != ""){
            QString inter(*it);
            
            if(win != NULL && win->is_Default())
                update_SourceInWin(win, inter);
            else
                create_New_Window(inter);
        }
    }
}

//--------------OPEN EXAMPLE

void FLApp::open_Example_From_FileMenu(){

    QAction* action = qobject_cast<QAction*>(sender());
    QString toto(action->data().toString());
        
    open_Example_Action(toto);
}

void FLApp::open_Example_Action(QString pathInQResource){
    
    QFileInfo toOpen(pathInQResource);
    fExampleToOpen = toOpen.baseName();
    open_Example_Action();
    
}

void FLApp::open_Example_Action(){
    
    QString pathInSession = fExamplesFolder + "/" + fExampleToOpen + ".dsp";
    
    fPresWin->hide();
        
    FLWindow* win = getActiveWin();
    
    if(win != NULL && win->is_Default())
        update_SourceInWin(win, pathInSession);
    else
        create_New_Window(pathInSession);    
}

//-------------OPEN RECENT

//--Save/Recall from file 
void FLApp::save_Recent_Files(){
    
    QFile f(fRecentsFile);
    
    if(f.open(QFile::WriteOnly | QFile::Truncate)){
        
        QTextStream text(&f);
        
        QList<std::pair<QString, QString> >::iterator it;
        
        for (it = fRecentFiles.begin(); it != fRecentFiles.end(); it++) {
            QString toto = it->first;
            QString tata = it->second;
            text << toto <<' '<<tata<< endl;
        }
        f.close();
    }
}

void FLApp::recall_Recent_Files(const QString& filename){
    
    QFile f(filename);
    QString toto, titi;
    int i=0;
    
    if(f.open(QFile::ReadOnly)){
        
        QTextStream textReading(&f);
        
        while(!textReading.atEnd() && i<kMAXRECENTFILES){
            textReading >> toto >> titi;
            QString tata = toto;
            QString tutu = titi;
            i++;
            
            if(tata.compare("") != 0)
                set_Current_File(tata, tutu);
        }
        f.close();
    }
    
    QList<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++)
        (*it)->set_RecentFile(fRecentFiles);
}

//--Add new recent file
void FLApp::set_Current_File(const QString& pathName, const QString& effName){
    
    std::pair<QString,QString> myPair;
	myPair = make_pair(pathName, effName);
    
    fRecentFiles.removeOne(myPair);
    fRecentFiles.push_front(myPair);
    
    update_Recent_File();
    
    QList<FLWindow*>::iterator it;
    
    for(it = FLW_List.begin(); it!=FLW_List.end() ; it++){
        (*it)->set_RecentFile(fRecentFiles);
        (*it)->update_RecentFileMenu();
    }
        
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
        update_SourceInWin(win, toto);
    else
        create_New_Window(toto);
}

//--------------------------------SESSION

//Write Current Session Properties into a File
void FLApp::sessionContentToFile(){

    QString filename = fSessionFile;
    
    QFile f(filename);
    
    if(f.open(QFile::WriteOnly | QIODevice::Truncate)){
        
        QList<WinInSession*>::iterator it;
        
        QTextStream textWriting(&f);
        
        for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
            
            textWriting<<(*it)->ID<<' '<<(*it)->source<<' '<<(*it)->name<<' '<<(*it)->x<<' '<<(*it)->y<<' '<<(*it)->opt_level<<' '<<(*it)->oscPort<<' '<<(*it)->portHttpd<<' '<<(*it)->compilationOptions<<endl;            
        }
        f.close();
    }
}

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

//Read Session File
void FLApp::fileToSessionContent(const QString& filename, QList<WinInSession*>* session){
    
    QFile f(filename);
    
    if(f.open(QFile::ReadOnly)){
        
        QList<WinInSession*>::iterator it;
        
        QTextStream textReading(&f);
        
        while(!textReading.atEnd()){
            
            QString line = textReading.readLine();
                
            WinInSession* intermediate = new WinInSession;
            intermediate->ID = parseNextOption(line).toInt();
            intermediate->source = parseNextOption(line);
            intermediate->name = parseNextOption(line);
            intermediate->x = parseNextOption(line).toFloat();
            intermediate->y = parseNextOption(line).toFloat();
            intermediate->opt_level = parseNextOption(line).toInt();
            intermediate->oscPort = parseNextOption(line).toInt();
            intermediate->portHttpd = parseNextOption(line).toInt();
        
            while(line.size() != 0){
                intermediate->compilationOptions += parseNextOption(line);
                
                if(line.size() != 0)
                    intermediate->compilationOptions += ' ';
            }
                
            session->push_back(intermediate);
        }
        f.close();
    }
}

//--------------RECENTLY OPENED

//--Save/Recall from file
void FLApp::save_Recent_Sessions(){
    
    QFile f(fHomeRecentSessions);
    
    if(f.open(QFile::WriteOnly | QFile::Truncate)){
        
        QTextStream text(&f);
        
        for (int i = min(kMAXRECENTSESSIONS, fRecentSessions.size()) - 1; i>=0; i--) {
            QString toto = fRecentSessions[i];
            text << toto << endl;
        }
        f.close();      
    }

}

void FLApp::recall_Recent_Sessions(const QString& filename){
    
    QFile f(filename);
    QString toto;
    
    int i=0;
    
    if(f.open(QFile::ReadOnly)){
        
        QTextStream textReading(&f);
        
        while(!textReading.atEnd() && i<kMAXRECENTSESSIONS){
            
            textReading >> toto;
            set_Current_Session(toto);
            i++;
        }
        f.close();
    }
    
    QList<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++)
        (*it)->set_RecentSession(fRecentSessions);
}

//Add new recent session
void FLApp::set_Current_Session(const QString& pathName){
    
    //    printf("SET CURRENT SESSION = %s\n", pathName);
    
    QString currentSess = pathName;
    fRecentSessions.removeAll(currentSess);
    fRecentSessions.prepend(currentSess);
    update_Recent_Session();
    
    QList<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++){
        
        (*it)->set_RecentSession(fRecentSessions);
        (*it)->update_RecentSessionMenu();
    }
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
            
            //            printf("TEXT = %s\n", text.toStdString());
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

//Add window in Current Session Structure
void FLApp::addWinToSessionFile(FLWindow* win){
    
    QString compilationOptions = win->get_Effect()->getCompilationOptions();
    
    WinInSession* intermediate = new WinInSession;
    intermediate->ID = win->get_indexWindow();
    intermediate->source = win->get_Effect()->getSource();
    intermediate->name = win->get_Effect()->getName();
    intermediate->x = (float)win->get_x()/(float)fScreenWidth;
    intermediate->y = (float)win->get_y()/(float)fScreenHeight;
    intermediate->compilationOptions = compilationOptions;
    intermediate->opt_level = win->get_Effect()->getOptValue();
    intermediate->oscPort= win->get_oscPort();
    intermediate->portHttpd = win->get_Port();
    intermediate->isLocal = win->get_Effect()->isLocal();
    
    fSessionContent.push_back(intermediate);
    
    QString name = win->get_nameWindow();
    name+=" : ";
    name+= win->get_Effect()->getName();
    
    printf("ADD = %s\n", name.toStdString().c_str());
    
    QAction* newWin = new QAction(name, fNavigateMenu);
    fFrontWindow.push_back(newWin);
    
    newWin->setData(QVariant(name));
    connect(newWin, SIGNAL(triggered()), win, SLOT(frontShowFromMenu()));
    
//    Update Navigate Menu For App & Wins
    fNavigateMenu->clear();
    for(QList<QAction*>::iterator ite = fFrontWindow.begin(); ite != fFrontWindow.end() ; ite++){
        fNavigateMenu->addAction(*ite);
    }
    
    QList<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++)
        (*it)->updateNavigateMenu(fFrontWindow);
}

//Delete window from Current Session Structure
void FLApp::deleteWinFromSessionFile(FLWindow* win){
    
    QString toto = win->get_Effect()->getSource();
    QString tutu = win->get_Effect()->getName();
    if(toto.indexOf(fSourcesFolder) == -1)
        set_Current_File(toto, tutu);
    
    QList<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it++){
        
        if((*it)->ID == win->get_indexWindow()){
            
            fSessionContent.removeOne(*it);
            
            QList<QAction*>::iterator it2;
            for(it2 = fFrontWindow.begin(); it2 != fFrontWindow.end() ; it2++){
                
                QString name = win->get_nameWindow();
                name+=" : ";
                name+= win->get_Effect()->getName();
                
                printf("DELETE = %s\n", name.toStdString().c_str());
                
                if((*it2)->text().compare(name) == 0){
                    fFrontWindow.removeOne(*it2);
                    
                    fNavigateMenu->clear();

                    for(QList<QAction*>::iterator ite = fFrontWindow.begin(); ite != fFrontWindow.end() ; ite++)
                        fNavigateMenu->addAction(*ite);
                    
                    QList<FLWindow*>::iterator it3;
                    
                    for (it3 = FLW_List.begin(); it3 != FLW_List.end(); it3++)
                            (*it3)->updateNavigateMenu(fFrontWindow);
                    
                    break;
                }
            }
            break;
        }
    }
    
    QList<FLWindow*>::iterator it2;
    
    for (it2 = FLW_List.begin(); it2 != FLW_List.end(); it2++){
        
        (*it2)->set_RecentFile(fRecentFiles);
        (*it2)->update_RecentFileMenu();
    }
}

//Update Current Session Structure with current parameters of the windows
void FLApp::update_CurrentSession(){
    
    QList<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++){
        
        deleteWinFromSessionFile(*it);
        addWinToSessionFile(*it);
        
        (*it)->save_Window();
    }
}

//Sorting out the element of the Current Session that have to be copied in the snapshot
void FLApp::createSnapshotFolder(const QString& snapshotFolder){
    
    QDir nv(snapshotFolder);
    nv.mkdir(snapshotFolder);
    
    QFile descriptionFile(fSessionFile);
    QString descFileCpy = snapshotFolder + "/Description.txt";
    
    printf("Description File saved = %s || Original = %s\n", descFileCpy.toStdString().c_str(), fSessionFile.toStdString().c_str());
    
    descriptionFile.copy(descFileCpy);
    
    QString ss(snapshotFolder);
    ss += "/Sources";
    nv.mkdir(ss);
    
    QString svg(snapshotFolder);
    svg += "/SVG";
    nv.mkdir(svg);
    
    QString ir(snapshotFolder);
    ir += "/IR";
    nv.mkdir(ir);
    
    for(QList<FLWindow*>::iterator it = FLW_List.begin(); it != FLW_List.end(); it++){
        if(isSourceInCurrentSession((*it)->get_Effect()->getSource())){
        
//            COPY WINDOW FOLDER
            QString winFolder = fSessionFolder + "/" + (*it)->get_nameWindow();
            QString winFolderCpy = snapshotFolder + "/" + (*it)->get_nameWindow();
            
            cpDir(winFolder, winFolderCpy);
            
//            COPY IR FILE
            QString irFile = fIRFolder + "/" + (*it)->get_Effect()->getName();
            QString irFileCpy = ir + "/" + (*it)->get_Effect()->getName();
            
            QFile irF(irFile);
            irF.copy(irFileCpy);
            
//            COPY SOURCE SAVED
            QString sourceFile = fSourcesFolder + "/" + (*it)->get_Effect()->getName() + ".dsp";
            QString sourceFileCpy = ss + "/" + (*it)->get_Effect()->getName() + ".dsp";
            
            QFile sourceF(sourceFile);
            sourceF.copy(sourceFileCpy);
            
//            COPY SVG
            QString svgFolder = fSVGFolder + "/" + (*it)->get_Effect()->getName()/* + "-svg"*/;
            QString svgFolderCpy = svg + "/" + (*it)->get_Effect()->getName() /*+ "-svg"*/;
            
            cpDir(svgFolder, svgFolderCpy);
        }
    }
}

//Reset Current Session Folder
void FLApp::reset_CurrentSession(){
    
    QDir srcDir(fSessionFolder);    
    
    deleteDirectoryAndContent(fSourcesFolder);
    deleteDirectoryAndContent(fSVGFolder);
    deleteDirectoryAndContent(fIRFolder);
    
    QFileInfoList children = srcDir.entryInfoList(QDir::Dirs | QDir::Drives | QDir::NoDotAndDotDot);
    
    QFileInfoList::iterator it;
    
    for(it = children.begin(); it != children.end(); it++){
        
        QString toRemove(it->absoluteFilePath());
        
        if(toRemove.indexOf("FLW-") != -1)
            deleteDirectoryAndContent(toRemove);
    }
    
    QDir nv(fSessionFolder);
    QString ss(fSessionFolder);
    ss += "/Sources";
    
    if(!QFileInfo(ss).exists())
        nv.mkdir(ss);
    
    QString svg(fSessionFolder);
    svg += "/SVG";
    
    if(!QFileInfo(svg).exists())
        nv.mkdir(svg);
    
    QString ir(fSessionFolder);
    ir += "/IR";
    
    if(!QFileInfo(ir).exists())
        nv.mkdir(ir);


    fSessionContent.clear();
    
    recall_Settings(fSettingsFolder);
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
        sessionContentToFile();
        
//------ Copy of current Session under a new name, at a different location
//----------- Not everything has to be copied, because some resources are kept for recycling
//----------- but are not usefull in Snapshot
        
    	printf("filename = %s\n", filename.toStdString().c_str());    
        createSnapshotFolder(filename);
        
//        cpDir(fSessionFolder, filename);
        
#ifndef _WIN32
        
        display_CompilingProgress("Saving your session...");
        
        QProcess myCmd;
        QByteArray error;
        
        QString systemInstruct("tar cfv ");
        systemInstruct += filename + ".tar " + filename;
        
        myCmd.start(systemInstruct);
        myCmd.waitForFinished();
        
        error = myCmd.readAllStandardError();
        
        if(myCmd.readChannel() == QProcess::StandardError )
            fErrorWindow->print_Error(error.data());
        
        QProcess myCmd2;
        QByteArray error2;
        
        QString rmInstruct("rm -r ");
        rmInstruct += filename;
        
        myCmd2.start(rmInstruct);
        myCmd2.waitForFinished();
        
        error2 = myCmd2.readAllStandardError();
        
        if(myCmd2.readChannel() == QProcess::StandardError )
            fErrorWindow->print_Error(error2.data());
        
        QString sessionName =  filename + ".tar ";
        set_Current_Session(sessionName);
        
        StopProgressSlot();
        //        deleteDirectoryAndContent(filename);
#endif
    }
}

//---------------RESTORE SNAPSHOT FUNCTIONS

//Behaviour of session restoration when opening a snapshot
//The user is notified that the backup file has been used to restore exact state.
void FLApp::snapshotRestoration(const QString& file, QList<WinInSession*>* session){
    
    //If 2 windows are pointing on the same lost source, the Dialog has not to appear twice
    std::map<QString,bool> updated;
    
    std::list<std::pair<std::string,std::string> > sourceChanges;
    
    QList<WinInSession*>::iterator it;
    for(it = session->begin() ; it != session->end() ; it++){
        
        QFileInfo infoSource((*it)->source);
        
        QString contentOrigin("");
        contentOrigin = pathToContent((*it)->source);
        
        QString sourceSaved = QFileInfo(file).absolutePath() + "/Sources/" + (*it)->name + ".dsp";
        QString contentSaved("");
         contentSaved = pathToContent(sourceSaved);
        
        //If one source (not in the Source folder) couldn't be found, the User is informed that we are now working on the copy
        if(updated.find((*it)->source) == updated.end() && infoSource.absolutePath().compare(fSourcesFolder) != 0  && (!infoSource.exists() || contentSaved.compare(contentOrigin) != 0)){
            
            QString error;
            
            if(!infoSource.exists()){
                error = "\nWARNING = ";
                error += (*it)->source;
                error += " cannot be found! It is reloaded from a copied file.";
            }
            else if(contentSaved.compare(contentOrigin) != 0){
                error = "\nWARNING = The content of ";
                error += (*it)->source;
                error += " has been modified! It is reloaded from a copied file.";
            }
            fErrorWindow->print_Error(error);
            QString newSource = fSourcesFolder + "/" + (*it)->name + ".dsp";
            updated[(*it)->source] = true;       
            sourceChanges.push_back(make_pair((*it)->source.toStdString(), newSource.toStdString()));
            
            (*it)->source = newSource;
        }
        else if(updated.find((*it)->source) != updated.end()){
            
            std::list<std::pair<std::string,std::string> >::iterator itS;
            
            for(itS = sourceChanges.begin(); itS != sourceChanges.end() ; itS++){
                if(itS->first.compare((*it)->source.toStdString()) == 0)
                    (*it)->source = itS->second.c_str();
            }
        }
    }
}

//Behaviour of session restoration when re-starting the application
//The user is notified in case of source file lost or modified. He can choose to reload from original file or backup.
void FLApp::currentSessionRestoration(QList<WinInSession*>& session){
    
    //If 2 windows are pointing on the same lost source, the Dialog has not to appear twice
    std::map<QString,bool> updated;
    
    //List of the sources to updated in Session File
    std::list<std::pair<std::string,std::string> > sourceChanges;
    
    QList<WinInSession*>::iterator it;
    
    for(it = session.begin() ; it != session.end() ; it++){
        
        QString contentOrigin("");
        QString contentSaved("");
        contentOrigin = pathToContent((*it)->source);
        QString sourceSaved = fSourcesFolder + "/" + (*it)->name + ".dsp";
        contentSaved = pathToContent(sourceSaved);
        
//        Original source 
        QFileInfo infoSource((*it)->source);
        
        //If the source lose was NOT already handled &&
        //If the source is NOT in the Source folder &&
        //If this source could NOT be found or that its content was modified &&
        //If the copied file exists
        // THEN the user is asked if he wants to reload his effect from original or copied file
        if(updated.find((*it)->source) == updated.end() && 
           (infoSource.absolutePath().compare(fSourcesFolder) != 0 && 
           ((!infoSource.exists() || (contentSaved.compare(contentOrigin) != 0)) &&
            QFileInfo(sourceSaved).exists()))){
            
            QString mesg;
            bool contentModif = false;
            
            QMessageBox* existingNameMessage = new QMessageBox(QMessageBox::Warning, tr("Notification"), mesg);
            QPushButton* yes_Button;
            QPushButton* cancel_Button; 
            
            if(!infoSource.exists()){
                mesg = (*it)->source + " cannot be found! Do you want to reload it from a copied file?";
                
                yes_Button = existingNameMessage->addButton(tr("Yes"), QMessageBox::AcceptRole);
                cancel_Button = existingNameMessage->addButton(tr("No"), QMessageBox::RejectRole);
            }
            else{
                mesg = "The content of " + (*it)->source + " has been modified, do you want to reload it from a copied file?";
                    contentModif = true;
                    
                yes_Button = existingNameMessage->addButton(tr("Copied File"),QMessageBox::AcceptRole);
                cancel_Button = existingNameMessage->addButton(tr("Original File"), QMessageBox::RejectRole);
            }
            
            existingNameMessage->setText(mesg);
            
            existingNameMessage->exec();
            updated[(*it)->source] = true;
            
//            ORIGINAL SOURCE CASE
            if (existingNameMessage->clickedButton() == cancel_Button) {
                
                QString toErase = fSourcesFolder + "/" + (*it)->name + ".dsp";
                removeFilesOfEffect(toErase, (*it)->name);
                
                //ORIGINAL - Source Erased - Copy not used --> Effect erased
                if(!contentModif)
                    deleteLineIndexed((*it)->ID, session);
                //ORIGINAL - Content modified
                else{
                    QString newSource = fSourcesFolder + "/" + (*it)->name + ".dsp";
                    QFile file((*it)->source);
                    file.copy(newSource);
                }
            }    
//          COPIED SOURCE CASE
            else{
                
                QString newSource = fSourcesFolder + "/" + (*it)->name + ".dsp";
                sourceChanges.push_back(make_pair((*it)->source.toStdString(), newSource.toStdString()));
                (*it)->source = newSource;
                infoSource = QFileInfo((*it)->source);
            }
            
            delete existingNameMessage;
            
        }
        //If the source was in the Source Folder and couldn't be found, it can't be reloaded.
        if(updated.find((*it)->source) == updated.end() && infoSource.absolutePath().compare(fSourcesFolder) == 0 && !infoSource.exists()){
            
            deleteLineIndexed((*it)->ID, session);
            QString msg = "\n" + (*it)->name + " could not be reload. The File is lost!"; 
            fErrorWindow->print_Error(msg);
            removeFilesOfEffect((*it)->source, (*it)->name);
        }
//        Lost Source was already handled
        else if(updated.find((*it)->source) != updated.end()){
        
            std::list<std::pair<std::string,std::string> >::iterator itS;
            
            for(itS = sourceChanges.begin(); itS != sourceChanges.end() ; itS++){
                if(itS->first.compare((*it)->source.toStdString()) == 0)
                    (*it)->source = itS->second.c_str();
                
            }
        }

//        If the copy was lost, it has to be recopied
        else if(!QFileInfo(sourceSaved).exists()){
            QFile toCopy((*it)->source);
            toCopy.copy(sourceSaved);
        }
    }
}

void FLApp::recallSnapshotFromMenu(){
	
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

//@param : filename = snapshot that is loaded
//@param : importOption = false for recalling | true for importing
void FLApp::recall_Snapshot(const QString& filename, bool importOption){ 
        
	fRecalling = true;

    set_Current_Session(filename);
    
#ifndef _WIN32
        
    display_CompilingProgress("Uploading your snapshot...");

    QProcess myCmd;
	QByteArray error;
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    
    QString systemInstruct("tar xfv ");
    systemInstruct += filename +" -C /";
    
	printf("Recall process\n");

	myCmd.setProcessEnvironment(env);
    myCmd.start(systemInstruct);
    myCmd.waitForFinished();
    
    error = myCmd.readAllStandardError();
    
    if(myCmd.readChannel() == QProcess::StandardError )
        fErrorWindow->print_Error(error.data());
    
    StopProgressSlot();
#endif

    if(!importOption){
        shut_AllWindows();
        reset_CurrentSession();
        fExecutedEffects.clear();
        fRemoteEffects.clear();
    }
    
    QString finalFile = QFileInfo(filename).canonicalPath() + "/" + QFileInfo(filename).baseName();
    
    QString finalFilename = finalFile + "/Description.txt";
    
    recall_Session(finalFilename);
    
#ifndef _WIN32
    
    display_CompilingProgress("Ending upload...");
    
    QProcess myCmd2;
    QByteArray error2;
    
    QString rmInstruct("rm -r ");
    rmInstruct += finalFile;
    
    myCmd2.start(rmInstruct);
    myCmd2.waitForFinished();
    
    error2 = myCmd2.readAllStandardError();

    if(myCmd2.readChannel() == QProcess::StandardError )
        fErrorWindow->print_Error(error2.data());
    
    StopProgressSlot();
#endif

	fRecalling = false;
}

//Recall for any type of session (current or snapshot)
//@param : filename = snapshot that is loaded
void FLApp::recall_Session(const QString& filename){
    
    printf("FOlder recalled = %s\n", filename.toStdString().c_str());
    
    //Temporary copies of the description files in which all the modifications due to conflicts will be saved
    QList<WinInSession*>  snapshotContent;
    
    fileToSessionContent(filename, &snapshotContent);
    printf("SIZE OF SNAPSHOT CONTENT = %i\n", snapshotContent.size());
    
    if(filename.compare(fSessionFile) == 0){
        
        //Reset current Session File to avoid the return true in all the isInCurrentSession? when recalling currentSession
        
        //In case of a disappearance of a source file
        currentSessionRestoration(snapshotContent);
    }
    else{
        //Different resolution of disappearance for a snapshot 
        snapshotRestoration(filename, &snapshotContent);
    }
    
    printf("SIZE OF SNAPSHOT CONTENT = %i\n", snapshotContent.size());
    
    //------------Resolution of the name conflicts (Window Names & Effect Names)
    QList<std::pair<int,int> > indexChanges = establish_indexChanges(&snapshotContent);
    
    std::list<std::pair<std::string,std::string> > windowNameChanges;
    
    QList<std::pair<int, int> >::iterator it2;
    for(it2 = indexChanges.begin(); it2 != indexChanges.end(); it2++){
        
        string nameW = fWindowBaseName.toStdString() + "-" + QString::number(it2->first).toStdString();
        
        string newNameW = fWindowBaseName.toStdString() + "-" + QString::number(it2->second).toStdString();
        
        windowNameChanges.push_back(make_pair(nameW, newNameW));
    }
    
    std::list<std::pair<std::string,std::string> > nameChanges = establish_nameChanges(&snapshotContent);
    
    //In case of Snapshot Restoration, sources and window parameters Folders have to be copied in currentSession
    if((QFileInfo(filename).absolutePath()).compare(fSessionFolder) != 0){
        establish_sourceChanges(nameChanges, &snapshotContent);
        
        QFileInfo sourceDir(filename);
        QString snapshotSourcesFolder = sourceDir.absolutePath() + "/Sources"; 
        copy_AllSources(snapshotSourcesFolder, fSourcesFolder, nameChanges,".dsp");
        QString snapshotIRFolder = sourceDir.absolutePath() + "/IR"; 
        copy_AllSources(snapshotIRFolder, fIRFolder, nameChanges, "");
        QString snapshotSVGFolder = sourceDir.absolutePath() + "/SVG";         
        copy_SVGFolders(snapshotSVGFolder, fSVGFolder, nameChanges);
        
        QString snapshotFolder = sourceDir.absolutePath();
        
        copy_WindowsFolders(snapshotFolder, fSessionFolder, windowNameChanges);
        
        
    }//Otherwise, one particular case has to be taken into account. If the content was modified and the Effect NOT recharged from source. The original has to be recopied!??
    else{
        
    }
    
    //--------------Recalling without conflict the session
    
    QList<WinInSession*>::iterator it;
    
    for(it = snapshotContent.begin() ; it != snapshotContent.end() ; it ++){
        
        QString error;
        
        (*it)->compilationOptions = (*it)->compilationOptions;
        
        FLEffect* newEffect = getEffectFromSource((*it)->source, (*it)->name, fSourcesFolder, (*it)->compilationOptions, (*it)->opt_level, error, true, true);
        
        //ICI ON NE VA PAS FAIRE LA COPIE DU FICHIER SOURCE!!!
        
        if(newEffect != NULL){
            
            if(error.compare("") != 0){
                fErrorWindow->print_Error(error);
            }
            
            if(FLW_List.size() >= numberWindows){
                fErrorWindow->print_Error("You cannot open more windows. If you are not happy with this limit, feel free to contact us : research.grame@gmail.com");
                return;
            }
            
            FLWindow* win = new FLWindow(fWindowBaseName, (*it)->ID, newEffect, (*it)->x*fScreenWidth, (*it)->y*fScreenHeight, fSessionFolder, (*it)->oscPort, (*it)->portHttpd);
            
            redirectMenuToWindow(win);
            
            //Modification of connection files with the new window & effect names
            
            win->update_ConnectionFile(windowNameChanges);
            win->update_ConnectionFile(nameChanges);
            
            if(win->init_Window(kNoInit, error)){
                
                FLW_List.push_back(win);
                newEffect->launch_Watcher();
                addWinToSessionFile(win);
                win->save_Window();
                
                //In case the compilation options have changed...
                if((*it)->compilationOptions.compare(newEffect->getCompilationOptions()) != 0)
                    newEffect->update_compilationOptions((*it)->compilationOptions, (*it)->opt_level);
            }
            else{
                delete win;
                fErrorWindow->print_Error(error);    
            }
        }
        else{
            fErrorWindow->print_Error(error);
        }
    }
    
//    If the current session is not recalled for some reason, the presentation menu is opened
    if(filename.compare(fSessionFile) == 0){
        if(FLW_List.size() == 0)
            show_presentation_Action();
    }
}


//---------------RENAMING AND ALL FUNCTIONS TO IMPORT

void FLApp::deleteLineIndexed(int index, QList<WinInSession*>& sessionToModify){
    
    QList<WinInSession*>::iterator it;
    
    for(it = sessionToModify.begin(); it != sessionToModify.end() ; it++){
        
        //Check if line wasn't empty
        if((*it)->ID == index){
            sessionToModify.removeOne(*it);
            break;    
        }
    }
}

QList<std::pair<int, int> > FLApp::establish_indexChanges(QList<WinInSession*>* session){
    
    //For each window of the session File, if it's index is used (in current session or by the previous windows re-indexed), it is associated to a new one (the smallest not used)
    
    QList<std::pair<int,int> > returning;
    
    QList<WinInSession*>::iterator it;
    
    QList<int> currentIndexes = get_currentIndexes();
    
    for(it = session->begin(); it != session->end() ; it++){
        
        int newID = (*it)->ID;
        
        if(isIndexUsed((*it)->ID, currentIndexes))
            newID = find_smallest_index(currentIndexes);
        
        currentIndexes.push_back(newID);
        returning.push_back(make_pair((*it)->ID, newID));
        (*it)->ID = newID;
    }
    return returning;
}

std::list<std::pair<std::string,std::string> > FLApp::establish_nameChanges(QList<WinInSession*>* session){
    
    //For each window of the session File, if it's index is used (in current session or by the previous windows re-indexed), it is associated to a new one (the smallest not used)
    
    std::list<std::pair<std::string,std::string> > nameChanges;
    QList<QString> currentDefault = get_currentDefault();
    
    //If 2 windows are holding the same name & source, to avoid renaming them twice
    std::map<QString,bool> updated;
    
    QList<WinInSession*>::iterator it;
    
    for(it = session->begin(); it != session->end() ; it++){
        
        QString newName = (*it)->name;
        

        //1- If the source is already is current Session (& not pointing in Sources Folder) ==> getName already given
        if(isSourceInCurrentSession((*it)->source) && QFileInfo((*it)->source).absolutePath().compare(fSourcesFolder) != 0){
            
            newName = getNameEffectFromSource((*it)->source);
            nameChanges.push_front(make_pair((*it)->name.toStdString(), newName.toStdString()));
            (*it)->name = newName;
        }
        
        //2- If the name was already updated ==> don't do nothing
        else if(updated.find(newName) != updated.end()){
            
            std::list<std::pair<std::string,std::string> >::iterator it2;
            
            for(it2 = nameChanges.begin(); it2 != nameChanges.end(); it2++){
                
                if(it2->first.compare((*it)->name.toStdString()) == 0){
                    (*it)->name = it2->second.c_str();
                    break;
                }
            }
            
        }
        
        //3- If the name is a DefaultName
        //==> if the Name is not used in current Session, we keep it and inform of its use
        //==>if it is used, it is renamed as the smallest available and we inform of its use
        
        else if(newName.indexOf(DEFAULTNAME) != -1){
            bool found = false;
            QList<QString>::iterator it2;
            for(it2 = currentDefault.begin(); it2 != currentDefault.end(); it2++){
                if(newName.compare(*it2) == 0){
                    found = true;
                    break;
                }
            }
            if(!found){
                
                //                printf("NO FOUND thE DEFAULT NAME\n");
                currentDefault.push_back(newName);
                nameChanges.push_front(make_pair((*it)->name.toStdString(), newName.toStdString()));
                updated[(*it)->name] = true;
                (*it)->name = newName;
            }
            else{
                //                printf("Found the default name\n");
                newName = find_smallest_defaultName(currentDefault);
                currentDefault.push_back(newName);
                nameChanges.push_front(make_pair((*it)->name.toStdString(), newName.toStdString()));
                updated[(*it)->name] = true;
                (*it)->name = newName;
            }
        }
        //4- If the source is in current Folder and its name is already used, it has to be renamed
        else if(QFileInfo((*it)->source).absolutePath().compare(fSourcesFolder) == 0){
            
            newName = renameEffect("", newName, true);
            
            nameChanges.push_front(make_pair((*it)->name.toStdString(), newName.toStdString()));
            updated[(*it)->name] = true;
            (*it)->name = newName;
        }
        //5- If the Name is in current session, is has to be renamed
        else{
            
            newName = renameEffect((*it)->source, newName, true);
            
            nameChanges.push_front(make_pair((*it)->name.toStdString(), newName.toStdString()));
            updated[(*it)->name] = true;
            (*it)->name = newName;
        }
    }
    
    return nameChanges;
}

void FLApp::establish_sourceChanges(std::list<std::pair<std::string,std::string> > nameChanges, QList<WinInSession*>* session){
    
    //For all the effects which source is in the current source Folder, the path in the description file has to be changed with the new name
    
    std::list<std::pair<std::string,std::string> > sourceChanges;
    
    QList<WinInSession*>::iterator it;
    
    std::list<std::pair<std::string,std::string> >::iterator it2;
    for(it2 = nameChanges.begin(); it2 != nameChanges.end() ; it2++){
        
        for(it = session->begin(); it != session->end() ; it++){
            
            if(QFileInfo((*it)->source).absolutePath().compare(fSourcesFolder)== 0){
                
                QString inter(it2->first.c_str());
                QFileInfo sourceInfo((*it)->source);
                QString inter2 = sourceInfo.baseName();
                
                if(inter2.compare(inter) == 0){
                    QString inter = fSourcesFolder + "/" + it2->second.c_str() + ".dsp";
                    (*it)->source = inter;
                }
            }
            
        }
    }
}

void FLApp::copy_AllSources(const QString& srcDir, const QString& dstDir, std::list<std::pair<std::string,std::string> > nameChanges, const QString extension){
    
    //Following the renaming table, the sources are copied from snapshot Folder to current Session Folder
    
    QDir src(srcDir);
    
    QFileInfoList chilDirs = src.entryInfoList(QDir::Files);
    
    QFileInfoList::iterator it;
    for(it = chilDirs.begin(); it!=chilDirs.end(); it++){
        
        string name = it->baseName().toStdString();
        
        std::list<std::pair<std::string,std::string> >::iterator it2;
        
        QFile source(it->filePath());
        
        for(it2 = nameChanges.begin(); it2 != nameChanges.end() ; it2++){
            if(name.compare(it2->first) == 0)
                name = it2->second;
        }
        
        QString newDir = dstDir + "/" + name.c_str() + extension;
        source.copy(newDir);
    }
    
}

void FLApp::copy_WindowsFolders(const QString& srcDir, const QString& dstDir, std::list<std::pair<std::string, std::string> > windowNameChanges){
    
    //Following the renaming table, the Folders containing the parameters of the window are copied from snapshot Folder to current Session Folder
    
    QDir src(srcDir);
    
    QFileInfoList chilDirs = src.entryInfoList(QDir::AllDirs);
    
    QFileInfoList::iterator it;
    
    for(it = chilDirs.begin(); it!=chilDirs.end(); it++){
        
        std::list<std::pair<std::string,std::string> >::iterator it2;
        
        string name = it->baseName().toStdString();
        
        for(it2 = windowNameChanges.begin(); it2 !=windowNameChanges.end() ; it2++){
            
            if(name.compare(it2->first) == 0){
                QString newDir(dstDir);
                newDir += "/";
                newDir += it2->second.c_str();
                
                QDir dir(newDir);
                dir.mkdir(newDir);
                
                QString fileRC = it->filePath() + "/" + it2->first.c_str() + ".rc";
                QString newFileRC = newDir + "/" + it2->second.c_str() +".rc";
                QFile toCopy(fileRC);
                toCopy.copy(newFileRC);
                
                QString fileJC = it->filePath() + "/" + it2->first.c_str() + ".jc";
                QString newFileJC = newDir + "/"+ it2->second.c_str() + ".jc";
                QFile toCpy(fileJC);
                toCpy.copy(newFileJC);
            }
        }
    }
    
}

void FLApp::copy_SVGFolders(const QString& srcDir, const QString& dstDir, std::list<std::pair<std::string,std::string> > nameChanges){
    
    QDir src(srcDir);
    
    QFileInfoList chilDirs = src.entryInfoList(QDir::AllDirs);
    
    QFileInfoList::iterator it;
    
    for(it = chilDirs.begin(); it!=chilDirs.end(); it++){
        
        std::list<std::pair<std::string,std::string> >::iterator it2;
        
        QString name = it->baseName();
        
        for(it2 = nameChanges.begin(); it2 !=nameChanges.end() ; it2++){
            
            QString toCompare = it2->first.c_str();
//            toCompare += "-svg";
            
            if(name.compare(toCompare) == 0){
                QString newDir = dstDir + "/" +it2->second.c_str()/* + "-svg"*/;
                printf("COPYING = %s TO %s\n", it->absoluteFilePath().toStdString().c_str(), newDir.toStdString().c_str());
                //                if(!QFileInfo(newDir).exists())
                cpDir(it->absoluteFilePath(), newDir);
            }
            
            
        }
    }
    
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
    
    //    printf("CLOSE ALL WINDOWS\n");
    
    display_Progress();
    
    update_CurrentSession();
    sessionContentToFile();
    
//    ICI, IL FAUT SUPPRIMER LES RESSOURCES LIEES AUX EFFETS NON UTILISÉS à la fin de la session.

    QList<FLEffect*>::iterator it2;
    for(it2 = fExecutedEffects.begin() ;it2 != fExecutedEffects.end(); it2++){
        if(!isSourceInCurrentSession((*it2)->getSource())){
            QString toErase = fSourcesFolder + "/" + (*it2)->getName() + ".dsp";
            removeFilesOfEffect(toErase, (*it2)->getName());
        }
    }
    
    QList<FLWindow*>::iterator it;
    
    for(it = FLW_List.begin(); it != FLW_List.end(); it++){
        
        QString toto = (*it)->get_Effect()->getSource();
        QString tutu = (*it)->get_Effect()->getName();
        
        if(toto.indexOf(fSourcesFolder) == -1)
            set_Current_File(toto, tutu);
        
        (*it)->close_Window();
        (*it)->deleteLater();
    }
    FLW_List.clear();
    
    
    for(it2 = fExecutedEffects.begin() ;it2 != fExecutedEffects.end(); it2++)
        delete (*it2);
    for(it2 = fRemoteEffects.begin() ;it2 != fRemoteEffects.end(); it2++)
        delete (*it2);
    
    fExecutedEffects.clear();
    fRemoteEffects.clear();
    
//#ifdef __linux__
//    exit();
//#endif
}

//Shut all Windows from Menu
void FLApp::shut_AllWindows(){
    
    while(FLW_List.size() != 0 ){
        printf("New ROund\n");
        FLWindow* win = *(FLW_List.begin());
        
        common_shutAction(win);
        
        printf(" FLApp::shut_AllWindows() || NB WIN = %i\n", FLW_List.size());
    }
}

//Close Window from Menu
void FLApp::shut_Window(){
    
    printf("void FLApp::shut_Window()\n");
    
    if(fErrorWindow->isActiveWindow())
        fErrorWindow->hideWin();
    
    else{
        
        QList<FLWindow*>::iterator it = FLW_List.begin();
        
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

//Close from Window Action
void FLApp::close_Window_Action(){
    
    printf("FLApp::close_Window_Action()\n");
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    common_shutAction(win);
}

//Shut a specific window 
void FLApp::common_shutAction(FLWindow* win){
    
    FLEffect* toDelete = NULL;
    
    QString toto = win->get_Effect()->getSource();
    QString tutu = win->get_Effect()->getName();
    if(toto.indexOf(fSourcesFolder) == -1)
        set_Current_File(toto, tutu);
    
    deleteWinFromSessionFile(win);
    sessionContentToFile();
    
    win->shut_Window();
    
    QFileInfo ff((win)->get_Effect()->getSource());
    QString toCompare = ff.absolutePath();
    
    if(toCompare.compare(fSourcesFolder) == 0 && !isSourceInCurrentSession((win)->get_Effect()->getSource())){
       
	 if((win)->get_Effect()->isLocal())
            fExecutedEffects.removeOne(win->get_Effect());
        else
            fRemoteEffects.removeOne(win->get_Effect());

        removeFilesOfEffect(toto, tutu);
        toDelete = (win)->get_Effect();
    }
    else if(!isSourceInCurrentSession((win)->get_Effect()->getSource()))
        (win)->get_Effect()->stop_Watcher();
    
    FLW_List.removeOne(win);
    win->deleteLater();
    
    if(toDelete != NULL){
        delete toDelete;
    }
    
    QList<FLWindow*>::iterator it2;
    
    for (it2 = FLW_List.begin(); it2 != FLW_List.end(); it2++){
        
        (*it2)->set_RecentFile(fRecentFiles);
        (*it2)->update_RecentFileMenu();
    }

//#ifndef __APPLE__
//    if(FLW_List.size() == 0 && !fRecalling)
//        exit();
//#endif
}

//--------------DELETION

//Delete File and Folder directly related to an effect
void FLApp::removeFilesOfEffect(const QString& sourceName, const QString& effName){
    
//    In case the source is a waveform, 2 files have to be deleted
    QString possibleWaveformFile = QFileInfo(sourceName).absolutePath() + "/" + QFileInfo(sourceName).baseName()  + "_waveform." + QFileInfo(sourceName).completeSuffix();
    
//    printf("Possible Waveform = %s\n", possibleWaveformFile.toStdString().c_str());
    
    if(QFileInfo(possibleWaveformFile).exists())
        QFile::remove(possibleWaveformFile);
    
    QFile::remove(sourceName);
    
    QString irFile = fIRFolder + "/" + effName;
    QFile::remove(irFile);
    
    QString svgFolder = fSVGFolder + "/" + effName /*+ "-svg"*/;
    deleteDirectoryAndContent(svgFolder);
    
}

//The effect is deleted, depending on its situation
void FLApp::deleteEffect(FLEffect* leavingEffect, FLEffect* newEffect){
    
    QFileInfo ff(leavingEffect->getSource());
    QString folderOfSource = ff.absolutePath(); //Getting root folder of source
    
    //If leaving effect is not used elsewhere : 
    if(!isSourceInCurrentSession(leavingEffect->getSource())){    
        
        //The effects pointing in the Sources Folder are not kept (nor in the list of exectued Effects, nor the source file)
        if(folderOfSource.compare(fSourcesFolder) == 0){
            
            printf("DELETING SOURCE = %s\n", leavingEffect->getSource().toStdString().c_str());
            
            //If newEffect source = oldEffect source the file is kept because it has been modified and is needed
            if(newEffect->getSource().compare(leavingEffect->getSource())!=0)
                removeFilesOfEffect(leavingEffect->getSource(), leavingEffect->getName());
            
            if(leavingEffect->isLocal())
                fExecutedEffects.removeOne(leavingEffect);
            else
                fRemoteEffects.removeOne(leavingEffect);
            
            delete leavingEffect;
        } 
        else{
            //The copy made of the source is erased
            QString toErase = fSourcesFolder + "/" + leavingEffect->getName() + ".dsp";
            
            removeFilesOfEffect(toErase, leavingEffect->getName());
        }
    }
    //If the effect is used in an other window, its watcher is reactivated
    else
        leavingEffect->launch_Watcher();
    
}

//--------------------------------Navigate---------------------------------

void FLApp::frontShow(QString name){
    
    QList<FLWindow*>::iterator it;
    
    for(it = FLW_List.begin() ; it != FLW_List.end(); it++){
    
        QString winName = (*it)->get_nameWindow();
        winName+=" : ";
        winName+= (*it)->get_Effect()->getName();
            
        if(winName.compare(name) == 0){
            (*it)->frontShow();
            break;
        }
    }
}

//--------------------------------Window----------------------------------------

//Open the source of a specific window
void FLApp::edit(FLWindow* win){
    
    QString source = win->get_Effect()->getSource();

//    In case the file is an example, it has to be relocated before being edited
    if(source.contains(fExamplesFolder, Qt::CaseInsensitive)){
        QString mesg;
        
        QMessageBox* existingNameMessage = new QMessageBox(QMessageBox::Warning, tr("Notification"), mesg);
        
        QPushButton* yes_Button;
        QPushButton* cancel_Button; 
        
        mesg = QFileInfo(source).baseName();
        mesg += " is an example. It cannot be modified! Do you want to save it in another location ?";
            
        yes_Button = existingNameMessage->addButton(tr("Yes"), QMessageBox::AcceptRole);
        cancel_Button = existingNameMessage->addButton(tr("No"), QMessageBox::RejectRole);
        
        existingNameMessage->setText(mesg);
        
        existingNameMessage->exec();
        if (existingNameMessage->clickedButton() != cancel_Button){
            QFileDialog* fileDialog = new QFileDialog;
            fileDialog->setConfirmOverwrite(true);
            
            QString filename;
            
            filename = fileDialog->getSaveFileName(NULL, "Rename File", tr(""), tr("(*.dsp)"));
            
            QFile f(filename); 
            
            if(f.open(QFile::WriteOnly | QIODevice::Truncate)){
                
                QTextStream textWriting(&f);
                
                textWriting<<pathToContent(source);
                
                f.close();
            }
            
            source = filename;    
            update_SourceInWin(win, source);
        }
    }
    
    printf("SOURCE = %s\n", source.toStdString().c_str());
    
    QUrl url = QUrl::fromLocalFile(source);
    bool b = QDesktopServices::openUrl(url);
    
    QString error = source + " could not be opened!";
    
    if(!b)
        fErrorWindow->print_Error(error);
}

//Edit Source from Menu
void FLApp::edit_Action(){
    
     FLWindow* win = (FLWindow*)QObject::sender();

    if(win != NULL)
        edit(win);
}

//Duplicate a specific window
void FLApp::duplicate(FLWindow* window){
    
    printf("SIZE OF LIST = %i\n", FLW_List.size());
    
    if(FLW_List.size() == numberWindows){
        fErrorWindow->print_Error("You cannot open more windows. If you are not happy with this limit, feel free to contact us : research.grame@gmail.com ^^");
        return;
    }
    
    FLEffect* commonEffect = window->get_Effect();
    //To avoid flicker of the original window, the watcher is stopped during operation
    commonEffect->stop_Watcher();
    
    QString source = commonEffect->getSource();
    
    QList<int> currentIndexes = get_currentIndexes();
    int val = find_smallest_index(currentIndexes);
    QString ss = QString::number(val);
    
    int x = window->get_x() + 10;
    int y = window->get_y() + 10;
    
    FLWindow* win = new FLWindow(fWindowBaseName, val, commonEffect, x, y, fSessionFolder, window->get_oscPort(), window->get_Port(), window->get_machineName(), window->get_ipMachine());
    
    redirectMenuToWindow(win);
    
    //Save then Copy of duplicated window's parameters
    window->save_Window();
    
    QString toFind = window->get_nameWindow();
    QString toReplace = win->get_nameWindow();
    
    QString savingsPath = fSessionFolder + "/" + toFind + "/" + toFind + ".rc";
    QFile toCpy(savingsPath);
    QString path = fSessionFolder + "/" + toReplace + "/" + toReplace + ".rc";
    toCpy.copy(path);
    
    savingsPath = fSessionFolder + "/" + toFind + "/" + toFind + ".jc";
    QFile toCy(savingsPath);
    path = fSessionFolder + "/" + toReplace + "/" + toReplace + ".jc";
    toCy.copy(path);
    
    //Replacement of WindowName in the audio Connections File to reconnect the new window as the duplicated one.
    std::list<std::pair<std::string,std::string> > changeTable;
    changeTable.push_back(make_pair(toFind.toStdString(), toReplace.toStdString()));
    win->update_ConnectionFile(changeTable);
    
    QString error;
    
    if(win->init_Window(kNoInit, error)){
        FLW_List.push_back(win);
        addWinToSessionFile(win);
    }
    else{
        QString toDelete = fSessionFolder + "/" + win->get_nameWindow(); 
        deleteDirectoryAndContent(toDelete);
        delete win;
        fErrorWindow->print_Error(error); 
    }
    
    //Whatever happens, the watcher has to be started (at least for the duplicated window that needs it)
    commonEffect->launch_Watcher();
}

//Duplication window from Menu
void FLApp::duplicate_Window(){ 
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    if(win != NULL)
        duplicate(win);
}

//Paste text in a specific window
void FLApp::paste(FLWindow* win){
    
    //Recuperation of Clipboard Content
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    
    if (mimeData->hasText()) {
        QString clipText = clipboard->text();

        update_SourceInWin(win, clipText);
        
    }
}

//Paste from Menu
void FLApp::paste_Text(){
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    if(win != NULL)
        paste(win);
} 

//View Httpd Window
void FLApp::viewHttpd(FLWindow* win){
    
    win->viewQrCode();
}

//View Httpd From Menu
void FLApp::httpd_View_Window(){
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    //Searching the active Window to show its QRcode
    if(win != NULL)
        viewHttpd(win);
    else
        fErrorWindow->print_Error("No active Window");
}

#include "faust/llvm-dsp.h"

//View SVG of a specific Window
void FLApp::viewSvg(FLWindow* win){
    
    QString source = win->get_Effect()->getSource();
    QString pathToOpen = fSVGFolder + "/" + win->get_Effect()->getName() + "/" + QFileInfo(win->get_Effect()->getSource()).baseName() + "-svg/process.svg";
    
    printf("PATH TO OPEN = %s\n", pathToOpen.toStdString().c_str());
    
    if(!QFileInfo(pathToOpen).exists()){
        
//------------This is ridiculous but hopefully TEMPORARY (12/05/14)
        
        
        int numberFixedParams = 7;
		int iteratorParams = 0;
        
#ifdef _WIN32
		numberFixedParams = numberFixedParams+2;
#endif
        
        //+7 = -I libraryPath -I currentFolder -O drawPath -svg
        int argc = numberFixedParams;
        argc += win->get_Effect()->get_numberParameters(win->get_Effect()->getCompilationOptions());
        
        const char** argv = new const char*[argc];
        
        argv[iteratorParams] = "-I";
		iteratorParams++;
        
        //The library path is where libraries like the scheduler architecture file are = currentSession
        string libPath = fLibsFolder.toStdString();
        argv[iteratorParams] = libPath.c_str();
		iteratorParams++;
        
        argv[iteratorParams] = "-I";   
		iteratorParams++;
        string sourcePath = QFileInfo(win->get_Effect()->getSource()).absolutePath().toStdString();
        argv[iteratorParams] = sourcePath.c_str();
		iteratorParams++;
        
        argv[iteratorParams] = "-O";
		iteratorParams++;
        
        QString svgPath = fSVGFolder + "/" + win->get_Effect()->getName();
        
        printf("svg path = %s\n", svgPath.toStdString().c_str());
        
        QDir direct(svgPath);
        direct.mkdir(svgPath);
        
        string pathSVG = svgPath.toStdString();
        
        argv[iteratorParams] = pathSVG.c_str();
		iteratorParams++;
        argv[iteratorParams] = "-svg";
		iteratorParams++;
		
#ifdef _WIN32
        //LLVM_MATH is added to resolve mathematical float functions, like powf
		argv[iteratorParams] = "-l";
		iteratorParams++;
		argv[iteratorParams] = "llvm_math.ll";
		iteratorParams++;
#endif
        string err;
        generateAuxFilesFromFile(win->get_Effect()->getSource().toStdString(), argc, argv, err);
    }
        
    
    
    QUrl url = QUrl::fromLocalFile(pathToOpen);
    bool b = QDesktopServices::openUrl(url);

   	QString error = pathToOpen + " could not be opened!";
    
    if(!b)
        fErrorWindow->print_Error(error);
}

//View SVG from Menu
void FLApp::svg_View_Action(){
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    //Searching the active Window to show its SVG Diagramm
    if(win != NULL)
        viewSvg(win);
    
}

//---------------Export

//Open ExportManager for a specific Window
void FLApp::export_Win(FLWindow* win){
    
    QString expanded_code = win->get_Effect()->get_expandedVersion().c_str();
    
    fExportDialog->exportFile(win->get_Effect()->getSource(), expanded_code);
}

//Export From Menu
void FLApp::export_Action(){ 
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    if(win != NULL)
        export_Win(win);
}

//---------------Drop

//For sound files, a pass transforms them into faust code through the waveforms
QString FLApp::soundFileToFaust(const QString& soundFile){
    
    QString soundFileName = QFileInfo(soundFile).baseName();
    soundFileName = nameWithoutSpaces(soundFileName);
    
    QString destinationFile = fSourcesFolder;
    destinationFile += "/" ;
    destinationFile += soundFileName;

    QString waveFile = destinationFile;
    waveFile += "_waveform.dsp";

    destinationFile += ".dsp";

    printf("FLAPP::destinationFile = %s\n", destinationFile.toStdString().c_str());
    
    QProcess myCmd;
//    myCmd.setProcessEnvironment(env);
//    printf("IS ENV EMPTY?? = %i\n", env.isEmpty());
    QByteArray error;
    
    QString systemInstruct;
#ifdef _WIN32
    systemInstruct += "/sound2faust.exe ";
#endif
#ifdef __linux__
    systemInstruct += "./sound2faust ";
#else
    
    QDir base;
    
    if(base.absolutePath().indexOf("Contents/MacOS") != -1)
        systemInstruct += "./sound2faust ";
    else
        systemInstruct += base.absolutePath() + "/FaustLive.app/Contents/MacOs/sound2faust ";
#endif
    
    systemInstruct += "\"" + soundFile + "\"" + " -o " + waveFile;
    
    printf("INSTRUCTION = %s\n", systemInstruct.toStdString().c_str());
    
    myCmd.start(systemInstruct);
    myCmd.waitForFinished();
    
    error = myCmd.readAllStandardError();
    
    if(myCmd.readChannel() == QProcess::StandardError )
        fErrorWindow->print_Error(error.data());
    
    QString finalFileContent = "import(\"";
    finalFileContent += soundFileName + "_waveform.dsp";
    finalFileContent += "\");\nprocess=";
    finalFileContent += QFileInfo(soundFile).baseName();
    finalFileContent += ";";
    
    printf("finalFileContent = %s\n", finalFileContent.toStdString().c_str());
    
    QFile f(destinationFile); 
    
    if(f.open(QFile::WriteOnly | QIODevice::Truncate)){
        
        QTextStream textWriting(&f);
        
        textWriting<<finalFileContent;
        
        f.close();
    }
    
    return destinationFile;
}

//Drop of sources on a window
void FLApp::drop_Action(QList<QString> sources){
    
    QList<QString>::iterator it;
    FLWindow* win = (FLWindow*)QObject::sender();
 
//    The sound files are transformated into faust files
    for(it = sources.begin(); it != sources.end(); it++){

        if((QFileInfo(*it).completeSuffix().compare("dsp") == 0) && (*it).indexOf(' ') != -1){
            sources.removeOne(*it);
            fErrorWindow->print_Error("Forbidden to drop a file with spaces in its name!");
            
        }
        else if(QFileInfo(*it).completeSuffix().compare("wav") == 0)
            *it = soundFileToFaust(*it);
    }
    
    it = sources.begin();
    
    if(it != sources.end()){
        
        //The first source dropped is updated in the dropped Window
        update_SourceInWin(win, *it);
        
        //The other source are opened in new Windows
        it++;
        while(it!=sources.end()){
            
            create_New_Window(*it);
            it++;
            
        }
    }
}

//--------------------------------HELP----------------------------------------

//Set Text in Tools Menu of HELP
void FLApp::setToolText(const QString & currentText){
    
    if(currentText.compare("FAUST") == 0)
        fToolText->setHtml("<br>\nTo develop your own effects, you will need to learn the Faust language.<br><br>""LEARN MORE ABOUT FAUST : <a href = http://faust.grame.fr>faust.grame.fr</a>");
    else if(currentText.compare("LLVM") == 0)
        fToolText->setHtml("<br>\nThanks to its embedded LLVM compiler, this application allows dynamic compilation of your Faust objects.<br><br>""LEARN MORE ABOUT LLVM : <a href = http://llvm.org>llvm.org</a>");
    else if(currentText.compare("COREAUDIO") == 0)
        fToolText->setHtml("<br>Core Audio is the digital audio infrastructure of MAC OS X.<br><br>""LEARN MORE ABOUT COREAUDIO : <a href = http://developer.apple.com/library/ios/#documentation/MusicAudio/Conceptual/CoreAudioOverview/WhatisCoreAudio/WhatisCoreAudio.html>developer.apple.com </a> ");
    else if(currentText.compare("JACK") == 0)
        fToolText->setHtml("<br>Jack (the Jack Audio Connection Kit) is a low-latency audio server. It can connect any number of different applications to a single hardware audio device.<br><br>YOU CAN DOWNLOAD IT HERE : <a href =http://www.jackosx.com> www.jackosx.com</a>\n");
    else if(currentText.compare("NETJACK") == 0)
        fToolText->setHtml("<br>NetJack (fully integrated in Jack) is a Realtime Audio Transport over a generic IP Network. It allows to send audio signals through the network to a server.<br><br>""LEARN MORE ABOUT NETJACK : <a href = http://netjack.sourceforge.net> netjack.sourceforge.net</a>\n");
    else if(currentText.compare("PORTAUDIO") == 0)
        fToolText->setHtml("<br>PortAudio is a free, cross-platform, open-source, audio I/O library. <br><br>""LEARN MORE ABOUT PORTAUDIO : <a href = http://www.portaudio.com/> portaudio.com</a>\n");
    else if(currentText.compare("LIB QRENCODE") == 0)
        fToolText->setHtml("<br>Libqrencode is a C library for encoding data in a QR Code symbol, a kind of 2D symbology that can be scanned by handy terminals such as a mobile phone with CCD.<br><br>""LEARN MORE ABOUT LIB QRENCODE : <a href = http://fukuchi.org/works/qrencode> fukuchi.org/works/qrencode</a>\n");
    else if(currentText.compare("LIB MICROHTTPD") == 0)
        fToolText->setHtml("<br>GNU libmicrohttpd is a small C library that allows running an HTTP server as part of an application.<br><br>""LEARN MORE ABOUT LIB MICROHTTPD : <a href = http://www.gnu.org/software/libmicrohttpd> gnu.org/software/libmicrohttpd</a>\n");
    else if(currentText.compare("OSC PACK") == 0)
        fToolText->setHtml("<br>Oscpack is simply a set of C++ classes for packing and unpacking OSC packets. Oscpack includes a minimal set of UDP networking classes for Windows and POSIX.<br><br>""LEARN MORE ABOUT OSC PACK : <a href = http://code.google.com/p/oscpack> code.google.com/p/oscpack</a>\n");
}

//Set Text in Application Properties Menu of HELP
void FLApp::setAppPropertiesText(const QString& currentText){
    
    if(currentText.compare("New Default Window")==0)
        fAppText->setPlainText("\nCreates a new window containing a simple Faust process.\n\n process = !,!:0,0; ");
    
    else if(currentText.compare("Open")==0)
        fAppText->setPlainText("\nCreates a new window containing the DSP you choose on disk.\n");
 
    else if(currentText.compare("Take Snapshot")==0)
        fAppText->setPlainText("\nSaves the actual state of the application in a folder : all the windows, their graphical parameters, their audio connections, their position on the screen, ...\n");
    
    else if(currentText.compare("Recall Snapshot")==0)
        fAppText->setPlainText("\nRestores the state of the application as saved. All current windows are closed. If one of the source file can't be found, a back up file is used.\n");
    
    else if(currentText.compare("Import Snapshot")==0)
        fAppText->setPlainText("\nAdds the state of the application as saved to the current state of the application. That way, current windows are not closed. Some audio application/windows may have to be renamed during the importation.\n");
    
    else if(currentText.compare("Navigate")==0)
        fAppText->setPlainText("\nBrings the chosen running window to front end.\n");
    
    else if(currentText.compare("Preferences")==0){
        
        QString text = "\nWINDOW PREFERENCES : You can choose default compilation options for new windows. (Faust & LLVM options)\n\nAUDIO PREFERENCES : If this version of FaustLive includes multiple audio architectures, you can switch from one to another in Audio Preferences. All opened windows will try to switch. If the update fails, former architecture will be reloaded.\n\nNETWORK PREFERENCES : The compilation web service URL can be modified.";
#ifdef HTTPDVAR
      text += "The port of remote drop (on html interface) can be modified.";  
#endif
        
        text += "\n\nSTYLE PREFERENCES : You can also choose the graphical style of the application.\n";
        
        fAppText->setPlainText(text);
    }
    else if(currentText.compare("Error Displaying")==0)
        fAppText->setPlainText("\nDisplays a window every time the program catches an error : whether it's a error in the edited code, a compilation problem, a lack of memory during saving action, ...");
            
}

//Set Text in Window Properties Menu of HELP
void FLApp::setWinPropertiesText(const QString& currentText){
    
    if(currentText.compare("Audio Cnx/Dcnx")==0)
        fWinText->setPlainText("\nWith JackRouter audio device, you can connect a window to another one or to an external application like iTunes, VLC or directly to the computer input/output.\nYou can choose Jack as the audio architecture in the preferences.");
        
    else if(currentText.compare("Edit Source")==0)
        fWinText->setPlainText("\nThe Faust code corresponding to the active window is opened in a text editor. When you save your modifications, the window(s) corresponding to this source will be updated. The graphical parameters and the audio connections that can be kept will stay unmodified.");
            
    else if(currentText.compare("Drag and Drop / Paste")==0)
        fWinText->setPlainText("\nIn a window, you can drop or paste : \n - File.dsp\n - Faust code\n - Faust URL\nAn audio crossfade will be calculated between the outcoming and the incoming audio application. The new application will be connected as the outcoming one.\n");
    
    else if(currentText.compare("Duplicate")==0)
        fWinText->setPlainText("\nCreates a new window, that has the same characteristics : same Faust code, same graphical parameters, same compilation options, ...\n");
    
    else if(currentText.compare("View QrCode")==0)
        fWinText->setPlainText("\nYou can display a new window with a QRcode so that you can remotely control the User Interface of the audio application.");
    
    else if(currentText.compare("Window Options")==0){
        
        QString text = "\nYou can add compilation options for Faust Compiler. You can also change the level of optimization for the LLVM compiler. If several windows correspond to the same audio application, they will load the chosen options.";
#ifdef HTTPDVAR
        text+="\n\nThe HTTPD Port corresponds to the TCP port used for remote HTTP control of the interface.\n";
#endif
#ifdef OSCVAR
        text+="\n\nThe OSC Port corresponds to the UDP port used for OSC control. \nWARNING : a port needs a few seconds to be released once a window is closed. Moreover, only 30 OSC ports can be opened at the same time.";
#endif
        fWinText->setPlainText(text);
    }
    else if(currentText.compare("View SVG")==0)
        fWinText->setPlainText("\nYou can display the SVG diagram of the active Window. It will be opened in your chosen default navigator.");
    
    else if(currentText.compare("Export")==0)
        fWinText->setPlainText("\nA web service is available to upload your Faust application for another platform or/and architecture.");
    
}

//Set Faust Lib Text in Help Menu
void FLApp::setLibText(QListWidgetItem * item){
    
    QString pathLib = fLibsFolder + "/" + item->text();
   
    QUrl url = QUrl::fromLocalFile(pathLib);
    bool b = QDesktopServices::openUrl(url);
    
    QString error = pathLib + " could not be opened!";
    
    if(!b)
        fErrorWindow->print_Error(error);    
}

void FLApp::init_HelpWindow(){
    
//----------------------Global Help Window
    
    QGroupBox* winGroup = new QGroupBox(fHelpWindow);
    QVBoxLayout* winLayout = new QVBoxLayout;
    
    QPushButton* pushButton = new QPushButton("OK", winGroup);
    pushButton->connect(pushButton, SIGNAL(clicked()), this, SLOT(end_apropos()));
    
    QTabWidget *myTabWidget = new QTabWidget(winGroup);
    
    //---------------------General
    
    QWidget* tab_general = new QWidget;
    
    QVBoxLayout* generalLayout = new QVBoxLayout;
    
    QPlainTextEdit* generalText = new QPlainTextEdit(tr("\nFaustLive is a dynamic compiler for audio DSP programs coded with Faust. It embeds Faust & LLVM compiler.\n\nEvery window of the application corresponds to an audio application, which parameters you can adjust."));
    
    QLineEdit* lineEdit = new QLineEdit(tr(" Distributed by GRAME - Centre de Creation Musicale"));
    
    generalText->setReadOnly(true);
    lineEdit->setReadOnly(true);
    
    myTabWidget->addTab(tab_general, QString(tr("General")));
    
    generalLayout->addWidget(generalText);
    generalLayout->addWidget(lineEdit);
    
    tab_general->setLayout(generalLayout);
    
    //----------------------Tools
    
    QWidget* tab_tool = new QWidget;
    
    QGridLayout* toolLayout = new QGridLayout;
    
    QListWidget *vue = new QListWidget;
    
    vue->addItem(QString(tr("FAUST")));
    vue->addItem(QString(tr("LLVM")));
    
#ifdef COREAUDIO
    vue->addItem(QString(tr("COREAUDIO")));
#endif
    
#ifdef JACK
    vue->addItem(QString(tr("JACK")));
#endif
    
#ifdef NETJACK
    vue->addItem(QString(tr("NETJACK")));
#endif
    
#ifdef PORTAUDIO
    vue->addItem(QString(tr("PORTAUDIO")));
#endif
    
#ifdef HTTPDVAR
    vue->addItem(QString(tr("LIB MICROHTTPD")));
    vue->addItem(QString(tr("LIB QRENCODE")));
    vue->addItem(QString(tr("OSC PACK")));
#endif
    
    vue->setMaximumWidth(150);
    connect(vue, SIGNAL(currentTextChanged(const QString&)), this, SLOT(setToolText(const QString &)));
    
    toolLayout->addWidget(vue, 0, 0, 1, 1);
    
    fToolText = new QTextBrowser;
    fToolText->setOpenExternalLinks(true);
    fToolText->setReadOnly(true);
    fToolText->setMinimumWidth(300);

    myTabWidget->addTab(tab_tool, QString(tr("Tools")));
    
    toolLayout->addWidget(fToolText, 0, 1, 1, 2);
    tab_tool->setLayout(toolLayout);
    
    vue->setCurrentRow(0);
    
    //-----------------------Faust Librairies
    
    QWidget* tab_app1 = new QWidget();
    
    QGridLayout* appLayout1 = new QGridLayout;
    
    QListWidget *vue1 = new QListWidget;
    
    //    Mettre en route d'ajouter les librairies présentes dans le dossier Libs
    
    QDir libsDir(fLibsFolder);
    
    QFileInfoList children = libsDir.entryInfoList(QDir::Files);
    
    QFileInfoList::iterator it;
    
    for(it = children.begin(); it != children.end(); it++){
        if(it->completeSuffix().compare("ll") != 0){
            QString completeName = it->baseName() + "." + it->completeSuffix();
            vue1->addItem(completeName);
        }
    }
    
    vue1->setMaximumWidth(150);
    connect(vue1, SIGNAL(itemDoubleClicked( QListWidgetItem *)), this, SLOT(setLibText(QListWidgetItem *)));
    
    appLayout1->addWidget(vue1, 0, 0, 1, 1);
    
    fLibsText = new QPlainTextEdit;
    fLibsText->setReadOnly(true);
    fLibsText->setMinimumWidth(300);
    fLibsText->setPlainText("\nDouble Click On a Librairy to Open It.\nBe sure to set a default editor for .lib files.\n\n!! WARNING !!\n\n These librairies exist in your (hidden) Current Session Folder. If you want to modify them, you better save them in an other location.");
    
    vue1->setCurrentRow(0);
    
    appLayout1->addWidget(fLibsText, 0, 1, 1, 2);
    
    myTabWidget->addTab(tab_app1, QString(tr("Faust Librairies")));
    
    tab_app1->setLayout(appLayout1);
    
    //-----------------------Faust Live Menu
    
    QWidget* tab_app = new QWidget();
    
    QGridLayout* appLayout = new QGridLayout;
    
    QListWidget *vue2 = new QListWidget;
    
    vue2->addItem(QString(tr("New Default Window")));
    vue2->addItem(QString(tr("Open")));    
    vue2->addItem(QString(tr("Take Snapshot")));
    vue2->addItem(QString(tr("Recall Snapshot")));
    vue2->addItem(QString(tr("Import Snapshot")));
    vue2->addItem(QString(tr("Navigate")));
    vue2->addItem(QString(tr("Preferences")));
    vue2->addItem(QString(tr("Error Displaying")));
    
    vue2->setMaximumWidth(150);
    connect(vue2, SIGNAL(currentTextChanged(const QString&)), this, SLOT(setAppPropertiesText(const QString &)));
    
    appLayout->addWidget(vue2, 0, 0, 1, 1);
    
    fAppText = new QPlainTextEdit;
    fAppText->setReadOnly(true);
    fAppText->setMinimumWidth(300);
    
    vue2->setCurrentRow(0);
    
    appLayout->addWidget(fAppText, 0, 1, 1, 2);

    myTabWidget->addTab(tab_app, QString(tr("Application Properties")));
    
    tab_app->setLayout(appLayout);
    
    //-----------------------Window Properties
    
    QWidget* tab_win = new QWidget();
    
    QGridLayout* winPropLayout = new QGridLayout;
    
    QListWidget *vue3 = new QListWidget;
    
#ifdef JACK
    vue3->addItem(QString(tr("Audio Cnx/Dcnx")));
#endif
    
    vue3->addItem(QString(tr("Edit Source")));    
    vue3->addItem(QString(tr("Drag and Drop / Paste")));
    vue3->addItem(QString(tr("Duplicate")));
#ifdef HTTPDVAR
    vue3->addItem(QString(tr("View QrCode")));
#endif
    
    vue3->addItem(QString(tr("Window Options")));
    vue3->addItem(QString(tr("View SVG")));
    vue3->addItem(QString(tr("Export")));
    
    vue3->setMaximumWidth(150);
    connect(vue3, SIGNAL(currentTextChanged(const QString&)), this, SLOT(setWinPropertiesText(const QString &)));
    
    winPropLayout->addWidget(vue3, 0, 0, 1, 1);
    
    fWinText = new QPlainTextEdit;
    fWinText->setReadOnly(true);
    fWinText->setMinimumWidth(300);
    
    vue3->setCurrentRow(0);
    
    winPropLayout->addWidget(fWinText, 0, 1, 1, 2);
    
    tab_win->setLayout(winPropLayout);
    
    myTabWidget->addTab(tab_win, QString(tr("Window Properties")));

//Help Window Layout

    winLayout->addWidget(myTabWidget);
    winLayout->addWidget(new QLabel(""));
    winLayout->addWidget(pushButton);
    winGroup->setLayout(winLayout);
    fHelpWindow->setCentralWidget(winGroup);
    
    myTabWidget->setCurrentIndex(0);
}

void FLApp::apropos(){
    fHelpWindow->show();
}

void FLApp::end_apropos(){
    fHelpWindow->hide(); 
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

//Store the item clicked to open it when the open button is pressed
void FLApp::itemClick(QListWidgetItem *item){
    fExampleToOpen = item->text();
}

//Opens directly a double clicked item
void FLApp::itemDblClick(QListWidgetItem* item){
    fExampleToOpen = item->text();
    open_Example_Action();
}

//Init Presentation Menu
void FLApp::init_presentationWindow(){
    
    QDir ImagesDir(":/");
    
    ImagesDir.cd("Images");
    
    QFileInfoList child = ImagesDir.entryInfoList(QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
    
    QGroupBox*      iconeBox = new QGroupBox;
    QGroupBox*      buttonBox = new QGroupBox;
    QGroupBox*      gridBox = new QGroupBox;
    QGroupBox*      textBox = new QGroupBox;
    QGroupBox*      openExamples = new QGroupBox;
    
    QPushButton*    new_Window;
    QPushButton*    open_Window;
    QPushButton*    open_Session;
    QPushButton*    preferences;
    QPushButton*    help;
    
    QPushButton*    ok;
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(iconeBox);
    mainLayout->addWidget(gridBox);
    mainLayout->addWidget(textBox);
    
    fPresWin->setLayout(mainLayout);
    
    //------------------------ICON
    
    QHBoxLayout *layout = new QHBoxLayout;
    
    QLabel *image = new QLabel();
    //Path of the executable + the presentation Image
    QPixmap presImg(ImagesDir.absoluteFilePath("Presentation_Image.png"));
    presImg.scaledToWidth(100, Qt::SmoothTransformation);
    presImg.scaledToHeight(100, Qt::SmoothTransformation);
    image->setPixmap(presImg);
    image->setAlignment(Qt::AlignCenter);
    layout->addWidget(image);
    iconeBox->setLayout(layout);
    
    QLabel* text = new QLabel("<h2>WELCOME TO FAUST LIVE</h2>\nDynamic Faust Compiler");  
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
    QPixmap newPix(ImagesDir.absoluteFilePath("InitWin.png"));
    newPix = newPix.scaledToWidth(60, Qt::SmoothTransformation);
    new_Image->setPixmap(newPix);
    new_Image->setAlignment(Qt::AlignCenter);
    layout2->addWidget(new_Image, 0, 0);
    
    new_Window = new QPushButton("New Default Window\nOpen a window with simple process");
    new_Window->setToolTip("Open a window containing a default process.");
    new_Window->setFlat(true);
    new_Window->setDefault(false);
    
    layout2->addWidget(new_Window, 0, 1);
    connect(new_Window, SIGNAL(clicked()), this, SLOT(new_Window_pres()));
    
    QLabel *open_Image = new QLabel(gridBox);
    QPixmap openPix(ImagesDir.absoluteFilePath("OpenWin.png"));
    openPix = openPix.scaledToWidth(60, Qt::SmoothTransformation);
    open_Image->setPixmap(openPix);
    layout2->addWidget(open_Image, 1, 0);
    
    open_Window = new QPushButton("Open your File.dsp\n");
    open_Window->setToolTip("Open the DSP you choose.");
    open_Window->setFlat(true);
    open_Window->setDefault(false);
    
    layout2->addWidget(open_Window, 1, 1);
    connect(open_Window, SIGNAL(clicked()), this, SLOT(open_Window_pres()));
    
    QLabel *snap_Image = new QLabel(gridBox);
    QPixmap snapPix(ImagesDir.absoluteFilePath("RecallMenu.png"));
    snapPix = snapPix.scaledToWidth(60, Qt::SmoothTransformation);
    snap_Image->setPixmap(snapPix);
    layout2->addWidget(snap_Image, 2, 0);
    
    open_Session = new QPushButton("Recall your Snapshot\n");
    open_Session->setToolTip("Open a saved snapshot.");
    open_Session->setFlat(true);
    open_Session->setDefault(false);
    
    layout2->addWidget(open_Session, 2, 1);
    connect(open_Session, SIGNAL(clicked()), this, SLOT(open_Session_pres()));
    
    QLabel *pref_Image = new QLabel(gridBox);
    QPixmap prefPix(ImagesDir.absoluteFilePath("Preferences.png"));
    prefPix = prefPix.scaledToWidth(60, Qt::SmoothTransformation);
    pref_Image->setPixmap(prefPix);
    layout2->addWidget(pref_Image, 3, 0);
    
    preferences = new QPushButton("Open Preferences Menu\n");
    preferences->setToolTip("Modify audio & compilation preferences");
    preferences->setFlat(true);
    preferences->setDefault(false);
    
    layout2->addWidget(preferences, 3, 1);
    connect(preferences, SIGNAL(clicked()), this, SLOT(Preferences()));
    
    
    QLabel *help_Image = new QLabel(gridBox);
    QPixmap helpPix(ImagesDir.absoluteFilePath("HelpMenu.png"));
    helpPix = helpPix.scaledToWidth(60, Qt::SmoothTransformation);
    help_Image->setPixmap(helpPix);
    layout2->addWidget(help_Image, 4, 0);
    
    help = new QPushButton("About Faust Live\n Learn all about FaustLive charateristics");
    help->setToolTip("Help Menu.");
    
    layout2->addWidget(help, 4, 1);
    connect(help, SIGNAL(clicked()), this, SLOT(apropos()));
    help->setFlat(true);
    help->setDefault(false);
    
    
    //-------------------------OPEN EXAMPLES
    
    
    QVBoxLayout *layout5 = new QVBoxLayout;
    
    QListWidget *vue = new QListWidget(openExamples);
    
    QDir examplesDir(":/");
    
    if(examplesDir.cd("Examples")){
        
        QFileInfoList children = examplesDir.entryInfoList(QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
        
        fExampleToOpen = (children.begin())->baseName();
        
        QFileInfoList::iterator it;
        
        for(it = children.begin(); it != children.end(); it++)
            vue->addItem(QString(it->baseName()));
        
        connect(vue, SIGNAL(itemDoubleClicked( QListWidgetItem *)), this, SLOT(itemDblClick(QListWidgetItem *)));
        connect(vue, SIGNAL(itemClicked( QListWidgetItem *)), this, SLOT(itemClick(QListWidgetItem *)));
    }
    
    layout5->addWidget(new QLabel(tr("<h2>Try Out an Example</h2>")));
    layout5->addWidget(vue);
    
    ok = new QPushButton("Open");
    connect(ok, SIGNAL(clicked()), this, SLOT(open_Example_Action()));
    ok->setDefault(true);
    //    ok->setStyleSheet("*{background-color: transparent;}");
    layout5->addWidget(ok);
    
    buttonBox->setLayout(layout2);
    openExamples->setLayout(layout5);
    
    layout3->addWidget(buttonBox);
    layout3->addWidget(openExamples); 
    
    gridBox->setLayout(layout3);
    
    QHBoxLayout *layout4 = new QHBoxLayout;
    
    QPushButton* cancel = new QPushButton("Cancel");
    //    cancel->setStyleSheet("*{background-color: transparent;}");
    connect(cancel, SIGNAL(clicked()), fPresWin, SLOT(hide()));
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
    
    preferences->setStyleSheet("QPushButton:flat{"
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

void FLApp::new_Window_pres(){
    fPresWin->hide();
    //    reset_CurrentSession();
    create_Empty_Window();
}

void FLApp::open_Window_pres(){
    fPresWin->hide();   
    //    reset_CurrentSession();
    open_New_Window();
}

void FLApp::open_Session_pres(){
    fPresWin->hide();
    //    reset_CurrentSession();
    recallSnapshotFromMenu();
}

void FLApp::show_presentation_Action(){
    
    fPresWin->show();
    fPresWin->raise();
}

//--------------------------------PREFERENCES---------------------------------------

//Style clicked in Menu
void FLApp::styleClicked(){
    
    QPushButton* item = (QPushButton*)QObject::sender();
    styleClicked(item->text());
}

//Modification of application style
void FLApp::styleClicked(const QString& style){
    
    if(style.compare("Default") == 0){
        
        fStyleChoice = "Default";
        
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
                      "background-color: transparent;"
                      "}"
                      
                      "QPushButton:hover{"
                      "border: 2px;"
                      "border-radius: 6px;"
                      "border-color: #811453;"
                      "background-color: #6A455D;"
                      "}"
                      
                      "QPushButton:pressed{"
                      "background-color: #6A455D;"
                      "border-radius: 6px;"
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
                      "background: transparent;"
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
                      
                      "QListWidget{"                      
                      "background-color: transparent;"
                      "}"
                      );
    }
    
    if(style.compare("Blue") == 0){    
        
        fStyleChoice = "Blue";
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
                      
                      "QListWidget{"                      
                      "background-color: transparent;"
                      "}"
                      );
    }
    
    if(style.compare("Grey") == 0){
        
        fStyleChoice = "Grey";
        setStyleSheet(
                      
                      // BUTTONS
                      "QPushButton {"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 0.8, y2: 0.8,"
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
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 0.8, y2: 0.8,"
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
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 0.8, y2: 0.8,"
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
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 0.8, y2: 0.8,"
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
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 0.8, y2: 0.8,"
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
                      
                      "QListWidget{"                      
                      "background-color: transparent;"
                      "}"
                      );
    }
    
    if(style.compare("Salmon") == 0){
        
        fStyleChoice = "Salmon";
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
                      
                      "QListWidget{"                      
                      "background-color: transparent;"
                      "}"
                      );
    }
}

//Preference triggered from Menu
void FLApp::Preferences(){
    
    fPrefDialog->exec();
}

//init the preferences dialog
void FLApp::init_PreferenceWindow(){
    
    fPrefDialog->setWindowTitle("PREFERENCES");
    
    QTabWidget* myTab = new QTabWidget(fPrefDialog);
    myTab->setStyleSheet("*{}""*::tab-bar{}");

    QGroupBox* menu1 = new QGroupBox(myTab);
    QGroupBox* menu2 = new QGroupBox(myTab);
    QGroupBox* menu3 = new QGroupBox(myTab);
    QGroupBox* menu4 = new QGroupBox(myTab);
    
    QFormLayout* layout1 = new QFormLayout;
    QFormLayout* layout2 = new QFormLayout;
    QFormLayout* layout3 = new QFormLayout;
    QFormLayout* layoutNet = new QFormLayout;
    QGridLayout* layout4 = new QGridLayout;
    QVBoxLayout* layout5 = new QVBoxLayout;
    QHBoxLayout* intermediateLayout = new QHBoxLayout;
    intermediateLayout->setAlignment(Qt::AlignCenter);
    
    QWidget* intermediateWidget = new QWidget(fPrefDialog);
    
    QPushButton* cancelButton = new QPushButton(tr("Cancel"), intermediateWidget);
    cancelButton->setDefault(false);;
    
    QPushButton* saveButton = new QPushButton(tr("Save"), intermediateWidget);
    saveButton->setDefault(true);
    
    connect(saveButton, SIGNAL(released()), this, SLOT(save_Mode()));
    connect(cancelButton, SIGNAL(released()), this, SLOT(cancelPref()));
    
    intermediateLayout->addWidget(cancelButton);
    intermediateLayout->addWidget(new QLabel(tr("")));
    intermediateLayout->addWidget(saveButton);
    
    intermediateWidget->setLayout(intermediateLayout);
    
    
//------------------WINDOW PREFERENCES    
    
    myTab->addTab(menu1, tr("Window"));
    
    fCompilModes = new QLineEdit(menu1);
    fOptVal = new QLineEdit(menu1);
    
    recall_Settings(fSettingsFolder);
    
    fCompilModes->setText(fCompilationMode);
    stringstream oV;
    
    oV << fOpt_level;
    fOptVal->setText(oV.str().c_str());
    
    layout1->addRow(new QLabel(tr("")));
    layout1->addRow(new QLabel(tr("Faust Compiler Options")), fCompilModes);
    layout1->addRow(new QLabel(tr("LLVM Optimization")), fOptVal);
    layout1->addRow(new QLabel(tr("")));
    
    menu1->setLayout(layout1);
   
//------------------AUDIO PREFERENCES  
    
    myTab->addTab(menu2, tr("Audio"));
    
    fAudioBox = new QGroupBox(menu2);
    fAudioCreator = AudioCreator::_Instance(fSettingsFolder, fAudioBox);

    layout2->addWidget(fAudioBox);
    menu2->setLayout(layout2);
    
//-----------------NETWORK PREFERENCES
    
    fServerLine = new QLineEdit(menu4);
    fServerLine->setText(fServerUrl);
    
#ifdef  HTTPDVAR
    fPortLine = new QLineEdit(menu4);
    QString p =  QString::number(fPort);
    fPortLine->setText(p);
#endif
    myTab->addTab(menu4, tr("Network"));
    layoutNet->addRow(new QLabel(tr("")));
    layoutNet->addRow(new QLabel(tr("Compilation Web Service")), fServerLine);
#ifdef HTTPDVAR
    layoutNet->addRow(new QLabel(tr("")));
    layoutNet->addRow(new QLabel(tr("Remote Dropping Port")), fPortLine);
#endif
    
    menu4->setLayout(layoutNet);
    
//------------------STYLE PREFERENCES
    myTab->addTab(menu3, tr("Style"));
    
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
    
    layout3->addRow(myTab);
    layout3->addRow(intermediateWidget);
    fPrefDialog->setLayout(layout3);
}

//Response to cancel button triggered in preferences
void FLApp::cancelPref(){
    fPrefDialog->hide();
    fAudioCreator->reset_Settings();
}

//Response to save button triggered in preferences
void FLApp::save_Mode(){

    fServerUrl = fServerLine->text();
    fExportDialog->set_URL(fServerUrl);
    
    fCompilationMode = fCompilModes->text();
    
	if(isStringInt(fOptVal->text().toLatin1().data()))
        fOpt_level = atoi(fOptVal->text().toLatin1().data());
    else
        fOpt_level = 3;
    
#ifdef HTTPDVAR
    if(isStringInt(fPortLine->text().toLatin1().data())){
        
        if(fPort != atoi(fPortLine->text().toLatin1().data())){
            fPort = atoi(fPortLine->text().toLatin1().data());
            
            stop_Server();
            launch_Server();
        }   
    }
    else
#endif
        fPort = 7777;
    
    QList<FLWindow*>::iterator it;
    
    for(it = FLW_List.begin(); it!= FLW_List.end(); it++)
        (*it)->set_GeneralPort(fPort);
    
    fPrefDialog->hide();

    if(fAudioCreator->didSettingChanged()){
        printf("WE ARE GOING TO UPDATE....\n");
        update_AudioArchitecture();
    }
    else
        fAudioCreator->reset_Settings();
}

//Write Setting "parameter" in file "home"
void FLApp::save_Setting(const QString& home, const QString& parameter){
    
    QFile f(home); 
    
    if(f.open(QFile::WriteOnly | QIODevice::Truncate)){
        
        QTextStream textWriting(&f);
        
        textWriting<<parameter;
        
        f.close();
    }
    else{
        printf("Impossible to open file = %s\n", home.toStdString().c_str());
    }
}

//Reading Setting returned form file "home"
QString FLApp::recall_Setting(const QString& home){
    
    QString parameter("");
    
    QFile f(home);
    
    if(f.open(QFile::ReadOnly)){
        
        QTextStream textReading(&f);
        QString tutu;
        
        while(!textReading.atEnd()){
            textReading >> tutu;
        
            parameter += tutu;
            
            if(!textReading.atEnd())
                parameter += ' ';
        }
        f.close();
    }
    return parameter;
}

//Each Setting is written and read in a specific file
void FLApp::save_Settings(const QString& home){
    
    QString homeSetting = home + kCompilationFile;
    save_Setting(homeSetting, fCompilationMode);
    
    homeSetting = home + kLLVMFile;
    QString s = QString::number(fOpt_level);
    save_Setting(homeSetting, s);
    
    homeSetting = home + kStyleFile;
    save_Setting(homeSetting, fStyleChoice);

    homeSetting = fSettingsFolder + kExportUrlFile;
    save_Setting(homeSetting, fServerUrl);
    
    homeSetting = fSettingsFolder + kDropPortFile;
    QString port = QString::number(fPort);
    save_Setting(homeSetting, port);
}

void FLApp::recall_Settings(const QString& home){
    
    QString homeSetting = home + kCompilationFile;
    fCompilationMode = recall_Setting(homeSetting);
    
    homeSetting = home + kLLVMFile;
    QString opt = recall_Setting(homeSetting);
    
    homeSetting = home + kStyleFile;
    fStyleChoice = recall_Setting(homeSetting);
    
    homeSetting = home + kExportUrlFile;
    fServerUrl = recall_Setting(homeSetting);
    
    homeSetting = home + kDropPortFile;
    QString port = recall_Setting(homeSetting);
    
    if(opt.compare("") == 0){
        fOpt_level = 3;
    }
    else
        fOpt_level = opt.toInt();
    
    if(fStyleChoice.compare("") == 0){
        fStyleChoice = "Default";
    }
    
    if(fServerUrl.compare("") == 0){
        fServerUrl = "http://faustservice.grame.fr";
    }
    
    if(port.compare("") == 0){
        fPort = 7777;
    }
    else
        fPort = port.toInt();
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
        
        printf("UPDATE AUDIO \n");
        
        if(!(*it)->update_AudioArchitecture(error)){
            updateSuccess = false;
            updateFailPointer = it;
            break;
        }
    }
    
    bool reinitSuccess = true;
    
    //If init failed, reinit old audio Architecture
    if(!updateSuccess){
        
        printf("Update Audio failed\n");
        errorToPrint = "Update not successfull";
    
        fErrorWindow->print_Error(errorToPrint);
        fErrorWindow->print_Error(error);
        
        for(it = FLW_List.begin() ; it != updateFailPointer; it++)
            (*it)->stop_Audio();

        fAudioCreator->reset_Settings();    
        
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
        fAudioCreator->saveCurrentSettings();
        
        errorToPrint = "Update successfull";
        fErrorWindow->print_Error(errorToPrint);
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

//Start FaustLive Server that wraps HTTP interface in droppable environnement 
void FLApp::launch_Server(){
    
#ifndef _WIN32
    bool returning = true;
    
    if(fServerHttp == NULL){
    
        fServerHttp = new FLServerHttp();
        
       int i = 0;
       
        while(!fServerHttp->start(fPort)){
           
            QString s("Server Could Not Start On Port ");
            s += QString::number(fPort);
           
            fErrorWindow->print_Error(s);
            
           fPort++;
           
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
        s += QString::number(fPort);
        fErrorWindow->print_Error(s);
    }
#endif
}

//Stop FaustLive Server
void FLApp::stop_Server(){
#ifndef _WIN32
    if(fServerHttp != NULL){
        fServerHttp->stop();
        delete fServerHttp;
        fServerHttp = NULL;
    }
#endif
}

//Update when a file is dropped on HTTP interface (= drop in FaustLive window)
void FLApp::compile_HttpData(const char* data, int port){
#ifndef _WIN32   
  string error("");

	QString source(data);
    
   FLWindow* win = getWinFromHttp(port);
    
    if(win != NULL){
    
       update_SourceInWin(win, source);
        
        win->resetHttpInterface();
       
       string url = win->get_HttpUrl().toStdString();
       
        fServerHttp->compile_Successfull(url);
    }
   else{
       fServerHttp->compile_Failed(error);
   }
#endif    
}




