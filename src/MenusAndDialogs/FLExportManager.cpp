//
//  FLExportManager.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLExportManager.h"

#ifndef _WIN32
	#include <unistd.h>
#endif
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <ctype.h>
#include "SimpleParser.h"

#include "utilities.h"
#include "faust/llvm-dsp.h"

#include "FLSettings.h"

#define JSON_ONLY

using namespace std;

FLTargetChooser* FLTargetChooser::_targetChooser = NULL;
FLExportManager* FLExportManager::_exportManager = NULL;

/*****************************FLTarget Chooser**********************************/
FLTargetChooser::FLTargetChooser(QWidget* parent) : QDialog(parent){
    
    fTargetReply = NULL;
    
    fLastPlatform = "";
    fLastArchi = "";
    fLastChoice = "";
    
    setWindowFlags(Qt::FramelessWindowHint);
    
    init();
}

FLTargetChooser::~FLTargetChooser(){
    
    abortReply();
    
    setLastState();
}

FLTargetChooser* FLTargetChooser::_Instance(){
    
    if(FLTargetChooser::_targetChooser  == NULL)
        FLTargetChooser::_targetChooser = new FLTargetChooser;
    
    _targetChooser->sendTargetRequest();
    
    return FLTargetChooser::_targetChooser;
}

//User Demande for the platforms
void FLTargetChooser::sendTargetRequest(){

//    prepare plaform menu
    fPlatforms.clear();
    fTargets.clear();
    fExportPlatform->clear();
    fExportArchi->clear();
    fErrorText->setText("Searching for available targets...");
    
    QString targetUrl= FLSettings::_Instance()->value("General/Network/FaustWebUrl", "http://faustservice.grame.fr").toString();
    targetUrl += "/targets";
//    Reset available targets
    
    QNetworkRequest request(targetUrl);
    QNetworkAccessManager * manager = new QNetworkAccessManager;
    
    fTargetReply = manager->get(request);
    connect(fTargetReply, SIGNAL(finished()), this, SLOT(targetsDescriptionReceived()));
}

//Init graphical elements of Export Manager Menu
void FLTargetChooser::init(){
    QFormLayout* exportLayout = new QFormLayout;
    
    QString title("<h2>Export Manager</2>");
    
    QLabel* dialogTitle = new QLabel(title);
    dialogTitle->setAlignment(Qt::AlignCenter);
    
    exportLayout->addRow(dialogTitle);
    
    fMenu2Export = new QGroupBox(this);
    fMenu2Layout = new QGridLayout;
    
    fExportPlatform = new QComboBox(fMenu2Export);
    connect(fExportPlatform, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(platformChanged(const QString&)));
    
    fExportArchi = new QComboBox(fMenu2Export);    
    
    fExportChoice = new QComboBox(fMenu2Export);
    fExportChoice->addItem("binary.zip");
    fExportChoice->addItem("src.cpp");
    
    fMenu2Layout->addWidget(new QLabel("Platform"), 0, 0);
    fMenu2Layout->addWidget(fExportPlatform, 0, 1);
    fMenu2Layout->addWidget(new QLabel("Architecture"), 1, 0);
    fMenu2Layout->addWidget(fExportArchi, 1, 1);
    fMenu2Layout->addWidget(new QLabel("source or binary"), 2, 0);
    fMenu2Layout->addWidget(fExportChoice, 2, 1);
    
    fErrorText = new QLabel("");
    fMenu2Layout->addWidget(fErrorText, 3, 0, 1, 3);
    
    fMenu2Export->setLayout(fMenu2Layout);
    exportLayout->addRow(fMenu2Export);
    
    QWidget* intermediateWidget = new QWidget(this);
    QHBoxLayout* intermediateLayout = new QHBoxLayout;
    
    QPushButton* cancel = new QPushButton(tr("Cancel"), intermediateWidget);
    cancel->setDefault(false);
    
    QPushButton* saveButton = new QPushButton(tr("Export"), intermediateWidget);
    saveButton->setDefault(true);
    
    connect(saveButton, SIGNAL(released()), this, SLOT(acceptDialog()));
    connect(cancel, SIGNAL(released()), this, SLOT(cancelDialog()));
    connect(cancel, SIGNAL(released()), this, SLOT(abortReply()));
    
    intermediateLayout->addWidget(cancel);
    intermediateLayout->addWidget(new QLabel(tr("")));
    intermediateLayout->addWidget(saveButton);
    
    intermediateWidget->setLayout(intermediateLayout);
    exportLayout->addRow(intermediateWidget);
    
    setLayout(exportLayout);
	centerOnPrimaryScreen(this);
}

