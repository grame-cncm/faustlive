//
//  FLWindow.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "faust/gui/faustqt.h"
#include "faust/gui/httpdUI.h"
#include "faust/gui/FUI.h"
#include "faust/gui/OSCUI.h"

#if __APPLE__
#define __MACOSX_CORE__ 1
#endif

#if __linux__
#define __LINUX_ALSA__ 1
#endif

#if _WIN32
#define __WINDOWS_MM__ 1
#endif

#include "faust/gui/MidiUI.h"
#include "faust/midi/RtMidi.cpp"

#include "FLWindow.h"
#include "HTTPWindow.h"
#include "FLInterfaceManager.h"
#include "FLToolBar.h"
#include "FLServerHttp.h"

#ifdef REMOTE
#include "FLStatusBar.h"
#endif

#include "AudioCreator.h"
#include "AudioManager.h"
#include "utilities.h"
#include "FLSettings.h"
#include "FLWinSettings.h"
#include "FLSessionManager.h"
#include "FLExportManager.h"
#include "FLFileWatcher.h"
#include "FLErrorWindow.h"
#include "FLMessageWindow.h"

#ifdef JACK
#include "JA_audioManager.h"
#include "JA_audioFader.h"
#endif

#ifdef REMOTE
#include "faust/dsp/remote-dsp.h"
#endif

#include "faust/dsp/llvm-dsp.h"

list<GUI*> GUI::fGuiList;
ztimedmap GUI::gTimedZoneMap;

/****************************FaustLiveWindow IMPLEMENTATION***************************/

//------------CONSTRUCTION WINDOW
//@param : baseName = Window name
//@param : index = Index of the window
//@param : home = current Session folder
//@param : windowSettings = parameters of the settings 
//@param : appMenus = list of menus contained in the window
FLWindow::FLWindow(QString& baseName, int index, const QString& home, FLWinSettings* windowSettings, QList<QMenu*> appMenus)
{
    connect(this, SIGNAL(audioError(const QString&)), this, SLOT(audioShutDown(const QString&)));
    
    fSettings = windowSettings;
    fSettings->setValue("Release/Number", 0);
    
    // Enable Drag & Drop on window
    setAcceptDrops(true);
    
    // Creating Window Name
    fWindowIndex = index;
    fWindowName = baseName +  QString::number(fWindowIndex);
    
    fIsDefault = false;
    fAudioManagerStopped = false;
    
    // Initializing class members
    
    fHttpdWindow = NULL;
    fHttpInterface = NULL;
	fOscInterface = NULL;
    fMIDIInterface = NULL;

    fInterface = NULL;
    fRCInterface = NULL;
    fCurrentDSP = NULL;
    
    fToolBar = NULL;
    
    // Creating Window Folder
    fHome = home;
    
    // Creating Audio Manager
    AudioCreator* creator = AudioCreator::_Instance(NULL);
    
    fAudioManager = creator->createAudioManager(FLWindow::audioShutDown, this);
    fClientOpen = false;
    
    // Set Menu & ToolBar
    fLastMigration = QDateTime::currentDateTime();
    set_ToolBar();
#ifdef REMOTE
    fStatusBar = NULL;
    set_StatusBar();
#endif
    set_MenuBar(appMenus);
    
#ifdef REMOTE
    connect(this, SIGNAL(remoteCnxLost(int)), this, SLOT(RemoteCallback(int)));
#endif
}

FLWindow::~FLWindow()
{
    delete menuBar();
}

//------------------------WINDOW ACTIONS

//Show Window on front end with standard size
void FLWindow::frontShow()
{
    int x = fSettings->value("Position/x", 0).toInt();
    int y = fSettings->value("Position/y", 0).toInt();
    int w = fSettings->value("Size/w", 0).toInt();
    int h = fSettings->value("Size/h", 0).toInt();
    setGeometry(x, y, w, h);
    if (w == 0 && h == 0) adjustSize();
    
    show();
    raise();
    
    setMinimumSize(QSize(0, 0));
    setMaximumSize(QSize(QApplication::desktop()->geometry().size().width(), QApplication::desktop()->geometry().size().height()));
}

void FLWindow::start_stop_watcher(bool on)
{
    QVector<QString> dependencies = FLSessionManager::_Instance()->readDependencies(fSettings->value("SHA", "").toString());
    
//--- Add possible wavfile to dependencies
    if (fWavSource != "") {
        dependencies.push_front(fWavSource);
    }
    
    FLSessionManager::_Instance()->writeDependencies(dependencies, getSHA());
    
    if (fSettings->value("Path", "").toString() != "")
        dependencies.push_front(fSettings->value("Path", "").toString());

    if (on) {
        fCreationDate = fCreationDate.currentDateTime();
        FLFileWatcher::_Instance()->startWatcher(dependencies, this);
    } else {
        FLFileWatcher::_Instance()->stopWatcher(dependencies, this);
    }
}

//Initialization of User Interface + StartUp of Audio Client
//@param : init = if the window created is a default window.
//@param : error = in case init fails, the error is filled
bool FLWindow::init_Window(int init, const QString& source, QString& errorMsg)
{
    fSource = source;
    
//---- If wav, sound2faust has to be executed
    fWavSource = "";
    
    if (ifWavToString(fSource, fWavSource)) {
        fSource = fWavSource;
        fWavSource = source;
    }
    
    FLMessageWindow::_Instance()->displayMessage("Compiling DSP...");
    FLMessageWindow::_Instance()->show();
    FLMessageWindow::_Instance()->raise();
    
    FLSessionManager* sessionManager = FLSessionManager::_Instance();
    
    QPair<QString, void*> factorySetts = sessionManager->createFactory(source, fSettings, errorMsg);

    FLMessageWindow::_Instance()->hide();
    
    if (!factorySetts.second) { // testing if the factory pointer is null (= the compilation failed)
        return false;
    }
  	
    if (!init_audioClient(errorMsg)) {
        return false;
    }

    fCurrentDSP = sessionManager->createDSP(factorySetts, source, fSettings, remoteDSPCallback, this, errorMsg);
    
    if (!fCurrentDSP) {
        return false;
    }
    
    if (init != kNoInit) {
        fIsDefault = true;
        print_initWindow(init);
    }

    if (allocateInterfaces(fSettings->value("Name", "").toString())) {
        buildInterfaces(fCurrentDSP);
        if (setDSP(errorMsg)) {
            start_Audio();
            frontShow();
            runInterfaces();
            start_stop_watcher(true);
            return true;
        } else {
            deleteInterfaces();
        }
    } else {
        errorMsg = "Interface could not be allocated";
    }
    
    sessionManager->deleteDSPandFactory(fCurrentDSP);
    fCurrentDSP = NULL;
    return false;
}

