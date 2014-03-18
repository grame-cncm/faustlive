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
#include "../API_FAUSTWEB/Faust_Exporter.h"

#define JSON_ONLY

using namespace std;

//Init graphical elements of Export Manager Menu
void FLExportManager::init()
{
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
    
    QPushButton* updateButton = new QPushButton(tr("\n Refresh Targets \n"));
    connect(updateButton, SIGNAL(released()), this, SLOT(targetsDescriptionReceived()));
    
//    fMenu2Layout->addRow(updateButton, new QLabel(""));
//    fMenu2Layout->addRow(new QLabel(""));
    
    fErrorText = new QLabel;
    fMenu2Layout->addRow(fErrorText);
    
    
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
}

FLExportManager::FLExportManager(QString url, QString sessionHome)
{
    std::cerr << "FLExportManager::FLExportManager(...)" << std::endl;

    fHome = sessionHome;

    fDialogWindow = new QDialog;
    fDialogWindow->setWindowFlags(Qt::FramelessWindowHint);
    
    fMessageWindow = new QDialog();
    fMessageWindow ->setWindowFlags(Qt::FramelessWindowHint);
    
    fTextZone = NULL;
    
    fServerUrl = QUrl(url);
    init();
    
    QDir ImagesDir(":/");
    ImagesDir.cd("Images");
    
    fCheckImg = QPixmap(ImagesDir.absoluteFilePath("Check.png")); 
    fNotCheckImg = QPixmap(ImagesDir.absoluteFilePath("NotCheck.png"));
}

FLExportManager::~FLExportManager()
{
    delete fDialogWindow;
    delete fMessageWindow ;

}

//Access Point for FaustLive to export a file
void FLExportManager::exportFile(QString file){

    fFileToExport = file;
    
    if(fPlatforms.size() == 0)
        targetsDescriptionReceived();
    
    fDialogWindow->setVisible(true);
}

void FLExportManager::set_URL(const QString& url){
    fServerUrl = QUrl(url);
    targetsDescriptionReceived();
}

//Build Graphical lists of OS and Platforms received from the server
void FLExportManager::targetsDescriptionReceived()
{
    QString targetUrl= fServerUrl.toString();

    string error("");
    
    fPlatforms.clear();
    fTargets.clear();
    
    fExportPlatform->clear();
    fExportArchi->clear();

    if(get_available_targets(targetUrl.toStdString(), fPlatforms, fTargets, error)){
        
        fErrorText->setText("");
        
        // prepare plaform menu
        for (size_t i=0; i<fPlatforms.size();i++) 
            fExportPlatform->addItem(fPlatforms[i].c_str());
        
        fExportPlatform->show();
        
        // prepare architecture menu

        vector<string> archs = fTargets[fPlatforms[0]];
        
        for (size_t i=0; i<archs.size();i++) 
            fExportArchi->addItem(archs[i].c_str());
        
        fExportArchi->show();
    }    
    else{
        fErrorText->setText(error.c_str());
    }
}

//Upload the file to the server with a post request
void FLExportManager::postExport(){
    
    fDialogWindow->hide();
    
    display_progress();

    string key("");
    string error("");
    string ip(fServerUrl.toString().toStdString());
    
    if(get_shaKey(ip, fFileToExport.toStdString(), key, error)){
        
        fCheck1->setPixmap(fCheckImg);
        
        fMsgLayout->insertRow(3, new QLabel(tr("Remote Compilation")), fCheck2);
        
        getFileFromKey(key.c_str());
    }
    else{
        fCheck1->setPixmap(fNotCheckImg);    

        showMsg(error.c_str());
    }
}

//Get File
void FLExportManager::getFileFromKey(const char* key){
    
    printf("Getting file from SHA1 Key...\n");
    
    QFileDialog* fileDialog = new QFileDialog;
    fileDialog->setConfirmOverwrite(true);
    
    QString filenameToSave;
	
    if(fExportChoice->currentText().compare("src.cpp") == 0)
        filenameToSave = fileDialog->getSaveFileName(NULL, "Save File", tr(""), tr("(*.cpp)"));
    else
        filenameToSave = fileDialog->getSaveFileName(NULL, "Save File", tr(""), tr("(*.zip)"));
    
    string output_file = filenameToSave.toStdString();
    string error("");
    QString msg("");
    
    if(filenameToSave.compare("") == 0){
        stopProgressSlot();
        return;
    }
    
    if(get_file_from_key(fServerUrl.toString().toStdString(), key, fExportPlatform->currentText().toStdString(), fExportArchi->currentText().toStdString(), fExportChoice->currentText().toStdString(), output_file, error)){
        
        fCheck2->setPixmap(fCheckImg);
        msg = filenameToSave + " was successfully exported";
        
    }
    else{
        fCheck2->setPixmap(fNotCheckImg);
        msg = error.c_str();
    }
    
    showMsg(msg);
}

//Dynamic changes of the available architectures depending on platform
void FLExportManager::platformChanged(const QString& index)
{
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
    
    fCloseB = new QPushButton(tr("Ok"), intermediateWidget);
    
    intermediateLayout->addWidget(fCloseB);
    
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
    
    connect(fCloseB, SIGNAL(released()), this, SLOT(stopProgressSlot()));
    
    fCloseB->hide();
    
    fMessageWindow->setLayout(fMsgLayout);

    fMessageWindow->adjustSize();
    fMessageWindow->show();
    fMessageWindow->raise();
}

void FLExportManager::showMsg(const QString& msg){
    
    delete fPrgBar;
    
    fTextZone = new QTextEdit;
    fTextZone->setPlainText(msg);
    fTextZone->setReadOnly(true);
    
    fMsgLayout->insertRow(5, fTextZone);
    
    fCloseB->show();
}

//Stop displaying the message
void FLExportManager::stopProgressSlot(){
    fMessageWindow->hide();
    
    delete fCheck1;
    delete fCheck2;

//    In case the save dialog is canceled, the textzone has been created yet 
//    and the progress bar has not been deleted
    if(fTextZone)
        delete fTextZone;
    else
        delete fPrgBar;
    
    delete fCloseB;
    delete fMsgLayout;
}




