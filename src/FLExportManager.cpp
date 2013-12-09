//
//  FLExportManager.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLExportManager.h"

#include <unistd.h>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <ctype.h>
#include "SimpleParser.h"

#define JSON_ONLY

using namespace std;

FLExportManager::FLExportManager(string url, string sessionHome, string file, string filename)
{
    std::cerr << "FLExportManager::FLExportManager(...)" << std::endl;

    fHome = sessionHome;

    fDialogWindow = new QDialog;
    fDialogWindow->setWindowFlags(Qt::FramelessWindowHint);
    
    fMessageWindow = new QDialog();
    fMessageWindow ->setWindowFlags(Qt::FramelessWindowHint);
    
    fServerUrl = QUrl(url.c_str());
    fFileToExport = file;
    fFilenameToExport = filename;
    fFilenameToSave = fFilenameToExport + "_";
}


FLExportManager::~FLExportManager()
{
    delete fDialogWindow;
    delete fMessageWindow ;

}


//Build Graphical lists of OS and Platforms received from the server
void FLExportManager::targetsDescriptionReceived()
{
    QNetworkReply* response = (QNetworkReply*)QObject::sender();
    
    if(response->error() == QNetworkReply::NoError){
        
        QByteArray key = response->readAll();
        const char* p = key.data();
        
        if (parseOperatingSystemsList(p, fPlatforms, fTargets)) {
            
            // prepare plaform menu
            
            fExportPlatform->hide();
            fExportPlatform->clear();
            for (size_t i=0; i<fPlatforms.size();i++) 
                fExportPlatform->addItem(fPlatforms[i].c_str());
            
            fExportPlatform->show();
            
            // prepare architecture menu
            fExportArchi->hide();
            fExportArchi->clear();
            vector<string> archs = fTargets[fPlatforms[0]];
            
            for (size_t i=0; i<archs.size();i++) 
                fExportArchi->addItem(archs[i].c_str());
            
            fExportArchi->show();
        } 
        else {
            
            std::cerr << "Error : targetsDescriptionReceived() received an incorrect JSON " << *p << std::endl;
            
            fMenu2Layout->addRow(new QLabel(""));
            fMenu2Layout->addRow(new QLabel("Web Service is not available."));
            fMenu2Layout->addRow(new QLabel("Verify the web service URL in the preferences."));
        }
    }
    else{
        
        fMenu2Layout->addRow(new QLabel(""));
        fMenu2Layout->addRow(new QLabel("Target Not received."));
    }

}

//Init graphical elements of Export Manager Menu
void FLExportManager::init()
{
    fStep = 0;
    
    //GET LIST OF ARCHITECTURE
    
    QString targetUrl= fServerUrl.toString();
    targetUrl += "/targets";
    
    QNetworkRequest request(targetUrl);
    QNetworkAccessManager *manager = new QNetworkAccessManager;
    
    QNetworkReply *targetReply = manager->get(request);
    connect(targetReply, SIGNAL(finished()), this, SLOT(targetsDescriptionReceived()));
    connect(targetReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(networkError(QNetworkReply::NetworkError)));
    
    std::cerr << "FLExportManager::init()" << std::endl;
    QFormLayout* exportLayout = new QFormLayout;
    
    QString title("<h2>EXPORT MANAGER</2>");
    
    QLabel* dialogTitle = new QLabel(title);
    dialogTitle->setStyleSheet("*{color : black}");
    dialogTitle->setAlignment(Qt::AlignCenter);
    
    exportLayout->addRow(dialogTitle);
    
    fMenu2Export = new QGroupBox(fDialogWindow);
    fMenu2Layout = new QFormLayout;
    
    fExportPlatform = new QComboBox(fMenu2Export);
    connect(fExportPlatform, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(platformChanged(const QString&)));
    
    fExportArchi = new QComboBox(fMenu2Export);    
    
    fExportChoice = new QComboBox(fMenu2Export);
    fExportChoice->addItem("binary.zip");
    fExportChoice->addItem("src.cpp");
    
    fMenu2Layout->addRow(new QLabel("Platform"), fExportPlatform);
    fMenu2Layout->addRow(new QLabel("Architecture"), fExportArchi);
    fMenu2Layout->addRow(new QLabel("source or binary"), fExportChoice);
    
    fMenu2Export->setLayout(fMenu2Layout);
    exportLayout->addRow(fMenu2Export);
    
    QWidget* intermediateWidget = new QWidget(fDialogWindow);
    QHBoxLayout* intermediateLayout = new QHBoxLayout;
    
    QPushButton* cancel = new QPushButton(tr("Cancel"), intermediateWidget);
    cancel->setDefault(false);
    
    QPushButton* saveButton = new QPushButton(tr("Export"), intermediateWidget);
    saveButton->setDefault(true);
    
    connect(saveButton, SIGNAL(released()), this, SLOT(postExport()));
    connect(cancel, SIGNAL(released()), fDialogWindow, SLOT(hide()));
    
    intermediateLayout->addWidget(cancel);
    intermediateLayout->addWidget(new QLabel(tr("")));
    intermediateLayout->addWidget(saveButton);
    
    intermediateWidget->setLayout(intermediateLayout);
    exportLayout->addRow(intermediateWidget);

    fDialogWindow->setLayout(exportLayout);
    
    fDialogWindow->setVisible(true);
}