//--Transforms Wav file into faust string
bool FLWindow::ifWavToString(const QString& source, QString& newSource)
{
    // --> à voir comment on gère, vu qu'on enregistre pas de fichier source "intermédiaire". Est-ce qu'on recalcule la waveform quand on demande d'éditer ??
    
    QString suffix = QFileInfo(source).completeSuffix();
    
    if (suffix == "wav" || suffix == "aif" || suffix == "aiff" || suffix == "aifc") {
        
        QString exeFile = "";
        QString soundFileName = QFileInfo(source).baseName();
        QString destinationFile = QFileInfo(source).absolutePath();
        destinationFile += "/" ;
        destinationFile += soundFileName;
        QString waveFile = destinationFile;
        waveFile += "_waveform.dsp";
        destinationFile += ".dsp";
        QString systemInstruct;
        
		// figure out the right name for faust2sound depending of the OS
        
#ifdef _WIN32
        exeFile = "sound2faust.exe";
#endif
        
#ifdef __linux__
        if(QFileInfo("/usr/local/bin/sound2faust").exists())
            exeFile = "/usr/local/bin/sound2faust";
        else
            exeFile = "./sound2faust";   
#endif
        
#ifdef __APPLE__
        //        FLErrorWindow::_Instance()->print_Error(QCoreApplication::applicationDirPath());
        exeFile = QCoreApplication::applicationDirPath() + "/sound2faust";
        //        if(QCoreApplication::applicationDirPath().indexOf("Contents/MacOS") != -1)
        //            exeFile = "./sound2faust";
        //        else
        //            exeFile = QCoreApplication::applicationDirPath() + "/FaustLive.app/Contents/MacOS/sound2faust";
#endif
        systemInstruct += exeFile + " ";
        systemInstruct += "\"" + source + "\"" + " -o " + waveFile;
        
        if (!QFileInfo(exeFile).exists()) {
            FLErrorWindow::_Instance()->print_Error("ERROR : soundToFaust executable could not be found!");
        }
        
        QString errorMsg("");
        if (!executeInstruction(systemInstruct, errorMsg)) {
            FLErrorWindow::_Instance()->print_Error(errorMsg);
        }
        
        QString finalFileContent = "//The waveform was automatically generated in :\nimport(\"";
        finalFileContent += soundFileName + "_waveform.dsp";
        finalFileContent += "\");\n\n//It can accessed with :\n//";
        finalFileContent += soundFileName + "_i" + " with i = [0, ..., n] and n the number of channels\n\n";
        finalFileContent += "//The example played here is :\n//";
        finalFileContent += soundFileName + " = (" + soundFileName + "_0, ..., " + soundFileName + "_n) : ((!,_), ..., (!,_));\n\n";
        finalFileContent +="process = ";
        finalFileContent += QFileInfo(source).baseName();
        finalFileContent += ";";
        finalFileContent += "\n\n//Also, rtables are created in " + soundFileName + "_waveform.dsp" + " and are named : \n//" + soundFileName + "_rtable_i";
        
        writeFile(destinationFile, finalFileContent);
        newSource = destinationFile;
        return true;
    } else {
        return false;
    }
}

void FLWindow::selfUpdate()
{
//Avoiding the flicker when the source is saved - Mostly seeable on 10.9
//    if(QFileInfo(fSource).exists()){
//        
//        QDateTime modifiedLast = QFileInfo(fSource).lastModified();
//        if(fCreationDate < modifiedLast)
//            update_Window(fSource);
//    }
//    else
    QString wavform = fWavSource;
    update_Window(fSource);
    fWavSource = wavform;
}

void FLWindow::selfNameUpdate(const QString& oldSource, const QString& newSource)
{
 //    In case name update is concerning source
    if (oldSource == fSource) {
        QString wavform = fWavSource;
        update_Window(newSource);
        fWavSource = wavform;
//    In case name update concerns a dependency
    } else {
        QString errorMsg = "WARNING : "+ fWindowName+". " + oldSource + " has been renamed as " + newSource + ". The dependency might be broken ! ";
        errorPrint(errorMsg);
    }
}

