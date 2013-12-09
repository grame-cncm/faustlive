//
//  FaustLiveApp.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLApp.h"
#include "FLrenameDialog.h"
#include "FLServerHttp.h"
#include "FLWindow.h"
#include "FLErrorWindow.h"
#include "FLExportManager.h"
#include "utilities.h"

#include <sstream>

//----------------------CONSTRUCTOR/DESTRUCTOR---------------------------

void FLApp::create_Session_Hierarchy(){    
    
    //Initialization of current Session Path  
    
    fSessionFolder = getenv("HOME");
    fSessionFolder += "/.CurrentSession-";
    fSessionFolder += FLVERSION;
    if(!QFileInfo(fSessionFolder.c_str()).exists()){
        QDir direct(fSessionFolder.c_str());
        direct.mkdir(fSessionFolder.c_str());
    }
    
    fSessionFile = fSessionFolder + "/Description.sffx";
    if(!QFileInfo(fSessionFile.c_str()).exists()){
        QFile f(fSessionFile.c_str());
        f.open(QFile::ReadOnly);
        f.close();
    }    
    
    fSourcesFolder = fSessionFolder + "/Sources";
    if(!QFileInfo(fSourcesFolder.c_str()).exists()){
        QDir direct(fSourcesFolder.c_str());
        direct.mkdir(fSourcesFolder.c_str());
    }    
    
    fSettingsFolder = fSessionFolder + "/Settings";
    if(!QFileInfo(fSettingsFolder.c_str()).exists()){
        QDir direct(fSettingsFolder.c_str());
        direct.mkdir(fSettingsFolder.c_str());
    }
    
    fHomeSettings = fSettingsFolder + "/FaustLive_Settings.rf"; 
    fRecentsFile = fSettingsFolder + "/FaustLive_FileSavings.rf"; 
    fHomeRecentSessions = fSettingsFolder + "/FaustLive_SessionSavings.rf"; 
    
    fSVGFolder = fSessionFolder + "/SVG";
    if(!QFileInfo(fSVGFolder.c_str()).exists()){
        QDir direct(fSVGFolder.c_str());
        direct.mkdir(fSVGFolder.c_str());
    }  
    
    fIRFolder = fSessionFolder + "/IR";
    if(!QFileInfo(fIRFolder.c_str()).exists()){
        QDir direct(fIRFolder.c_str());
        direct.mkdir(fIRFolder.c_str());
    }  
    
    fExamplesFolder = fSessionFolder + "/Examples";
    if(!QFileInfo(fExamplesFolder.c_str()).exists()){
        QDir direct(fExamplesFolder.c_str());
        direct.mkdir(fExamplesFolder.c_str());
    }  
    
    QDir direc(fSessionFile.c_str());
}

//void FLApp::WriteInFile(const string& file, ...){
//    
//    
//}
//
//void FLApp::ReadInFile(const string& file, ...){
//    
//    va_list va;
//    va_start (va, n);
//    
//    int i;
//    
//    for (i = 0; i < n; i++)
//    {
//        int c = va_arg (va, int);
//    }
//    putchar ('\n');
//    va_end (va);
//    
//    
//    QFile f(fSessionFile.c_str());
//    QString text("");
//    
//    if(f.open(QFile::ReadOnly)){
//        
//        QTextStream textReading(&f);
//        
//        textReading>>text;
//        f.close();
//    }
//
//    return text.toStdString();
//}

FLApp::FLApp(int& argc, char** argv) : QApplication(argc, argv){
    
    fWindowBaseName = "FLW";

    //Create Session Folder
    create_Session_Hierarchy();
    
    //Initializing screen parameters
    QSize screenSize = QApplication::desktop()->geometry().size(); 
    fScreenWidth = screenSize.width();
    fScreenHeight = screenSize.height();
    
    //Initializing preference
    fOpt_level = 3;
    fServerUrl = "http://faust.grame.fr:8888";
    fPort = 7777;
    fStyleChoice = "Default";
    recall_Settings(fHomeSettings);
    styleClicked(fStyleChoice);
    
    //If no event opened a window half a second after the application was created, a initialized window is created
    fInitTimer = new QTimer(this);
    connect(fInitTimer, SIGNAL(timeout()), this, SLOT(init_Timer_Action()));
    fInitTimer->start(500);
    
    // Presentation Window Initialization
    fPresWin = new QDialog;
    fPresWin->setWindowFlags(Qt::FramelessWindowHint);
    init_presentationWindow();   
    
    //Initializing menu options 
    fRecentFileAction = new QAction* [kMAXRECENTFILES];
    fRrecentSessionAction = new QAction* [kMAXRECENTSESSIONS];
    fIrecentSessionAction = new QAction* [kMAXRECENTSESSIONS];
    
    //For the application not to quit when the last window is closed
    setQuitOnLastWindowClosed(false);
    
    fMenuBar = new QMenuBar;
    setup_Menu();
    
    recall_Recent_Files(fRecentsFile);
    recall_Recent_Sessions(fHomeRecentSessions);
    
    //Initializing preference and save dialog
    fErrorWindow = new FLErrorWindow();
    fErrorWindow->setWindowTitle("MESSAGE_WINDOW");
    fErrorWindow->init_Window();
    connect(fErrorWindow, SIGNAL(closeAll()), this, SLOT(shut_AllWindows()));
    
    fServerHttp = NULL;
    fCompilingMessage = NULL;
    launch_Server();
}

FLApp::~FLApp(){
    
    save_Recent_Files();
    save_Recent_Sessions();
    save_Settings(fHomeSettings);
    
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
    
    fSessionContent.clear();
}

//---------------------