//Upload the file to the server with a post request
void FLExportManager::postExport(){
    
    fDialogWindow->hide();
    
    display_progress();
    
    printf("SERVER URL = %s\n", fServerUrl.toString().toStdString().c_str());
    
    QNetworkRequest requete(fServerUrl);
    QNetworkAccessManager *m = new QNetworkAccessManager;
    
    //The boundary to recognize the end of the file. It should be random.
    QByteArray boundary = "87142694621188";
    QByteArray data;
    
    // Open the file to send
    QFile file(fFileToExport.c_str());
    if(file.open( QIODevice::ReadOnly)){
        
        data = "--" + boundary + "\r\n";
        data += "Content-Disposition: form-data; name=\"file\"; filename=\"";
        data += fFilenameToExport.c_str();
        data += ".dsp\";\r\nContent-Type: text/plain\r\n\r\n";
        data += file.readAll();
        data += "\r\n--" + boundary + "--\r\n";
    }
//    printf("DATA TO SEND = %s\n", data.data());
    
    requete.setRawHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
    requete.setRawHeader("Content-Length", QString::number(data.size()).toAscii());
    
    QNetworkReply *r = m->post(requete, data);
    
    connect(r, SIGNAL(finished()), this, SLOT(readKey()));
    connect(r, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(networkError(QNetworkReply::NetworkError)));
}

//Receiving the key generated by the server, responding to the post request
void FLExportManager::readKey(){
    
    QNetworkReply* response = (QNetworkReply*)QObject::sender();
    
    if(response->error() == QNetworkReply::NoError){
        
        fStep++;
        
        string ImagesPath = QFileInfo(QFileInfo( QCoreApplication::applicationFilePath()).absolutePath()).absolutePath().toStdString() + "/Resources/Images/";
        string pathCheck = ImagesPath + "Check.png";
        
        QPixmap checkImg(pathCheck.c_str());    
        fCheck1->setPixmap(checkImg);
        
        fMsgLayout->insertRow(3, new QLabel(tr("Remote Compilation")), fCheck2);
        
        QByteArray key = response->readAll();
        
        printf("SHA1 KEY = %s\n", key.data());
        
        getFileFromKey(key.data());
    }
}

//When the Server sends back an error
void FLExportManager::networkError(QNetworkReply::NetworkError /*msg*/){
 
    QNetworkReply* response = (QNetworkReply*)QObject::sender();
    
//    printf("NETWORK ERROR CODE = %i\n", msg);
    
    string ImagesPath = QFileInfo(QFileInfo( QCoreApplication::applicationFilePath()).absolutePath()).absolutePath().toStdString() + "/Resources/Images/";
    
    string pathNotCheck = ImagesPath + "NotCheck.png";
    QPixmap notCheckImg(pathNotCheck.c_str());
    
    if(fStep == 0)
        fCheck1->setPixmap(notCheckImg);
    else
        fCheck2->setPixmap(notCheckImg);
    
    delete fPrgBar;
    
    QTextEdit* textZone = new QTextEdit;
    textZone->setPlainText(response->errorString());
    textZone->setReadOnly(true);
    
    fMsgLayout->insertRow(5, textZone);
    
    fCloseB->show();
}

//Send new request : urlServer/SHA1Key/Platform/Architecture/BinaryOrSource
void FLExportManager::getFileFromKey(const char* key){
    
    printf("Getting file from SHA1 Key...\n");
    
    QString urlString(fServerUrl.toString());
    urlString += ("/");
    urlString += key;
    urlString += "/";
    
    urlString += fExportPlatform->currentText();
    fFilenameToSave += fExportPlatform->currentText().toStdString();
    urlString += "/";
    fFilenameToSave += "_";
    urlString += fExportArchi->currentText();
    fFilenameToSave += fExportArchi->currentText().toStdString();
    urlString += "/";
    fFilenameToSave += "_";
    urlString += fExportChoice->currentText();
    fFilenameToSave += fExportChoice->currentText().toStdString();
    
    printf("urlRequest = %s\n", urlString.toStdString().c_str());
    const QUrl url = QUrl(urlString);
    QNetworkRequest requete(url);
    QNetworkAccessManager *m = new QNetworkAccessManager;
    
    QNetworkReply *r = m->get(requete);
    
    connect(r, SIGNAL(finished()), this, SLOT(showSaveB()));
    connect(r, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(networkError(QNetworkReply::NetworkError)));    

}