//Keeping in memory the graphical state of the dialog
void FLTargetChooser::setLastState(){
    
    fLastPlatform = fExportPlatform->currentText();
    fLastArchi = fExportArchi->currentText();
    fLastChoice = fExportArchi->currentText();
}

void FLTargetChooser::cancelDialog(){
    setLastState();
    reject();
}

void FLTargetChooser::acceptDialog(){
    setLastState();
    accept();
}

//Build Graphical lists of OS and Platforms received from the server
void FLTargetChooser::targetsDescriptionReceived(){
    
    fErrorText->setText("");
    
    QNetworkReply* response = (QNetworkReply*)QObject::sender();
    
    if(response->error() == QNetworkReply::NoError){
        
        // prepare plaform menu
        fPlatforms.clear();
        fTargets.clear();
        fExportPlatform->clear();
        fExportArchi->clear();
        
        QByteArray key = response->readAll();
        const char* p = key.data();
        
        if (parseOperatingSystemsList(p, fPlatforms, fTargets)) {
            
            for (size_t i=0; i<fPlatforms.size();i++) 
                fExportPlatform->addItem(fPlatforms[i].c_str());
            
            fExportPlatform->show();
            
            // prepare architecture menu
            vector<string> archs = fTargets[fPlatforms[0]];
            
            for (size_t i=0; i<archs.size();i++) 
                fExportArchi->addItem(archs[i].c_str());
            
            fExportArchi->show();
            
            //    Recall last target choices
            int index = fExportPlatform->findText(fLastPlatform);
            if(index != -1)
                fExportPlatform->setCurrentIndex(index);
            
            index = fExportArchi->findText(fLastArchi);
            if(index != -1)
                fExportArchi->setCurrentIndex(index);
            
            index = fExportChoice->findText(fLastChoice);
            if(index != -1)
                fExportChoice->setCurrentIndex(index);
        } 
        else {
            
            fErrorText->setText("Targets Could not be parsed.");
        }
    }
    else{
        fErrorText->setText("Web Service is not available.\nVerify the web service URL in the preferences.");
    }
    
    fTargetReply = NULL;
}

//Dynamic changes of the available architectures depending on platform
void FLTargetChooser::platformChanged(const QString& index){
    
    fExportArchi->hide();
    fExportArchi->clear();
    
    vector<string> architectures = fTargets[index.toStdString()];
    vector<string>::iterator it;
    
    for (it = architectures.begin(); it!=architectures.end(); it++) {
        fExportArchi->addItem((*it).c_str());
    }
    fExportArchi->show();
}

//When Cancel is pressed, the request is aborted
void FLTargetChooser::abortReply(){
    
    if(fTargetReply) {
        disconnect(fTargetReply, 0, 0, 0);
        
        if(fTargetReply->isRunning())
            fTargetReply->close();

        fTargetReply->deleteLater();
    }
}

QString FLTargetChooser::platform(){
    return fExportPlatform->currentText();
}

QString FLTargetChooser::architecture(){
    return fExportArchi->currentText();
}

QString FLTargetChooser::binOrSource(){
    return fExportChoice->currentText();
}

/*****************************FLExport Manager**********************************/

FLExportManager::FLExportManager(){
    
    fStep = 0;

    fPostReply = NULL;
    fGetKeyReply = NULL;
    
    setWindowFlags(Qt::FramelessWindowHint);
    
    fTextZone = NULL;

    init();
    
    QDir ImagesDir(":/");
    ImagesDir.cd("Images");
    
    fCheckImg = QPixmap(ImagesDir.absoluteFilePath("Check.png")); 
    fNotCheckImg = QPixmap(ImagesDir.absoluteFilePath("NotCheck.png"));
    
}

FLExportManager::~FLExportManager(){
    
//    In case the export is aborted during its execution
    if(fPostReply)
        abortReply(fPostReply);
    if(fGetKeyReply)
        abortReply(fGetKeyReply);
}

FLExportManager* FLExportManager::_Instance(){
    if(_exportManager  == NULL)
        _exportManager = new FLExportManager;
    
    return _exportManager;
}

