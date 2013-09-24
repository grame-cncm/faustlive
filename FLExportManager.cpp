//
//  FLExportManager.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLExportManager.h"

FLExportManager::FLExportManager(QUrl server, string file, string filename){

    dialogWindow = new QDialog;
    
    fServerUrl = server;
    fFileToExport = file;
    fFilenameToExport = filename;
    fFilenameToSave = fFilenameToExport + "_";
    
    dialogWindow->setWindowFlags(Qt::FramelessWindowHint);
}

FLExportManager::~FLExportManager(){

    delete dialogWindow;
}
    
void FLExportManager::init(){
        
    QFormLayout* exportLayout = new QFormLayout;
    
    QString title("<h2>DOWNLOAD</2>");
    
    QLabel* dialogTitle = new QLabel(title);
    dialogTitle->setStyleSheet("*{color : black}");
    dialogTitle->setAlignment(Qt::AlignCenter);
    
    exportLayout->addRow(dialogTitle);
    exportLayout->addRow(new QLabel(""));
    
    QString groupTitle1("Export ");
    groupTitle1 += fFilenameToExport.c_str();
    groupTitle1 += " as ...";
    
    QString groupTitle2("Build ");
    groupTitle2 += fFilenameToExport.c_str();
    groupTitle2 += " in ...";
    
    menu1Export = new QGroupBox(groupTitle1, dialogWindow);
    menu1Export->setCheckable(true);
    connect(menu1Export, SIGNAL(toggled(bool)), this, SLOT(exportChecked(bool)));
    
    menu2Export = new QGroupBox(groupTitle2, dialogWindow);
    menu2Export->setCheckable(true);
    connect(menu2Export, SIGNAL(toggled(bool)), this, SLOT(exportChecked(bool)));
    
    QFormLayout* menu1Layout = new QFormLayout;
    QFormLayout* menu2Layout = new QFormLayout;
    
    exportFormat = new QComboBox(menu1Export);
    exportFormat->addItem("src.cpp");
    exportFormat->addItem("svg.zip");
    exportFormat->addItem("mdoc.zip");
    
    menu1Layout->addRow(new QLabel("Format"), exportFormat);
    menu1Export->setLayout(menu1Layout);
    exportLayout->addRow(menu1Export);
    
    exportLayout->addRow(new QLabel(""));
    exportLayout->addRow(new QLabel(""));
    
    exportPlatform = new QComboBox(menu2Export);
    exportPlatform->addItem("osx");
    exportPlatform->addItem("windows");
    exportPlatform->addItem("linux");
    
    exportArchi = new QComboBox(menu2Export);
    exportArchi->addItem("coreaudio-qt");
    exportArchi->addItem("jack-qt");
    exportArchi->addItem("supercollider");
    exportArchi->addItem("vst");
    exportArchi->addItem("csound");
    exportArchi->addItem("max-msp");
    exportArchi->addItem("puredata");
    exportArchi->addItem("vsti");
    
    exportChoice = new QComboBox(menu2Export);
    exportChoice->addItem("binary.zip");
    exportChoice->addItem("src.cpp");
    
    menu2Layout->addRow(new QLabel("Platform"), exportPlatform);
    menu2Layout->addRow(new QLabel("Architecture"), exportArchi);
    menu2Layout->addRow(new QLabel("source or binary"), exportChoice);
    
    menu2Export->setLayout(menu2Layout);
    exportLayout->addRow(menu2Export);
    
    QWidget* intermediateWidget = new QWidget(dialogWindow);
    QHBoxLayout* intermediateLayout = new QHBoxLayout;
    
    QPushButton* cancel = new QPushButton(tr("Cancel"), intermediateWidget);
    cancel->setDefault(false);;
    
    save = new QPushButton(tr("Export"), intermediateWidget);
    save->setDefault(true);
    
    connect(save, SIGNAL(released()), this, SLOT(postExport()));
    connect(cancel, SIGNAL(released()), dialogWindow, SLOT(hide()));
    
    intermediateLayout->addWidget(cancel);
    intermediateLayout->addWidget(new QLabel(tr("")));
    intermediateLayout->addWidget(save);
    
    intermediateWidget->setLayout(intermediateLayout);
    exportLayout->addRow(intermediateWidget);
    
    dialogWindow->setLayout(exportLayout);
    
    menu1Export->setChecked(true);
    menu2Export->setChecked(false);
    
    dialogWindow->setVisible(true);
}
        
void FLExportManager::postExport(){
    
    dialogWindow->hide();
    
    emit start_progressing("Connecting with the server...");
    
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

void FLExportManager::exportChecked(bool on){
    
    if(on){
        if(save)
            save->setEnabled(true);
        
        QGroupBox* group = (QGroupBox*)QObject::sender();
        
        if(group == menu1Export){
            if(menu2Export->isChecked()){
                menu2Export->setChecked(false);
            }
        }
        else if(group == menu2Export){
            if(menu1Export->isChecked()){
                menu1Export->setChecked(false);
            }
        }
    }
    else if(!menu2Export->isChecked() && !menu1Export->isChecked())
        save->setEnabled(false);
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
    
//    printf("Getting file from SHA1 Key...\n");
    
    emit start_progressing("Saving your file...");
    
    QString urlString(fServerUrl.toString());
    urlString += ("/");
    urlString += key;
    urlString += "/";
    
    if(menu1Export->isChecked()){
        urlString += exportFormat->currentText();
        fFilenameToSave += exportFormat->currentText().toStdString();
        
    }
    else if(menu2Export->isChecked()){
    
        urlString += exportPlatform->currentText();
        fFilenameToSave += exportPlatform->currentText().toStdString();
        urlString += "/";
        fFilenameToSave += "_";
        urlString += exportArchi->currentText();
        fFilenameToSave += exportArchi->currentText().toStdString();
        urlString += "/";
        fFilenameToSave += "_";
        urlString += exportChoice->currentText();
        fFilenameToSave += exportChoice->currentText().toStdString();
        
    }
    
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
        string toSaveFile = fileDialog->getExistingDirectory().toStdString();
        
        toSaveFile += "/" + fFilenameToSave;
        
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
        
        emit processEnded();   
    }
}