//Modification of the process in the window
//@param : source = source that reemplaces the current one
bool FLWindow::update_Window(const QString& source)
{
    
//    bool update = false;
//    bool update = true;  
//Avoiding the flicker when the source is saved - Mostly seeable on 10.9
//FIND THE RIGHT CONDITION !!!!
//    
//    if(QFileInfo(source).exists()){
//        
//        QDateTime modifiedLast = QFileInfo(source).lastModified();
//        if(fSource != source || fCreationDate < modifiedLast)
//            update = true;
//    }
//    else
//        update = true; 
//    ---- AVOIDs flicker but switch remote machine doesnt update && compilation options either!!!
//    printf("is update not true??? = %i\n", update); --> moved to selfUpdate !! 
    
//    if(update){
        
    float saveW = 0.0;
    float saveH = 0.0;
    
    float newW = 0.0;
    float newH = 0.0;
    
    if (fInterface) {
        saveW = fInterface->minimumSizeHint().width();
        saveH = fInterface->minimumSizeHint().height();
    }
    
    start_stop_watcher(false);
    
    FLMessageWindow::_Instance()->displayMessage("Updating DSP...");
    FLMessageWindow::_Instance()->show();
    FLMessageWindow::_Instance()->raise();
    hide();
  
    QString savedName = fSettings->value("Name", "").toString();
    QString savedPath = fSettings->value("Path", "").toString();
    QString savedSHA = fSettings->value("SHA", "").toString();
    
    saveWindow();
    hide();
    
    //creating the new DSP instance
    dsp* charging_dsp = NULL;
    //    if(newEffect->isLocal())
    
    QString errorMsg("");
    FLSessionManager* sessionManager = FLSessionManager::_Instance();
    QString sourceToCompile = source;
    QString wavsource = "";

    if (ifWavToString(sourceToCompile, wavsource)) {
        sourceToCompile = wavsource;
        wavsource = source;
    }

    QPair<QString, void*> factorySetts = sessionManager->createFactory(sourceToCompile, fSettings, errorMsg);
    bool isUpdateSucessfull = true;
    
    if (!factorySetts.second) {
        isUpdateSucessfull = false;
    }
    
    if (isUpdateSucessfull) {
        
        charging_dsp = sessionManager->createDSP(factorySetts, source, fSettings, remoteDSPCallback, this, errorMsg);
         
        if (charging_dsp) {
            
            QString newName = fSettings->value("Name", "").toString();
            
            if (fAudioManager->init_FadeAudio(errorMsg, newName.toStdString().c_str(), charging_dsp)) {
                
                fIsDefault = false;
                deleteInterfaces();
                
                //Set the new interface & Recall the parameters of the window
                if (allocateInterfaces(newName)) {
                    
                    buildInterfaces(charging_dsp);
                    recall_Window();
                    
                    //Start crossfade and wait for its end
                    fAudioManager->start_Fade();
                    fAudioManager->wait_EndFade();
                    
                    //SWITCH the current DSP as the dropped one
                    dsp* tmp = fCurrentDSP;
                    fCurrentDSP = charging_dsp;
                    charging_dsp = tmp;
                    
                    FLSessionManager::_Instance()->deleteDSPandFactory(charging_dsp);
                    
                    fSource = sourceToCompile;
                    fWavSource = wavsource;
                    
                    isUpdateSucessfull = true;
                    
                } else if (allocateInterfaces(savedName)) {
                
                    buildInterfaces(fCurrentDSP);
                    recall_Window();
                    errorMsg = "Impossible to allocate new interface";
                    isUpdateSucessfull = false; 
                    sessionManager->deleteDSPandFactory(charging_dsp);
                    charging_dsp = NULL;
                    
                    if (fSettings) {
                        fSettings->setValue("Path", savedPath);
                        fSettings->setValue("Name", savedName);
                        fSettings->setValue("SHA", savedSHA);
                    }
                }
                
                //Step 12 : Launch User Interface
                runInterfaces();
            }
        }
    }
    
    start_stop_watcher(true);
    
    if (!isUpdateSucessfull) {
        errorPrint(errorMsg);
    } else {
        emit windowNameChanged();
    }

    FLMessageWindow::_Instance()->hide();

    if (fInterface) {
        newW = fInterface->minimumSizeHint().width();
        newH = fInterface->minimumSizeHint().height();
    }

// 2 cases : 
//    1- Updating with a new DSP --> adjusting Size to the new interface
//    2- Self Updating --> keeping the window as it is (could have been opened or shred)
    if (newH != saveH || newW != saveW)
        adjustSize();

    show();
    return isUpdateSucessfull;
}

//Reaction to source deletion
void FLWindow::source_Deleted()
{
    QString msg = "Warning your file : " + fSource + " was deleted. You are now working on an internal copy of this file.";
    fSource = FLSessionManager::_Instance()->contentOfShaSource(fSettings->value("SHA", "").toString());
    fSettings->setValue("Path", "");
    errorPrint(msg);
}

//------------------------MENUS ACTIONS

//Right-click
void FLWindow::contextMenuEvent(QContextMenuEvent* ev) 
{
    fWindowMenu->exec(ev->globalPos());
}

//Menu Bar
void FLWindow::set_MenuBar(QList<QMenu*> appMenus)
{
    //----------------FILE
    //    
    QMenuBar *myMenuBar = new QMenuBar(NULL);
    setMenuBar(myMenuBar);
    QList<QMenu*>::iterator it = appMenus.begin();
    myMenuBar->addMenu(*it);
    myMenuBar->addSeparator();
    it++;
    
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
    connect(httpdViewAction, SIGNAL(triggered()), this, SLOT(view_qrcode()));
    
    QAction* svgViewAction = new QAction(tr("&View SVG Diagram"),this);
    svgViewAction->setShortcut(tr("Ctrl+G"));
    svgViewAction->setToolTip(tr("Open the SVG Diagram in a browser"));
    connect(svgViewAction, SIGNAL(triggered()), this, SLOT(view_svg()));
    
    QAction* exportAction = new QAction(tr("&Export As..."), this);
    exportAction->setShortcut(tr("Ctrl+P"));
    exportAction->setToolTip(tr("Export the DSP in whatever architecture you choose"));
    connect(exportAction, SIGNAL(triggered()), this, SLOT(export_file()));
    
    QAction* shutAction = new QAction(tr("&Close Window"),this);
    shutAction->setShortcut(tr("Ctrl+W"));
    shutAction->setToolTip(tr("Close the current Window"));
    connect(shutAction, SIGNAL(triggered()), this, SLOT(shut()));
    
    fWindowMenu = new QMenu(tr("&Window"), NULL);
    fWindowMenu->addAction(editAction);
    fWindowMenu->addAction(pasteAction);
    fWindowMenu->addAction(duplicateAction);
    fWindowMenu->addSeparator();
    fWindowMenu->addAction(httpdViewAction);

    fWindowMenu->addAction(svgViewAction);
    fWindowMenu->addSeparator();
    fWindowMenu->addAction(exportAction);
    fWindowMenu->addSeparator();
    fWindowMenu->addAction(shutAction);
    
    myMenuBar->addMenu(fWindowMenu);
    myMenuBar->addSeparator();
    
    while (it != appMenus.end()) {
        myMenuBar->addMenu(*it);
        myMenuBar->addSeparator();
        it++;
    }
}

//----SLOTS
void FLWindow::edit()
{
    QString sourcePath = fSettings->value("Path", "").toString();
    QString pathToOpen = sourcePath;
    
    if (sourcePath == "") { 
        pathToOpen = FLSessionManager::_Instance()->askForSourceSaving(FLSessionManager::_Instance()->contentOfShaSource(getSHA()));
        //    In case user has saved his file in a new location
        if (pathToOpen != "" && pathToOpen != ".dsp") {
            update_Window(pathToOpen);
        } else {
            return;
        }
    }
    
    FLSessionManager::_Instance()->updateFolderDate( fSettings->value("SHA", "").toString());
    
    QUrl url = QUrl::fromLocalFile(pathToOpen);
    bool b = QDesktopServices::openUrl(url);
    
    if (!b) {
        errorPrint("Your DSP file could not be opened!\nMake sure you have a default application configured for DSP Files.");
    }
}

void FLWindow::paste()
{
    // Recuperation of Clipboard Content
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    
    if (mimeData->hasText()) {
        QString clipText = clipboard->text();
        update_Window(clipText);
    }
}

void FLWindow::duplicate()
{
    emit duplicate_Action();
}

void FLWindow::view_qrcode()
{
    fToolBar->switchHttp(true);    
    viewQRCode();
}

