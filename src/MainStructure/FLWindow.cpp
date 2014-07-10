//
//  FLWindow.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLWindow.h"

#include "faust/gui/faustqt.h"
#ifndef _WIN32
#include "faust/gui/OSCUI.h"
#include "faust/gui/httpdUI.h"
#endif


list<GUI*>               GUI::fGuiList;

#include "FLToolBar.h"
#include "utilities.h"
#include "FLSettings.h"
#include "FLWinSettings.h"
#include "FLSessionManager.h"

#include "FLServerHttp.h"

#include "FLFileWatcher.h"

#ifdef REMOTE
#include "faust/remote-dsp.h"
#endif

#include "faust/llvm-dsp.h"

/****************************FaustLiveWindow IMPLEMENTATION***************************/

//------------CONSTRUCTION WINDOW
//@param : baseName = Window name
//@param : index = Index of the window
//@param : effect = effect that will be contained in the window
//@param : x,y = position on screen
//@param : home = current Session folder
//@param : osc/httpd port = port on which remote interface will be built 
//@param : machineName = in case of remote processing, the name of remote machine
FLWindow::FLWindow(QString& baseName, int index, const QString& home, FLWinSettings* windowSettings, QList<QMenu*> appMenus){
    
    fSettings = windowSettings;
    
    //  Enable Drag & Drop on window
    setAcceptDrops(true);
    
    //  Creating Window Name
    fWindowIndex = index;
    fWindowName = baseName +  QString::number(fWindowIndex);
    
    fIsDefault = false;
    
    //    Initializing class members
    
#ifndef _WIN32
    fHttpdWindow = NULL;
    fHttpInterface = NULL;
    fOscInterface = NULL;
#endif
    fRCInterface = NULL;
    fCurrent_DSP = NULL;
    
    fToolBar = NULL;
    
    fIPToHostName = new map<QString, std::pair<QString, int> >;
    
    //    Creating Window Folder
    fHome = home;
    
    //    Creating Audio Manager
    AudioCreator* creator = AudioCreator::_Instance(NULL);
    
    fAudioManager = creator->createAudioManager(FLWindow::audioShutDown, this);
    fClientOpen = false;
    
    //    Set Menu & ToolBar
    fLastMigration = QDateTime::currentDateTime();
    setToolBar();
    set_MenuBar(appMenus);
}

FLWindow::~FLWindow(){}

//------------------------WINDOW ACTIONS

//Show Window on front end with standard size
void FLWindow::frontShow(){
    
    setGeometry(fSettings->value("Position/x", 0).toInt(), fSettings->value("Position/y", 0).toInt(), 0, 0);
    adjustSize();
    
    show();
    raise();
    
    setMinimumSize(QSize(0, 0));
    setMaximumSize(QSize(QApplication::desktop()->geometry().size().width(), QApplication::desktop()->geometry().size().height()));
}

//Initialization of User Interface + StartUp of Audio Client
//@param : init = if the window created is a default window.
//@param : error = in case init fails, the error is filled
bool FLWindow::init_Window(int init, const QString& source, QString& errorMsg){
    
    fSource = source;

    FLSessionManager* sessionManager = FLSessionManager::_Instance();
    
    QPair<QString, void*> factorySetts = sessionManager->createFactory(source, fSettings, errorMsg);
    
    if(factorySetts.second == NULL)
        return false;
    
    if(!init_audioClient(errorMsg))
        return false;
    
    fCurrent_DSP = sessionManager->createDSP(factorySetts, fSettings, RemoteDSPCallback, this, errorMsg);
    
    if (fCurrent_DSP == NULL)
        return false;
    
    if(allocateInterfaces(fSettings->value("Name", "").toString())){
       
        buildInterfaces(fCurrent_DSP);
        
        if(init != kNoInit){
            fIsDefault = true;
            print_initWindow(init);
        }
        
        if(setDSP(errorMsg)){
        
            start_Audio();
            frontShow();
                        
            runInterfaces();
            
            fCreationDate = fCreationDate.currentDateTime();
            FLFileWatcher::getInstance()->startWatcher(fSettings->value("Path", "").toString(), this);
            
            return true;
        } 
        else
            deleteInterfaces();
    }
    else
        errorMsg = "Interface could not be allocated";
    
    return false;
}