//Displaying the progress of remote compilation
void FLExportManager::init(){
    
    QLabel* title = new QLabel(tr("<h2>Export Manager</h2>"));
    title->setAlignment(Qt::AlignCenter);
    
    fMsgLayout = new QGridLayout;
    
    fCheck1 = new QLabel("");
    fConnectionLabel = new QLabel(tr("Connection to Server"));
    
    fCheck2 = new QLabel("");
    fCompilationLabel = new QLabel(tr("Remote Compilation"));
    
    QPushButton* closeB = new QPushButton(tr("Cancel"), this);
    fSaveB = new QPushButton(tr("Save"), this);
    fOkB = new QPushButton(tr("Ok"), this);
    
    fPrgBar = new QProgressDialog;
    fPrgBar->setRange(0,0);
    fPrgBar->setCancelButton(NULL);
    
    fTextZone = new QTextEdit;
    fTextZone->setReadOnly(true);
    
    fMsgLayout->addWidget(title, 0, 0, 1, 0, Qt::AlignCenter);
    fMsgLayout->addWidget(new QLabel(tr("")), 1, 0, 1, 1, Qt::AlignCenter);
    fMsgLayout->addWidget(fConnectionLabel, 2, 0, Qt::AlignCenter); 
    fMsgLayout->addWidget(fCheck1, 2, 1, Qt::AlignCenter);
    fMsgLayout->addWidget(fCompilationLabel, 3, 0, Qt::AlignCenter);
    fMsgLayout->addWidget(fCheck2, 3, 1, Qt::AlignCenter);
    fMsgLayout->addWidget(new QLabel(tr("")), 4, 0, 1, 4, Qt::AlignCenter);
    fMsgLayout->addWidget(fPrgBar, 5, 0, 1, 5, Qt::AlignCenter);
    fMsgLayout->addWidget(fTextZone, 5, 0, 1, 5, Qt::AlignCenter);
    fMsgLayout->addWidget(new QLabel(tr("")), 6, 0, 1, 6, Qt::AlignCenter);
    fMsgLayout->addWidget(closeB, 7, 0, Qt::AlignCenter);
    fMsgLayout->addWidget(fSaveB, 7, 1, Qt::AlignCenter);
    fMsgLayout->addWidget(fOkB, 7, 0, 1, 7, Qt::AlignCenter);
    
    connect(closeB, SIGNAL(released()), this, SLOT(hide()));
    connect(closeB, SIGNAL(released()), this, SLOT(redirectAbort()));
    connect(fSaveB, SIGNAL(released()), this, SLOT(saveFileOnDisk()));
    connect(fOkB, SIGNAL(released()), this, SLOT(hide()));
    
    setLayout(fMsgLayout);
    adjustSize();
    hide();
    centerOnPrimaryScreen(this);
}

//When Cancel is pressed, the request is aborted
void FLExportManager::abortReply(QNetworkReply* reply){
    
    if(reply) {
        disconnect(reply, 0, 0, 0);
        
        if(reply->isRunning())
            reply->close();
            
        reply->deleteLater();
    }
}

void FLExportManager::redirectAbort(){
    
    QNetworkReply* response;
    
    if(fStep == 1)
        response = fPostReply;
    else if(fStep == 2)
        response = fGetKeyReply;
    else
        return;

    disconnect(response, 0, 0, 0);
}

//Access Point for FaustLive to export a file
void FLExportManager::exportFile(const QString& name, const QString& faustCode, const QString& p, const QString& a, const QString& sb){
    
    fPlatform = p;
    fArchi = a;
    fChoice = sb;
    
    fAppName = name;
    fCodeToSend = faustCode;
    
//    Reset message dialog graphical elements
    fStep = 0;
    fTextZone->clear();
    fSaveB->hide();
    fOkB->hide();
    fCheck1->hide();
    fConnectionLabel->hide();
    fCheck2->hide();
    fCompilationLabel->hide();
    fTextZone->hide();
    
    setVisible(true);
    
    postExport();
}

//Upload the file to the server with a post request
void FLExportManager::postExport(){
    
    fStep++;
    
    fConnectionLabel->show();
    fPrgBar->show();    
    adjustSize();
    show();
    
    QString destinedUrl = FLSettings::_Instance()->value("General/Network/FaustWebUrl", "http://faustservice.grame.fr").toString();
    
    QNetworkRequest requete(destinedUrl);
    QNetworkAccessManager *m = new QNetworkAccessManager;
    
    //The boundary to recognize the end of the file. It should be random.
    QByteArray boundary = "87142694621188";
    QByteArray data;
    
    // Open the file to send
    data = "--" + boundary + "\r\n";
    data += "Content-Disposition: form-data; name=\"file\"; filename=\"";
    data += fAppName;
    data += ".dsp\";\r\nContent-Type: text/plain\r\n\r\n";
    data += fCodeToSend;
    data += "\r\n--" + boundary + "--\r\n";
    
    requete.setRawHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
    requete.setRawHeader("Content-Length", QString::number(data.size()).toLatin1());
    
    fPostReply = m->post(requete, data);
    
    connect(fPostReply, SIGNAL(finished()), this, SLOT(readKey()));
    connect(fPostReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(networkError(QNetworkReply::NetworkError)));
}