void FLWindow::view_svg()
{
    QString svgPath = fHome + "/Windows/" + fWindowName;
    QString errorMsg;
    if (FLSessionManager::_Instance()->generateSVG(getSHA(), getPath(), svgPath, fWindowName, errorMsg)) {
        QString pathToOpen = fHome + "/Windows/" + fWindowName + "/" + fWindowName + "-svg/process.svg";
        QUrl url = QUrl::fromLocalFile(pathToOpen);
        if (!QDesktopServices::openUrl(url)) {
            errorPrint("Your SVG could not be opened!\nMake sure you have a default application configured for SVG Files.");
        }
    } else {
        QString err ="Could not generate SVG diagram : " + errorMsg; 
        errorPrint(err);
    }
}

void FLWindow::export_file()
{
    FLTargetChooser* targetDialog = FLTargetChooser::_Instance();
    if (targetDialog->exec()) {
        QString expandedCode = FLSessionManager::_Instance()->getExpandedVersion(fSettings, FLSessionManager::_Instance()->contentOfShaSource(getSHA()));
        FLExportManager* exportDialog = FLExportManager::_Instance();
        exportDialog->exportFile(getName(), expandedCode, targetDialog->platform(), targetDialog->architecture(), targetDialog->binOrSource());
    }
}

void FLWindow::shut()
{
    emit close();
    // emit closeWin();
}

//------------TOOLBAR RELATED ACTIONS

//Set up of the Window ToolBar
void FLWindow::set_ToolBar()
{
    fToolBar = new FLToolBar(fSettings, this);
    addToolBar(Qt::TopToolBarArea, fToolBar);
    
    connect(fToolBar, SIGNAL(compilationOptionsChanged()), this, SLOT(modifiedOptions()));
    connect(fToolBar, SIGNAL(generateNewAuxFiles()), this, SLOT(generateAuxFiles()));
//    connect(fToolBar, SIGNAL(execScript()), this, SLOT(scriptExecution()));
    connect(fToolBar, SIGNAL(sizeGrowth()), this, SLOT(resizingBig()));
    connect(fToolBar, SIGNAL(sizeReduction()), this, SLOT(resizingSmall()));
	connect(fToolBar, SIGNAL(switch_http(bool)), this, SLOT(switchHttp(bool)));
	connect(fToolBar, SIGNAL(oscPortChanged()), this, SLOT(updateOSCInterface()));
	connect(fToolBar, SIGNAL(switch_osc(bool)), this, SLOT(switchOsc(bool)));
    connect(fToolBar, SIGNAL(switch_midi(bool)), this, SLOT(switchMIDI(bool)));
    connect(fToolBar, SIGNAL(switch_poly(bool)), this, SLOT(switchPoly(bool)));

#ifdef REMOTE
    connect(fToolBar, SIGNAL(switch_release(bool)), this, SLOT(switchRelease(bool)));
    connect(fToolBar, SIGNAL(switch_remotecontrol(bool)), this, SLOT(switchRemoteControl(bool)));
#endif
}

//Set the windows options with current values
void FLWindow::setWindowsOptions()
{
    if (fHttpInterface) {
        fSettings->setValue("Http/Port", fHttpInterface->getTCPPort());
    }

	if (fOscInterface) {        
        fSettings->setValue("Osc/InPort", QString::number(fOscInterface->getUDPPort()));
        fSettings->setValue("Osc/OutPort", QString::number(fOscInterface->getUDPOut()));
        fSettings->setValue("Osc/DestHost", fOscInterface->getDestAddress());
        fSettings->setValue("Osc/ErrPort", QString::number(fOscInterface->getUDPErr()));
    }
    
    fToolBar->syncVisualParams();
}

//Reaction to the modifications of the ToolBar options
void FLWindow::modifiedOptions()
{
    update_Window(fSource);
}

//Reaction to the modification of outfile options
void FLWindow::generateAuxFiles()
{
	QString errorMsg;

    if (!FLSessionManager::_Instance()->generateAuxFiles(getSHA(), getPath(), fSettings->value("AutomaticExport/Options", "").toString(), getSHA(), errorMsg))
		FLErrorWindow::_Instance()->print_Error(QString("Additional Compilation Step : ")+ errorMsg);
}

//Reaction to the resizing the toolbar
void FLWindow::resizingSmall()
{
//  fSettings->setValue("Osc/DestHost", fOscInterface->getDestAddress());
//  setWindowsOptions();
    
    setMinimumSize(QSize(0,0));
    adjustSize();
    addToolBar(Qt::TopToolBarArea, fToolBar);
}

void FLWindow::resizingBig()
{
    addToolBar(Qt::LeftToolBarArea, fToolBar);
    
    //    QSize winSize = fToolBar->geometry().size();
    //    winSize += fToolBar->minimumSize();
    //   
    //    
//    QSize winMinSize = minimumSize();
//    winMinSize += fToolBar->geometry().size();
    
    //    setGeometry(0,0,winSize.width(), winSize.height());
//    setMinimumSize(winMinSize);
    //
    adjustSize();
}

//------------STATUSBAR RELATED ACTIONS

void FLWindow::set_StatusBar()
{
#ifdef REMOTE
    fStatusBar = new FLStatusBar(fSettings, this);
    connect(fStatusBar, SIGNAL(switchMachine()), this, SLOT(redirectSwitch()));
    setStatusBar(fStatusBar);
#endif
}

//Redirection machine switch
void FLWindow::redirectSwitch()
{
#ifdef REMOTE
    if(!update_Window(fSource)){
        fStatusBar->remoteFailed();
    }
#endif
}

//------------ALLOCATION/DESALLOCATION OF INTERFACES

void FLWindow::disableOSCInterface()
{
    fToolBar->switchOsc(false);
}

void FLWindow::switchOsc(bool on)
{
    if (on) {
        updateOSCInterface();
    } else {
        deleteOscInterface();
    }
}

void FLWindow::switchMIDI(bool on)
{
    // Needed for MidiUI
    stop_Audio();
    
    if (on) {
        updateMIDIInterface();
    } else {
        deleteMIDIInterface();
    }
    
    // Needed for MidiUI
    start_Audio();
}

void FLWindow::switchPoly(bool on)
{
    // Needed for Poly DSP
    stop_Audio();
    
    printf("switchPoly %d\n", on);
    string voice = fSettings->value("Polyphony/Voice", "4").toString().toStdString();
    printf("switchPoly  voice %s\n", voice.c_str());
  
    if (on) {
        ///
    } else {
        ///
    }
    
    // Needed for Poly DSP
    start_Audio();
} 