void FLWindow::pressEvent()
{
    QDrag* reverseDrag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;
    reverseDrag->setMimeData(mimeData);
    
    QPixmap fileIcon;
    fileIcon.load(":/Images/FileIcon.png");
    
    reverseDrag->setPixmap(fileIcon);
    
    if(QApplication::keyboardModifiers() == Qt::AltModifier){
        //        mimeData->setText(pathToContent(fEffect->getSource()));
    }
    else{
        //        QList<QUrl> listUrls;
        //        QUrl newURL(fEffect->getSource());
        //        listUrls.push_back(newURL);
        //        mimeData->setUrls(listUrls);
    }
    
    if (reverseDrag->exec(Qt::CopyAction) == Qt::CopyAction){}
    
}

bool FLWindow::eventFilter( QObject *obj, QEvent *ev ){
    
    if (ev->type() == QEvent::MouseMove && QApplication::mouseButtons()==Qt::LeftButton){
        
        pressEvent();
        return true;
    }
    else
        return QMainWindow::eventFilter(obj, ev);
}

//Modification of the process in the window
//@param : source = source that reemplaces the current one
bool FLWindow::update_Window(const QString& source){

    //    ERREUR à ENVOYER EN SIGNAL A lAPPLI
    
    bool update = true;
    
//Avoiding the flicker when the source is saved
//FIND THE RIGHT CONDITION !!!!
    
//    if(QFileInfo(source).exists()){
//        
//        QDateTime modifiedLast = QFileInfo(source).lastModified();
//        if(fSource != source || fCreationDate < modifiedLast)
//            update = true;
//    }
//    else
//        update = true; 
    
    FLFileWatcher::getInstance()->stopWatcher(fSettings->value("Path", "").toString(), this);
    
    if(update){

        QString savedName = fSettings->value("Name", "").toString();
        
        save_Window();
        hide();
        
        //creating the new DSP instance
        dsp* charging_DSP = NULL;
        //    if(newEffect->isLocal())
        
        QString errorMsg("");
            
        FLSessionManager* sessionManager = FLSessionManager::_Instance();
        
        QPair<QString, void*> factorySetts = sessionManager->createFactory(source, fSettings, errorMsg);
        
        if(factorySetts.second == NULL)
            return false;
        
        charging_DSP = sessionManager->createDSP(factorySetts, fSettings, RemoteDSPCallback, this, errorMsg);
        
        bool isUpdateSucessfull = false;
        
        if(charging_DSP){
//            fToolBar->switchHttp(false);
            
            QString newName =  fSettings->value("Name", "").toString();
            
            if(fAudioManager->init_FadeAudio(errorMsg, newName.toStdString().c_str(), charging_DSP)){
                
                deleteInterfaces();
                
                //Set the new interface & Recall the parameters of the window
                if(allocateInterfaces(newName)){
                   
                    buildInterfaces(charging_DSP);
                    recall_Window();
                    
                    //Start crossfade and wait for its end
                    fAudioManager->start_Fade();
                    
//                    setGeometry(fSettings->value("Position/x", 0).toInt(), fSettings->value("Position/y", 0).toInt(), 0, 0);
                    adjustSize();
                    show();
                    
                    fAudioManager->wait_EndFade();
                    
                    
                    
                    //SWITCH the current DSP as the dropped one
                    
                    dsp* VecInt = fCurrent_DSP;
                    fCurrent_DSP = charging_DSP;
                    charging_DSP = VecInt;
                    
                    FLSessionManager::_Instance()->deleteDSPandFactory(charging_DSP);
                    
                    fSource = source;
                    fIsDefault = false;
                    isUpdateSucessfull = true;
                }
                else{
                    if(allocateInterfaces(savedName)){
                        buildInterfaces(fCurrent_DSP);
                        recall_Window();
                    
                        errorMsg = "Impossible to allocate new interface";
                    }
                }
                
                //Step 12 : Launch User Interface
                runInterfaces();
            }
        }
        
        show();
        FLFileWatcher::getInstance()->startWatcher(fSettings->value("Path", "").toString(), this);
        
        if(!isUpdateSucessfull)
            errorPrint(errorMsg);
        else
            emit windowNameChanged();
    
        return isUpdateSucessfull;
    }
    else
        return false;
}