//--Build FaustLive Menu
void FLApp::setup_Menu(){
    
    //----------------FILE
    
    QMenu* fileMenu = new QMenu;
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
    
        fExampleToOpen = (children.begin())->baseName().toStdString();
        
        QFileInfoList::iterator it;
        int i = 0; 
        
        QAction** openExamples = new QAction* [children.size()];
        
        for(it = children.begin(); it != children.end(); it++){
            
            openExamples[i] = new QAction(it->baseName(), menuOpen_Example);
            openExamples[i]->setData(QVariant(it->absoluteFilePath()));
            connect(openExamples[i], SIGNAL(triggered()), this, SLOT(open_Example_Action()));
            
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
    fileMenu->addAction(closeAllAction);
    
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
    
    //--------------------HELP
    
    fHelpWindow = new QMainWindow;
    fHelpWindow->setWindowFlags(Qt::FramelessWindowHint);
    
    this->init_HelpWindow();
    fHelpWindow->move(fScreenWidth/3, fScreenHeight/3);
    
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
}

//--Print errors in errorWindow
void FLApp::errorPrinting(const char* msg){
    
    fErrorWindow->print_Error(msg);
}

//--Starts the presentation menu if no windows are opened (session restoration or drop on icon that opens the application)
void FLApp::init_Timer_Action(){
    fInitTimer->stop();
    
    if(FLW_List.size()==0){
        
        if(QFileInfo(fSessionFile.c_str()).exists()){
            
            QFile f(fSessionFile.c_str());
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

//-----------------Operations on indexes of the windows contained in the application
list<int> FLApp::get_currentIndexes(){
    list<int> currentIndexes;
    
    list<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++){
        currentIndexes.push_back((*it)->get_indexWindow());
    }
    
    return currentIndexes;
    
}

int FLApp::find_smallest_index(list<int> currentIndexes){
    
    list<int>::iterator it;
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

bool FLApp::isIndexUsed(int index, list<int> currentIndexes){
    
    list<int>::iterator it;
    
    for(it = currentIndexes.begin(); it != currentIndexes.end(); it++){
        if(index == *it ){
            return true;
        }
    }
    return false;
}

//--Calculates the position of a new window to avoid overlapping
void FLApp::calculate_position(int index, int* x, int* y){
    
    int multiplCoef = index;
    while(multiplCoef > 20){
        multiplCoef-=20;
    }
    
    *x = fScreenWidth/3 + multiplCoef*10;
    *y = fScreenHeight/3 + multiplCoef*10;
}


//-----------------Operation on DefaultName to avoid bad indexing
list<string> FLApp::get_currentDefault(){
    
    list<string> currentDefault;
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin(); it != fSessionContent.end() ; it++){
        
        if((*it)->name.find(DEFAULTNAME)!=string::npos)
            currentDefault.push_back((*it)->name);
    }
    
    
//    printf("WIN CONTENT = %i\n", currentDefault.size());
    return currentDefault;
}

string FLApp::find_smallest_defaultName(list<string> currentDefault){
    
    //Conditional jump on currentDefault List...
    
    int index = 1;
    string nomEffet("");
    bool found = false;
    
    stringstream ss;
    
    do{
        ss.str("");
        ss << index;
        
        nomEffet = DEFAULTNAME;
        nomEffet += "-" + ss.str();
        
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

//-----------------Recognize Window from one of its parameter 

FLWindow* FLApp::getActiveWin(){
    
    list<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++) {

        if((*it)->isActiveWindow())
            return *it;
    }
    
    return NULL;
}

FLWindow* FLApp::getWinFromHttp(int port){

    list<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++) {

        if(port == (*it)->get_Port())
            return *it;
    }
    
    return NULL;
}

//--------------------------CREATE EFFECT-------------------------------------

//--Creates/Recycles an Effect depending on it's source
FLEffect* FLApp::getEffectFromSource(string source, string nameEffect, const string& sourceFolder, string& compilationOptions, int opt_Val, string& error, bool init){
    
    bool fileSourceMark = false;
    string content = "";
    string fichierSource = "";
    
    //SOURCE = URL --> source becomes text
    source = ifUrlToText(source);
    
    //SOURCE = FILE.DSP    
    if(source.find(".dsp") != string::npos){
        
        list<FLEffect*>::iterator it;
        for(it = fExecutedEffects.begin(); it!= fExecutedEffects.end(); it++){
            
            //Effect already compiled
            if(source.compare((*it)->getSource()) == 0){
                
                //Effect in current session = direct
                if(isEffectInCurrentSession((*it)->getSource())){
                    return *it;                       
                }
                //Effect gardé en memoire mais potentiellement son nom est utilisé par un autre effet dans la session courante
                else{
                    string effetName = (*it)->getName();
                    (*it)->setName(renameEffect(source, (*it)->getName()));
                    printf("RENAME 1\n");
                    
                    return *it;
                }
            }
        }
        
        // Si l'effet est pour la première fois compilé, il faut récupérer son nom et vérifier que son nom n'existe pas. 
        // Le marqueur
        
        fileSourceMark = true;
        fichierSource = source;
        
        if(!init){
            //            if(nameEffect.compare("") == 0){
            nameEffect = (QFileInfo(fichierSource.c_str()).baseName()).toStdString();
            nameEffect = renameEffect(fichierSource, nameEffect);
            printf("RENAME 2\n");
            //            }
            
            printf("NAME EFFECT = %s\n", nameEffect.c_str());
            
            while(nameEffect.find(' ') != string::npos)
                nameEffect.erase(nameEffect.find(' '), 1);
        }
    }
    
    //SOURCE = TEXT
    else{
        
        string name = getDeclareName(source);
        
        printf("NAME OF EFFECT = %s\n", name.c_str());
        
        if(name.compare("") == 0){
            //            list<string> currentDefault = get_currentDefault();
            string defaultName = find_smallest_defaultName(get_currentDefault());
            name = defaultName;
        }
        
        //Erase the spaces because it brings problems ^^
        while(name.find(' ') != string::npos)
            name.erase(name.find(' '), 1);
        
        fichierSource = sourceFolder + "/" + name + ".dsp";
        
        if(QFileInfo(fichierSource.c_str()).exists()){
            content = pathToContent(fichierSource);
            //            printf("CONTENT = %s\n", content.c_str());
        }
        createSourceFile(fichierSource, source);
        nameEffect = name;
    }
    
    display_CompilingProgress("Compiling your DSP...");
    
    FLEffect* myNewEffect = new FLEffect(init, fichierSource, nameEffect);
    
    if(myNewEffect->init(fSVGFolder, fIRFolder, compilationOptions, opt_Val, error)){
        
        StopProgressSlot();
        
        connect(myNewEffect, SIGNAL(effectChanged()), this, SLOT(synchronize_Window()));
        
        fExecutedEffects.push_back(myNewEffect);
        
        return myNewEffect;
    }
    else{
        
        // If init failed  
        if(!fileSourceMark){
            
            // If file did not exist, it is removed
            if(content.compare("") == 0){
                
                if(QFile(fichierSource.c_str()).remove())
                    printf("FICHIER SOURCE WAS REMOVED\n");
            }
            // Otherwise, its previous content is restored
            else
                createSourceFile(fichierSource, content);
        }
        
        StopProgressSlot();
        delete myNewEffect;
        return NULL;
    }
}

//--Creates an new file in the Source Folder of the Current Session
void FLApp::createSourceFile(const string& sourceName, const string& content){
    
    QFile f(sourceName.c_str());
    
    if(f.open(QFile::WriteOnly)){
        
        QTextStream textWriting(&f);
        
        textWriting<<content.c_str();
        
        f.close();
    }
}

//--Update FileName
void FLApp::update_Source(const string& oldSource, const string& newSource){
    
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

//--Return declare name if there is one.
//--If the name exists in the session, the user is asked to choose another one
string FLApp::getDeclareName(string text){
    
    string returning = "";
    size_t pos = text.find("declare name");
    
    if(pos != string::npos){
        text.erase(0, pos);
        
        pos=text.find("\"");
        if(pos != string::npos){
            text.erase(0, pos+1);
        }
        pos = text.find("\"");
        text.erase(pos, text.length()-pos);
        
        text = renameEffect("", text);
        
        returning = text;
    }
    
    return returning;
}

//--Dialog with the user to modify the name of a faust application
string FLApp::renameEffect(const string& source, const string& nomEffet){
    
    string newName(nomEffet);
    
    while(isEffectNameInCurrentSession(source , newName)){
    
        FLrenameDialog* Msg = new FLrenameDialog(newName, 0);
        Msg->raise();
        Msg->exec();
        newName = Msg->getNewName();
        
        while(newName.find(' ') != string::npos)
            newName.erase(newName.find(' '), 1);
        
        delete Msg;
    }
    return newName;
}

//--Transforms an Url into Text if it is one.
string FLApp::ifUrlToText(const string& source){
    
    //In case the text dropped is a web url
    size_t pos = source.find("http://");
    
    string UrlText(source);
    
    if(pos != string::npos){
        UrlText = "process = component(\"";
        UrlText += source;
        UrlText +="\");";
//        source = UrlText;
    }
    
    return UrlText;
}

//--Find name of an Effect in CurrentSession, depending on its source file
string FLApp::getNameEffectFromSource(const string& sourceToCompare){
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
        
        if((*it)->source.compare(sourceToCompare) == 0)
            return (*it)->name;
    }
    return "";
}

//--Finds out if an Effect exist in CurrentSession, depending on its source file
bool FLApp::isEffectInCurrentSession(const string& sourceToCompare){
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
        
        if((*it)->source.compare(sourceToCompare) == 0)
            return true;
    }
    return false;
    
}

//--Returns the list of the names of all running Effects 
list<string> FLApp::getNameRunningEffects(){
    
    list<string> returning; 
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++)
        returning.push_back((*it)->name);   
    
    return returning;
}

//Finds out if an Effect Name exist in CurrentSession
//DefaultName are not taken into account
//If the sources of the 2 effects are the same, then the effects are the same and not only their names.
bool FLApp::isEffectNameInCurrentSession(const string& sourceToCompare , const string& nom){
    
    list<int> returning;
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
        
        if((*it)->name.compare(nom)==0 && ((*it)->source.compare(sourceToCompare) != 0 || (*it)->name.find(DEFAULTNAME) != string::npos))
            
            return true;
    }
    return false;
}

//Lists all the windows (through their indexes) that contain the same Effect
list<int> FLApp::WindowCorrespondingToEffect(FLEffect* eff){
    
    list<int> returning;
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
        
        if((*it)->source.compare(eff->getSource()) == 0)
            returning.push_back((*it)->ID);
    }
    return returning;
}

//Delete File and Folder directly related to a window
void FLApp::removeFilesOfWin(const string& sourceName, const string& effName){
    
    QFile::remove(sourceName.c_str());
    
    string irFile = fIRFolder + "/" + effName;
    QFile::remove(irFile.c_str());
    
    string svgFolder = fSVGFolder + "/" + effName + "-svg";
    deleteDirectoryAndContent(svgFolder);
    
}