void FLWindow::updateMIDIInterface()
{
    if (fSettings->value("MIDI/Enabled", FLSettings::_Instance()->value("General/Control/MIDIDefaultChecked", false)).toBool()) {
        saveWindow();
        deleteMIDIInterface();
        allocateMIDIInterface();
        fCurrentDSP->buildUserInterface(fMIDIInterface);
        recall_Window();
        fMIDIInterface->run();
        FLInterfaceManager::_Instance()->registerGUI(fMIDIInterface);
        setWindowsOptions();
    }
}

void FLWindow::allocateMIDIInterface()
{
#ifdef JACK
    JA_audioManager* manager = dynamic_cast<JA_audioManager*>(fAudioManager);
    // Special case for JACK audio manager
    if (manager) {
        fMIDIInterface = new MidiUI(manager->getAudioFader());
    } else {
        fMIDIInterface = new MidiUI(fWindowName.toStdString());
    }
#else
    fMIDIInterface = new MidiUI(fWindowName.toStdString());
#endif
}

void FLWindow::deleteMIDIInterface()
{
    if (fMIDIInterface) {
        FLInterfaceManager::_Instance()->unregisterGUI(fMIDIInterface);
        delete fMIDIInterface;
        fMIDIInterface = NULL;
    }
}

void catch_OSCError(void* arg)
{
    FLWindow* win = (FLWindow*)(arg);
    win->errorPrint("Too many OSC interfaces are opened at the same time! A new connection could not start");
    win->disableOSCInterface();
}
        
//Allocation of Interfaces
void FLWindow::allocateOscInterface()
{
    int argc = 11; 
    
//---- Allocation for windows needs
    char** argv = new char*[argc + 1];
    argv[0] = (char*)(fWindowName.toStdString().c_str());
    argv[1] = (char*)"-port";
    
    string inport = fSettings->value("Osc/InPort", "5510").toString().toStdString();
    argv[2] = (char*) (inport.c_str());
    argv[3] = (char*)"-xmit";
    argv[4] = (char*)"1";
    argv[5] = (char*)"-outport";
    string outport = fSettings->value("Osc/OutPort", "5511").toString().toStdString();
    argv[6] = (char*) (outport.c_str());
    argv[7] = (char*)"-desthost";
    string dest = fSettings->value("Osc/DestHost", "localhost").toString().toStdString();
    argv[8] = (char*) (dest.c_str());
    argv[9] = (char*)"-errport";
    string errport = fSettings->value("Osc/ErrPort", "5512").toString().toStdString();
    argv[10] = (char*) (errport.c_str());
    
    argv[argc] = 0; // NULL terminated argv

    fOscInterface = new OSCUI(argv[0], argc, argv, NULL, &catch_OSCError, this, false);
    delete [] argv;
}

void FLWindow::deleteOscInterface()
{
    if (fOscInterface) {
        FLInterfaceManager::_Instance()->unregisterGUI(fOscInterface);
        delete fOscInterface;
        fOscInterface = NULL;
    }
}

void FLWindow::updateOSCInterface()
{
    saveWindow();
    deleteOscInterface();
    allocateOscInterface();
    fCurrentDSP->buildUserInterface(fOscInterface);
    recall_Window();
    fOscInterface->run();
    FLInterfaceManager::_Instance()->registerGUI(fOscInterface);
    setWindowsOptions();
}

//----4 STEP OF THE INTERFACES LIFE

bool FLWindow::allocateInterfaces(const QString& nameEffect)
{
    QString intermediate = fWindowName + " : " + nameEffect;
    setWindowTitle(intermediate);
    
    if (!fIsDefault) {
        QScrollArea *sa = new QScrollArea(this);
        
        fInterface = new QTGUI(sa);
        sa->setWidgetResizable(true);
        sa->setWidget(fInterface);
        sa->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded);
        sa->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded);
        
        setCentralWidget(sa);
        fInterface->installEventFilter(this);
    }
    
    fRCInterface = new FUI;
  
    if (fSettings->value("Http/Enabled", FLSettings::_Instance()->value("General/Network/HttpDefaultChecked", false)).toBool()) {
        allocateHttpInterface();
    }

	if (fSettings->value("Osc/Enabled", FLSettings::_Instance()->value("General/Network/OscDefaultChecked", false)).toBool()) {
        allocateOscInterface();
    }
    
    if (fSettings->value("MIDI/Enabled", FLSettings::_Instance()->value("General/Control/MIDIDefaultChecked", false)).toBool()) {
        allocateMIDIInterface();
    }

    return true;
}

//Building QT Interface | Osc Interface | Parameter saving Interface | ToolBar
void FLWindow::buildInterfaces(dsp* compiledDSP)
{
    if (fInterface) {
        compiledDSP->buildUserInterface(fInterface);
    }
    
    if (fRCInterface) {
        compiledDSP->buildUserInterface(fRCInterface);
    }
      
    if (fHttpInterface) {
        compiledDSP->buildUserInterface(fHttpInterface);  
    }
   
    if (fOscInterface) {
        compiledDSP->buildUserInterface(fOscInterface);
    }
    
    if (fMIDIInterface) {
        compiledDSP->buildUserInterface(fMIDIInterface);
    }
}

void FLWindow::runInterfaces()
{
    if (fHttpInterface) {
        fHttpInterface->run();
        FLServerHttp::_Instance()->declareHttpInterface(fHttpInterface->getTCPPort(), getName().toStdString());
    }

    if (fOscInterface) {
        fOscInterface->run();
        FLInterfaceManager::_Instance()->registerGUI(fOscInterface);
    }
    
    if (fMIDIInterface) {
        fMIDIInterface->run();
        FLInterfaceManager::_Instance()->registerGUI(fMIDIInterface);
    }
    
    if (fInterface) {
        //fInterface->run();
        fInterface->installEventFilter(this);
        FLInterfaceManager::_Instance()->registerGUI(fInterface);
    }

    setWindowsOptions();
}

//Delete of QTinterface and of saving graphical interface
void FLWindow::deleteInterfaces()
{
    if (fInterface) {
        FLInterfaceManager::_Instance()->unregisterGUI(fInterface);
        delete fInterface;
        fInterface = NULL;
    }
    
    deleteOscInterface();
    deleteHttpInterface();
    deleteMIDIInterface();
    
    delete fRCInterface;
    fRCInterface = NULL;
}

//------------DEFAULT WINDOW FUNCTIONS