//------------TOOLBAR RELATED ACTIONS

//Set up of the Window ToolBar
void FLWindow::setToolBar(){
    
    fToolBar = new FLToolBar(fSettings, this);
    
    addToolBar(fToolBar);
    
    connect(fToolBar, SIGNAL(oscPortChanged()), this, SLOT(updateOSCInterface()));
    connect(fToolBar, SIGNAL(httpPortChanged()), this, SLOT(updateHTTPInterface()));
    connect(fToolBar, SIGNAL(compilationOptionsChanged()), this, SLOT(modifiedOptions()));
    connect(fToolBar, SIGNAL(sizeGrowth()), this, SLOT(resizingBig()));
    connect(fToolBar, SIGNAL(sizeReduction()), this, SLOT(resizingSmall()));
    connect(fToolBar, SIGNAL(switchMachine()), this, SLOT(redirectSwitch()));
    connect(fToolBar, SIGNAL(switch_http(bool)), this, SLOT(switchHttp(bool)));
    connect(fToolBar, SIGNAL(switch_osc(bool)), this, SLOT(switchOsc(bool)));
}

//Set the windows options with current values
void FLWindow::setWindowsOptions(){
    
    QString textOptions = fSettings->value("FaustOptions", "").toString();
    if(textOptions.compare(" ") == 0)
        textOptions = "";
    
    fSettings->setValue("FaustOptions", textOptions);
    fToolBar->setOptions(textOptions);
    
    fSettings->setValue("OptValue", fSettings->value("OptValue", 3).toInt());
    fToolBar->setVal(fSettings->value("OptValue", 3).toInt());
    
    int port = 5510;
    
    if(fHttpInterface)
        port = fHttpInterface->getTCPPort();
    
    fSettings->setValue("HttpPort", port);
    fToolBar->setPort(port);
    
    int oscPort = 5510;
    
    if(fOscInterface)
        oscPort = fOscInterface->getUDPPort();
    
    fSettings->setValue("OscPort", oscPort);
    fToolBar->setPortOsc(oscPort);
}

#ifndef _WIN32
void FLWindow::updateOSCInterface(){
    
    save_Window();
    
    allocateOscInterface();
    
    fCurrent_DSP->buildUserInterface(fOscInterface);
    recall_Window();
    fOscInterface->run();
    
    setWindowsOptions();
}
#endif

//Reaction to the modifications of the ToolBar options
void FLWindow::modifiedOptions(){
    update_Window(fSource);
}

//Reaction to the resizing the toolbar
void FLWindow::resizingSmall(){
    
    setMinimumSize(QSize(0,0));
    adjustSize();
}

void FLWindow::resizingBig(){
    
    //    QSize winSize = fToolBar->geometry().size();
    //    winSize += fToolBar->minimumSize();
    //   
    //    
    QSize winMinSize = minimumSize();
    winMinSize += fToolBar->geometry().size();
    
    //    setGeometry(0,0,winSize.width(), winSize.height());
    setMinimumSize(winMinSize);
    //
    adjustSize();
}

//Redirection machine switch
void FLWindow::redirectSwitch(){
    if(!update_Window(fSource))
        fToolBar->remoteFailed();
}

//Accessor to Http & Osc Port
int FLWindow::get_Port(){
    
#ifndef _WIN32
    if(fHttpInterface != NULL)
        return fHttpInterface->getTCPPort();
    else
#endif
        // If the interface is not enabled, it's not running on any port
        return 0;
}

//------------ALLOCATION/DESALLOCATION OF INTERFACES

void FLWindow::disableOSCInterface(){
    fToolBar->switchOsc(false);
}

void FLWindow::switchOsc(bool on){
    
    if(on)
        updateOSCInterface();
    else{
        delete fOscInterface;
        fOscInterface = NULL;
    }
}

void catch_OSCError(void* arg){
    
    FLWindow* win = (FLWindow*)(arg);
    
    win->errorPrint("Too many OSC interfaces opened at the same time! Connection could not start");
    win->disableOSCInterface();
}

