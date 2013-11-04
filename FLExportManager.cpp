//
//  FLExportManager.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLExportManager.h"

#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <ctype.h>
#include "SimpleParser.h"

#define JSON_ONLY

using namespace std;

#define kTmpJson "/targets.json"

FLExportManager::FLExportManager(string url, string sessionHome, string file, string filename)
{
    std::cerr << "FLExportManager::FLExportManager(...)" << std::endl;

    fHome = sessionHome;

    fDialogWindow = new QDialog;
    
    fServerUrl = QUrl(url.c_str());
    fFileToExport = file;
    fFilenameToExport = filename;
    fFilenameToSave = fFilenameToExport + "_";
    
    fDialogWindow->setWindowFlags(Qt::FramelessWindowHint);
    
    
    //GET LIST OF ARCHITECTURE
    
    QString targetUrl= fServerUrl.toString();
    targetUrl += "/targets";
    
    QNetworkRequest request(targetUrl);
    QNetworkAccessManager *manager = new QNetworkAccessManager;
    
    QNetworkReply *targetReply = manager->get(request);
    connect(targetReply, SIGNAL(finished()), this, SLOT(targetsDescriptionReceived()));
}


FLExportManager::~FLExportManager()
{
    delete fDialogWindow;
}



void FLExportManager::targetsDescriptionReceived()
{
    QNetworkReply* response = (QNetworkReply*)QObject::sender();
    QByteArray key = response->readAll();
    const char* p = key.data();

    if (parseOperatingSystemsList(p, fPlatforms, fTargets)) {

        // prepare plaform menu
        fExportPlatform->hide();
        fExportPlatform->clear();
        for (size_t i=0; i<fPlatforms.size();i++) fExportPlatform->addItem(fPlatforms[i].c_str());
        fExportPlatform->show();

        // prepare architecture menu
        fExportArchi->hide();
        fExportArchi->clear();
        vector<string> archs = fTargets[fPlatforms[0]];
        for (size_t i=0; i<archs.size();i++) fExportArchi->addItem(archs[i].c_str());
        fExportArchi->show();

    } else {

        std::cerr << "Error : targetsDescriptionReceived() received an incorrect JSON " << *p << std::endl;

    }

}


void FLExportManager::init()
{
    std::cerr << "FLExportManager::init()" << std::endl;
    QFormLayout* exportLayout = new QFormLayout;
    
    QString title("<h2>DOWNLOAD</2>");
    
    QLabel* dialogTitle = new QLabel(title);
    dialogTitle->setStyleSheet("*{color : black}");
    dialogTitle->setAlignment(Qt::AlignCenter);
    
    exportLayout->addRow(dialogTitle);
    exportLayout->addRow(new QLabel(""));
//    QFormLayout* layoutExportServer = new QFormLayout;
    
//    fServIPLine = new QLineEdit(fDialogWindow);
//    fServIPLine->setText(fServerUrl.toString());
    
//    exportLayout->addRow(new QLabel(tr("Exportation server ")), fServIPLine);
    exportLayout->addRow(new QLabel(tr("")));
    
    QString groupTitle1("Export ");
    groupTitle1 += fFilenameToExport.c_str();
    groupTitle1 += " as ...";
    
    QString groupTitle2("Build ");
    groupTitle2 += fFilenameToExport.c_str();
    groupTitle2 += " in ...";
    
    fMenu1Export = new QGroupBox(groupTitle1, fDialogWindow);
    fMenu1Export->setCheckable(true);
    connect(fMenu1Export, SIGNAL(toggled(bool)), this, SLOT(exportChecked(bool)));
    
    fMenu2Export = new QGroupBox(groupTitle2, fDialogWindow);
    fMenu2Export->setCheckable(true);
    connect(fMenu2Export, SIGNAL(toggled(bool)), this, SLOT(exportChecked(bool)));
    
    QFormLayout* menu1Layout = new QFormLayout;
    QFormLayout* menu2Layout = new QFormLayout;
    
    fExportFormat = new QComboBox(fMenu1Export);
    fExportFormat->addItem("src.cpp");
    fExportFormat->addItem("svg.zip");
    fExportFormat->addItem("mdoc.zip");
    
    menu1Layout->addRow(new QLabel("Format"), fExportFormat);
    fMenu1Export->setLayout(menu1Layout);
    exportLayout->addRow(fMenu1Export);
    
    exportLayout->addRow(new QLabel(""));
    exportLayout->addRow(new QLabel(""));
    
    fExportPlatform = new QComboBox(fMenu2Export);
    connect(fExportPlatform, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(platformChanged(const QString&)));
    
    fExportArchi = new QComboBox(fMenu2Export);    
    
    fExportChoice = new QComboBox(fMenu2Export);
    fExportChoice->addItem("binary.zip");
    fExportChoice->addItem("src.cpp");
    
    menu2Layout->addRow(new QLabel("Platform"), fExportPlatform);
    menu2Layout->addRow(new QLabel("Architecture"), fExportArchi);
    menu2Layout->addRow(new QLabel("source or binary"), fExportChoice);
    
    fMenu2Export->setLayout(menu2Layout);
    exportLayout->addRow(fMenu2Export);
    
    QWidget* intermediateWidget = new QWidget(fDialogWindow);
    QHBoxLayout* intermediateLayout = new QHBoxLayout;
    
    QPushButton* cancel = new QPushButton(tr("Cancel"), intermediateWidget);
    cancel->setDefault(false);;
    
    fSaveButton = new QPushButton(tr("Export"), intermediateWidget);
    fSaveButton->setDefault(true);
    
    connect(fSaveButton, SIGNAL(released()), this, SLOT(postExport()));
    connect(cancel, SIGNAL(released()), this, SLOT(cancelDialog()));
    
    intermediateLayout->addWidget(cancel);
    intermediateLayout->addWidget(new QLabel(tr("")));
    intermediateLayout->addWidget(fSaveButton);
    
    intermediateWidget->setLayout(intermediateLayout);
    exportLayout->addRow(intermediateWidget);
    
    fDialogWindow->setLayout(exportLayout);
    
    fMenu1Export->setChecked(true);
    fMenu2Export->setChecked(false);
    
    fDialogWindow->setVisible(true);
}
        
