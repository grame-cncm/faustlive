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
void FLExportManager::init_DialogWindow()
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
    connect(fSaveB, SIGNAL(released()), this, SLOT(saveFile()));
    connect(fOkB, SIGNAL(released()), fMessageWindow, SLOT(hide()));
    
    fMessageWindow->setLayout(fMsgLayout);
    fMessageWindow->adjustSize();
    fMessageWindow->hide();
}

FLExportManager::FLExportManager(QString url, QString sessionHome)
{
    std::cerr << "FLExportManager::FLExportManager(...)" << std::endl;

    fHome = sessionHome;
    
    fLastPlatform = "";
    fLastArchi = "";
    fLastChoice = "";

    fDialogWindow = new QDialog;
    fDialogWindow->setWindowFlags(Qt::FramelessWindowHint);
    
    fMessageWindow = new QDialog();
    fMessageWindow->setWindowFlags(Qt::FramelessWindowHint);
    
    fTextZone = NULL;
    
    fServerUrl = QUrl(url);
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

}

//Access Point for FaustLive to export a file
void FLExportManager::exportFile(QString file){

//    Reset message dialog graphical elements
    fCloseB->hide();
    fSaveB->hide();
    fOkB->hide();
    fCheck1->hide();
    fConnectionLabel->hide();
    fCheck2->hide();
    fCompilationLabel->hide();
    fTextZone->hide();
    
    fFileToExport = file;
    
//    Reset available targets 
    targetsDescriptionReceived();
    
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

//Upload the file to the server with a post request
void FLExportManager::postExport(){
    
    fLastPlatform = fExportPlatform->currentText();
    fLastArchi = fExportArchi->currentText();
    fLastChoice = fExportChoice->currentText();
    
    fDialogWindow->hide();
    
    fConnectionLabel->show();
    fPrgBar->show();
    fCloseB->show();
    fMessageWindow->show();
    fMessageWindow->raise();
    
    string key("");
    string error("");
    string ip(fServerUrl.toString().toStdString());
    
    if(get_shaKey(ip, fFileToExport.toStdString(), key, error)){
        
        fCheck1->setPixmap(fCheckImg);
        fCheck1->show();
        
        fMessageWindow->repaint();
        getFileFromKey(key.c_str());
    }
    else{
        fCheck1->setPixmap(fNotCheckImg);    
        fCheck1->show();
        
        fPrgBar->hide();
        fCloseB->hide();
        
        fTextZone->setPlainText(error.c_str());
        fTextZone->show();
        fOkB->show();
        
        fMessageWindow->repaint();
    }
}

//Get File
void FLExportManager::getFileFromKey(const char* key){
    
    printf("Getting file from SHA1 Key...\n");
    
    fTemporaryFile = fHome + "/TemporaryFile.";
    
    if(fExportChoice->currentText().compare("src.cpp") == 0)
        fTemporaryFile += "cpp";
    else
        fTemporaryFile += "zip";
    
    string error("");
    
    fCompilationLabel->show();
    fMessageWindow->repaint();
    
    if(get_file_from_key(fServerUrl.toString().toStdString(), key, fExportPlatform->currentText().toStdString(), fExportArchi->currentText().toStdString(), fExportChoice->currentText().toStdString(), fTemporaryFile.toStdString(), error)){
        
        fCheck2->setPixmap(fCheckImg);
        fCheck2->show();
        
        fPrgBar->hide();
        fTextZone->setPlainText("Export was successfull");
        fTextZone->show();
        fCloseB->show();
        fSaveB->show();
        
        fMessageWindow->repaint();
    }
    else{
            
        fCheck2->setPixmap(fNotCheckImg);
        fCheck2->show();
        
        fPrgBar->hide();
        fCloseB->hide();
        
        fTextZone->setPlainText(error.c_str());
        fTextZone->show();
        fOkB->show();
    }
}

void FLExportManager::saveFile(){
    
    QFileDialog* fileDialog = new QFileDialog;
    fileDialog->setConfirmOverwrite(true);
    
    QString filenameToSave;
    
    if(fExportChoice->currentText().compare("src.cpp") == 0)
        filenameToSave = fileDialog->getSaveFileName(NULL, "Save File", tr(""), tr("(*.cpp)"));
    else
        filenameToSave = fileDialog->getSaveFileName(NULL, "Save File", tr(""), tr("(*.zip)"));
    
//        Temporary file is copied and removed
    QFile f(fTemporaryFile);
    
    if(filenameToSave.compare("") != 0)
        f.copy(filenameToSave);
    
    f.remove();
    
    QString msg = filenameToSave + " was successfully saved";
    
    fTextZone->setText(msg);
    
    fSaveB->hide();
    fCloseB->hide();
    fOkB->show();
}