//Allocation of Interfaces
void FLWindow::allocateOscInterface(){
    
    if(fOscInterface!=NULL){
        delete fOscInterface;
        fOscInterface = NULL;
    }
    
    if(fOscInterface == NULL){
        
        char* argv[3];
        argv[0] = (char*)(fWindowName.toStdString().c_str());
        argv[1] = "-port";
        
        argv[2] = (char*) (QString::number(fSettings->value("OscPort", 5510).toInt()).toLatin1().data());
        
#ifndef WIN32
        fOscInterface = new OSCUI(argv[0], 3, argv, NULL, &catch_OSCError, this);
#endif
    }
}

//----4 STEP OF THE INTERFACES LIFE

bool FLWindow::allocateInterfaces(const QString& nameEffect){
    
    fRCInterface = new FUI;
    
    if(!fRCInterface)
        return false;
    
    //Window tittle is build with the window Name + effect Name
    QString intermediate = fWindowName + " : " + nameEffect;
    
    fInterface = new QTGUI(this, intermediate.toStdString().c_str());
    
    if(!fInterface){
        delete fRCInterface;
        fRCInterface = NULL;
        return false;
    }
    
#ifndef _WIN32
    if(fToolBar->isOscOn())
        allocateOscInterface();
    
    if(fToolBar->isHttpOn())
        allocateHttpInterface();
#endif
    return true;
}

//Building QT Interface | Osc Interface | Parameter saving Interface | ToolBar
bool FLWindow::buildInterfaces(dsp* dsp){
      
    dsp->buildUserInterface(fInterface);
    dsp->buildUserInterface(fRCInterface);
            
#ifndef _WIN32
    if(fOscInterface)
        dsp->buildUserInterface(fOscInterface);
        
    if(fHttpInterface)
        dsp->buildUserInterface(fHttpInterface);
#endif
}

void FLWindow::runInterfaces(){
    
#ifndef _WIN32  
    if(fOscInterface)
        fOscInterface->run();
    
    if(fHttpInterface){
        fHttpInterface->run();
        FLServerHttp::getInstance()->declareHttpInterface(fHttpInterface->getTCPPort(), getName().toStdString());
    }
#endif
    setWindowsOptions();
    
    fInterface->run();
    fInterface->installEventFilter(this);
}

//Delete of QTinterface and of saving graphical interface
void FLWindow::deleteInterfaces(){
    
#ifndef _WIN32
    if(fOscInterface){
        delete fOscInterface;
        fOscInterface = NULL;
    }
    if(fHttpInterface){
        deleteHttpInterface();
    }
#endif
    
    delete fInterface;
    fInterface = NULL;
    
    delete fRCInterface;
    fRCInterface = NULL;
}

//------------DEFAULT WINDOW FUNCTIONS

//Does window contain a default Faust process?
bool FLWindow::is_Default(){
    return fIsDefault;
}

//Artificial content of a default window
void FLWindow::print_initWindow(int typeInit){
    
    QPixmap dropImage;
    
    if(typeInit == kInitBlue)
        dropImage.load(":/Images/DropYourFaustLife_Blue.png");
    else if(typeInit == kInitWhite)
        dropImage.load(":/Images/DropYourFaustLife_White.png");
    
    dropImage.scaledToHeight(10, Qt::SmoothTransformation);
    
    QLabel *image = new QLabel();
    //    image->setMinimumSize (dropImage.width()*3, dropImage.height()*3);
    
    image->installEventFilter(this);
    
    image->setPixmap(dropImage);
    image->setAlignment(Qt::AlignCenter);
    setCentralWidget(image);
}

//------------------------CLOSING ACTIONS

//Reaction to click an x button
void FLWindow::closeEvent(QCloseEvent* event){
    
    if(QApplication::keyboardModifiers() == Qt::AltModifier)
        emit shut_AllWindows();
    else
        emit closeWin();
}

//During the execution, when a window is shut, its associate folder has to be removed
void FLWindow::shut_Window(){
    
    close_Window();
    const QString winFolder = fHome + "/Windows/" + fWindowName;
    delete fSettings;
    deleteDirectoryAndContent(winFolder);
}

