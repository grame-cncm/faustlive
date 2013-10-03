//
//  FaustLiveApp.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLApp.h"
#include "FLrenameDialog.h"

#include <QFileOpenEvent>
#include <QClipboard>
#include <QMimeData>
#include <QString>
#include <QTextStream>

#include <sstream>

//--------------------GENERAL METHODS-------------------------------------

string FLApp::pathToContent(string path){
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

bool FLApp::deleteDirectoryAndContent(string& directory){
    
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

bool FLApp::rmDir(const QString &dirPath){
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

bool FLApp::cpDir(const QString &srcPath, const QString &dstPath){
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

bool FLApp::isStringInt(const char* word){
    
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

FLApp::FLApp(int& argc, char** argv) : QApplication(argc, argv){
    
    fWindowBaseName = "FLW";
    
    //Initialization of current Session Path  
    
    fSessionFolder = getenv("HOME");
    fSessionFolder += "/CurrentSession";
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
    
    QDir direc(fSessionFile.c_str());
    
    //Initializing screen parameters
    QSize screenSize = QApplication::desktop()->geometry().size(); 
    fScreenWidth = screenSize.width();
    fScreenHeight = screenSize.height();
    
    //Initializing preference
    fHomeSettings = fSessionFolder + "/FaustLive_Settings.rf"; 

    fOpt_level = 3;
    fStyleChoice = "Default";
    recall_Settings(fHomeSettings);
    styleClicked(fStyleChoice);
    
    //If no event opened a window half a second after the application was created, a initialized window is created
    fInitTimer = new QTimer(this);
    connect(fInitTimer, SIGNAL(timeout()), this, SLOT(init_Timer_Action()));
    fInitTimer->start(500);
    
    
    //Initializing menu options 
    fRecentFileAction = new QAction* [kMAXRECENTFILES];
    fRrecentSessionAction = new QAction* [kMAXRECENTSESSIONS];
    fIrecentSessionAction = new QAction* [kMAXRECENTSESSIONS];
//    frontWindow = new QAction* [200];
    
    
    //For the application not to quit when the last window is closed
    setQuitOnLastWindowClosed(false);
    
    fMenuBar = new QMenuBar(0);
    fFileMenu = new QMenu;
    fEditMenu = new QMenu;
    fWindowsMenu = new QMenu;
    fSessionMenu = new QMenu;
    fViewMenu = new QMenu;
    fHelpMenu = new QMenu;
    
    setup_Menu();
    
//    homeRecents = getenv("HOME");
    fRecentsFile = fSessionFolder + "/FaustLive_FileSavings.rf"; 
    recall_Recent_Files(fRecentsFile);
//    homeRecents = getenv("HOME");
    fHomeRecentSessions = fSessionFolder + "/FaustLive_SessionSavings.rf"; 
    recall_Recent_Sessions(fHomeRecentSessions);
    
    //Initializing preference and save dialog
    fErrorWindow = new FLErrorWindow();
    fErrorWindow->setWindowTitle("ERROR_WINDOW");
    fErrorWindow->init_Window();
    connect(fErrorWindow, SIGNAL(closeAll()), this, SLOT(shut_AllWindows()));
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
    
    delete fNewAction;
    delete fOpenAction;
    delete fOpenRecentAction;
    delete fShutAction;
    delete fShutAllAction;
    delete fCloseAllAction;
    delete fEditAction;
    delete fPasteAction;
    delete fDuplicateAction;
    delete fTakeSnapshotAction;
    delete fRecallSnapshotAction;
    delete fImportSnapshotAction;
    delete fHttpdViewAction;
    delete fAboutQtAction;
    delete fPreferencesAction;
    delete fAboutAction; 
    
    delete fHelpMenu;
    delete fViewMenu;
    delete fEditMenu;
    delete fWindowsMenu;
    delete fFileMenu;
    delete fMenuBar;
    
    delete fInitTimer;
    
    if(fPresWin != NULL)
        delete fPresWin;
    
    delete fHelpWindow;
    delete fErrorWindow;
    
    fSessionContent.clear();
}

//---------------------

void FLApp::setup_Menu(){
    
    //----------------FILE
    
    fNewAction = new QAction(tr("&New Default Window"), this);
    fNewAction->setShortcut(tr("Ctrl+N"));
    fNewAction->setToolTip(tr("Open a new empty file"));
    connect(fNewAction, SIGNAL(triggered()), this, SLOT(create_Empty_Window()));
    
    fOpenAction = new QAction(tr("&Open..."),this);
    fOpenAction->setShortcut(tr("Ctrl+O"));
    fOpenAction->setToolTip(tr("Open a DSP file"));
    connect(fOpenAction, SIGNAL(triggered()), this, SLOT(open_New_Window()));
    
    fMenuOpen_Example = new QMenu(tr("&Open Example"), fFileMenu);
    
    QString examplesPath = QFileInfo(QFileInfo( QCoreApplication::applicationFilePath()).absolutePath()).absolutePath();
    examplesPath += "/Resources/Examples";
    
    //SE PROTEGER AU CAS OU IL NE TROUVE PAS LE DOSSIER D'EXEMPLES
    
    if(QFileInfo(examplesPath).exists()){
    
        QDir examplesDir(examplesPath);
    
        QFileInfoList children = examplesDir.entryInfoList(QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
    
        fExampleToOpen = (children.begin())->baseName().toStdString();
        
        QFileInfoList::iterator it;
        int i = 0; 
        
        fOpenExamples = new QAction* [children.size()];
        
        for(it = children.begin(); it != children.end(); it++){
            
            fOpenExamples[i] = new QAction(it->baseName(), fMenuOpen_Example);
            fOpenExamples[i]->setData(QVariant(it->absoluteFilePath()));
            connect(fOpenExamples[i], SIGNAL(triggered()), this, SLOT(open_Recent_File()));
            
            fMenuOpen_Example->addAction(fOpenExamples[i]);
            i++;
        }
        
        fFileMenu->addAction(fMenuOpen_Example->menuAction());
    }
    
    fOpenRecentAction = new QAction(tr("&Open Recent..."),this);
    fOpenRecentAction->setEnabled(false);
    fOpenRecentAction->setToolTip(tr("Open a recently opened DSP file"));
    
    for(int i=0; i<kMAXRECENTFILES; i++){
        fRecentFileAction[i] = new QAction(this);
        fRecentFileAction[i]->setVisible(false);
        connect(fRecentFileAction[i], SIGNAL(triggered()), this, SLOT(open_Recent_File()));
    }
    
    fExportAction = new QAction(tr("&Export As..."), this);
    fExportAction->setShortcut(tr("Ctrl+P"));
    fExportAction->setToolTip(tr("Export the DSP in whatever architecture you choose"));
    connect(fExportAction, SIGNAL(triggered()), this, SLOT(export_Action()));
    
    fShutAction = new QAction(tr("&Close Window"),this);
    fShutAction->setShortcut(tr("Ctrl+W"));
    fShutAction->setToolTip(tr("Close the current Window"));
    connect(fShutAction, SIGNAL(triggered()), this, SLOT(shut_Window()));
    
    fShutAllAction = new QAction(tr("&Close All Windows"),this);
    fShutAllAction->setShortcut(tr("Ctrl+Alt+W"));
    fShutAllAction->setToolTip(tr("Close all the Windows"));
    connect(fShutAllAction, SIGNAL(triggered()), this, SLOT(shut_AllWindows()));
    
    fCloseAllAction = new QAction(tr("&Closing"),this);
    fCloseAllAction = new QAction(tr("&Quit FaustLive"),this);
    fCloseAllAction->setToolTip(tr("Close the application"));   
    connect(fCloseAllAction, SIGNAL(triggered()), this, SLOT(closeAllWindows()));
    
    fFileMenu = fMenuBar->addMenu(tr("&File"));
    fFileMenu->addAction(fNewAction);    
    fFileMenu->addSeparator();
    fFileMenu->addAction(fOpenAction);
    fFileMenu->addAction(fMenuOpen_Example->menuAction());
    fFileMenu->addSeparator();
    fFileMenu->addAction(fOpenRecentAction);
    for(int i=0; i<kMAXRECENTFILES; i++){
        fFileMenu->addAction(fRecentFileAction[i]);
    }
    fFileMenu->addSeparator();
    fFileMenu->addAction(fExportAction);
    fFileMenu->addSeparator();
    fFileMenu->addAction(fShutAction);
    fFileMenu->addAction(fShutAllAction);
    
    fMenuBar->addSeparator();
    fFileMenu->addAction(fCloseAllAction);
    
    //-----------------EDIT
    
    fEditAction = new QAction(tr("&Edit Faust Source"), this);
    fEditAction->setShortcut(tr("Ctrl+E"));
    fEditAction->setToolTip(tr("Edit the source"));
    connect(fEditAction, SIGNAL(triggered()), this, SLOT(edit_Action()));
    
    fPasteAction = new QAction(tr("&Paste"),this);
    fPasteAction->setShortcut(tr("Ctrl+V"));
    fPasteAction->setToolTip(tr("Paste a DSP"));
    connect(fPasteAction, SIGNAL(triggered()), this, SLOT(paste_Text()));
    
    fDuplicateAction = new QAction(tr("&Duplicate"),this);
    fDuplicateAction->setShortcut(tr("Ctrl+D"));
    fDuplicateAction->setToolTip(tr("Duplicate current DSP"));
    connect(fDuplicateAction, SIGNAL(triggered()), this, SLOT(duplicate_Window()));
    
    fEditMenu = fMenuBar->addMenu(tr("&Edit"));
    fEditMenu->addAction(fEditAction);
    fEditMenu->addSeparator();
    fEditMenu->addAction(fPasteAction);
    fEditMenu->addSeparator();
    fEditMenu->addAction(fDuplicateAction);
    fMenuBar->addSeparator();
    
    fMenuBar->addSeparator();
    
    //-----------------WINDOWS
    
    fWindowsMenu = fMenuBar->addMenu(tr("&Windows"));
    
    fMenuBar->addSeparator();
    //------------------SESSION
    
    fTakeSnapshotAction = new QAction(tr("&Take Snapshot"),this);
    fTakeSnapshotAction->setShortcut(tr("Ctrl+S"));
    fTakeSnapshotAction->setToolTip(tr("Save current state"));
    connect(fTakeSnapshotAction, SIGNAL(triggered()), this, SLOT(take_Snapshot()));
    
    fRecallSnapshotAction = new QAction(tr("&Recall Snapshot..."),this);
    fRecallSnapshotAction->setShortcut(tr("Ctrl+R"));
    fRecallSnapshotAction->setToolTip(tr("Close all the opened window and open your snapshot"));
    connect(fRecallSnapshotAction, SIGNAL(triggered()), this, SLOT(recallSnapshotFromMenu()));
    
    fRecallRecentAction = new QAction(tr("&Recall Recent..."),this);
    fRecallRecentAction->setEnabled(false);
    
    for(int i=0; i<kMAXRECENTSESSIONS; i++){
        fRrecentSessionAction[i] = new QAction(this);
        fRrecentSessionAction[i]->setVisible(false);
        connect(fRrecentSessionAction[i], SIGNAL(triggered()), this, SLOT(recall_Recent_Session()));
        
        fIrecentSessionAction[i] = new QAction(this);
        fIrecentSessionAction[i]->setVisible(false);
        connect(fIrecentSessionAction[i], SIGNAL(triggered()), this, SLOT(import_Recent_Session()));
    }
    
    fImportSnapshotAction = new QAction(tr("&Import Snapshot..."),this);
    fImportSnapshotAction->setShortcut(tr("Ctrl+I"));
    fImportSnapshotAction->setToolTip(tr("Import your snapshot in the current session"));
    connect(fImportSnapshotAction, SIGNAL(triggered()), this, SLOT(importSnapshotFromMenu()));
    
    fImportRecentAction = new QAction(tr("&Import Recent..."),this);
    fImportRecentAction->setEnabled(false);
    
    fSessionMenu = fMenuBar->addMenu(tr("&Snapshot"));
    fSessionMenu->addAction(fTakeSnapshotAction);
    fSessionMenu->addSeparator();
    fSessionMenu->addAction(fRecallSnapshotAction);
    fSessionMenu->addAction(fRecallRecentAction);
    fSessionMenu->addAction(fRecallRecentAction);
    for(int i=0; i<kMAXRECENTSESSIONS; i++){
        fSessionMenu->addAction(fRrecentSessionAction[i]);
    }
    fSessionMenu->addSeparator();
    fSessionMenu->addAction(fImportSnapshotAction);
    fSessionMenu->addAction(fImportRecentAction);
    for(int i=0; i<kMAXRECENTSESSIONS; i++){
        fSessionMenu->addAction(fIrecentSessionAction[i]);
    }
    
    fMenuBar->addSeparator();
    
    //---------------------VIEW
    
    fHttpdViewAction = new QAction(tr("&View QRcode"),this);
    fHttpdViewAction->setShortcut(tr("Ctrl+K"));
    fHttpdViewAction->setToolTip(tr("Print the QRcode of TCP protocol"));
    connect(fHttpdViewAction, SIGNAL(triggered()), this, SLOT(httpd_View_Window()));
    
    fSvgViewAction = new QAction(tr("&View SVG Diagram"),this);
    fSvgViewAction->setShortcut(tr("Ctrl+G"));
    fSvgViewAction->setToolTip(tr("Open the SVG Diagram in a browser"));
    connect(fSvgViewAction, SIGNAL(triggered()), this, SLOT(svg_View_Action()));
    
    fViewMenu = fMenuBar->addMenu(tr("&View"));
    fViewMenu->addAction(fHttpdViewAction);
    fViewMenu->addSeparator();
    fViewMenu->addAction(fSvgViewAction);
    
    fMenuBar->addSeparator();
    
    //---------------------MAIN MENU
    
    fAboutQtAction = new QAction(tr("&About Qt"), this);
    fAboutQtAction->setToolTip(tr("Show the library's About Box"));
    connect(fAboutQtAction, SIGNAL(triggered()), this, SLOT(aboutQt()));
    
    fPreferencesAction = new QAction(tr("&Preferences"), this);
    fPreferencesAction->setToolTip(tr("Set the preferences of the application"));
    connect(fPreferencesAction, SIGNAL(triggered()), this, SLOT(Preferences()));
    
    fPrefDialog = new QDialog;
    fPrefDialog->setWindowFlags(Qt::FramelessWindowHint);
    init_PreferenceWindow();
    
    //--------------------HELP
    
    fHelpWindow = new QMainWindow;
    fHelpWindow->setWindowFlags(Qt::FramelessWindowHint);
    fHelpWindow->setGeometry(fScreenWidth/2, 312, 0, 0);
    this->init_HelpWindow();
    
    fAboutAction = new QAction(tr("&Help..."), this);
    fAboutAction->setToolTip(tr("Show the library's About Box"));
    connect(fAboutAction, SIGNAL(triggered()), this, SLOT(apropos()));
    
    fVersionWindow = new QDialog;
    
    fVersionAction = new QAction(tr("&Version"), this);
    fVersionAction->setToolTip(tr("Show the version of the libraries used"));
    connect(fVersionAction, SIGNAL(triggered()), this, SLOT(version_Action()));
    
    fPresentationAction = new QAction(tr("&About FaustLive"), this);
    fPresentationAction->setToolTip(tr("Show the presentation Menu"));
    connect(fPresentationAction, SIGNAL(triggered()), this, SLOT(show_presentation_Action()));
    
    fHelpMenu = fMenuBar->addMenu(tr("&Help"));
    
    fHelpMenu->addAction(fAboutQtAction);
    fHelpMenu->addSeparator();
    fHelpMenu->addAction(fAboutAction);
    fHelpMenu->addAction(fVersionAction);
    fHelpMenu->addSeparator();
    fHelpMenu->addAction(fPresentationAction);
    fHelpMenu->addSeparator();
    fHelpMenu->addAction(fPreferencesAction);
}

void FLApp::errorPrinting(const char* msg){
    
    fErrorWindow->print_Error(msg);
}

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
            
            if(text.toStdString().compare("") == 0){
                
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
    //    else
    //        presWin = NULL;
}

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

bool FLApp::isIndexUsed(int index, list<int> currentIndexes){
    
    list<int>::iterator it;
    
    for(it = currentIndexes.begin(); it != currentIndexes.end(); it++){
        if(index == *it ){
            return true;
        }
    }
    return false;
}

void FLApp::calculate_position(int index, int* x, int* y){
    
    int multiplCoef = index;
    while(multiplCoef > 20){
        multiplCoef-=20;
    }
    
    *x = fScreenWidth/3 + multiplCoef*10;
    *y = fScreenHeight/3 + multiplCoef*10;
}

list<string> FLApp::get_currentDefault(){
    
    list<string> currentDefault;
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin(); it != fSessionContent.end() ; it++){
        
        if((*it)->name.find(DEFAULTNAME)!=string::npos)
            currentDefault.push_back((*it)->name);
    }
    
    return currentDefault;
}

string FLApp::find_smallest_defaultName(string& sourceToCompare, list<string> currentDefault){
    
    //Conditional jump on currentDefault List...
    
    int index = 1;
    string nomEffet;
    bool found;
    
    do{
        stringstream ss;
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

FLWindow* FLApp::getActiveWin(){
    
    list<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++) {

        if((*it)->isActiveWindow())
            return *it;
    }
    
    return NULL;
}

//--------------------------CREATE EFFECT-------------------------------------

void FLApp::createSourceFile(string& sourceName, string& content){
    
    QFile f(sourceName.c_str());
    
    if(f.open(QFile::WriteOnly)){
        
        QTextStream textWriting(&f);
        
        textWriting<<content.c_str();
        
        f.close();
    }
}

void FLApp::update_Source(string& oldSource, string& newSource){
    
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

bool FLApp::doesEffectNameExists(string nomEffet, list<string> runningEffects){
    
    list<string>::iterator it;
    for(it = runningEffects.begin(); it!= runningEffects.end(); it++){
        
        if(it->compare(nomEffet) == 0)
            return true;
    }
    return false;
}

string FLApp::getDeclareName(string text, list<string> runningEffects){
    
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

string FLApp::renameEffect(string nomEffet, list<string> runningEffects){
    
    while(doesEffectNameExists(nomEffet, runningEffects)){
        
        FLrenameDialog* Msg = new FLrenameDialog(nomEffet, 0);
        printf("RENAME 3\n");
        Msg->raise();
        Msg->exec();
        nomEffet = Msg->getNewName();
        delete Msg;
    }
    
    return nomEffet;
}

string FLApp::ifUrlToText(string& source){
    
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

string FLApp::getNameEffectFromSource(string sourceToCompare){
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
        
        if((*it)->source.compare(sourceToCompare) == 0)
            return (*it)->name;
    }
    return "";
}

bool FLApp::isEffectInCurrentSession(string sourceToCompare){
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
        
        if((*it)->source.compare(sourceToCompare) == 0)
            return true;
    }
    return false;
    
}

list<string> FLApp::getNameRunningEffects(){
    
    list<string> returning; 
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++)
        returning.push_back((*it)->name);   
    
    return returning;
}

bool FLApp::isEffectNameInCurrentSession(string& sourceToCompare ,string& nom){
    
    list<int> returning;
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
        
        if((*it)->name.compare(nom)==0 && ((*it)->source.compare(sourceToCompare) != 0 || (*it)->name.find(DEFAULTNAME) != string::npos))
            return true;
    }
    return false;
}

FLEffect* FLApp::getEffectFromSource(string& source, string& nameEffect, string& sourceFolder, string compilationOptions, int opt_Val, char* error, bool init){
    
    bool fileSourceMark = false;
    string content = "";
    string fichierSource = "";
    
    list<string> currentDefault = get_currentDefault();
    string defaultName = find_smallest_defaultName(source, currentDefault);
    
    source = ifUrlToText(source);
    
    //SOURCE = FILE.DSP    
    if(source.find(".dsp") != string::npos){
        
        list<FLEffect*>::iterator it;
        for(it = fExecutedEffects.begin(); it!= fExecutedEffects.end(); it++){
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
    
    FLEffect* myNewEffect = new FLEffect(init, fichierSource, nameEffect);
    //    list<string> runningEffects = getNameRunningEffects();
    
    printf("PARAMETERS = SVG %s// IR %s // Options %s // opt =%i \n", fSVGFolder.c_str(), fIRFolder.c_str(), compilationOptions.c_str(), opt_Val);
    
    if(myNewEffect->init(fSVGFolder, fIRFolder, compilationOptions, opt_Val, error)){
        
        StopProgressSlot();
        
        connect(myNewEffect, SIGNAL(effectChanged()), this, SLOT(synchronize_Window()));
        
        fExecutedEffects.push_back(myNewEffect);
        
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

list<int> FLApp::WindowCorrespondingToEffect(FLEffect* eff){
    
    list<int> returning;
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
        
        if((*it)->source.compare(eff->getSource()) == 0)
            returning.push_back((*it)->ID);
    }
    return returning;
}

void FLApp::removeFilesOfWin(string sourceName, string effName){
    
    QFile::remove(sourceName.c_str());
    
    string irFile = fIRFolder + "/" + effName;
    QFile::remove(irFile.c_str());
    
    string svgFolder = fSVGFolder + "/" + effName + "-svg";
    deleteDirectoryAndContent(svgFolder);
    
}

void FLApp::synchronize_Window(){ 

    FLEffect* modifiedEffect = (FLEffect*)QObject::sender();
    
    string modifiedSource = modifiedEffect->getSource();
    char error[256];
    snprintf(error, 255, "");
    
    QDateTime modifiedLast = QFileInfo(modifiedSource.c_str()).lastModified();
    QDateTime creationDate = modifiedEffect->get_creationDate();
    
    if(QFileInfo(modifiedSource.c_str()).exists() && (modifiedEffect->isSynchroForced() || creationDate<modifiedLast)){
        
        modifiedEffect->setForceSynchro(false);
        
        modifiedEffect->stop_Watcher();
        
        //        display_CompilingProgress("Updating your DSP...");
        
        bool update = modifiedEffect->update_Factory(error, fSVGFolder, fIRFolder);
        
        if(!update){
            //            StopProgressSlot();
            fErrorWindow->print_Error(error);
            modifiedEffect->launch_Watcher();
            return;
        }
        else if(strcmp(error, "") != 0){
            fErrorWindow->print_Error(error);
        }
        
        //        StopProgressSlot();
        
        list<int> indexes = WindowCorrespondingToEffect(modifiedEffect);
        
        list<int>::iterator it;
        for(it=indexes.begin(); it!=indexes.end(); it++){
            list<FLWindow*>::iterator it2;
            
            for (it2 = FLW_List.begin(); it2 != FLW_List.end(); it2++) {
                if((*it2)->get_indexWindow() == *it){
                    if(!(*it2)->update_Window(modifiedEffect, modifiedEffect->getCompilationOptions(), modifiedEffect->getOptValue(),error)){
                        fErrorWindow->print_Error(error);
                        break;
                    }
                    else{
                        
                        //                        printf("WINDOW INDEX = %i\n", *it);
                        
                        deleteWinFromSessionFile(*it2);
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
    else if(!QFileInfo(modifiedSource.c_str()).exists()){
        
        modifiedEffect->stop_Watcher();
        
        snprintf(error, 255, "WARNING = %s has been deleted or moved\n You are now working on its copy.", modifiedSource.c_str());
        fErrorWindow->print_Error(error);
        
        string toReplace = fSourcesFolder + "/" + modifiedEffect->getName() +".dsp";
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
void FLApp::create_New_Window(string& source){
    
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
    FLEffect* first = getEffectFromSource(source, empty, fSourcesFolder, fCompilationMode, fOpt_level ,error, false); 
    
    if(first != NULL){
        
        bool optionChanged = (fCompilationMode.compare(first->getCompilationOptions()) != 0 || fOpt_level != (first->getOptValue())) && !isEffectInCurrentSession(first->getSource());
        
        //ICI ON VA FAIRE LA COPIE DU FICHIER SOURCE
        string copySource = fSourcesFolder +"/" + first->getName() + ".dsp";
        string toCopy = first->getSource();
        
        update_Source(toCopy, copySource);
        
        if(strcmp(error, "") != 0){
            fErrorWindow->print_Error(error);
        }
        
        int x, y;
        calculate_position(val, &x, &y);
        
        FLWindow* win = new FLWindow(fWindowBaseName, val, first, x, y, fSessionFolder);
        
        connect(win, SIGNAL(drop(list<string>)), this, SLOT(drop_Action(list<string>)));
        
        connect(win, SIGNAL(close()), this, SLOT(close_Window_Action()));
        connect(win, SIGNAL(closeAll()), this, SLOT(shut_AllWindows()));
        connect(win, SIGNAL(rightClick(const QPoint &)), this, SLOT(redirect_RCAction(const QPoint &)));
        connect(win, SIGNAL(error(const char*)), this, SLOT(errorPrinting(const char*)));
        
        if(win->init_Window(init, false, error)){
            FLW_List.push_back(win);
            addWinToSessionFile(win);
            first->launch_Watcher();
            
            //In case the compilation options have changed...
            if(optionChanged)
                first->update_compilationOptions(fCompilationMode, fOpt_level);
        }
        else{
            delete win;
            fErrorWindow->print_Error(error); 
        }
    }
    else
        fErrorWindow->print_Error(error);
    
}

void FLApp::create_Empty_Window(){ 
    string empty("");
    create_New_Window(empty);
}

//--------------OPEN

bool FLApp::event(QEvent *ev){
    
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

void FLApp::open_Example_Action(){
    
    string path = QFileInfo(QFileInfo( QCoreApplication::applicationFilePath()).absolutePath()).absolutePath().toStdString() + "/Resources/Examples/" + fExampleToOpen + ".dsp";
    
    if(QFileInfo(path.c_str()).exists()){
        
        fPresWin->hide();
        
        FLWindow* win = getActiveWin();
        
        if(win != NULL && win->is_Default())
            update_SourceInWin(win, path);
        else
            create_New_Window(path);
    }
}

//-------------OPEN RECENT

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
    }
    f.close();
}

void FLApp::recall_Recent_Files(string& filename){
    
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

void FLApp::set_Current_File(string& pathName, string& effName){
    
    pair<string,string> myPair = make_pair(pathName, effName);
    
    fRecentFiles.remove(myPair);
    fRecentFiles.push_front(myPair);
    
    update_Recent_File();
}

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
    
    FLWindow* win = getActiveWin();
    
    if(win != NULL && win->is_Default())
        update_SourceInWin(win, toto);
    else
        create_New_Window(toto);
}

//---------------Export

void FLApp::export_Win(FLWindow* win){
    
    fExportDialog = new FLExportManager(QUrl("http://localhost:8888"), win->get_Effect()->getSource(), win->get_Effect()->getName());
    
    connect(fExportDialog, SIGNAL(error(const char*)), this, SLOT(errorPrinting(const char*)));
    connect(fExportDialog, SIGNAL(start_progressing(const char*)), this, SLOT(display_CompilingProgress(const char*)));
    connect(fExportDialog, SIGNAL(stop_progressing()), this, SLOT(StopProgressSlot()));
    connect(fExportDialog, SIGNAL(processEnded()), this, SLOT(destroyExportDialog()));
    
    fExportDialog->init();
}

void FLApp::export_Action(){ 
    
    FLWindow* win = getActiveWin();
    
    if(win != NULL)
        export_Win(win);
}

void FLApp::destroyExportDialog(){
    delete fExportDialog;
}


//--------------CLOSE

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
        fEndTimer->start(25);
    }
    else
        quit();
}

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
        delete (*it);
    }
    FLW_List.clear();
    
    list<FLEffect*>::iterator it2;
    for(it2 = fExecutedEffects.begin() ;it2 != fExecutedEffects.end(); it2++)
        delete (*it2);
    
    fExecutedEffects.clear();
    
}

void FLApp::common_shutAction(FLWindow* win){
    
    FLEffect* toDelete = NULL;
    
    string toto = win->get_Effect()->getSource();
    string tutu = win->get_Effect()->getName();
    if(toto.find(fSourcesFolder) == string::npos)
        set_Current_File(toto, tutu);
    
    deleteWinFromSessionFile(win);
    
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
    delete (win);
    
    if(toDelete != NULL){
        delete toDelete;
        toDelete = NULL;
    }
}

void FLApp::shut_AllWindows(){
    
    while(FLW_List.size() != 0 ){
        
        FLWindow* win = *(FLW_List.begin());
        
        common_shutAction(win);
    }
}

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

void FLApp::close_Window_Action(){
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    common_shutAction(win);
}

//--------------------------------EDITION----------------------------------------

void FLApp::edit(FLWindow* win){
    
    string source = win->get_Effect()->getSource();
    
    string cmd("open ");
    
    cmd += source;
    
    QString pgm("TextEdit");
    QStringList args;
    args<<source.c_str();
    //    args.push_back(source.c_str());
    
    string error = source + " could not be opened!";
    
    if(system(cmd.c_str()))
        fErrorWindow->print_Error(error.c_str());
}

void FLApp::edit_Action(){
    
    FLWindow* win = getActiveWin();
    if(win != NULL)
        edit(win);
}

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
    
    FLWindow* win = new FLWindow(fWindowBaseName, val, commonEffect, x, y, fSessionFolder);
    
    connect(win, SIGNAL(drop(list<string>)), this, SLOT(drop_Action(list<string>)));
    connect(win, SIGNAL(close()), this, SLOT(close_Window_Action()));
    connect(win, SIGNAL(closeAll()), this, SLOT(shut_AllWindows()));
    connect(win, SIGNAL(rightClick(const QPoint &)), this, SLOT(redirect_RCAction(const QPoint &)));
    connect(win, SIGNAL(error(const char*)), this, SLOT(errorPrinting(const char*)));
    
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
    
    char error[256];
    snprintf(error, 255, "");
    
    if(win->init_Window(false, true, error)){
        FLW_List.push_back(win);
        addWinToSessionFile(win);
    }
    else{
        string toDelete = fSessionFolder + "/" + win->get_nameWindow(); 
        deleteDirectoryAndContent(toDelete);
        delete win;
        fErrorWindow->print_Error(error); 
    }
    
    //Whatever happens, the watcher has to be started (at least for the duplicated window that needs it)
    commonEffect->launch_Watcher();
}

void FLApp::duplicate_Window(){ 
    
    //copy parameters of previous window
    FLWindow* win = getActiveWin();
    if(win != NULL)
        duplicate(win);
}

void FLApp::update_SourceInWin(FLWindow* win, string source){
    
    char error[256];
    snprintf(error, 255, "");
    string empty("");
    
    //Deletion of reemplaced effect from session
    FLEffect* leavingEffect = win->get_Effect();
    leavingEffect->stop_Watcher();
    deleteWinFromSessionFile(win);
    
    FLEffect* newEffect = getEffectFromSource(source, empty, fSourcesFolder, fCompilationMode, fOpt_level, error, false);
    
    bool optionChanged;
    
    if(newEffect != NULL)
        optionChanged = (fCompilationMode.compare(newEffect->getCompilationOptions()) != 0 || fOpt_level != (newEffect->getOptValue())) && !isEffectInCurrentSession(newEffect->getSource());
    
    
    if(newEffect == NULL || (!(win)->update_Window(newEffect, fCompilationMode, fOpt_level,error))){
        //If the change fails, the leaving effect has to be reimplanted
        leavingEffect->launch_Watcher();
        addWinToSessionFile(win);
        fErrorWindow->print_Error(error);
        return;
    }
    else{
        
        //ICI ON VA FAIRE LA COPIE DU FICHIER SOURCE
        string copySource = fSourcesFolder +"/" + newEffect->getName() + ".dsp";
        string toCopy = newEffect->getSource();
        
        update_Source(toCopy, copySource);
        
        if(strcmp(error, "") != 0){
            fErrorWindow->print_Error(error);
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

void FLApp::paste_Text(){
    
    FLWindow* win = getActiveWin();
    if(win != NULL)
        paste(win);
} 

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

//-----------------------------RIGHT CLICK EVENT

void FLApp::redirect_RCAction(const QPoint & p){
    
    FLWindow* win = (FLWindow*)QObject::sender();
    
    QMenu* rcMenu = new QMenu();
    
    rcMenu->addAction(fEditAction);
    rcMenu->addAction(fPasteAction);
    rcMenu->addAction(fDuplicateAction);
    rcMenu->addSeparator();
    rcMenu->addAction(fHttpdViewAction);
    rcMenu->addAction(fSvgViewAction);
    rcMenu->addSeparator();
    rcMenu->addAction(fExportAction);
    
    rcMenu->exec(p);
}

//--------------------------------SESSION----------------------------------------

void FLApp::sessionContentToFile(string filename){
    
    //    printf("SIZE OF CONTENT SESSION = %i\n", session->size());
    
    QFile f(filename.c_str());
    
    if(f.open(QFile::WriteOnly | QIODevice::Truncate)){
        
        list<WinInSession*>::iterator it;
        
        QTextStream textWriting(&f);
        
        for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it ++){
            
            //            printf("ID = %i// Source = %s // Name = %s // X = %f // Y = %f // Option = %s\n", (*it)->ID, (*it)->source.c_str(), (*it)->name.c_str(), (*it)->x, (*it)->y, (*it)->compilationOptions.c_str());
            
            
            textWriting<<(*it)->ID<<' '<<QString((*it)->source.c_str())<<' '<<QString((*it)->name.c_str())<<' '<<(*it)->x<<' '<<(*it)->y<<' '<<QString((*it)->compilationOptions.c_str())<<' '<<(*it)->opt_level<<endl;
        }
        f.close();
    }
}

void FLApp::fileToSessionContent(string filename, list<WinInSession*>* session){
    
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

string FLApp::restore_compilationOptions(string compilationOptions){
    
    int pos = compilationOptions.find("/");
    
    while(pos != string::npos){
        
        compilationOptions.erase(pos, 1);
        compilationOptions.insert(pos, " ");
        
        pos = compilationOptions.find("/");
    }
    
    return compilationOptions;
}

//Recall for any type of session (current or snapshot)
void FLApp::recall_Session(string filename){
    
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
        
        
    }//Otherwise, one particular case has to be taken into account. If the content was modified and the Effect NOT recharged from source. The original has to be recopied!
    else{
        
    }
    
    //--------------Recalling without conflict the session
    
    list<WinInSession*>::iterator it;
    
    for(it = snapshotContent.begin() ; it != snapshotContent.end() ; it ++){
        
        char error[256];
        snprintf(error, 255, "");
        
        (*it)->compilationOptions = restore_compilationOptions((*it)->compilationOptions);
        
        FLEffect* newEffect = getEffectFromSource((*it)->source, (*it)->name, fSourcesFolder, (*it)->compilationOptions, (*it)->opt_level, error, true);
        //            printf("THE new Factory = %p\n", newEffect->getFactory());
        
        //ICI ON NE VA PAS FAIRE LA COPIE DU FICHIER SOURCE!!!
        
        if(newEffect != NULL){
            
            if(strcmp(error, "") != 0){
                fErrorWindow->print_Error(error);
            }
            
            FLWindow* win = new FLWindow(fWindowBaseName, (*it)->ID, newEffect, (*it)->x*fScreenWidth, (*it)->y*fScreenHeight, fSessionFolder);
            
            connect(win, SIGNAL(drop(list<string>)), this, SLOT(drop_Action(list<string>)));
            connect(win, SIGNAL(close()), this, SLOT(close_Window_Action()));
            connect(win, SIGNAL(closeAll()), this, SLOT(shut_AllWindows()));
            connect(win, SIGNAL(rightClick(const QPoint &)), this, SLOT(redirect_RCAction(const QPoint &)));
            connect(win, SIGNAL(error(const char*)), this, SLOT(errorPrinting(const char*)));
            
            //Modification of connection files with the new window & effect names
            
            win->update_ConnectionFile(windowNameChanges);
            win->update_ConnectionFile(nameChanges);
            
            if(win->init_Window(false, true, error)){
                //                    printf("The new DSP INstance = %p\n", win->current_DSP);
                FLW_List.push_back(win);
                newEffect->launch_Watcher();
                addWinToSessionFile(win);
                win->save_Window();
                
                //In case the compilation options have changed...
                if((*it)->compilationOptions.compare(newEffect->getCompilationOptions()) != 0)
                    newEffect->update_compilationOptions((*it)->compilationOptions, (*it)->opt_level);
            }
            else{
                //                    printf("deleting DSP... : %p\n", win->current_DSP);
                delete win;
                fErrorWindow->print_Error(error);    
            }
        }
        else{
            fErrorWindow->print_Error(error);
        }
    }
    //    sessionContent.merge(snapshotContent);
}

//--------------RECENTLY OPENED

void FLApp::save_Recent_Sessions(){
    
    QFile f(fHomeRecentSessions.c_str());
    
    if(f.open(QFile::WriteOnly | QFile::Truncate)){
        
        QTextStream text(&f);
        
        for (int i = min(kMAXRECENTSESSIONS, fRecentSessions.size()) - 1; i>=0; i--) {
            QString toto = fRecentSessions[i];
            text << toto << endl;
        }
    }
    f.close();
}

void FLApp::recall_Recent_Sessions(string& filename){
    
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

void FLApp::set_Current_Session(string& pathName){
    QString currentSess = pathName.c_str();
    fRecentSessions.removeAll(currentSess);
    fRecentSessions.prepend(currentSess);
    update_Recent_Session();
}

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
string FLApp::convert_compilationOptions(string compilationOptions){
    int pos = compilationOptions.find(" ");
    
    while(pos != string::npos){
        
        compilationOptions.erase(pos, 1);
        compilationOptions.insert(pos, "/");
        
        pos = compilationOptions.find(" ");
    }
    
    return compilationOptions;
}

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
    
    
    int i = fFrontWindow.size();
    
    QString name = win->get_nameWindow().c_str();
    name+=" : ";
    name+= win->get_Effect()->getName().c_str();
    
    QAction* fifiWindow = new QAction(name, fWindowsMenu);
    fFrontWindow.push_back(fifiWindow);
    
    fifiWindow->setData(QVariant(win->get_nameWindow().c_str()));
    connect(fifiWindow, SIGNAL(triggered()), win, SLOT(frontShow()));
    
    fWindowsMenu->addAction(fifiWindow);
    
    //    printf("ADDING ID = %i// Source = %s // Name = %s // X = %f // Y = %f // Option = %s\n", intermediate->ID, intermediate->source.c_str(), intermediate->name.c_str(), intermediate->x, intermediate->y, intermediate->compilationOptions.c_str());
    
    fSessionContent.push_back(intermediate);
}

void FLApp::deleteWinFromSessionFile(FLWindow* win){
    
    list<WinInSession*>::iterator it;
    
    for(it = fSessionContent.begin() ; it != fSessionContent.end() ; it++){
        
        if((*it)->ID == win->get_indexWindow()){
            //            printf("REMOVING = %i\n", win->get_indexWindow());
            fSessionContent.remove(*it);
            
            QAction* toRemove = NULL;
            
            QList<QAction*>::iterator it;
            for(it = fFrontWindow.begin(); it != fFrontWindow.end() ; it++){
                if((*it)->data().toString().toStdString().compare(win->get_nameWindow()) == 0){
                    fWindowsMenu->removeAction(*it);
                    fFrontWindow.removeOne(*it);
                    //                    toRemove = *it;
                    break;
                }
            }
            //            delete toRemove;
            break;
        }
    }
}

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

void FLApp::update_CurrentSession(){
    
    list<FLWindow*>::iterator it;
    
    for (it = FLW_List.begin(); it != FLW_List.end(); it++){
        
        deleteWinFromSessionFile(*it);
        addWinToSessionFile(*it);
        (*it)->save_Window();
    }
}

void FLApp::currentSessionRestoration(list<WinInSession*>* session){
    
    //If 2 windows are pointing on the same lost source, the Dialog has not to appear twice
    std::map<string,bool> updated;
    
    //List of the sources to updated in Session File
    list<pair<string, string> > sourceChanges;
    
    list<WinInSession*>::iterator it;
    
    for(it = session->begin() ; it != session->end() ; it++){
        
        string contentOrigin = pathToContent((*it)->source);
        string sourceSaved = fSourcesFolder + "/" + (*it)->name + ".dsp";
        string contentSaved = pathToContent(sourceSaved);
        
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
            QPushButton*  cancel_Button; 
            
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

void FLApp::take_Snapshot(){
    
    QFileDialog* fileDialog = new QFileDialog;
    //    QWidget* parent = this;
    
    string filename = fileDialog->getSaveFileName().toStdString();
    
    //If no name is placed, nothing happens
    if(filename.compare("") != 0){
        
        update_CurrentSession();
        sessionContentToFile(fSessionFile);
        
        //Copy of current Session under a new name, at a different location
        cpDir(fSessionFolder.c_str(), filename.c_str());
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

void FLApp::snapshotRestoration(string& file, list<WinInSession*>* session){
    
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
        if(updated.find((*it)->source) == updated.end() && infoSource.absolutePath().toStdString().compare(fSourcesFolder) != 0  && (!infoSource.exists() || contentSaved.compare(contentOrigin) != 0)){
            
            char error[256];
            
            if(!infoSource.exists())
                snprintf(error, 255, "WARNING = %s cannot be found! It is reloaded from a copied file.", (*it)->source.c_str());
            
            else if(contentSaved.compare(contentOrigin) != 0)
                snprintf(error, 255, "WARNING = The content of %s has been modified! It is reloaded from a copied file.", (*it)->source.c_str());
            
            fErrorWindow->print_Error(error);
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

void FLApp::recall_Snapshot(string filename, bool importOption){ 
    
    set_Current_Session(filename);
    
    QProcess myCmd;
    QByteArray error;
    
    string systemInstruct("tar xfv ");
    systemInstruct += filename +" -C /";
    
    myCmd.start(systemInstruct.c_str());
    myCmd.waitForFinished();
    
    error = myCmd.readAllStandardError();
    
    if(strcmp(error.data(), "") != 0)
        fErrorWindow->print_Error(error.data());
    
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
        fErrorWindow->print_Error(error2.data());
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
                newName = find_smallest_defaultName((*it)->source, currentDefault);
                currentDefault.push_back(newName);
                nameChanges.push_front(make_pair((*it)->name, newName));
                updated[(*it)->name] = true;
                (*it)->name = newName;
            }
        }
        //4- If the source is in current Folder and its name is already used, it has to be renamed
        else if(QFileInfo((*it)->source.c_str()).absolutePath().toStdString().compare(fSourcesFolder) == 0){
            
            string intermediateSource = "";
            while(isEffectNameInCurrentSession(intermediateSource, newName)){
                
                FLrenameDialog* Msg = new FLrenameDialog((*it)->name, 0);
                printf("RENAME 1\n");
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
                printf("RENAME 2\n");
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

void FLApp::copy_AllSources(string& srcDir, string& dstDir, list<std::pair<string,string> > nameChanges, string extension){
    
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

void FLApp::copy_WindowsFolders(string& srcDir, string& dstDir, list<std::pair<string,string> > windowNameChanges){
    
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

void FLApp::copy_SVGFolders(string& srcDir, string& dstDir, list<std::pair<string,string> > nameChanges){
    
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

void FLApp::viewHttpd(FLWindow* win){
    
    char error[256];
    
    if(!win->init_Httpd(error))
        fErrorWindow->print_Error(error);
}

void FLApp::httpd_View_Window(){
    
    //Searching the active Window to show its QRcode
    FLWindow* win = getActiveWin();
    if(win != NULL)
        viewHttpd(win);
    else
        fErrorWindow->print_Error("No active Window");
}

void FLApp::viewSvg(FLWindow* win){
    
    string source = win->get_Effect()->getSource();
    string pathToOpen = fSVGFolder + "/" + win->get_Effect()->getName() + "-svg/process.svg";
    
    string cmd = "open " + pathToOpen;
    string error = pathToOpen + " could not be opened!";
    
    if(system(cmd.c_str()))
        fErrorWindow->print_Error(error.c_str());
}

void FLApp::svg_View_Action(){
    
    //Searching the active Window to show its SVG Diagramm
    FLWindow* win = getActiveWin();
    if(win != NULL)
        viewSvg(win);
    
}

//--------------------------------HELP----------------------------------------
void FLApp::init_HelpWindow(){
    
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
    
    fHelpWindow->setWindowTitle(QString::fromUtf8("FAUST LIVE HELP"));
    
    groupBox = new QGroupBox(fHelpWindow);
    
    MyTabWidget = new QTabWidget(groupBox);
    layout->addWidget(MyTabWidget);
    
    QMetaObject::connectSlotsByName(fHelpWindow);
    
    fHelpWindow->setGeometry(QRect(fScreenWidth/4, 0, fScreenWidth/2, fScreenHeight*3/4));
    
    pushButton = new QPushButton("OK", fHelpWindow);
    pushButton->connect(pushButton, SIGNAL(clicked()), this, SLOT(end_apropos()));
    
    //---------------------General
    
    tab_1 = new QWidget();
    groupBox_1 = new QGroupBox(tab_1);
    groupBox_1->setGeometry(QRect(0,0, 100, 100));
    
    plainTextEdit_7 = new QPlainTextEdit(tr("\nFaustLive is a dynamical compiler for processors coded with Faust language.\nThanks to its embedded Faust & LLVM compiler, this application allows dynamical compilation of your faust objects.\n""\n""Every window of the application corresponds to an audio application, which parameters you can adjust, that you can auditorely connect with Jack to other audio applications!\n"),groupBox_1);
    
    lineEdit = new QPlainTextEdit(tr("\n\n\n\nDISTRIBUTED by GRAME - Centre de Creation Musicale"),groupBox_1);
    
    MyTabWidget->addTab(tab_1, QString());
    MyTabWidget->setTabText(MyTabWidget->indexOf(tab_1), QApplication::translate("HelpMenu", "General", 0, QApplication::UnicodeUTF8));
    
    MyTabWidget->setMaximumSize(fScreenWidth/2, fScreenHeight*3/4);
    
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
    groupBox->setGeometry(QRect(fScreenWidth/4, 0, fScreenWidth/4, fScreenHeight/4));
    fHelpWindow->setCentralWidget(groupBox);
    
    MyTabWidget->setCurrentIndex(0);
}

void FLApp::apropos(){
    fHelpWindow->show();
}

void FLApp::end_apropos(){
    fHelpWindow->hide(); 
}

void FLApp::version_Action(){
    
    QVBoxLayout* layoutGeneral = new QVBoxLayout;
    
    string text = "This application is using ""\n""- Jack 2";
//    text += jack_get_version_string();
    text += "\n""- NetJack ";
    text += "2.1";
    text += "\n""- CoreAudio API ";
    text += "4.0";
    text += "\n""- LLVM Compiler ";
    text += "3.1";
    
    QPlainTextEdit* versionText = new QPlainTextEdit(tr(text.c_str()), fVersionWindow);
    
    layoutGeneral->addWidget(versionText);
    fVersionWindow->setLayout(layoutGeneral);
    
    fVersionWindow->exec();
    
    delete versionText;
    delete layoutGeneral;
}

//-------------------------------PRESENTATION WINDOW-----------------------------

void FLApp::itemClick(QListWidgetItem *item){
    fExampleToOpen = item->text().toStdString();
}

void FLApp::itemDblClick(QListWidgetItem* item){
    fExampleToOpen = item->text().toStdString();
    open_Example_Action();
}

void FLApp::init_presentationWindow(){
    
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
    
    fPresWin->setLayout(mainLayout);
    
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
    
    if(QFileInfo(examplesPath).exists()){
    
    QDir examplesDir(examplesPath);
    
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
    
    fPresWin = new QDialog;
    fPresWin->setWindowFlags(Qt::FramelessWindowHint);
    init_presentationWindow();
    
    fPresWin->show();
    fPresWin->raise();
}

//--------------------------------PREFERENCES---------------------------------------

void FLApp::styleClicked(){
    
    QPushButton* item = (QPushButton*)QObject::sender();
    styleClicked(item->text().toStdString());
}

void FLApp::styleClicked(string style){
    
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

void FLApp::Preferences(){
        
    fPrefDialog->exec();
}

void FLApp::init_PreferenceWindow(){
    
    fPrefDialog->setWindowTitle("PREFERENCES");
    
    QTabWidget* myTab = new QTabWidget(fPrefDialog);
    myTab->setStyleSheet("*{}""*::tab-bar{}");
    QTabBar* myTabBar = new QTabBar(myTab);
    
    QGroupBox* menu1 = new QGroupBox(myTab);
    QGroupBox* menu2 = new QGroupBox(myTab);
    QGroupBox* menu3 = new QGroupBox(myTab);
    
    myTab->addTab(menu1, tr("Window Preferences"));
    myTab->addTab(menu2, tr("Audio Preferences"));
    myTab->addTab(menu3, tr("Style Preferences"));
    
    fAudioBox = new QGroupBox(menu2);
    fAudioCreator = AudioCreator::_Instance(fSettingsFolder, fAudioBox);
    
    fCompilModes = new QLineEdit(menu1);
    fOptVal = new QLineEdit(menu1);
    
    QWidget* intermediateWidget = new QWidget(fPrefDialog);
    
    QPushButton* cancelButton = new QPushButton(tr("Cancel"), intermediateWidget);
    cancelButton->setDefault(false);;
    
    QPushButton* saveButton = new QPushButton(tr("Save"), intermediateWidget);
    saveButton->setDefault(true);
    
    connect(saveButton, SIGNAL(released()), this, SLOT(save_Mode()));
    connect(cancelButton, SIGNAL(released()), this, SLOT(cancelPref()));
    
    recall_Settings(fHomeSettings);
    
    fCompilModes->setText(fCompilationMode.c_str());
    stringstream oV;
    
    oV << fOpt_level;
    fOptVal->setText(oV.str().c_str());
    
    QFormLayout* layout1 = new QFormLayout;
    QFormLayout* layout2 = new QFormLayout;
    QFormLayout* layout3 = new QFormLayout;
    QGridLayout* layout4 = new QGridLayout;
    QVBoxLayout* layout5 = new QVBoxLayout;
    QHBoxLayout* intermediateLayout = new QHBoxLayout;
    intermediateLayout->setAlignment(Qt::AlignCenter);
    
    layout1->addRow(new QLabel(tr("")));
    QLabel* la1 = new QLabel(tr("Default Compilation Options"));
    layout1->addRow(la1, fCompilModes);
    QLabel* la2 = new QLabel(tr("Optimization Value of compilation"));
    layout1->addRow(la2, fOptVal);
    layout1->addRow(new QLabel(tr("")));
    
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
    
    intermediateLayout->addWidget(cancelButton);
    intermediateLayout->addWidget(new QLabel(tr("")));
    intermediateLayout->addWidget(saveButton);
    
    intermediateWidget->setLayout(intermediateLayout);
    layout3->addRow(intermediateWidget);
    
    menu1->setLayout(layout1);
    
    layout2->addWidget(fAudioBox);
    menu2->setLayout(layout2);
    
    fPrefDialog->setLayout(layout3);
}

void FLApp::cancelPref(){
    fPrefDialog->hide();
    fAudioCreator->reset_Settings();
}

void FLApp::save_Mode(){
    
    fCompilationMode = fCompilModes->text().toStdString();
    
    const char* intermediate = fOptVal->text().toStdString().c_str();
    if(isStringInt(intermediate))
        fOpt_level = atoi(intermediate);
    else
        fOpt_level = 3;
    
    fPrefDialog->hide();

    if(fAudioCreator->didSettingChanged()){
        printf("WE ARE GOING TO UPDATE....\n");
        update_AudioArchitecture();
    }
    else
        fAudioCreator->reset_Settings();
}

void FLApp::save_Settings(string& home){
    
    string modeText = fCompilationMode;
    
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
        
        textWriting<<modeText.c_str()<<' '<<fOpt_level<<fStyleChoice.c_str();
        
        f.close();
    }
}

void FLApp::recall_Settings(string& home){

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
    
    int pos = 0;
    
    while(modeText.find("/", pos) != string::npos){
        
        if(pos != string::npos && modeText[pos] != '-')
            modeText.replace(pos, 1, " ");
        
        pos = modeText.find("/", pos+1);
    }
    
    if(modeText.compare(" ") == 0)
        modeText = "";
    
    fCompilationMode = modeText;
}

void FLApp::update_AudioArchitecture(){
    
    list<FLWindow*>::iterator it;
    list<FLWindow*>::iterator updateFailPointer;
    
    bool updateSuccess = true;
    string errorToPrint;
    char error[256];
    snprintf(error, 255, "");
    
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
            
            fErrorWindow->print_Error(errorToPrint.c_str());
            fErrorWindow->print_Error(error);
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

void FLApp::display_CompilingProgress(const char* msg){
    fCompilingMessage = new QDialog();
    fCompilingMessage->setWindowFlags(Qt::FramelessWindowHint);
    
    QLabel* tittle = new QLabel(tr("<h2>FAUSTLIVE</h2>"));
    tittle->setAlignment(Qt::AlignCenter);
    QLabel* text = new QLabel(tr(msg), fCompilingMessage);
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

void FLApp::StopProgressSlot(){
    fCompilingMessage->hide();
    delete fCompilingMessage;
}