//Does window contain a default Faust process?
bool FLWindow::is_Default()
{
    return fIsDefault;
}

//Artificial content of a default window
void FLWindow::print_initWindow(int typeInit)
{
    QPixmap dropImage;
    
    if (typeInit == kInitBlue) {
        dropImage.load(":/Images/DropYourFaustLife_Blue.png");
    } else if(typeInit == kInitWhite) {
        dropImage.load(":/Images/DropYourFaustLife_White.png");
    }
    
    dropImage.scaledToHeight(10, Qt::SmoothTransformation);
    QLabel *image = new QLabel();
    // image->setMinimumSize (dropImage.width()*3, dropImage.height()*3);
    
    image->installEventFilter(this);
    image->setPixmap(dropImage);
    image->setAlignment(Qt::AlignCenter);
    setCentralWidget(image);
}

//------------------------CLOSING ACTIONS

//Reaction to click an x button
void FLWindow::closeEvent(QCloseEvent* event)
{
    if (QApplication::keyboardModifiers() == Qt::AltModifier) {
        emit shut_AllWindows();
    } else {
        emit closeWin();
    }

	event->accept();
}

//During the execution, when a window is shut, its associate folder has to be removed
void FLWindow::shutWindow()
{
    closeWindow();
    const QString winFolder = fHome + "/Windows/" + fWindowName;
    delete fSettings;
    deleteDirectoryAndContent(winFolder);
}

//Closing the window without removing its property for example when the application quits
void FLWindow::closeWindow()
{
    hide();
    start_stop_watcher(false);
    fSettings->sync();
    
    if (fClientOpen && fAudioManager) {
        fAudioManager->stop();
    }

    deleteInterfaces();

    if (fHttpdWindow) {
        fHttpdWindow->deleteLater();
        fHttpdWindow = NULL;
    }
    
    FLSessionManager::_Instance()->deleteDSPandFactory(fCurrentDSP);

#ifdef REMOTE
    delete fStatusBar;
#endif

    delete fAudioManager;
    delete fToolBar;
    
    blockSignals(true);
}

//------------------------DRAG AND DROP ACTIONS

//Reaction to drop on the window
void FLWindow::dropEvent(QDropEvent* event)
{
    //The widget was hidden from crossing of an object through the window
    this->centralWidget()->show();
/*
	int numberCharToErase = 0;
#ifndef _WIN32
	numberCharToErase = 8;
#else
	numberCharToErase = 7;
#endif
	*/
    if (event->mimeData()->hasUrls()) {
        
        QList<QString> sourceList;
        QList<QUrl> urls = event->mimeData()->urls();
        QList<QUrl>::iterator i;
        
        for (i = urls.begin(); i != urls.end(); i++) {
            
            QString fileName;
                
            if (i->isLocalFile()) {
                fileName = i->toLocalFile();
            } else {
                fileName = i->toString();
            }
                
            if (i == urls.begin()) {
                update_Window(fileName);
            } else {
                sourceList.push_back(fileName);
            }
            
            event->accept();
        }   
        emit drop(sourceList);
    } else if (event->mimeData()->hasText()) {
        event->accept();
        update_Window(event->mimeData()->text());
    }
}

//That way the drag movement is more visible : the central widget is hidden when an object is crossing the window and reset visible when the object leaves the window
void FLWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->text() == fSource) {
        return;
    } else if (event->mimeData()->hasUrls() && *(event->mimeData()->urls().begin()) == fSource) {
        return;
    }
    
    if (event->mimeData()->hasFormat("text/uri-list") || event->mimeData()->hasFormat("text/plain")) {
        
        if (event->mimeData()->hasUrls()) {
            QList<QString> sourceList;
            QList<QUrl> urls = event->mimeData()->urls();
            QList<QUrl>::iterator i;
            
            for (i = urls.begin(); i != urls.end(); i++) {
                QString suffix = QFileInfo(i->toString()).completeSuffix();
                if (suffix == "dsp" || suffix == "wav" || suffix == "aif" || suffix == "aiff" || suffix == "aifc") {
                    centralWidget()->hide();
                    event->acceptProposedAction();   
                }
            }
            
        } else if (event->mimeData()->hasFormat("text/plain")) {
            centralWidget()->hide();
            event->acceptProposedAction();      
        }
    }
}

void FLWindow::dragLeaveEvent(QDragLeaveEvent* /*event*/)
{
    //setWindowFlags();
    centralWidget()->show();
}

//---Reversed DnD
void FLWindow::pressEvent()
{
    QDrag* reverseDrag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;
    reverseDrag->setMimeData(mimeData);
    QPixmap fileIcon;
    fileIcon.load(":/Images/FileIcon.png");
    reverseDrag->setPixmap(fileIcon);
    
    if (QFileInfo(fSource).exists()) {
        QList<QUrl> listUrls;
        QUrl newURL(fSource);
        listUrls.push_back(newURL);
        mimeData->setUrls(listUrls);
    } else {
        mimeData->setText(fSource);
    }
    
    if (reverseDrag->exec(Qt::CopyAction) == Qt::CopyAction){}
}

bool FLWindow::eventFilter(QObject* obj, QEvent* ev)
{
    if (ev->type() == QEvent::MouseMove && QApplication::mouseButtons()==Qt::LeftButton) {
        pressEvent();
        return true;
    } else {
        return QMainWindow::eventFilter(obj, ev);
    }
}

//-------------------------AUDIO FUNCTIONS

//Start/Stop of audio
void FLWindow::stop_Audio()
{
#ifdef REMOTE
    
    //    if (!fEffect->isLocal()) {
    //        remote_dsp* currentDSP = (remote_dsp*) fCurrentDSP;
    //        currentDSP->stop();
    //    }
    
#endif
    if (fClientOpen) {
        fAudioManager->stop();
        fClientOpen = false;
    }
}

void FLWindow::start_Audio()
{
    if (!fClientOpen) {
        recall_Window();
        fAudioManager->start();
        QString connectFile = fHome + "/Windows/" + fWindowName + "/Connections.jc";
        fAudioManager->connect_Audio(connectFile.toStdString());
        fClientOpen = true;
        
    #ifdef REMOTE
        //    if (!fEffect->isLocal()) {
        //        remote_dsp* currentDSP = (remote_dsp*) fCurrentDSP;
        //        currentDSP->start();
        //    }
    #endif
    }
}

//In case audio clients collapse, the architecture has to be changed
void FLWindow::audioShutDown(const char* msg, void* arg)
{
    ((FLWindow*)arg)->audioShutDown_redirect(msg);
}