//Closing the window without removing its property for example when the application is quit
void FLWindow::close_Window(){
    
    hide();
    FLFileWatcher::getInstance()->stopWatcher(fSettings->value("Path", "").toString(), this);
    
    fSettings->sync();
    
    if(fClientOpen && fAudioManager)
        fAudioManager->stop();

    deleteInterfaces();

    
#ifndef _WIN32
    if(fHttpdWindow){
        fHttpdWindow->deleteLater();
        fHttpdWindow = NULL;
    }
#endif
    
    FLSessionManager::_Instance()->deleteDSPandFactory(fCurrent_DSP);
  
    if(fAudioManager)
        delete fAudioManager;
       
    if(fToolBar)
        delete fToolBar;
    
    blockSignals(true);

}

//------------------------DRAG AND DROP ACTIONS

//Reaction to drop on the window
void FLWindow::dropEvent ( QDropEvent * event ){
    
    //The widget was hidden from crossing of an object through the window
    this->centralWidget()->show();
    
    QList<QString>    sourceList;    
    
	int numberCharToErase = 0;
#ifndef _WIN32
	numberCharToErase = 8;
#else
	numberCharToErase = 7;
#endif
	
    if (event->mimeData()->hasUrls()) {
        
        QList<QString>    sourceList;
        QList<QUrl> urls = event->mimeData()->urls();
        QList<QUrl>::iterator i;
        
        for (i = urls.begin(); i != urls.end(); i++) {
            
            QString fileName;
                
            if(i->isLocalFile())
                fileName = i->toLocalFile();
            else
                fileName =  i->toString();
                
            if(i == urls.begin())
                update_Window(fileName);
            else
                sourceList.push_back(fileName);
            
            event->accept();
        }   
        emit drop(sourceList);
    }
    else if (event->mimeData()->hasText()){
        
        event->accept();
        update_Window(event->mimeData()->text());
    }
}

//That way the drag movement is more visible : the central widget is hidden when an object is crossing the window and reset visible when the object leaves the window
void FLWindow::dragEnterEvent ( QDragEnterEvent * event ){
    
    if (event->mimeData()->hasFormat("text/uri-list") || event->mimeData()->hasFormat("text/plain")){
        
        if (event->mimeData()->hasUrls()) {
            QList<QString>    sourceList;
            QList<QUrl> urls = event->mimeData()->urls();
            QList<QUrl>::iterator i;
            
            for (i = urls.begin(); i != urls.end(); i++) {
                
                if(QFileInfo(i->toString()).completeSuffix().compare("dsp") == 0 || QFileInfo(i->toString()).completeSuffix().compare("wav") == 0){
                    
                    centralWidget()->hide();
                    event->acceptProposedAction();   
                }
            }
            
        }
        else if(event->mimeData()->hasFormat("text/plain")){
            centralWidget()->hide();
            event->acceptProposedAction();      
        }
    }
}

void FLWindow::dragLeaveEvent ( QDragLeaveEvent * /*event*/ ){
    //    setWindowFlags();
    centralWidget()->show();
}

//-------------------------AUDIO FUNCTIONS

//Start/Stop of audio
void FLWindow::stop_Audio(){
    
#ifdef REMOTE
    
    //    if(!fEffect->isLocal()){
    //        
    //        remote_dsp* currentDSP = (remote_dsp*) fCurrent_DSP;
    //        currentDSP->stopAudio();
    //    }
    
#endif
    
    fAudioManager->stop();
    fClientOpen = false;
}

void FLWindow::start_Audio(){
    
    recall_Window();
    
    fAudioManager->start();
    
    QString connectFile = fHome + "/Windows/" + fWindowName + "/Connections.jc";
    
    printf("Connect Audio = %s\n", connectFile.toStdString().c_str());
    
    fAudioManager->connect_Audio(connectFile.toStdString());
    
    fClientOpen = true;
    
#ifdef REMOTE
    //    if(!fEffect->isLocal()){
    //        
    //        remote_dsp* currentDSP = (remote_dsp*) fCurrent_DSP;
    //        currentDSP->startAudio();
    //    }
#endif
}

