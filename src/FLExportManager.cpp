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

//Init graphical elements of Export Manager Menu
void FLExportManager::init_DialogWindow()
{
    QFormLayout* exportLayout = new QFormLayout;
    
    QString title("<h2>Export Manager</2>");
    
    QLabel* dialogTitle = new QLabel(title);
    dialogTitle->setStyleSheet("*{color : black}");
    dialogTitle->setAlignment(Qt::AlignCenter);
    
    exportLayout->addRow(dialogTitle);
    
    fMenu2Export = new QGroupBox(fDialogWindow);
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
    
    QWidget* intermediateWidget = new QWidget(fDialogWindow);
    QHBoxLayout* intermediateLayout = new QHBoxLayout;
    
    QPushButton* cancel = new QPushButton(tr("Cancel"), intermediateWidget);
    cancel->setDefault(false);
    
    QPushButton* saveButton = new QPushButton(tr("Export"), intermediateWidget);
    saveButton->setDefault(true);
    
    connect(saveButton, SIGNAL(released()), this, SLOT(postExport()));
    connect(cancel, SIGNAL(released()), this, SLOT(setLastState()));
    connect(cancel, SIGNAL(released()), this, SLOT(redirectAbort()));
    
    intermediateLayout->addWidget(cancel);
    intermediateLayout->addWidget(new QLabel(tr("")));
    intermediateLayout->addWidget(saveButton);
    
    intermediateWidget->setLayout(intermediateLayout);
    exportLayout->addRow(intermediateWidget);
    
    fDialogWindow->setLayout(exportLayout);
    fDialogWindow->move((QApplication::desktop()->geometry().size().width() - fDialogWindow->width())/2, (QApplication::desktop()->geometry().size().height()- fDialogWindow->height())/2);
}

void FLExportManager::setLastState(){
    
    fDialogWindow->hide();
    fLastPlatform = fExportPlatform->currentText();
    fLastArchi = fExportArchi->currentText();
    fLastChoice = fExportArchi->currentText();
}

bool FLExportManager::isDialogVisible(){

	if(fDialogWindow->isVisible())
		return true;
	else if(fMessageWindow->isVisible())
		return true;
	else
		return false;
}

//Displaying the progress of remote compilation
void FLExportManager::init_MessageWindow(){
    
    QLabel* title = new QLabel(tr("<h2>Export Manager</h2>"));
    title->setAlignment(Qt::AlignCenter);
    
    fMsgLayout = new QGridLayout;
    
    //    fMsgLayout->setFormAlignment(Qt::AlignLeft);
    
    fCheck1 = new QLabel("");
    fConnectionLabel = new QLabel(tr("Connection to Server"));
    
    fCheck2 = new QLabel("");
    fCompilationLabel = new QLabel(tr("Remote Compilation"));
    
    fCloseB = new QPushButton(tr("Cancel"), fDialogWindow);
    fSaveB = new QPushButton(tr("Save"), fDialogWindow);
    fOkB = new QPushButton(tr("Ok"), fDialogWindow);
    
    fPrgBar = new QProgressDialog;
    fPrgBar->setRange(0,0);
    fPrgBar->setCancelButton(false);
    
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
    fMsgLayout->addWidget(fCloseB, 7, 0, Qt::AlignCenter);
    fMsgLayout->addWidget(fSaveB, 7, 1, Qt::AlignCenter);
    fMsgLayout->addWidget(fOkB, 7, 0, 1, 7, Qt::AlignCenter);
    
    connect(fCloseB, SIGNAL(released()), fMessageWindow, SLOT(hide()));
    connect(fCloseB, SIGNAL(released()), this, SLOT(redirectAbort()));
    connect(fSaveB, SIGNAL(released()), this, SLOT(saveFileOnDisk()));
    connect(fOkB, SIGNAL(released()), fMessageWindow, SLOT(hide()));
    
    fMessageWindow->setLayout(fMsgLayout);
    fMessageWindow->adjustSize();
    fMessageWindow->hide();
    fMessageWindow->move((QApplication::desktop()->geometry().size().width() - fMessageWindow->width())/2, (QApplication::desktop()->geometry().size().height()- fMessageWindow->height())/2);
}