void FLWindow::audioShutDown_redirect(const char* msg)
{
// Redirect with SIGNAL TO SWITCH THREAD (leave audio thread to go to the window thread) + Copy char* for it might be destructed within audio driver before we have time to print it
    QString errorMsg(msg);
    emit audioError(errorMsg);
}

void FLWindow::audioShutDown(const QString& msg)
{
    AudioCreator* creator = AudioCreator::_Instance(NULL);
    creator->reset_AudioArchitecture();
    errorPrint(msg);
    emit audioPrefChange();
}

//Switch of Audio architecture
bool FLWindow::update_AudioArchitecture(QString& error)
{
    AudioCreator* creator = AudioCreator::_Instance(NULL);
    delete fAudioManager;
    fAudioManager = creator->createAudioManager(FLWindow::audioShutDown, this);
    return (init_audioClient(error) && setDSP(error));
}

//Initialization of audio client reimplemented
bool FLWindow::init_audioClient(QString& error)
{
    int numberInputs = fSettings->value("InputNumber", 0).toInt();
    int numberOutputs = fSettings->value("OutputNumber", 0).toInt();
    
//  if(numberInputs == 0 && numberOutputs == 0)
//      return fAudioManager->initAudio(error, fWindowName.toStdString().c_str());
    
	if (fAudioManager->initAudio(error, fWindowName.toStdString().c_str(), fSettings->value("Name", "").toString().toStdString().c_str(), numberInputs, numberOutputs)) {
        update_AudioParams();
        return true;
    } else {
        return false;
    }
}

void FLWindow::update_AudioParams()
{
    fSettings->setValue("SampleRate", fAudioManager->get_sample_rate());
    fSettings->setValue("BufferSize", fAudioManager->get_buffer_size());
}

bool FLWindow::setDSP(QString& error)
{
    bool success = fAudioManager->setDSP(error, fCurrentDSP, fSettings->value("Name", "").toString().toStdString().c_str());
    update_AudioParams();
    return success;
}

//------------------------SAVING WINDOW ACTIONS

//Read/Write window properties in saving file
void FLWindow::saveWindow()
{
    //Save the parameters of the actual interface
    fSettings->setValue("Position/x", this->geometry().x());
    fSettings->setValue("Position/y", this->geometry().y());
    fSettings->setValue("Size/w", this->geometry().width());
    fSettings->setValue("Size/h", this->geometry().height());
    
    //Graphical parameters//
    QString rcfilename = fHome + "/Windows/" + fWindowName + "/Graphics.rc";
    fRCInterface->saveState(rcfilename.toLatin1().data());
    
    //Audio Connections parameters
    QString connectFile = fHome + "/Windows/" + fWindowName + "/Connections.jc";
    
    fAudioManager->save_Connections(connectFile.toStdString());
    
    //Writing new settings in file (for infos to be synchronized)
    fSettings->sync();
 }

void FLWindow::recall_Window()
{
    //Graphical parameters//
    QString rcfilename = fHome + "/Windows/" + fWindowName + "/Graphics.rc";
    
    if (QFileInfo(rcfilename).exists()) {
        fRCInterface->recallState(rcfilename.toStdString().c_str());
    }
}

//------------------------ACCESSORS

QString FLWindow::get_nameWindow()
{
    return fWindowName;
}

QString FLWindow::getSHA()
{
    return fSettings->value("SHA", "").toString();
}

QString FLWindow::getName()
{
    return fSettings->value("Name", "").toString();
}

QString FLWindow::getPath(){
    return fSettings->value("Path", "").toString();
}

int FLWindow::get_indexWindow()
{
    return fWindowIndex;
}

QString FLWindow::get_source()
{
    return fSource;
}

//------------------------HTTPD

//Calculation of screen position of the HTTP window, depending on its index
int FLWindow::calculate_Coef()
{
    int multiplCoef = fWindowIndex;
    while (multiplCoef > 20) {
        multiplCoef -= 20;
    }
    return multiplCoef;
}

void FLWindow::allocateHttpInterface()
{
    QString windowTitle = fWindowName + ":" + getName();
    int argc = 3;
    char* argv[4];  // NULL terminated argv
	char charport[5];
	int port = 5510 + fWindowIndex;
	sprintf(charport, "%d", port);

    argv[0] = (char*)(windowTitle.toStdString().c_str());
    argv[1] = "-port";
	argv[2] = charport;
    
    argv[argc] = 0; // NULL terminated argv
    
    fHttpInterface = new httpdUI(getName().toStdString().c_str(), fCurrentDSP->getNumInputs(), fCurrentDSP->getNumOutputs(), 3, argv, false);
}

void FLWindow::deleteHttpInterface()
{
    if (fHttpInterface) {
        FLServerHttp::_Instance()->removeHttpInterface(fHttpInterface->getTCPPort());
        delete fHttpInterface;
        fHttpInterface = NULL;
    }
}

void FLWindow::updateHttpInterface()
{
    saveWindow();
    deleteHttpInterface();
    allocateHttpInterface();
    fCurrentDSP->buildUserInterface(fHttpInterface);
    recall_Window();
    fHttpInterface->run();
    FLServerHttp::_Instance()->declareHttpInterface(fHttpInterface->getTCPPort(), getName().toStdString());
    setWindowsOptions();
}

void FLWindow::switchHttp(bool on)
{
    if (on) {
        updateHttpInterface();
    } else {
        deleteHttpInterface();
    }
}

void FLWindow::viewQRCode()
{
    if (!fIsDefault) {
        
        if (!fHttpInterface) {
            fToolBar->switchHttp(true);
        }
        
        if (fHttpdWindow) {
            delete fHttpdWindow;
            fHttpdWindow = NULL;
        }
        
        fHttpdWindow = new HTTPWindow();
        connect(fHttpdWindow, SIGNAL(toPNG()), this, SLOT(exportToPNG()));
        
        if (fHttpdWindow) {
            int dropPort = FLSettings::_Instance()->value("General/Network/HttpDropPort", 7777).toInt();
            QString fullUrl = "http://" + searchLocalIP() + ":" + QString::number(dropPort) + "/" + QString::number(fHttpInterface->getTCPPort());
            fInterface->displayQRCode(fullUrl, fHttpdWindow);
            fHttpdWindow->move(calculate_Coef()*10, 0);
            QString windowTitle = fWindowName + ":" + fSettings->value("Name", "").toString().toStdString().c_str();
            fHttpdWindow->setWindowTitle(windowTitle);
            fHttpdWindow->raise();
            fHttpdWindow->show();
            fHttpdWindow->adjustSize();
        } else {
            errorPrint("Impossible to create QRCode window");
        }
    }
}