//In case audio clients collapse, the architecture has to be changed
void FLWindow::audioShutDown(const char* msg, void* arg){
    
    ((FLWindow*)arg)->audioShutDown(msg);
}

void FLWindow::audioShutDown(const char* msg){
    AudioCreator* creator = AudioCreator::_Instance(NULL);
    
    //    creator->change_Architecture();
    emit errorPrint(msg);
}

//Switch of Audio architecture
bool FLWindow::update_AudioArchitecture(QString& error){
    
    AudioCreator* creator = AudioCreator::_Instance(NULL);
    delete fAudioManager;
    
    fAudioManager = creator->createAudioManager();
    
    if(init_audioClient(error) && setDSP(error))
        return true;
    else
        return false;
}

//Initialization of audio Client Reimplemented
bool FLWindow::init_audioClient(QString& error){
    
    int numberInputs = fSettings->value("InputNumber", 0).toInt();
    int numberOutputs = fSettings->value("OutputNumber", 0).toInt();
    
//    if(numberInputs == 0 && numberOutputs == 0)
//        return fAudioManager->initAudio(error, fWindowName.toStdString().c_str());
    
	if(fAudioManager->initAudio(error, fWindowName.toStdString().c_str(), fSettings->value("Name", "").toString().toStdString().c_str(), numberInputs, numberOutputs)){
     
        fSettings->setValue("SampleRate", fAudioManager->get_sample_rate());
        fSettings->setValue("BufferSize", fAudioManager->get_buffer_size());
        
        return true;
    }
    else
        return false;
    
}

bool FLWindow::setDSP(QString& error){
    return fAudioManager->setDSP(error, fCurrent_DSP, fSettings->value("Name", "").toString().toStdString().c_str());
}

//------------------------SAVING WINDOW ACTIONS

//Read/Write window properties in saving file
void FLWindow::save_Window(){

    setWindowsOptions();
    
    //Save the parameters of the actual interface
    fSettings->setValue("Position/x", this->geometry().x());
    fSettings->setValue("Position/y", this->geometry().y());
    
    //Graphical parameters//
    QString rcfilename = fHome + "/Windows/" + fWindowName + "/Graphics.rc";
    fRCInterface->saveState(rcfilename.toLatin1().data());
    
    //Audio Connections parameters
    QString connectFile = fHome + "/Windows/" + fWindowName + "/Connections.jc";
    
    fAudioManager->save_Connections(connectFile.toStdString());
}

void FLWindow::recall_Window(){
    
    //Graphical parameters//
    QString rcfilename = fHome + "/Windows/" + fWindowName + "/Graphics.rc";
    
    if(QFileInfo(rcfilename).exists())
        fRCInterface->recallState(rcfilename.toStdString().c_str());
}

//------------------------ACCESSORS

QString FLWindow::get_nameWindow(){
    return fWindowName;
}

QString FLWindow::getSHA(){
    return fSettings->value("SHA", "").toString();
}

QString FLWindow::getName(){
    return fSettings->value("Name", "").toString();
}

QString FLWindow::getPath(){
    return fSettings->value("Path", "").toString();
}

int FLWindow::get_indexWindow(){
    return fWindowIndex;
}