//Refresh a window. In case the source has been modified.
void FLApp::synchronize_Window(){ 

    FLEffect* modifiedEffect = (FLEffect*)QObject::sender();
    
    string modifiedSource = modifiedEffect->getSource();
    string error;
    
    QDateTime modifiedLast = QFileInfo(modifiedSource.c_str()).lastModified();
    QDateTime creationDate = modifiedEffect->get_creationDate();
    
//Avoiding the flicker when the source is saved
    if(QFileInfo(modifiedSource.c_str()).exists() && (modifiedEffect->isSynchroForced() || creationDate<modifiedLast)){
        
        modifiedEffect->setForceSynchro(false);
        
        modifiedEffect->stop_Watcher();
        
        //        display_CompilingProgress("Updating your DSP...");
        
        bool update = modifiedEffect->update_Factory(error, fSVGFolder, fIRFolder);
        
        if(!update){
            //            StopProgressSlot();
            fErrorWindow->print_Error(error.c_str());
            modifiedEffect->launch_Watcher();
            return;
        }
        else if(error.compare("") != 0){
            fErrorWindow->print_Error(error.c_str());
        }
        
        //        StopProgressSlot();
        
        list<int> indexes = WindowCorrespondingToEffect(modifiedEffect);
        
        list<int>::iterator it;
        for(it=indexes.begin(); it!=indexes.end(); it++){
            list<FLWindow*>::iterator it2;
            
            for (it2 = FLW_List.begin(); it2 != FLW_List.end(); it2++) {
                if((*it2)->get_indexWindow() == *it){
                    if(!(*it2)->update_Window(kCrossFade, modifiedEffect, modifiedEffect->getOptValue(),error)){
                        fErrorWindow->print_Error(error.c_str());
                        break;
                    }
                    else{
                        
                        //                        printf("WINDOW INDEX = %i\n", *it);
                        
                        deleteWinFromSessionFile(*it2);
                        QString name = (*it2)->get_nameWindow().c_str();
                        name+=" : ";
                        name+= (*it2)->get_Effect()->getName().c_str();
                        (*it2)->deleteWinInMenu(name);
                        addWinToSessionFile(*it2);
                        
                        string oldSource = modifiedEffect->getSource();
                        string newSource = fSourcesFolder + "/" + modifiedEffect->getName() + ".dsp";
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
//In case the file is erased during the execution
    else if(!QFileInfo(modifiedSource.c_str()).exists()){
        
        modifiedEffect->stop_Watcher();
        
        error = "WARNING = ";
        error += modifiedSource.c_str(); 
        error += "has been deleted or moved\n You are now working on its copy.";
        fErrorWindow->print_Error(error.c_str());
        
        string toReplace = fSourcesFolder + "/" + modifiedEffect->getName() +".dsp";      
        modifiedEffect->setSource(toReplace);
        modifiedEffect->launch_Watcher();
    }
    
}

//Modify the content of a specific window with a new source
void FLApp::update_SourceInWin(FLWindow* win, const string& source){
    
    string error;
    string empty("");
    
    //Deletion of reemplaced effect from session
    FLEffect* leavingEffect = win->get_Effect();
    leavingEffect->stop_Watcher();
    deleteWinFromSessionFile(win);
    
    QString name = win->get_nameWindow().c_str();
    name+=" : ";
    name+= win->get_Effect()->getName().c_str();
    
    win->deleteWinInMenu(name);
    
    
    FLEffect* newEffect = getEffectFromSource(source, empty, fSourcesFolder, fCompilationMode, fOpt_level, error, false);
    
    bool optionChanged;
    
    if(newEffect != NULL)
        optionChanged = (fCompilationMode.compare(newEffect->getCompilationOptions()) != 0 || fOpt_level != (newEffect->getOptValue())) && !isEffectInCurrentSession(newEffect->getSource());
    
    
    if(newEffect == NULL || (!(win)->update_Window(kCrossFade, newEffect, fOpt_level, error))){
        //If the change fails, the leaving effect has to be reimplanted
        leavingEffect->launch_Watcher();
        addWinToSessionFile(win);
        fErrorWindow->print_Error(error.c_str());
        return;
    }
    else{
        
        //ICI ON VA FAIRE LA COPIE DU FICHIER SOURCE
        string copySource = fSourcesFolder +"/" + newEffect->getName() + ".dsp";
        string toCopy = newEffect->getSource();
        
        update_Source(toCopy, copySource);
        
        if(error.compare("") != 0){
            fErrorWindow->print_Error(error.c_str());
        }
        
        //If the change is successfull : 
        QFileInfo ff(leavingEffect->getSource().c_str());
        string toCompare = ff.absolutePath().toStdString();
        
        string inter = leavingEffect->getSource();
        
        //If leaving effect is not used elsewhere : 
        if(!isEffectInCurrentSession(inter)){                        
            if(toCompare.compare(fSourcesFolder) == 0){
                
                //The effects pointing in the Sources Folder are not kept (nor in the list of exectued Effects, nor the source file)
                //If newEffect source = oldEffect source the file is kept because it has been modified and is needed
                if(newEffect->getSource().compare(leavingEffect->getSource())!=0)
                    removeFilesOfWin(leavingEffect->getSource(), leavingEffect->getName().c_str());
                //                        QFile::remove(leavingEffect->getSource().c_str());
                
                fExecutedEffects.remove(leavingEffect);
                delete leavingEffect;
            }
            else{
                //The copy made of the source is erased
                string toErase = fSourcesFolder + "/" + leavingEffect->getName() + ".dsp";
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
            newEffect->update_compilationOptions(fCompilationMode, fOpt_level);
    }
    
}

//--------------------------------FILE----------------------------------------

//---------------NEW

void FLApp::redirectMenuToWindow(FLWindow* win){
    
    win->set_RecentFile(fRecentFiles);
    win->update_RecentFileMenu();
    
    win->set_RecentSession(fRecentSessions);
    win->update_RecentSessionMenu();
    win->initNavigateMenu(fFrontWindow);
    
    connect(win, SIGNAL(drop(list<string>)), this, SLOT(drop_Action(list<string>)));
    
    connect(win, SIGNAL(error(const char*)), this, SLOT(errorPrinting(const char*)));
    connect(win, SIGNAL(create_Empty_Window()), this, SLOT(create_Empty_Window()));
    connect(win, SIGNAL(open_New_Window()), this, SLOT(open_New_Window()));
    connect(win, SIGNAL(open_Ex(QString)), this, SLOT(open_Example_Action(QString)));
    connect(win, SIGNAL(open_File(string)), this, SLOT(open_Recent_File(string)));
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
    connect(win, SIGNAL(recall_Snapshot(string, bool)), this, SLOT(recall_Snapshot(string, bool)));
    connect(win, SIGNAL(front(QString)), this, SLOT(frontShow(QString)));
}

//--Creation of a new window
FLWindow* FLApp::new_Window(const string& mySource, string& error){
    
    bool init = false;
    
    string source(mySource);
    
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
    string empty("");
    FLEffect* first = getEffectFromSource(source, empty, fSourcesFolder, fCompilationMode, fOpt_level ,error, false); 
    
    if(first != NULL){
        
        bool optionChanged = (fCompilationMode.compare(first->getCompilationOptions()) != 0 || fOpt_level != (first->getOptValue())) && !isEffectInCurrentSession(first->getSource());
        
        //Copy of the source File in the CurrentSession Source Folder
        string copySource = fSourcesFolder +"/" + first->getName() + ".dsp";
        string toCopy = first->getSource();
        
        update_Source(toCopy, copySource);
        
        printf("Update Source\n");
        
        if(error.compare("") != 0){
            fErrorWindow->print_Error(error.c_str());
        }
        
        int x, y;
        calculate_position(val, &x, &y);
        
        FLWindow* win = new FLWindow(fWindowBaseName, val, first, x, y, fSessionFolder, fPort);
        
        printf("WIN = %p\n", win);
        
        redirectMenuToWindow(win);
        
        if(win->init_Window(init, false, error)){
            
            printf("INIT\n");
            
            FLW_List.push_back(win);
            addWinToSessionFile(win);
            
            first->launch_Watcher();
            
            //In case the compilation options have changed...
            if(optionChanged)
                first->update_compilationOptions(fCompilationMode, fOpt_level);
            
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
void FLApp::create_New_Window(const string& source){
    
    string error("");
    
    if(new_Window(source, error) == NULL){
        fErrorWindow->print_Error(error.c_str());
        printf("NULLLLLLLLLL\n");
    }
    
}

//--Creation of Default Window from Menu
void FLApp::create_Empty_Window(){ 
    string empty("");
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
        string name = fileName.toStdString();
        
        if(name.find(".tar") != string::npos)
            recall_Snapshot(name, true);
        if(name.find(".dsp") != string::npos)
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

void FLApp::open_Example_Action(QString pathInQResource){
    
    QFileInfo toOpen(pathInQResource);
    fExampleToOpen = toOpen.baseName().toStdString();
    open_Example_Action();
    
}

void FLApp::open_Example_Action(){
    
    QDir examplesDir(":/");
    
    if(examplesDir.cd("Examples")){
    
        string path = fExampleToOpen + ".dsp";
        
        if(QFileInfo(examplesDir.absoluteFilePath(path.c_str())).exists()){
            
            path = examplesDir.absoluteFilePath(path.c_str()).toStdString();
            
            string pathInSession = fExamplesFolder + "/" + fExampleToOpen + ".dsp";
            
            QFile file(examplesDir.absoluteFilePath(path.c_str()));
            
            file.copy(pathInSession.c_str());
            
            fPresWin->hide();
            
            FLWindow* win = getActiveWin();
            
            if(win != NULL && win->is_Default())
                update_SourceInWin(win, pathInSession);
            else
                create_New_Window(pathInSession);
        }
    }
}

//-------------OPEN RECENT

//--Save/Recall from file 
void FLApp::save_Recent_Files(){
    
    QFile f(fRecentsFile.c_str());
    
    if(f.open(QFile::WriteOnly | QFile::Truncate)){
        
        QTextStream text(&f);
        
        list<pair<string, string> >::iterator it;
        
        for (it = fRecentFiles.begin(); it != fRecentFiles.end(); it++) {
            QString toto = it->first.c_str();
            QString tata = it->second.c_str();
            text << toto <<' '<<tata<< endl;
        }
        f.close();
    }
}

void FLApp::recall_Recent_Files(const string& filename){
    
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
    
    list<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++)
        (*it)->set_RecentFile(fRecentFiles);
}

//--Add new recent file
void FLApp::set_Current_File(const string& pathName, const string& effName){
    
    pair<string,string> myPair = make_pair(pathName, effName);
    
    fRecentFiles.remove(myPair);
    fRecentFiles.push_front(myPair);
    
    update_Recent_File();
    
    list<FLWindow*>::iterator it;
    
    for(it = FLW_List.begin(); it!=FLW_List.end() ; it++){
        (*it)->set_RecentFile(fRecentFiles);
        (*it)->update_RecentFileMenu();
    }
        
}

//--Visual Update
void FLApp::update_Recent_File(){
    
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

void FLApp::open_Recent_File(){
    QAction* action = qobject_cast<QAction*>(sender());
    string toto(action->data().toString().toStdString());
    
    open_Recent_File(toto);
}

//--Open a recent file
void FLApp::open_Recent_File(const string& toto){
    
    FLWindow* win = getActiveWin();
    
    if(win != NULL && win->is_Default())
        update_SourceInWin(win, toto);
    else
        create_New_Window(toto);
}

//--------------------------------SESSION----------------------------------------

//Write Current Session Properties into a File
void FLApp::sessionContentToFile(const string& filename){
    
    //    printf("SIZE OF CONTENT SESSION = %i\n", session->size());
    
    QFile f(filename.c_str());
    
    if(f.open(QFile::WriteOnly | QIODevice::Truncate)){
        
        list<WinInSession*>::iterator it;
        
        QTextStream textWriting(&f);
        
        for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
            
            //     printf("ID = %i// Source = %s // Name = %s // X = %f // Y = %f // Option = %s\n", (*it)->ID, (*it)->source.c_str(), (*it)->name.c_str(), (*it)->x, (*it)->y, (*it)->compilationOptions.c_str());
            
            
            textWriting<<(*it)->ID<<' '<<QString((*it)->source.c_str())<<' '<<QString((*it)->name.c_str())<<' '<<(*it)->x<<' '<<(*it)->y<<' '<<QString((*it)->compilationOptions.c_str())<<' '<<(*it)->opt_level<<' '<<(*it)->portHttpd<<' '<<(*it)->remoteServer.c_str()<<endl;
            
        }
        f.close();
    }
}

//Read Session File
void FLApp::fileToSessionContent(const string& filename, list<WinInSession*>* session){
    
    QFile f(filename.c_str());
    
    if(f.open(QFile::ReadOnly)){
        
        list<WinInSession*>::iterator it;
        
        QTextStream textReading(&f);
        
        while(!textReading.atEnd()){
            
            int id = 0;
            int opt, port;
            QString Source, Nom, CompilationOptions, ServerIP;
            float x,y;
            
            textReading>>id>>Source>>Nom>>x>>y>>CompilationOptions>>opt>>port>>ServerIP;
            
            if(id != 0){
                
                WinInSession* intermediate = new WinInSession();
                intermediate->ID = id;
                intermediate->source = Source.toStdString();
                intermediate->name = Nom.toStdString();
                intermediate->x = x;
                intermediate->y = y;
                intermediate->compilationOptions = CompilationOptions.toStdString();
                intermediate->opt_level = opt;
                intermediate->portHttpd = port;
                intermediate->remoteServer = ServerIP.toStdString();
                
                //                printf("FILLING ID = %i// Source = %s // Name = %s // X = %f // Y = %f // Option = %s\n", intermediate->ID, intermediate->source.c_str(), intermediate->name.c_str(), intermediate->x, intermediate->y, intermediate->compilationOptions.c_str());
                
                session->push_back(intermediate);
                //                printf("SNAPSHOT SIZE = %i\n", session->size());
            }
        }
        f.close();
    }
}

//Spaces in Compilation Options are erased/then restored for saving process.
string FLApp::convert_compilationOptions(string compilationOptions){
    
    size_t pos = compilationOptions.find(" ");
    
    while(pos != string::npos){
        
        compilationOptions.erase(pos, 1);
        compilationOptions.insert(pos, "/");
        
        pos = compilationOptions.find(" ");
    }
    
    return compilationOptions;
}

string FLApp::restore_compilationOptions(string compilationOptions){
    
    size_t pos = compilationOptions.find("/");
    
    while(pos != string::npos){
        
        compilationOptions.erase(pos, 1);
        compilationOptions.insert(pos, " ");
        
        pos = compilationOptions.find("/");
    }
    
    return compilationOptions;
}

//Recall for any type of session (current or snapshot)
void FLApp::recall_Session(const string& filename){
    
    //    printf("FILENAME TO RECALL = %s\n", filename.c_str());
    
    //Temporary copies of the description files in which all the modifications due to conflicts will be saved
    list<WinInSession*>  snapshotContent;
    
    fileToSessionContent(filename, &snapshotContent);
    
    if(filename.compare(fSessionFile) == 0){
        
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
        
        string nameW = fWindowBaseName + "-" + s1.str();
        
        stringstream s2;
        s2<<it2->second;
        string newNameW = fWindowBaseName + "-" + s2.str();
        
        windowNameChanges.push_back(make_pair(nameW, newNameW));
    }
    
    list<std::pair<string,string> > nameChanges = establish_nameChanges(&snapshotContent);
    
    //In case of Snapshot Restoration, sources and window parameters Folders have to be copied in currentSession
    if((QFileInfo(filename.c_str()).absolutePath().toStdString()).compare(fSessionFolder) != 0){
        establish_sourceChanges(nameChanges, &snapshotContent);
        
        QFileInfo sourceDir(filename.c_str());
        string snapshotSourcesFolder = sourceDir.absolutePath().toStdString() + "/Sources"; 
        copy_AllSources(snapshotSourcesFolder, fSourcesFolder, nameChanges,".dsp");
        string snapshotIRFolder = sourceDir.absolutePath().toStdString() + "/IR"; 
        copy_AllSources(snapshotIRFolder, fIRFolder, nameChanges, "");
        string snapshotSVGFolder = sourceDir.absolutePath().toStdString() + "/SVG";         
        copy_SVGFolders(snapshotSVGFolder, fSVGFolder, nameChanges);
        
        string snapshotFolder = sourceDir.absolutePath().toStdString();
        
        copy_WindowsFolders(snapshotFolder, fSessionFolder, windowNameChanges);
        
        
    }//Otherwise, one particular case has to be taken into account. If the content was modified and the Effect NOT recharged from source. The original has to be recopied!??
    else{
        
    }
    
    //--------------Recalling without conflict the session
    
    list<WinInSession*>::iterator it;
    
    for(it = snapshotContent.begin() ; it != snapshotContent.end() ; it ++){
        
        string error;
        
        (*it)->compilationOptions = restore_compilationOptions((*it)->compilationOptions);
        
        FLEffect* newEffect = getEffectFromSource((*it)->source, (*it)->name, fSourcesFolder, (*it)->compilationOptions, (*it)->opt_level, error, true);
        
        //ICI ON NE VA PAS FAIRE LA COPIE DU FICHIER SOURCE!!!
        
        if(newEffect != NULL){
            
            if(error.compare("") != 0){
                fErrorWindow->print_Error(error.c_str());
            }
            
            FLWindow* win = new FLWindow(fWindowBaseName, (*it)->ID, newEffect, (*it)->x*fScreenWidth, (*it)->y*fScreenHeight, fSessionFolder, fPort, (*it)->portHttpd, (*it)->remoteServer);
            
            redirectMenuToWindow(win);
            
            //Modification of connection files with the new window & effect names
            
            win->update_ConnectionFile(windowNameChanges);
            win->update_ConnectionFile(nameChanges);
            
            if(win->init_Window(false, true, error)){
                
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
                fErrorWindow->print_Error(error.c_str());    
            }
        }
        else{
            fErrorWindow->print_Error(error.c_str());
        }
    }
}

//--------------RECENTLY OPENED

//--Save/Recall from file
void FLApp::save_Recent_Sessions(){
    
    QFile f(fHomeRecentSessions.c_str());
    
    if(f.open(QFile::WriteOnly | QFile::Truncate)){
        
        QTextStream text(&f);
        
        for (int i = min(kMAXRECENTSESSIONS, fRecentSessions.size()) - 1; i>=0; i--) {
            QString toto = fRecentSessions[i];
            text << toto << endl;
        }
        f.close();      
    }

}

void FLApp::recall_Recent_Sessions(const string& filename){
    
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
    
    list<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++)
        (*it)->set_RecentSession(fRecentSessions);
}

//Add new recent session
void FLApp::set_Current_Session(const string& pathName){
    
    //    printf("SET CURRENT SESSION = %s\n", pathName.c_str());
    
    QString currentSess = pathName.c_str();
    fRecentSessions.removeAll(currentSess);
    fRecentSessions.prepend(currentSess);
    update_Recent_Session();
    
    list<FLWindow*>::iterator it;
    
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
            
            //            printf("TEXT = %s\n", text.toStdString().c_str());
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
    string toto(action->data().toString().toStdString());
    recall_Snapshot(toto, false);
}

void FLApp::import_Recent_Session(){
    
    QAction* action = qobject_cast<QAction*>(sender());
    string toto(action->data().toString().toStdString());
    recall_Snapshot(toto, true);
}

//---------------CURRENT SESSION FUNCTIONS

//Add window in Current Session Structure
void FLApp::addWinToSessionFile(FLWindow* win){
    
    string compilationOptions = convert_compilationOptions(win->get_Effect()->getCompilationOptions());
    
    if(compilationOptions.compare("") == 0)
        compilationOptions = "/";
    
    WinInSession* intermediate = new WinInSession;
    intermediate->ID = win->get_indexWindow();
    intermediate->source = win->get_Effect()->getSource().c_str();
    intermediate->name = win->get_Effect()->getName().c_str();
    intermediate->x = (float)win->get_x()/(float)fScreenWidth;
    intermediate->y = (float)win->get_y()/(float)fScreenHeight;
    intermediate->compilationOptions = compilationOptions.c_str();
    intermediate->opt_level = win->get_Effect()->getOptValue();
    intermediate->portHttpd = win->get_Port();
    intermediate->remoteServer = win->get_RemoteServerIP();
    
    QString name = win->get_nameWindow().c_str();
    name+=" : ";
    name+= win->get_Effect()->getName().c_str();
    
    fFrontWindow.push_back(name);
    
    fSessionContent.push_back(intermediate);
    
    list<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++)
        (*it)->addWinInMenu(name);
}

//Add window from Current Session Structure
void FLApp::deleteWinFromSessionFile(FLWindow* win){
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it++){
        
        if((*it)->ID == win->get_indexWindow()){
            //            printf("REMOVING = %i\n", win->get_indexWindow());
            fSessionContent.remove(*it);
            
            //            QAction* toRemove = NULL;
            
            QList<QString>::iterator it2;
            for(it2 = fFrontWindow.begin(); it2 != fFrontWindow.end() ; it2++){
                
                QString name = win->get_nameWindow().c_str();
                name+=" : ";
                name+= win->get_Effect()->getName().c_str();
                
                if((*it2).compare(name) == 0){
                    fFrontWindow.removeOne(*it2);
                    
                    list<FLWindow*>::iterator it3;
                    
                    for (it3 = FLW_List.begin(); it3 != FLW_List.end(); it3++){
                     
                        if(win != *it3)
                            (*it3)->deleteWinInMenu(name);
                    }
                    //                    toRemove = *it;
                    break;
                }
            }
            //            delete toRemove;
            break;
        }
    }
}

//Update Current Session Structure with current parameters of the windows
void FLApp::update_CurrentSession(){
    
    list<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++){
        
        deleteWinFromSessionFile(*it);
        addWinToSessionFile(*it);
        
        (*it)->save_Window();
    }
}

//Reset Current Session Folder
void FLApp::reset_CurrentSession(){
    
    QDir srcDir(fSessionFolder.c_str());
    
    QFileInfoList children = srcDir.entryInfoList(QDir::Dirs | QDir::Drives | QDir::NoDotAndDotDot);
    
    QFileInfoList::iterator it;
    
    for(it = children.begin(); it != children.end(); it++){
        
        string toRemove(it->absoluteFilePath().toStdString());
        //        printf("WATH IS BEING REMOVED??? = %s\n", toRemove.c_str());
        deleteDirectoryAndContent(toRemove);
    }
    
    QDir nv(fSessionFolder.c_str());
    QString ss(fSessionFolder.c_str());
    ss += "/Sources";
    nv.mkdir(ss);
    
    QString svg(fSessionFolder.c_str());
    svg += "/SVG";
    nv.mkdir(svg);
    
    QString ir(fSessionFolder.c_str());
    ir += "/IR";
    nv.mkdir(ir);
    
    fSessionContent.clear();
    
    recall_Settings(fHomeSettings);
}

//Behaviour of session restoration when re-starting the application
//The user is notified in case of source file lost or modified. He can choose to reload from original file or backup.
void FLApp::currentSessionRestoration(list<WinInSession*>* session){
    
    //If 2 windows are pointing on the same lost source, the Dialog has not to appear twice
    std::map<string,bool> updated;
    
    //List of the sources to updated in Session File
    list<pair<string, string> > sourceChanges;
    
    list<WinInSession*>::iterator it;
    
    for(it = session->begin() ; it != session->end() ; it++){
        
        string contentOrigin("");
        string contentSaved("");
        contentOrigin = pathToContent((*it)->source);
        string sourceSaved = fSourcesFolder + "/" + (*it)->name + ".dsp";
        contentSaved = pathToContent(sourceSaved);
        
        QFileInfo infoSource((*it)->source.c_str());
        //If one source (not in the Source folder) couldn't be found, the User is asked to decide whether to reload it from the copied file
        if(updated.find((*it)->source) == updated.end() && infoSource.absolutePath().toStdString().compare(fSourcesFolder) != 0 && (!infoSource.exists() || contentSaved.compare(contentOrigin) != 0) ){
            
            string mesg;
            bool contentModif = false;
            
            if(!infoSource.exists())
                mesg = (*it)->source + " cannot be found! Do you want to reload it from a copied file?";
            else{
                mesg = "The content of " + (*it)->source + " has been modified, do you want to reload it from a copied file?";
                contentModif = true;
            }
            
            QPushButton* yes_Button;
            QPushButton* cancel_Button; 
            
            QString msg(mesg.c_str());
            
            QMessageBox* existingNameMessage = new QMessageBox(QMessageBox::Warning, tr("Notification"), msg);
            
            yes_Button = existingNameMessage->addButton(tr("Copied File"), QMessageBox::AcceptRole);
            cancel_Button = existingNameMessage->addButton(tr("Original File"), QMessageBox::RejectRole);
            
            existingNameMessage->exec();
            updated[(*it)->source] = true;
            
            if (existingNameMessage->clickedButton() == cancel_Button) {
                delete existingNameMessage;
                
                string toErase = fSourcesFolder + "/" + (*it)->name + ".dsp";
                removeFilesOfWin(toErase, (*it)->name);
                if(!contentModif){
                    deleteLineIndexed((*it)->ID);
                    //                    string toErase = currentSourcesFolder + "/" + (*it)->name + ".dsp";
                    //                    removeFilesOfWin(toErase, (*it)->name);
                }
                else{
                    string newSource = fSourcesFolder + "/" + (*it)->name + ".dsp";
                    QFile file((*it)->source.c_str());
                    file.copy(newSource.c_str());
                }
            }    
            else{
                delete existingNameMessage;
                string newSource = fSourcesFolder + "/" + (*it)->name + ".dsp";
                sourceChanges.push_back(make_pair((*it)->source, newSource));
                (*it)->source = newSource;
            }
        }
        //If the source was in the Source Folder and couldn't be found, it can't be reloaded.
        else if(updated.find((*it)->source) == updated.end() && infoSource.absolutePath().toStdString().compare(fSourcesFolder) == 0 && !infoSource.exists()){
            
            deleteLineIndexed((*it)->ID);
            string msg = (*it)->name + " could not be reload. The File is lost!"; 
            fErrorWindow->print_Error(msg.c_str());
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

//Save the current State in SnapshotFolder.tar
//It copies the hidden Session Folder and compresses it
void FLApp::take_Snapshot(){
    
    QFileDialog* fileDialog = new QFileDialog;
    fileDialog->setConfirmOverwrite(true);
    
    string filename = fileDialog->getSaveFileName(NULL, "Take Snapshot", tr(""), tr("(*.tar)")).toStdString();
    
    printf("filename = %s\n", filename.c_str());
    
    //If no name is placed, nothing happens
    if(filename.compare("") != 0){
        
        size_t pos = filename.find(".tar");
        
        if(pos != string::npos)
            filename = filename.substr(0, pos);
        
        update_CurrentSession();
        sessionContentToFile(fSessionFile);
        
        //Copy of current Session under a new name, at a different location
        cpDir(fSessionFolder.c_str(), filename.c_str());
        
        //        string descriptionFile = filename + "/Description.sffx";
        
        QProcess myCmd;
        QByteArray error;
        
        string systemInstruct("tar cfv ");
        systemInstruct += filename + ".tar " + filename;
        
        myCmd.start(systemInstruct.c_str());
        myCmd.waitForFinished();
        
        error = myCmd.readAllStandardError();
        
        if(strcmp(error.data(), "") != 0)
            fErrorWindow->print_Error(error.data());
        
        QProcess myCmd2;
        QByteArray error2;
        
        string rmInstruct("rm -r ");
        rmInstruct += filename;
        
        myCmd2.start(rmInstruct.c_str());
        myCmd2.waitForFinished();
        
        error2 = myCmd2.readAllStandardError();
        
        if(strcmp(error2.data(), "") != 0)
            fErrorWindow->print_Error(error2.data());
        
        string sessionName =  filename + ".tar ";
        set_Current_Session(sessionName);
        
        //        deleteDirectoryAndContent(filename);
    }
}

//---------------RESTORE SNAPSHOT FUNCTIONS

//Behaviour of session restoration when opening a snapshot
//The user is notified that the backup file has been used to restore exact state.
void FLApp::snapshotRestoration(const string& file, list<WinInSession*>* session){
    
    //If 2 windows are pointing on the same lost source, the Dialog has not to appear twice
    std::map<string,bool> updated;
    
    list<pair<string,string> > sourceChanges;
    
    list<WinInSession*>::iterator it;
    for(it = session->begin() ; it != session->end() ; it++){
        
        
        QFileInfo infoSource((*it)->source.c_str());
        
        string contentOrigin("");
        contentOrigin = pathToContent((*it)->source);
        
        string sourceSaved = QFileInfo(file.c_str()).absolutePath().toStdString() + "/Sources/" + (*it)->name + ".dsp";
        string contentSaved("");
         contentSaved = pathToContent(sourceSaved);
        
        //If one source (not in the Source folder) couldn't be found, the User is informed that we are now working on the copy
        if(updated.find((*it)->source) == updated.end() && infoSource.absolutePath().toStdString().compare(fSourcesFolder) != 0  && (!infoSource.exists() || contentSaved.compare(contentOrigin) != 0)){
            
            string error;
            
            if(!infoSource.exists()){
                error = "WARNING = ";
                error += (*it)->source.c_str();
                error += "cannot be found! It is reloaded from a copied file.";
            }
            else if(contentSaved.compare(contentOrigin) != 0){
                error = "WARNING = The content of ";
                error += (*it)->source.c_str();
                error += "has been modified! It is reloaded from a copied file.";
            }
            fErrorWindow->print_Error(error.c_str());
            string newSource = fSourcesFolder + "/" + (*it)->name + ".dsp";
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

void FLApp::recallSnapshotFromMenu(){
    
    QString fileName = QFileDialog::getOpenFileName(NULL, tr("Recall a Session"), "",tr("Files (*.tar)"));
    
    string filename = fileName.toStdString();
    
    if(filename != ""){
        
        recall_Snapshot(filename, false);
    }
}

void FLApp::importSnapshotFromMenu(){
    
    QString fileName = QFileDialog::getOpenFileName(NULL, tr("Recall a Session"), "",tr("Files (*.tar)"));
    
    string filename = fileName.toStdString();
    
    if(filename != ""){
        
        recall_Snapshot(filename, true);
    }
}

void FLApp::recall_Snapshot(const string& filename, bool importOption){ 
    
	fRecalling = true;

    set_Current_Session(filename);
    
    QProcess myCmd;
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QByteArray error;
    
    string systemInstruct("tar xfv ");
    systemInstruct += filename +" -C /";
    
	printf("Recall process\n");

	myCmd.setProcessEnvironment(env);
    myCmd.start(systemInstruct.c_str());
    myCmd.waitForFinished();
    
    error = myCmd.readAllStandardError();
    
    if(strcmp(error.data(), "") != 0)
        fErrorWindow->print_Error(error.data());
    
    if(!importOption){
        shut_AllWindows(); 
        reset_CurrentSession();
        fExecutedEffects.clear();
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
        fErrorWindow->print_Error(error2.data());

	fRecalling = false;
}

//---------------RENAMING AND ALL FUNCTIONS TO IMPORT

void FLApp::deleteLineIndexed(int index){
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin(); it != fSessionContent.end() ; it++){
        
        //Check if line wasn't empty
        if((*it)->ID == index){
            fSessionContent.remove(*it);
            break;    
        }
    }
}

list<std::pair<int, int> > FLApp::establish_indexChanges(list<WinInSession*>* session){
    
    //For each window of the session File, if it's index is used (in current session or by the previous windows re-indexed), it is associated to a new one (the smallest not used)
    
    list<std::pair<int,int> > returning;
    
    list<WinInSession*>::iterator it;
    
    list<int> currentIndexes = get_currentIndexes();
    
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

list<std::pair<string,string> > FLApp::establish_nameChanges(list<WinInSession*>* session){
    
    //For each window of the session File, if it's index is used (in current session or by the previous windows re-indexed), it is associated to a new one (the smallest not used)
    
    list<std::pair<string,string> > nameChanges;
    list<string> currentDefault = get_currentDefault();
    
    //If 2 windows are holding the same name & source, to avoid renaming them twice
    std::map<string,bool> updated;
    
    list<WinInSession*>::iterator it;
    
    for(it = session->begin(); it != session->end() ; it++){
        
        string newName = (*it)->name;
        
        //1- If the source is already is current Session (& not pointing in Sources Folder) ==> getName already given
        if(isEffectInCurrentSession((*it)->source) && QFileInfo((*it)->source.c_str()).absolutePath().toStdString().compare(fSourcesFolder) != 0){
            
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
        
        else if(newName.find(DEFAULTNAME) != string::npos){
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
                newName = find_smallest_defaultName(currentDefault);
                currentDefault.push_back(newName);
                nameChanges.push_front(make_pair((*it)->name, newName));
                updated[(*it)->name] = true;
                (*it)->name = newName;
            }
        }
        //4- If the source is in current Folder and its name is already used, it has to be renamed
        else if(QFileInfo((*it)->source.c_str()).absolutePath().toStdString().compare(fSourcesFolder) == 0){
            
            newName = renameEffect("", newName);
            
            nameChanges.push_front(make_pair((*it)->name, newName));
            updated[(*it)->name] = true;
            (*it)->name = newName;
        }
        //5- If the Name is in current session, is has to be renamed
        else{
            
            newName = renameEffect((*it)->source, newName);
            
            nameChanges.push_front(make_pair((*it)->name, newName));
            updated[(*it)->name] = true;
            (*it)->name = newName;
        }
    }
    
    return nameChanges;
}

void FLApp::establish_sourceChanges(list<std::pair<string,string> > nameChanges, list<WinInSession*>* session){
    
    //For all the effects which source is in the current source Folder, the path in the description file has to be changed with the new name
    
    list<pair<string, string> > sourceChanges;
    
    list<WinInSession*>::iterator it;
    
    list<std::pair<string,string> >::iterator it2;
    for(it2 = nameChanges.begin(); it2 != nameChanges.end() ; it2++){
        
        for(it = session->begin(); it != session->end() ; it++){
            
            if(QFileInfo((*it)->source.c_str()).absolutePath().toStdString().compare(fSourcesFolder)== 0){
                
                string inter = it2->first;
                QFileInfo sourceInfo((*it)->source.c_str());
                string inter2 = sourceInfo.baseName().toStdString();
                
                if(inter2.compare(inter) == 0){
                    string inter = fSourcesFolder + "/" + it2->second + ".dsp";
                    (*it)->source = inter;
                }
            }
            
        }
    }
}

void FLApp::copy_AllSources(const string& srcDir, const string& dstDir, list<std::pair<string,string> > nameChanges, const string extension){
    
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
        source.copy(newDir.c_str());
    }
    
}

void FLApp::copy_WindowsFolders(const string& srcDir, const string& dstDir, list<std::pair<string,string> > windowNameChanges){
    
    //Following the renaming table, the Folders containing the parameters of the window are copied from snapshot Folder to current Session Folder
    
    QDir src(srcDir.c_str());
    
    QFileInfoList chilDirs = src.entryInfoList(QDir::AllDirs);
    
    QFileInfoList::iterator it;
    
    for(it = chilDirs.begin(); it!=chilDirs.end(); it++){
        
        list<std::pair<string,string> >::iterator it2;
        
        string name = it->baseName().toStdString();
        
        for(it2 = windowNameChanges.begin(); it2 !=windowNameChanges.end() ; it2++){
            
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
            }
        }
    }
    
}

void FLApp::copy_SVGFolders(const string& srcDir, const string& dstDir, list<std::pair<string,string> > nameChanges){
    
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
    sessionContentToFile(fSessionFile);
    
    list<FLWindow*>::iterator it;
    
    for(it = FLW_List.begin(); it != FLW_List.end(); it++){
        
        string toto = (*it)->get_Effect()->getSource();
        string tutu = (*it)->get_Effect()->getName();
        
        if(toto.find(fSourcesFolder) == string::npos)
            set_Current_File(toto, tutu);
        
        (*it)->close_Window();
        (*it)->deleteLater();
    }
    FLW_List.clear();
    
    list<FLEffect*>::iterator it2;
    for(it2 = fExecutedEffects.begin() ;it2 != fExecutedEffects.end(); it2++)
        delete (*it2);
    
    fExecutedEffects.clear();
    
#ifdef __linux__
    exit();
#endif
}

//Shut all Windows from Menu
void FLApp::shut_AllWindows(){
    
    while(FLW_List.size() != 0 ){
        
        FLWindow* win = *(FLW_List.begin());
        
        common_shutAction(win);
    }
}

//Close Window from Menu
void FLApp::shut_Window(){
    
    if(fErrorWindow->isActiveWindow())
        fErrorWindow->hideWin();
    
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

//Close from Window Action
void FLApp::close_Window_Action(){
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    common_shutAction(win);
}

//Shut a specific window 
void FLApp::common_shutAction(FLWindow* win){
    
    FLEffect* toDelete = NULL;
    
    string toto = win->get_Effect()->getSource();
    string tutu = win->get_Effect()->getName();
    if(toto.find(fSourcesFolder) == string::npos)
        set_Current_File(toto, tutu);
    
    deleteWinFromSessionFile(win);
    sessionContentToFile(fSessionFile);
    
    win->shut_Window();
    
    QFileInfo ff((win)->get_Effect()->getSource().c_str());
    string toCompare = ff.absolutePath().toStdString();
    
    if(toCompare.compare(fSourcesFolder) == 0 && !isEffectInCurrentSession((win)->get_Effect()->getSource())){
        fExecutedEffects.remove((win)->get_Effect());
        //        QFile::remove((win)->get_Effect()->getSource().c_str());
        removeFilesOfWin((win)->get_Effect()->getSource().c_str(), (win)->get_Effect()->getName().c_str());
        toDelete = (win)->get_Effect();
    }
    else if(!isEffectInCurrentSession((win)->get_Effect()->getSource())){
        (win)->get_Effect()->stop_Watcher();
        string toErase = fSourcesFolder + "/" + (win)->get_Effect()->getName() + ".dsp";
        //        QFile::remove(toErase.c_str());
        removeFilesOfWin(toErase, (win)->get_Effect()->getName());
    }
    
    FLW_List.remove(win);
    win->deleteLater();
    
    if(toDelete != NULL){
        delete toDelete;
    }
    
    list<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++){
        
        (*it)->set_RecentFile(fRecentFiles);
        (*it)->update_RecentFileMenu();
    }

#ifdef __linux__
    if(FLW_List.size() == 0 && !fRecalling)
        exit();
#endif
}

//--------------------------------Navigate----------

void FLApp::frontShow(QString name){
    
    list<FLWindow*>::iterator it;
    
    for(it = FLW_List.begin() ; it != FLW_List.end(); it++){
    
        QString winName = (*it)->get_nameWindow().c_str();
        winName+=" : ";
        winName+= (*it)->get_Effect()->getName().c_str();
            
        if(winName.compare(name) == 0){
            (*it)->frontShow();
            break;
        }
    }
}

//--------------------------------Window----------------------------------------

//Open the source of a specific window
void FLApp::edit(FLWindow* win){
    
    string source = win->get_Effect()->getSource();
    
    printf("SOURCE = %s\n", source.c_str());
    
    QUrl url = QUrl::fromLocalFile(source.c_str());
    bool b = QDesktopServices::openUrl(url);
    
    string error = source + " could not be opened!";
    
    if(!b)
        fErrorWindow->print_Error(error.c_str());
}

//Edit Source from Menu
void FLApp::edit_Action(){
    
    FLWindow* win = getActiveWin();
    if(win != NULL)
        edit(win);
}

//Duplicate a specific window
void FLApp::duplicate(FLWindow* window){
    
    FLEffect* commonEffect = window->get_Effect();
    //To avoid flicker of the original window, the watcher is stopped during operation
    commonEffect->stop_Watcher();
    
    string source = commonEffect->getSource();
    
    list<int> currentIndexes = get_currentIndexes();
    int val = find_smallest_index(currentIndexes);
    stringstream ss;
    ss << val;
    
    int x = window->get_x() + 10;
    int y = window->get_y() + 10;
    
    FLWindow* win = new FLWindow(fWindowBaseName, val, commonEffect, x, y, fSessionFolder, fPort);
    
    redirectMenuToWindow(win);
    
    //Save then Copy of duplicated window's parameters
    window->save_Window();
    
    string toFind = window->get_nameWindow();
    string toReplace = win->get_nameWindow();
    
    string savingsPath = fSessionFolder + "/" + toFind + "/" + toFind + ".rc";
    QFile toCpy(savingsPath.c_str());
    string path = fSessionFolder + "/" + toReplace + "/" + toReplace + ".rc";
    toCpy.copy(path.c_str());
    
    savingsPath = fSessionFolder + "/" + toFind + "/" + toFind + ".jc";
    QFile toCy(savingsPath.c_str());
    path = fSessionFolder + "/" + toReplace + "/" + toReplace + ".jc";
    toCy.copy(path.c_str());
    
    //Replacement of WindowName in the audio Connections File to reconnect the new window as the duplicated one.
    list<pair<string, string> > changeTable;
    changeTable.push_back(make_pair(toFind, toReplace));
    win->update_ConnectionFile(changeTable);
    
    string error;
    
    if(win->init_Window(false, true, error)){
        FLW_List.push_back(win);
        addWinToSessionFile(win);
    }
    else{
        string toDelete = fSessionFolder + "/" + win->get_nameWindow(); 
        deleteDirectoryAndContent(toDelete);
        delete win;
        fErrorWindow->print_Error(error.c_str()); 
    }
    
    //Whatever happens, the watcher has to be started (at least for the duplicated window that needs it)
    commonEffect->launch_Watcher();
}

//Duplication window from Menu
void FLApp::duplicate_Window(){ 
    
    //copy parameters of previous window
    FLWindow* win = getActiveWin();
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
        string text = clipText.toStdString();
        
        update_SourceInWin(win, text);
        
    }
}

//Paste from Menu
void FLApp::paste_Text(){
    
    FLWindow* win = getActiveWin();
    if(win != NULL)
        paste(win);
} 

//View Httpd Window
void FLApp::viewHttpd(FLWindow* win){
    
    string error;
    
    if(!win->init_Httpd(error))
        fErrorWindow->print_Error(error.c_str());
}

//View Httpd From Menu
void FLApp::httpd_View_Window(){
    
    //Searching the active Window to show its QRcode
    FLWindow* win = getActiveWin();
    if(win != NULL)
        viewHttpd(win);
    else
        fErrorWindow->print_Error("No active Window");
}

//View SVG of a specific Window
void FLApp::viewSvg(FLWindow* win){
    
    string source = win->get_Effect()->getSource();
    string pathToOpen = fSVGFolder + "/" + win->get_Effect()->getName() + "-svg/process.svg";
    

    QUrl url = QUrl::fromLocalFile(pathToOpen.c_str());
    bool b = QDesktopServices::openUrl(url);

   	string error = pathToOpen + " could not be opened!";
    
    if(!b)
        fErrorWindow->print_Error(error.c_str());
}

//View SVG from Menu
void FLApp::svg_View_Action(){
    
    //Searching the active Window to show its SVG Diagramm
    FLWindow* win = getActiveWin();
    if(win != NULL)
        viewSvg(win);
    
}

//---------------Export

//Open ExportManager for a specific Window
void FLApp::export_Win(FLWindow* win){
    
    fExportDialog = new FLExportManager(fServerUrl, fSettingsFolder, win->get_Effect()->getSource(), win->get_Effect()->getName());
    fExportDialog->init();
}

//--Export From Menu
void FLApp::export_Action(){ 
    
    FLWindow* win = getActiveWin();
    
    if(win != NULL)
        export_Win(win);
}

//---------------Drop

//Drop of sources on a window
void FLApp::drop_Action(list<string> sources){
    
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

//--------------------------------HELP----------------------------------------

//Set Text in Tools Menu of HELP
void FLApp::setToolText(const QString & currentText){
    
    if(currentText.compare("FAUST") == 0)
        fToolText->setHtml("<br>\nTo develop your own effects, you will need to handle Faust language.<br><br>""LEARN MORE ABOUT FAUST : <a href = http://faust.grame.fr>faust.grame.fr</a>");
    else if(currentText.compare("LLVM") == 0)
        fToolText->setHtml("<br>\nThanks to its embedded LLVM compiler, this application allows dynamical compilation of your faust objects.<br><br>""LEARN MORE ABOUT LLVM : <a href = http://llvm.org>llvm.org</a>");
    else if(currentText.compare("COREAUDIO") == 0)
        fToolText->setHtml("<br>Core Audio is the digital audio infrastructure of MAC OS X.<br><br>""LEARN MORE ABOUT COREAUDIO : <a href = http://developer.apple.com/library/ios/#documentation/MusicAudio/Conceptual/CoreAudioOverview/WhatisCoreAudio/WhatisCoreAudio.html>developer.apple.com </a> ");
    else if(currentText.compare("JACK") == 0)
        fToolText->setHtml("<br>Jack (the Jack Audio Connection Kit) is a low-latency audio server. It can connect any number of different applications to a single hardware audio device.<br><br>YOU CAN DOWNLOAD IT HERE : <a href =http://www.jackosx.com> www.jackosx.com</a>\n");
    else if(currentText.compare("NETJACK") == 0)
        fToolText->setHtml("<br>NetJack (fully integrated in Jack) is a Realtime Audio Transport over a generic IP Network. That way you can send your audio signals through the network to a server.<br><br>""LEARN MORE ABOUT NETJACK : <a href = http://netjack.sourceforge.net> netjack.sourceforge.net</a>\n");
    else if(currentText.compare("PORTAUDIO") == 0)
        fToolText->setHtml("<br>PortAudio is a free, cross-platform, open-source, audio I/O library. <br><br>""LEARN MORE ABOUT PORTAUDIO : <a href = http://netjack.sourceforge.net> netjack.sourceforge.net</a>\n");
    else if(currentText.compare("LIB QRENCODE") == 0)
        fToolText->setHtml("<br>Libqrencode is a C library for encoding data in a QR Code symbol, a kind of 2D symbology that can be scanned by handy terminals such as a mobile phone with CCD.<br><br>""LEARN MORE ABOUT LIB QRENCODE : <a href = http://fukuchi.org/works/qrencode> fukuchi.org/works/qrencode</a>\n");
    else if(currentText.compare("LIB MICROHTTPD") == 0)
        fToolText->setHtml("<br>GNU libmicrohttpd is a small C library that is supposed to make it easy to run an HTTP server as part of an application.<br><br>""LEARN MORE ABOUT LIB MICROHTTPD : <a href = http://www.gnu.org/software/libmicrohttpd> gnu.org/software/libmicrohttpd</a>\n");
}

//Set Text in Application Properties Menu of HELP
void FLApp::setAppPropertiesText(const QString& currentText){
    
    if(currentText.compare("New Default Window")==0)
        fAppText->setPlainText("\nCreates a new window containing a simple faust process.\n\n process = !,!:0,0; ");
    
    else if(currentText.compare("Open")==0)
        fAppText->setPlainText("\nCreates a new window containing the DSP you choose on disk.\n");
 
    else if(currentText.compare("Take Snapshot")==0)
        fAppText->setPlainText("\nSaves the actual state of the application in a folder.tar : all the windows, their graphical parameters, their audio connections, their position on the screen.\n");
    
    else if(currentText.compare("Recall Snapshot")==0)
        fAppText->setPlainText("\nRestores the state of the application as saved. All current windows are closed. If one of the source file can't be found, a back up file is used\n");
    
    else if(currentText.compare("Import Snapshot")==0)
        fAppText->setPlainText("\nAdds the state of the application as saved to the current state of the application. That way, current windows are not closed. Some audio application/windows may have to be renamed during the importation.\n");
    
    else if(currentText.compare("Navigate")==0)
        fAppText->setPlainText("\nBrings to front end the chosen running window\n");
    
    else if(currentText.compare("Preferences")==0)
        fAppText->setPlainText("\nYou can choose default compilation options for new windows.\n\n The compilation web service URL can be modified.\n\n If this version of FaustLive includes multiple audio architectures, you can switch from one to another in Audio Preferences. All opened windows will try to switch. If the update fails, former architecture will be reloaded.\n\n You can also choose the graphical style of the application.\n");
    
    else if(currentText.compare("Error Displaying")==0)
        fAppText->setPlainText("\nDisplays a window every time the program catches an error : whether it's a error in the edited code, a compilation problem, a lack of memory during saving action, ...");
            
}

//Set Text in Window Properties Menu of HELP
void FLApp::setWinPropertiesText(const QString& currentText){
    
    if(currentText.compare("Audio Cnx/Dcnx")==0)
        fWinText->setPlainText("\nWith Jack router, you can connect a window to another one or to an external application like I-Tunes, VLC or directly to the computer input/output.\nYou can choose Jack as the audio architecture in the preferences.");
        
    else if(currentText.compare("Edit Source")==0)
        fWinText->setPlainText("\nThe code Faust corresponding to the active window is opened in a text editor for you to change it. When you save your modification, the window(s) corresponding to this source will be updated. The graphical parameters and the audio connections that can be kept will stay unmodified.");
            
    else if(currentText.compare("Drag and Drop / Paste")==0)
        fWinText->setPlainText("\nIn a window, you can drop or paste : \n - File.dsp\n - Faust code\n - Faust URL\n An audio crossfade will be calculated between the outcoming and the incoming audio application. The new application will be connected as the outcoming one.\n");
    
    else if(currentText.compare("Duplicate")==0)
        fWinText->setPlainText("\nCreates a new window, that has the same characteristics : same Faust code, same graphical parameters, same audio connections as the active window.\n");
    
    else if(currentText.compare("View QrCode")==0)
        fWinText->setPlainText("\nYou can display a new window with a QRcode so that you can control the userInterface of the audio application remotely.");
    
    else if(currentText.compare("Window Options")==0)
        fWinText->setPlainText("\nYou can add compilation options for Faust Compiler. You can also change the level of optimization for the Llvm compiler. If several windows correspond to the same audio application, they will load the chosen options.\n The Httpd Port corresponds to the connection port for remote controlling of the interface.");
    
    else if(currentText.compare("View SVG")==0)
        fWinText->setPlainText("\nYou can display the SVG diagramm of the active Window. It we be open in your chosen default navigator.");
    
    else if(currentText.compare("Export")==0)
        fWinText->setPlainText("\nA web service is available to export your Faust application for another Platform or/and architecture.");
    
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
    
    QPlainTextEdit* generalText = new QPlainTextEdit(tr("\nFaustLive is a dynamical compiler for processors coded with Faust.\nIt embeds Faust & LLVM compiler.\n\nEvery window of the application corresponds to an audio application, which parameters you can adjust."));
    
    QLineEdit* lineEdit = new QLineEdit(tr("DISTRIBUTED by GRAME - Centre de Creation Musicale"));
    
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
    
    vue->addItem(QString(tr("LIB MICROHTTPD")));
    vue->addItem(QString(tr("LIB QRENCODE")));
    
    vue->setMaximumWidth(100);
    connect(vue, SIGNAL(currentTextChanged(const QString&)), this, SLOT(setToolText(const QString &)));
    
    toolLayout->addWidget(vue, 0, 0, 1, 1);
    
    fToolText = new QTextBrowser;
    fToolText->setOpenExternalLinks(true);
    fToolText->setReadOnly(true);

    myTabWidget->addTab(tab_tool, QString(tr("Tools")));
    
    toolLayout->addWidget(fToolText, 0, 1, 1, 2);
    tab_tool->setLayout(toolLayout);
    
    vue->setCurrentRow(0);
    
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
    vue3->addItem(QString(tr("View QrCode")));
    vue3->addItem(QString(tr("Window Options")));
    vue3->addItem(QString(tr("View SVG")));
    vue3->addItem(QString(tr("Export")));
    
    vue3->setMaximumWidth(150);
    connect(vue3, SIGNAL(currentTextChanged(const QString&)), this, SLOT(setWinPropertiesText(const QString &)));
    
    winPropLayout->addWidget(vue3, 0, 0, 1, 1);
    
    fWinText = new QPlainTextEdit;
    fWinText->setReadOnly(true);
    
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
//    QPlainTextEdit* versionText = new QPlainTextEdit(tr(text.c_str()), fVersionWindow);
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
    fExampleToOpen = item->text().toStdString();
}

//Opens directly a double clicked item
void FLApp::itemDblClick(QListWidgetItem* item){
    fExampleToOpen = item->text().toStdString();
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
    QPixmap newPix(ImagesDir.absoluteFilePath("InitWin.png"));
    newPix = newPix.scaledToWidth(60, Qt::SmoothTransformation);
    new_Image->setPixmap(newPix);
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
    
    open_Session = new QPushButton("Recall your Snapshot.tar\n");
    open_Session->setToolTip("Open a saved snapshot.");
    open_Session->setFlat(true);
    open_Session->setDefault(false);
    
    layout2->addWidget(open_Session, 2, 1);
    connect(open_Session, SIGNAL(clicked()), this, SLOT(open_Session_pres()));
    
    QLabel *help_Image = new QLabel(gridBox);
    QPixmap helpPix(ImagesDir.absoluteFilePath("HelpMenu.png"));
    helpPix = helpPix.scaledToWidth(60, Qt::SmoothTransformation);
    help_Image->setPixmap(helpPix);
    layout2->addWidget(help_Image, 3, 0);
    
    help = new QPushButton("About Faust Live\n Learn all about FaustLive charateristics");
    help->setToolTip("Help Menu.");
    
    layout2->addWidget(help, 3, 1);
    connect(help, SIGNAL(clicked()), this, SLOT(apropos()));
    help->setFlat(true);
    help->setDefault(false);
    
    
    //-------------------------OPEN EXAMPLES
    
    
    QVBoxLayout *layout5 = new QVBoxLayout;
    
    QListWidget *vue = new QListWidget(openExamples);
    
    QDir examplesDir(":/");
    
    if(examplesDir.cd("Examples")){
    
        QFileInfoList children = examplesDir.entryInfoList(QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
        
        fExampleToOpen = (children.begin())->baseName().toStdString();
        
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
    styleClicked(item->text().toStdString());
}

//Modification of application style
void FLApp::styleClicked(const string& style){
    
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
                      );
    }
    
    if(style.compare("Grey") == 0){
        
        fStyleChoice = "Grey";
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
    
    recall_Settings(fHomeSettings);
    
    fCompilModes->setText(fCompilationMode.c_str());
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
    fServerLine->setText(fServerUrl.c_str());
    
    fPortLine = new QLineEdit(menu4);
    stringstream p;
    p<<fPort;
    fPortLine->setText(p.str().c_str());
    
    myTab->addTab(menu4, tr("Network"));
    layoutNet->addRow(new QLabel(tr("")));
    layoutNet->addRow(new QLabel(tr("Compilation Web Service")), fServerLine);
    layoutNet->addRow(new QLabel(tr("")));
    layoutNet->addRow(new QLabel(tr("Remote Dropping Service Port")), fPortLine);
    
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

    fServerUrl = fServerLine->text().toStdString();
    
    fCompilationMode = fCompilModes->text().toStdString();
    
    if(isStringInt(fOptVal->text().toStdString().c_str()))
        fOpt_level = atoi(fOptVal->text().toStdString().c_str());
    else
        fOpt_level = 3;
    
    if(isStringInt(fPortLine->text().toStdString().c_str())){
        
        if(fPort != atoi(fPortLine->text().toStdString().c_str())){
            fPort = atoi(fPortLine->text().toStdString().c_str());
            
            stop_Server();
            launch_Server();
            
            list<FLWindow*>::iterator it;
            
            for(it = FLW_List.begin() ; it != FLW_List.end(); it++)    
                (*it)->set_generalPort(fPort);
        }   
    }
    else
        fPort = 7777;
    
    fPrefDialog->hide();

    if(fAudioCreator->didSettingChanged()){
        printf("WE ARE GOING TO UPDATE....\n");
        update_AudioArchitecture();
    }
    else
        fAudioCreator->reset_Settings();
}

//Save/Recall Settings from file
void FLApp::save_Settings(const string& home){
    
    string modeText = fCompilationMode;
    
    size_t pos = 0;
    
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
        
        textWriting<<modeText.c_str()<<' '<<fOpt_level<<fStyleChoice.c_str();
        
        f.close();
    }
    
//    SAVING THE EXPORT URL
    
    string homeFile = fSettingsFolder + kExportUrlFile;
    
    QFile g(homeFile.c_str()); 
    QString server(fServerUrl.c_str());
    
    if(g.open(QFile::WriteOnly | QIODevice::Truncate)){
        
        QTextStream textWriting(&g);
        
        textWriting<<server<<' '<<fPort;
        g.close();
    }    
}

void FLApp::recall_Settings(const string& home){

    QString ModeText;
    
    QFile f(home.c_str()); 
    
    if(f.open(QFile::ReadOnly)){
        
        QTextStream textReading(&f);
        QString styleIntermediate;
        
        textReading>>ModeText>>fOpt_level>>styleIntermediate;

        fStyleChoice = styleIntermediate.toStdString();
        
        f.close();
    }
    
    string modeText = ModeText.toStdString();
    
    size_t pos = 0;
    
    while(modeText.find("/", pos) != string::npos){
        
        if(pos != string::npos && modeText[pos] != '-')
            modeText.replace(pos, 1, " ");
        
        pos = modeText.find("/", pos+1);
    }
    
    if(modeText.compare(" ") == 0)
        modeText = "";
    
    fCompilationMode = modeText;
    
//    RECALL THE URL FOR EXPORTATION SERVICE

    QString server("http://faust.grame.fr:8888");
        
    string homeFile = fSettingsFolder + kExportUrlFile;
    
    QFile g(homeFile.c_str()); 
    
    if(g.open(QFile::ReadOnly)){
        
        QTextStream textReading(&g);
        textReading>>server>>fPort;
        
        g.close();
    }
    
    fServerUrl = server.toStdString();
    
}

//Update Audio Architecture of all opened windows
void FLApp::update_AudioArchitecture(){
    
    list<FLWindow*>::iterator it;
    list<FLWindow*>::iterator updateFailPointer;
    
    bool updateSuccess = true;
    string errorToPrint;
    string error;
    
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
    
        fErrorWindow->print_Error(errorToPrint.c_str());
        fErrorWindow->print_Error(error.c_str());
        
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
            
            fErrorWindow->print_Error(errorToPrint.c_str());
            fErrorWindow->print_Error(error.c_str());
        }
        else{
            
            for(it = FLW_List.begin() ; it != FLW_List.end(); it++)
                (*it)->start_Audio();
            
            errorToPrint = fAudioCreator->get_ArchiName();
            errorToPrint += " was reinitialized";
            fErrorWindow->print_Error(errorToPrint.c_str());
            
        }

    }
    else{
        
        for(it = FLW_List.begin() ; it != FLW_List.end(); it++)
            (*it)->start_Audio();
        fAudioCreator->saveCurrentSettings();
        
        errorToPrint = "Update successfull";
        fErrorWindow->print_Error(errorToPrint.c_str());
    }
    
    StopProgressSlot();
}

//--------------------------LONG WAITING PROCESSES------------------------------

//Display Messages like "Compiling..." / "Connection with server..."
void FLApp::display_CompilingProgress(const string& msg){
    fCompilingMessage = new QDialog();
    fCompilingMessage->setWindowFlags(Qt::FramelessWindowHint);
    
    QLabel* tittle = new QLabel(tr("<h2>FAUSTLIVE</h2>"));
    tittle->setAlignment(Qt::AlignCenter);
    QLabel* text = new QLabel(tr(msg.c_str()), fCompilingMessage);
    text->setAlignment(Qt::AlignCenter);
    text->setStyleSheet("*{color: black}");
    
    QVBoxLayout* layoutSave = new QVBoxLayout;
    
    layoutSave->addWidget(tittle);
    layoutSave->addWidget(new QLabel(tr("")));
    layoutSave->addWidget(text);
    layoutSave->addWidget(new QLabel(tr("")));
    fCompilingMessage->setLayout(layoutSave);
    
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

    bool returning = true;
    
    if(fServerHttp == NULL){
    
        fServerHttp = new FLServerHttp();
        
        int i = 0;
        
        while(!fServerHttp->start(fPort)){
            
            stringstream s;
            s<<"Server Could Not Start On Port "<<fPort;
            
            fErrorWindow->print_Error(s.str().c_str());
            
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
    else{
        stringstream s;
        s<<"Server Started On Port "<<fPort;
        fErrorWindow->print_Error(s.str().c_str());
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
    
    string error;
    
    string source(data);
    
    FLWindow* win = getWinFromHttp(port);
    
    if(win != NULL){
    
        update_SourceInWin(win, source);
    
        viewHttpd(win);
        
        string url = win->get_HttpUrl();
        
        fServerHttp->compile_Successfull(url);
    }
    else{
        fServerHttp->compile_Failed(error.c_str());
    }
    
}