void FLWindow::exportToPNG()
{
    QFileDialog* fileDialog = new QFileDialog;
    fileDialog->setConfirmOverwrite(true);
    QString filename;
    filename = fileDialog->getSaveFileName(NULL, "PNG Name", tr(""), tr("(*.png)"));
    QString errorMsg;
    
    if (!fInterface->toPNG(filename, errorMsg)) {
        errorPrint(errorMsg.toStdString().c_str());
    }
}

QString FLWindow::get_HttpUrl() 
{
    return (fHttpInterface) ? ("http://" + searchLocalIP() + ":" + QString::number(fHttpInterface->getTCPPort()) + "/") : "";
}

//Accessor to Http & Osc Port
int FLWindow::get_Port()
{
    // If the interface is not enabled, it's not running on any port
    return (fHttpInterface) ? fHttpInterface->getTCPPort() : 0;
}

//Redirection of a received error
void FLWindow::errorPrint(const QString& msg)
{
    FLErrorWindow::_Instance()->print_Error(msg);
}

#ifdef REMOTE
//------------------REMOTE PROCESSING
//---We have to separate into 2 functions because the action cannot be done in the audio thread that's why remoteDSPCallback has to send a signal, received in the graphical thread.
int FLWindow::remoteDSPCallback(int error_code, void* arg)
{
    FLWindow* errorWin = (FLWindow*) arg;
    errorWin->emit remoteCnxLost(error_code);
    return -1;
}

void FLWindow::RemoteCallback(int error_code)
{
    QDateTime currentTime(QDateTime::currentDateTime());
    if (fLastMigration.secsTo(currentTime) > 3) {
        if (error_code == ERROR_NETJACK_WRITE || error_code == ERROR_NETJACK_READ) {
            errorPrint("Remote Connection Error.\nSwitching back to local processing.");
            fStatusBar->setRemoteSettings("local processing", "127.0.0.1", 7777, "");
            redirectSwitch();
        }
    }
    
    fLastMigration = currentTime;
}

//----------------REMOTE CONTROL
/*This is a never used attempt to implement remote control
 This is to checkout with Network/FLRemoteServer
 */

//void FLWindow::switchRemoteControl(bool){
//        
//    printf("");
//    
//    Server::_Instance()->declareRemoteControl(fSettings->value("SHA", "").toString().toStdString(), getName().toStdString(), this);
//}
//
//bool    FLWindow::createNJdspInstance(const string& name, const string& key, const string& celt, const string& ip, const string& port, const string& mtu, const string& latency){
//    
//	Q_UNUSED(name);
//	Q_UNUSED(key);
//
//    FLSettings::_Instance()->setValue("General/Audio/NetJackMaster/CV", celt.c_str());
//    FLSettings::_Instance()->setValue("General/Audio/NetJackMaster/IP", ip.c_str());
//    FLSettings::_Instance()->setValue("General/Audio/NetJackMaster/Port", port.c_str());
//    FLSettings::_Instance()->setValue("General/Audio/NetJackMaster/MTU", mtu.c_str());
//    FLSettings::_Instance()->setValue("General/Audio/NetJackMaster/Latency", latency.c_str());
//
//	return true;
//}
//
//void FLWindow::stopNJdspAudio(const char* /*errorMsg*/){
//    
//    printf("FLWindow::stopNJdspAudio\n");
//    
//    fAudio->stop();
//    
//    fAudioManager->start();
//    fAudioManagerStopped = false;
//    
//    errorPrint(errorMsg);
//}

//void* FLWindow::startAudioSlave(void* /*arg*/){
//    
//    FLWindow * dspToStart = (FLWindow*) arg;
//    
//    bool success = false;
//    
//    if(Slave_DSP::fLocker.Lock()){
//        
//        dspToStart->fAudio = new NJm_audioManager(NULL, NULL);
//    connect(dspToStart->fAudio, SIGNAL(errorSignal(const char*)), dspToStart, SLOT(stopNJdspAudio(const char*)));
//    
//        if (dspToStart->fAudio->init(dspToStart->getName().toStdString().c_str(), dspToStart->fCurrentDSP)) {
//            if (!dspToStart->fAudio->start())
//                printf("Start slave audio failed\n");
//            else{
//                printf("SLAVE WITH %i INPUTS || %i OUTPUTS\n", dspToStart->fCurrentDSP->getNumInputs(), dspToStart->fCurrentDSP->getNumOutputs());
//                success = true;
//            }
//        }
//        else
//            printf("Init slave audio failed\n");
//        
//        if(!success)
//            deleteSlaveDSPInstance(dspToStart);
        
//        Slave_DSP::fLocker.Unlock();
//    }
//    return NULL;
//}
//
//bool FLWindow::startNJdspAudio(){
//    
//    fAudioManager->stop();
//    fAudioManagerStopped = true;
//    
//    pthread_t myNewThread;
//    
//    if(!pthread_create(&myNewThread, NULL, FLWindow::startAudioSlave, this)){
//        return true;
//    }
//    else 
//        return false;
//	return false;
//}
//
//void FLWindow::cleanInactiveNJdspInstance(){
//    
//    if(fAudio && !fAudio->is_connexion_active() && fAudioManagerStopped){
//        fAudioManager->start();
//        fAudioManagerStopped = false;
//    }
//
//    printf("CLEAN INACTIVE\n");
//}
//
//string FLWindow::json(){
//    JSONUI json(fCurrentDSP->getNumInputs(), fCurrentDSP->getNumOutputs());
//    fCurrentDSP->buildUserInterface(&json);    
//    string answer = json.JSON();
//    
//    return answer;
//}
//
//void FLWindow::switchRelease(bool on){
//    
//    if(on){
//        printf("SWITCH RELEASE\n");
//        if(!FLSessionManager::_Instance()->addWinToServer(fSettings))
//            FLErrorWindow::_Instance()->print_Error("Impossible to Publish Factory");
////        IL FAUT SWITCHER LA CHECKBOX SI ça NA PAS MARCHEE
//    }
//    else{
//       FLSessionManager::_Instance()->deleteWinFromServer(fSettings); 
//    }
//}
#else
int FLWindow::remoteDSPCallback(int error_code, void* arg)
{
	Q_UNUSED(error_code);
	Q_UNUSED(arg);
    return -1;   
}
#endif