//Receiving the key generated by the server, responding to the post request
void FLExportManager::readKey(){
    
    QNetworkReply* response = (QNetworkReply*)QObject::sender();
    
    if(response->error() == QNetworkReply::NoError){
        
        fStep++;
    
        fCheck1->setPixmap(fCheckImg);
        fCheck1->show();
        
        fCompilationLabel->show();
        
        QByteArray key = response->readAll();
        
        getFileFromKey(key.data());
    }

    fPostReply = NULL;
}

//When the Server sends back an error
void FLExportManager::networkError(QNetworkReply::NetworkError /*msg*/){
    
    QNetworkReply* response = (QNetworkReply*)QObject::sender();
    
    if(fStep == 1){
        fCheck1->setPixmap(fNotCheckImg);
        fCheck1->show();
    }
    else{
        fCheck2->setPixmap(fNotCheckImg);
        fCheck2->show();
    }
    fPrgBar->hide();
    
    fTextZone->setText(response->errorString());
    fTextZone->show();
}

//Send new request : urlServer/SHA1Key/Platform/Architecture/BinaryOrSource
void FLExportManager::getFileFromKey(const char* key){
    
    fStep++;
    
    QString urlString = FLSettings::_Instance()->value("General/Network/FaustWebUrl", "http://faustservice.grame.fr").toString();
    urlString += ("/");
    urlString += key;
    urlString += "/";
    
    urlString += fPlatform;
    urlString += "/";
    urlString += fArchi;
    urlString += "/";
    urlString += fChoice;
    
    const QUrl url = QUrl(urlString);
    QNetworkRequest requete(url);
    QNetworkAccessManager *m = new QNetworkAccessManager;
    
    QNetworkReply * fGetKeyReply = m->get(requete);
    
    connect(fGetKeyReply, SIGNAL(finished()), this, SLOT(showSaveB()));
    connect(fGetKeyReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(networkError(QNetworkReply::NetworkError)));    
}

//Manage the file received from server
void FLExportManager::saveFileOnDisk(){
    
    QFileDialog* fileDialog = new QFileDialog;
    fileDialog->setConfirmOverwrite(true);
    
    QString filenameToSave;
    
    //     nom par défaut dans le dialogue
    
    QString fullName = fAppName + "_" + fPlatform + "_" + fArchi;
    
    QString defaultFilename = getenv("HOME");
    defaultFilename += "/*/" + fullName;
    
    if(fChoice.compare("src.cpp") == 0){
        defaultFilename += ".cpp";
        
        filenameToSave = fileDialog->getSaveFileName(NULL, "Save File", defaultFilename, tr("(*.cpp)"));
    }
    else{
        
        //s defaultFilename += ".zip";
        
        filenameToSave = fileDialog->getSaveFileName(NULL, "Save File", defaultFilename, tr("(*.zip)"));
    }
    
    if(filenameToSave != ""){
        
        QFile f(filenameToSave); //On ouvre le fichier
        
        if ( f.open(QIODevice::WriteOnly) )
        {
            f.write(fDataReceived); ////On lit la réponse du serveur que l'on met dans un fichier
            f.close(); //On ferme le fichier
        }
    }
        hide();
}

void FLExportManager::showSaveB(){
    
    QNetworkReply* response = (QNetworkReply*)QObject::sender();
    
    if(response->error() == QNetworkReply::NoError){
        
        fDataReceived = response->readAll();
        
        fCheck2->setPixmap(fCheckImg);
        fCheck2->show();
        
        fPrgBar->hide();
        
        QString sucessMsg = fAppName + "_" + fPlatform + "_" + fArchi;
        sucessMsg += " was successfully exported";
        
        fTextZone->setText(sucessMsg);
        fTextZone->show();
        
        fSaveB->show();
        
        response->deleteLater(); //IMPORTANT : on emploie la fonction deleteLater() pour supprimer la réponse du serveur.
        //Si vous ne le faites pas, vous risquez des fuites de mémoire ou autre.
    } 
    
}