FLExportManager::FLExportManager(QString sessionHome){
    std::cerr << "FLExportManager::FLExportManager(...)" << std::endl;
    
    fHome = sessionHome;
    
    fStep = 0;
    fTargetReply = NULL;
    fPostReply = NULL;
    fGetKeyReply = NULL;
    
    fLastPlatform = "";
    fLastArchi = "";
    fLastChoice = "";
    
    fDialogWindow = new QDialog();
    fDialogWindow->setWindowFlags(Qt::FramelessWindowHint);
    
    fMessageWindow = new QDialog();
    fMessageWindow->setWindowFlags(Qt::FramelessWindowHint);
    
    fTextZone = NULL;
    
    FLSettings* settings = FLSettings::getInstance();
    connect(settings, SIGNAL(urlChanged()), this, SLOT(set_URL()));
    
    fServerUrl = QUrl(settings->value("General/Network/FaustWebUrl", "http://faustservice.grame.fr").toString());
    init_DialogWindow();
    init_MessageWindow();
    
    QDir ImagesDir(":/");
    ImagesDir.cd("Images");
    
    fCheckImg = QPixmap(ImagesDir.absoluteFilePath("Check.png")); 
    fNotCheckImg = QPixmap(ImagesDir.absoluteFilePath("NotCheck.png"));
    
}

FLExportManager::~FLExportManager()
{
    delete fDialogWindow;
    delete fMessageWindow ;
    
//    In case the export is aborted during its execution
    if(fTargetReply)
        abortReply(fTargetReply);
    if(fPostReply)
        abortReply(fPostReply);
    if(fGetKeyReply)
        abortReply(fGetKeyReply);
}

//When Cancel is pressed, the request is aborted
void FLExportManager::abortReply(QNetworkReply* reply){
    
    if(reply) {
        disconnect(reply, 0, 0, 0);
        printf("FLExportManager::disconnected\n");
        
        if(reply->isRunning()) {
            reply->close();
            printf("FLExportManager::aborted\n");
        }
        reply->deleteLater();
    }
}

void FLExportManager::redirectAbort(){
    
    QNetworkReply* response;
    
    if(fStep == 0)
        response = fTargetReply;
    else if(fStep == 1)
        response = fPostReply;
    else if(fStep == 2)
        response = fGetKeyReply;
    else
        return;

    disconnect(response, 0, 0, 0);
}

//Access Point for FaustLive to export a file
void FLExportManager::exportFile(QString file, QString faustCode){
    
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
    
    // prepare plaform menu
    fPlatforms.clear();
    fTargets.clear();
    fExportPlatform->clear();
    fExportArchi->clear();
    
    fFileToExport = file;
    
    fFilenameToSave = QFileInfo(file).baseName() + "_";
    
    fErrorText->setText("Searching for available targets...");
    
    QString targetUrl= fServerUrl.toString();
    targetUrl += "/targets";
    
    //    Reset available targets
    QNetworkRequest request(targetUrl);
    QNetworkAccessManager * manager = new QNetworkAccessManager;
    
    fTargetReply = manager->get(request);
    connect(fTargetReply, SIGNAL(finished()), this, SLOT(targetsDescriptionReceived()));
    
    fDialogWindow->setVisible(true);
}