void FLExportManager::postExport(){
    
    fDialogWindow->hide();
//    fServerUrl = QUrl(fServIPLine->text());
    
    emit start_progressing("Connecting with the server...");
    
    printf("SERVER URL = %s\n", fServerUrl.toString().toStdString().c_str());
    
    QNetworkRequest requete(fServerUrl);
    QNetworkAccessManager *m = new QNetworkAccessManager;
    
    //The boundary to recognize the end of the file. It should be random.
    QByteArray boundary = "87142694621188";
    QByteArray data;
    
    // On ouvre notre fichier à envoyer
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

void FLExportManager::cancelDialog(){
    
//    fServIPLine->setText(fServerUrl.toString()); 
    fDialogWindow->hide();
}

void FLExportManager::exportChecked(bool on){
    
    if(on){
        if(fSaveButton)
            fSaveButton->setEnabled(true);
        
        QGroupBox* group = (QGroupBox*)QObject::sender();
        
        if(group == fMenu1Export){
            if(fMenu2Export->isChecked()){
                fMenu2Export->setChecked(false);
            }
        }
        else if(group == fMenu2Export){
            if(fMenu1Export->isChecked()){
                fMenu1Export->setChecked(false);
            }
        }
    }
    else if(!fMenu2Export->isChecked() && !fMenu1Export->isChecked())
        fSaveButton->setEnabled(false);
}

void FLExportManager::readKey(){
    
    QNetworkReply* response = (QNetworkReply*)QObject::sender();
    
    if(response->error() == QNetworkReply::NoError){
        
        stop_progressing();
        QByteArray key = response->readAll();
        
        printf("SHA1 KEY = %s\n", key.data());
        
        getFileFromKey(key.data());
    }
}

void FLExportManager::networkError(QNetworkReply::NetworkError msg){
 
    QNetworkReply* response = (QNetworkReply*)QObject::sender();
    printf("NETWORK ERROR CODE = %i\n", msg);
    
    emit stop_progressing();
    emit error(response->errorString().toStdString().c_str());
    emit processEnded();
}

void FLExportManager::getFileFromKey(const char* key){
    
    printf("Getting file from SHA1 Key...\n");
    
    emit start_progressing("Saving your file...");
    
    QString urlString(fServerUrl.toString());
    urlString += ("/");
    urlString += key;
    urlString += "/";
    
    if(fMenu1Export->isChecked()){
        
//        printf("MENU1 check\n");
        urlString += fExportFormat->currentText();
        fFilenameToSave += fExportFormat->currentText().toStdString();
//        printf("fFilenameToSave = %s\n", fFilenameToSave.c_str());
    }
    else if(fMenu2Export->isChecked()){
    
//        printf("MENU2 check\n");
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
        
    }
    
    printf("urlRequest = %s\n", urlString.toStdString().c_str());
    const QUrl url = QUrl(urlString);
    QNetworkRequest requete(url);
    QNetworkAccessManager *m = new QNetworkAccessManager;
    
    QNetworkReply *r = m->get(requete);
    
    connect(r, SIGNAL(finished()), this, SLOT(endProcess()));
    connect(r, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(networkError(QNetworkReply::NetworkError)));    

}

void FLExportManager::endProcess(){
    
    QNetworkReply* response = (QNetworkReply*)QObject::sender();
    
    if(response->error() == QNetworkReply::NoError){
        
        stop_progressing();
        
        QFileDialog* fileDialog = new QFileDialog;
        string toSaveDirectory = fileDialog->getExistingDirectory().toStdString();
        
        
        string toSaveFile = toSaveDirectory + "/" + fFilenameToSave;
        
//        printf("toSaveFile = %s\n", toSaveFile.c_str());
        
        if(toSaveFile != ""){
            
            QFile f(toSaveFile.c_str()); //On ouvre le fichier
            
            if ( f.open(QIODevice::WriteOnly) )
            {
                f.write(response->readAll()); ////On lit la réponse du serveur que l'on met dans un fichier
                f.close(); //On ferme le fichier
                response->deleteLater(); //IMPORTANT : on emploie la fonction deleteLater() pour supprimer la réponse du serveur.
                //Si vous ne le faites pas, vous risquez des fuites de mémoire ou autre.
            }
        }
        
        string toPrint = "\n" + fFilenameToSave + " was successfully exported in : " + toSaveDirectory;
        emit error(toPrint.c_str());
        emit processEnded();
    }
}

//void FLExportManager::writeURL(QUrl server){
//    
//    string homeFile = fHome + kSaveFile;
//    
//    QFile f(homeFile.c_str()); 
//    
//    if(f.open(QFile::WriteOnly | QIODevice::Truncate)){
//        
//        QTextStream textWriting(&f);
//        
//        textWriting<<server.toString();
//        f.close();
//    }    
//}
//
//QUrl FLExportManager::readURL(){
//    
//    QString server("http://localhost:8888");
//    
//    string homeFile = fHome + kSaveFile;
//    
//    QFile f(homeFile.c_str()); 
//    
//    if(f.open(QFile::ReadOnly)){
//        
//        QTextStream textReading(&f);
//        textReading>>server;
//        
//        f.close();
//    }
//    
//    return QUrl(server);
//}

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