QString FLWindow::get_source(){
    return fSource;
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

#ifndef _WIN32
void FLWindow::allocateHttpInterface(){
    
    QString optionPort = "-port";
    QString windowTitle = fWindowName + ":" + getName();
        
    char* argv[3];
        
    argv[0] = (char*)(windowTitle.toLatin1().data());
    argv[1] = (char*)(optionPort.toLatin1().data());
    argv[2] = (char*)(QString::number(fSettings->value("HttpPort", 5510).toInt()).toStdString().c_str());
        
    fHttpInterface = new httpdUI(argv[0], 3, argv);
}

void FLWindow::deleteHttpInterface(){
    
    FLServerHttp::getInstance()->removeHttpInterface(fHttpInterface->getTCPPort());
    delete fHttpInterface;
    fHttpInterface = NULL;
}

void FLWindow::updateHTTPInterface(){
    
    save_Window();
    
    if(fHttpInterface)
        deleteHttpInterface();
    
    allocateHttpInterface();
    
    fCurrent_DSP->buildUserInterface(fHttpInterface);
    recall_Window();
    fHttpInterface->run();
    FLServerHttp::getInstance()->declareHttpInterface(fHttpInterface->getTCPPort(), getName().toStdString());
    
    setWindowsOptions();
}

void FLWindow::switchHttp(bool on){

    if(on)
        updateHTTPInterface();
    else
        deleteHttpInterface();
}

void FLWindow::viewQrCode(){
    
    if(fHttpInterface == NULL){
        fToolBar->switchHttp(true);
    }
    
    if(fHttpdWindow != NULL){
        delete fHttpdWindow;
        fHttpdWindow = NULL;
    }
    
    fHttpdWindow = new HTTPWindow();
    connect(fHttpdWindow, SIGNAL(toPNG()), this, SLOT(exportToPNG()));
    
    if(fHttpdWindow){
        
        QString fullUrl("");
        
        fullUrl = "http://";
        fullUrl += searchLocalIP();
        fullUrl += ":";
        
        fullUrl += QString::number(FLSettings::getInstance()->value("General/Network/HttpDropPort", 7777).toInt());
        fullUrl += "/";
        fullUrl += QString::number(fHttpInterface->getTCPPort());
        
        fInterface->displayQRCode(fullUrl, fHttpdWindow);
        fHttpdWindow->move(calculate_Coef()*10, 0);
        
        QString windowTitle = fWindowName + ":" + fSettings->value("Name", "").toString().toStdString().c_str();
        
        fHttpdWindow->setWindowTitle(windowTitle);
        fHttpdWindow->raise();
        fHttpdWindow->show();
        fHttpdWindow->adjustSize();
    }
    else
        emit error("Enable Http Before Asking for Qr Code");
}

void FLWindow::exportToPNG(){
    
    QFileDialog* fileDialog = new QFileDialog;
    fileDialog->setConfirmOverwrite(true);
    
    QString filename;
    
    filename = fileDialog->getSaveFileName(NULL, "PNG Name", tr(""), tr("(*.png)"));
    
    QString errorMsg;
    
    if(!fInterface->toPNG(filename, errorMsg))
        emit error(errorMsg.toStdString().c_str());
}

QString FLWindow::get_HttpUrl() {

    QString url("");
    
    if(fToolBar->isHttpOn()){

        url = "http://";
        url += searchLocalIP();   
        url += ":";
        url += QString::number(fHttpInterface->getTCPPort());
        url += "/";
    }
    else
        return url;
}
#endif

//------------------------Right Click Reaction

void FLWindow::contextMenuEvent(QContextMenuEvent* ev) {
    
    fWindowMenu->exec(ev->globalPos());
}

void FLWindow::set_MenuBar(QList<QMenu*> appMenus){
    
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
    
    //#if-group:
#ifndef _WIN32
    //#ifdef HTTPCTRL
    QAction* httpdViewAction = new QAction(tr("&View QRcode"),this);
    httpdViewAction->setShortcut(tr("Ctrl+K"));
    httpdViewAction->setToolTip(tr("Print the QRcode of TCP protocol"));
    connect(httpdViewAction, SIGNAL(triggered()), this, SLOT(httpd_View()));
#endif
    
    QAction* svgViewAction = new QAction(tr("&View SVG Diagram"),this);
    svgViewAction->setShortcut(tr("Ctrl+G"));
    svgViewAction->setToolTip(tr("Open the SVG Diagram in a browser"));
    connect(svgViewAction, SIGNAL(triggered()), this, SLOT(svg_View()));
    
    QAction* exportAction = new QAction(tr("&Export As..."), this);
    exportAction->setShortcut(tr("Ctrl+P"));
    exportAction->setToolTip(tr("Export the DSP in whatever architecture you choose"));
    connect(exportAction, SIGNAL(triggered()), this, SLOT(exportManage()));
    
    QAction* shutAction = new QAction(tr("&Close Window"),this);
    shutAction->setShortcut(tr("Ctrl+W"));
    shutAction->setToolTip(tr("Close the current Window"));
    connect(shutAction, SIGNAL(triggered()), this, SLOT(shut()));
    
    fWindowMenu = new QMenu(tr("&Window"), NULL);
    fWindowMenu->addAction(editAction);
    fWindowMenu->addAction(pasteAction);
    fWindowMenu->addAction(duplicateAction);
    fWindowMenu->addSeparator();
#if !defined (_WIN32) /*|| defined HTTPCTRL*/    
    fWindowMenu->addAction(httpdViewAction);
#endif
    fWindowMenu->addAction(svgViewAction);
    fWindowMenu->addSeparator();
    fWindowMenu->addAction(exportAction);
    fWindowMenu->addSeparator();
    fWindowMenu->addAction(shutAction);
    
    myMenuBar->addMenu(fWindowMenu);
    myMenuBar->addSeparator();
    
    while(it != appMenus.end()){
        
        myMenuBar->addMenu(*it);
        myMenuBar->addSeparator();
        
        it++;
    }
}

//------SLOTS FROM MENU ACTIONS THAT ARE REDIRECTED

void FLWindow::edit(){
    
//    ATTENTION IL FAUT PRENDRE EN COMPTE LE RESTE DES CAS ET CREER UN FICHIER TEMP !! 
    
    QString source = fSettings->value("Path", "").toString();
    
    QString pathToOpen = source;
    
    if(source == ""){

        QString tempPath = fHome + "/Windows/" + fWindowName + "/_TEMP_" + fSettings->value("Name", "").toString() + ".dsp";
//        
//        QFile f(tempPath);
//        
//        if(f.open(QFile::WriteOnly)){
//            
//            QTextStream textWriting(&f);
//            
//            textWriting<<fSource;
//            
//            f.close();
//        }
//        f.setPermissions(QFile::ReadOwner);
        
        FLFileWatcher::getInstance()->startWatcher(tempPath, this);
        
        pathToOpen = tempPath;
    }
    
    QString shaFolder = fHome + "/SHAFolder/" + fSettings->value("SHA", "").toString();
    
    touchFolder(shaFolder);
    
    
    QUrl url = QUrl::fromLocalFile(pathToOpen);
    bool b = QDesktopServices::openUrl(url);
    
    QString error = source + " could not be opened!";
    
    if(!b)
        errorPrint(error);

}

void FLWindow::paste(){
    
    //Recuperation of Clipboard Content
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    
    if (mimeData->hasText()) {
        QString clipText = clipboard->text();
        
        update_Window(clipText);
    }
    
}

void FLWindow::duplicate(){
    emit duplicate_Action();
}

#ifndef _WIN32
void FLWindow::httpd_View(){
    
    fToolBar->switchHttp(true);    
    viewQrCode();
}
#endif

void FLWindow::svg_View(){
    
    QString shaValue = fSettings->value("SHA", "").toString();
    
    QString shaFolder = fHome + "/SHAFolder/" + shaValue;
    QString pathToOpen = shaFolder + "/" + shaValue + "-svg/process.svg";
    
    touchFolder(shaFolder);
    
    QUrl url = QUrl::fromLocalFile(pathToOpen);
    bool b = QDesktopServices::openUrl(url);
    
    QString error = pathToOpen + " could not be opened!";
        
//    if(!b)
//        fErrorWindow->print_Error(error);
    
}

void FLWindow::exportManage(){
    emit export_Win();
}

void FLWindow::shut(){
    emit close();
    //    emit closeWin();
}

//Redirection of a received error
void FLWindow::errorPrint(const QString& msg){
    emit error(msg);
}

int FLWindow::RemoteDSPCallback(int error_code, void* arg){
    
#ifdef REMOTE
    QDateTime currentTime(QDateTime::currentDateTime());
    
    FLWindow* errorWin = (FLWindow*) arg;
    
    if(errorWin->fLastMigration.secsTo(currentTime) > 3){
        
        if(error_code == WRITE_ERROR || error_code == READ_ERROR){
            
            errorWin->errorPrint("Remote Connection Error.\n Switching back to local processing.");
            
            errorWin->fToolBar->setRemote("local processing", "", 0);
            errorWin->redirectSwitch();
        }
    }
    
    errorWin->fLastMigration = currentTime;
#endif
    return -1;
}