//Build Graphical lists of OS and Platforms received from the server
void FLExportManager::targetsDescriptionReceived(){
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
            
            printf("fExportPlatform SIZE = %i\n", fExportPlatform->count());
            
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

//Upload the file to the server with a post request
void FLExportManager::postExport(){
    
    fStep++;
    
    setLastState();
    
    fConnectionLabel->show();
    fPrgBar->show();    
    fMessageWindow->adjustSize();
    fMessageWindow->show();
    
    printf("SERVER URL = %s\n", fServerUrl.toString().toStdString().c_str());
    
    QNetworkRequest requete(fServerUrl);
    QNetworkAccessManager *m = new QNetworkAccessManager;
    
    //The boundary to recognize the end of the file. It should be random.
    QByteArray boundary = "87142694621188";
    QByteArray data;
    
    // Open the file to send
    QFile file(fFileToExport);
    if(file.open( QIODevice::ReadOnly)){
        
        data = "--" + boundary + "\r\n";
        data += "Content-Disposition: form-data; name=\"file\"; filename=\"";
        data += QFileInfo(fFileToExport).baseName();
        data += ".dsp\";\r\nContent-Type: text/plain\r\n\r\n";
        data += fCodeToSend;
        data += "\r\n--" + boundary + "--\r\n";
    }
    printf("DATA TO SEND = %s\n", data.data());
    
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
    
    printf("Getting file from SHA1 Key...\n");
    
    QString urlString(fServerUrl.toString());
    urlString += ("/");
    urlString += key;
    urlString += "/";
    
    urlString += fExportPlatform->currentText();
    fFilenameToSave += fExportPlatform->currentText();
    urlString += "/";
    fFilenameToSave += "_";
    urlString += fExportArchi->currentText();
    fFilenameToSave += fExportArchi->currentText();
    urlString += "/";
    fFilenameToSave += "_";
    urlString += fExportChoice->currentText();
    fFilenameToSave += fExportChoice->currentText();
    
    printf("urlRequest = %s\n", urlString.toStdString().c_str());
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
    
    QString defaultFilename = getenv("HOME");
    defaultFilename += "/Desktop/" + fFilenameToSave;
    
    printf("DEFAULT FILENAME = %s\n", defaultFilename.toStdString().c_str());
    
    if(fExportChoice->currentText().compare("src.cpp") == 0){
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
        fMessageWindow->hide();
}

//Dynamic changes of the available architectures depending on platform
void FLExportManager::platformChanged(const QString& index){
    printf("INDEX = %s\n", index.toStdString().c_str());
    
    fExportArchi->hide();
    fExportArchi->clear();
    
    vector<string> architectures = fTargets[index.toStdString()];
    vector<string>::iterator it;
    
    for (it = architectures.begin(); it!=architectures.end(); it++) {
        fExportArchi->addItem((*it).c_str());
    }
    fExportArchi->show();
}

void FLExportManager::showSaveB(){
    
    QNetworkReply* response = (QNetworkReply*)QObject::sender();
    
    if(response->error() == QNetworkReply::NoError){
        
        fDataReceived = response->readAll();
        
        fCheck2->setPixmap(fCheckImg);
        fCheck2->show();
        
        fPrgBar->hide();
        
        QString sucessMsg = fFilenameToSave;
        sucessMsg += " was successfully exported";
        
        fTextZone->setText(sucessMsg);
        fTextZone->show();
        
        fSaveB->show();
        
        response->deleteLater(); //IMPORTANT : on emploie la fonction deleteLater() pour supprimer la réponse du serveur.
        //Si vous ne le faites pas, vous risquez des fuites de mémoire ou autre.
    } 
    
}

void FLExportManager::set_URL(){
    
    bool isVisible = true;
    
    if(fDialogWindow->isVisible())
        isVisible = false;
    
    setLastState();
    
    if(!isVisible)
        fDialogWindow->show();

//    DO SOMETHING ABOUT THIS
    
//    if(url.indexOf("http://") != 0){
//        QString fullUrl = "http://" + url;
//        fServerUrl = QUrl(fullUrl);
//    }
//    else
    fServerUrl = QUrl(FLSettings::getInstance()->value("General/Network/FaustWebUrl", "http://faustservice.grame.fr").toString());
    
    QString targetUrl= fServerUrl.toString();
    targetUrl += "/targets";
    
    //    Reset available targets
    QNetworkRequest request(targetUrl);
    QNetworkAccessManager *manager = new QNetworkAccessManager;
    
    fTargetReply = manager->get(request);
    connect(fTargetReply, SIGNAL(finished()), this, SLOT(targetsDescriptionReceived()));   
}