//Manage the file received from server
void FLExportManager::saveFileOnDisk(){
    
    QFileDialog* fileDialog = new QFileDialog;
    string toSaveDirectory = fileDialog->getExistingDirectory().toStdString();
    
    string toSaveFile = toSaveDirectory + "/" + fFilenameToSave;
    
//        printf("toSaveFile = %s\n", toSaveFile.c_str());
    
    if(toSaveDirectory != ""){
        
        QFile f(toSaveFile.c_str()); //On ouvre le fichier
        
        if ( f.open(QIODevice::WriteOnly) )
        {
            f.write(fDataReceived); ////On lit la réponse du serveur que l'on met dans un fichier
            f.close(); //On ferme le fichier
        }
    }
    StopProgressSlot();
}

//Dynamic changes of the available architectures depending on platform
void FLExportManager::platformChanged(const QString& index)
{
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

//Displaying the progress of remote compilation
void FLExportManager::display_progress(){
    
    QLabel* title = new QLabel(tr("<h2>Export Manager</h2>"));
    title->setAlignment(Qt::AlignCenter);
    
    fMsgLayout = new QFormLayout;
    fMsgLayout->setFormAlignment(Qt::AlignLeft);
    
    fCheck1 = new QLabel("");
    fCheck2 = new QLabel("");
    
    QWidget* intermediateWidget = new QWidget(fDialogWindow);
    QHBoxLayout* intermediateLayout = new QHBoxLayout;
    
    fCloseB = new QPushButton(tr("Cancel"), intermediateWidget);    
    fSaveB = new QPushButton(tr("Save"), intermediateWidget);
    
    intermediateLayout->addWidget(fCloseB);
    intermediateLayout->addWidget(new QLabel(tr("")));
    intermediateLayout->addWidget(fSaveB);
    
    intermediateWidget->setLayout(intermediateLayout);
    
    fPrgBar = new QProgressDialog;
    fPrgBar->setRange(0,0);
    fPrgBar->setCancelButton(false);
    
    fMsgLayout->addRow(title);
    fMsgLayout->addRow(new QLabel(tr("")));
    fMsgLayout->addRow(new QLabel(tr("Connection to Server")), fCheck1);
    fMsgLayout->addRow(new QLabel(tr("")));
    fMsgLayout->addRow(fPrgBar);
    fMsgLayout->addRow(new QLabel(tr("")));
    fMsgLayout->addRow(intermediateWidget);
    
    connect(fCloseB, SIGNAL(released()), this, SLOT(StopProgressSlot()));
    connect(fSaveB, SIGNAL(released()), this, SLOT(saveFileOnDisk()));
    
    fSaveB->hide();
    fCloseB->hide();
    
    fMessageWindow->setLayout(fMsgLayout);

    fMessageWindow->adjustSize();
    fMessageWindow->show();
    fMessageWindow->raise();
}

void FLExportManager::showSaveB(){
    
    QNetworkReply* response = (QNetworkReply*)QObject::sender();
    
    if(response->error() == QNetworkReply::NoError){
    
        fDataReceived = response->readAll();
    
        string ImagesPath = QFileInfo(QFileInfo( QCoreApplication::applicationFilePath()).absolutePath()).absolutePath().toStdString() + "/Resources/Images/";
        
        string pathCheck = ImagesPath + "Check.png";
        QPixmap checkImg(pathCheck.c_str());
        fCheck2->setPixmap(checkImg);
        fCheck2->show();
        
        delete fPrgBar;
        
        QString sucessMsg = fFilenameToSave.c_str();
        sucessMsg += " was successfully exported";
        
        QTextEdit* textZone = new QTextEdit;
        textZone->setPlainText(sucessMsg);
        textZone->setReadOnly(true);
        
        fMsgLayout->insertRow(5, textZone);
        
        fSaveB->show();
        fCloseB->show();
        
        response->deleteLater(); //IMPORTANT : on emploie la fonction deleteLater() pour supprimer la réponse du serveur.
        //Si vous ne le faites pas, vous risquez des fuites de mémoire ou autre.
    } 
    
}

//Stop displaying the message
void FLExportManager::StopProgressSlot(){
    fMessageWindow->hide();
    
    delete fCheck1;
    delete fCheck2;
    
    delete fCloseB;
    delete fSaveB;
    delete fMsgLayout;
}




