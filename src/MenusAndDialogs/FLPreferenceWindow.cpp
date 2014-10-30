//
//  FaustLiveApp.cpp
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLPreferenceWindow.h"
#include "FLSettings.h"

#include "utilities.h"
#include <sstream>

FLPreferenceWindow* FLPreferenceWindow::_prefWindow = NULL;

//----------------------CONSTRUCTOR/DESTRUCTOR---------------------------

FLPreferenceWindow::FLPreferenceWindow(QWidget * parent) : QDialog(parent){
    
    setWindowFlags(Qt::FramelessWindowHint);

    init();
}

FLPreferenceWindow::~FLPreferenceWindow(){}

FLPreferenceWindow* FLPreferenceWindow::_Instance(){
    if(_prefWindow == NULL)
        _prefWindow = new FLPreferenceWindow;
    
    return _prefWindow;
}

void FLPreferenceWindow::init(){
    
    setWindowTitle("Preferences");
    
    QTabWidget* generalTabStructure = new QTabWidget(this);
    
    QGroupBox* compilationTab = new QGroupBox(generalTabStructure);
    QGroupBox* audioTab = new QGroupBox(generalTabStructure);
    QGroupBox* networkTab = new QGroupBox(generalTabStructure);
    QGroupBox* styleTab = new QGroupBox(generalTabStructure);
    
    QFormLayout* compilationLayout = new QFormLayout;
    QFormLayout* audioLayout = new QFormLayout;
    QFormLayout* generalTabLayout = new QFormLayout;
    QFormLayout* networkLayout = new QFormLayout;
    QGridLayout* styleLayout = new QGridLayout;
    QVBoxLayout* styleContainerLayout = new QVBoxLayout;
    QHBoxLayout* buttonContainerLayout = new QHBoxLayout;
    
    buttonContainerLayout->setAlignment(Qt::AlignCenter);
    
    QWidget* buttonContainer = new QWidget(this);
    
    QPushButton* cancelButton = new QPushButton(tr("Cancel"), buttonContainer);
    cancelButton->setDefault(false);;
    
    QPushButton* saveButton = new QPushButton(tr("Save"), buttonContainer);
    saveButton->setDefault(true);
    
    connect(saveButton, SIGNAL(released()), this, SLOT(save()));
    connect(cancelButton, SIGNAL(released()), this, SLOT(cancel()));
    
    buttonContainerLayout->addWidget(cancelButton);
    buttonContainerLayout->addWidget(new QLabel(tr("")));
    buttonContainerLayout->addWidget(saveButton);
    
    buttonContainer->setLayout(buttonContainerLayout);
    
    //------------------AUDIO PREFERENCES  
    
    generalTabStructure->addTab(audioTab, tr("Audio"));
    
    fAudioBox = new QGroupBox(audioTab);
    fAudioCreator = AudioCreator::_Instance(fAudioBox);
    
    audioLayout->addWidget(fAudioBox);
    audioTab->setLayout(audioLayout);
    
    //------------------WINDOW PREFERENCES    
    
    generalTabStructure->addTab(compilationTab, tr("Compilation"));
    
    fCompilModes = new QLineEdit(compilationTab);
    fOptVal = new QLineEdit(compilationTab);
    
    //    recall_Settings(fSettingsFolder);
    
    compilationLayout->addRow(new QLabel(tr("")));
    compilationLayout->addRow(new QLabel(tr("Faust Compiler Options")), fCompilModes);
    compilationLayout->addRow(new QLabel(tr("LLVM Optimization")), fOptVal);
    compilationLayout->addRow(new QLabel(tr("")));
    
    compilationTab->setLayout(compilationLayout);
    
    //-----------------NETWORK PREFERENCES
    
    fServerLine = new QLineEdit(networkTab);

    generalTabStructure->addTab(networkTab, tr("Network"));
    networkLayout->addRow(new QLabel(tr("")));
    networkLayout->addRow(new QLabel(tr("Compilation Web Service")), fServerLine);
#ifdef REMOTE
    fRemoteServerLine = new QLineEdit(networkTab);
    networkLayout->addRow(new QLabel(tr("")));
    networkLayout->addRow(new QLabel(tr("Remote Compilation Port")), fRemoteServerLine);
#endif
#ifdef HTTPCTRL
    fPortLine = new QLineEdit(networkTab);
    fHttpAuto = new QCheckBox;
    
    networkLayout->addRow(new QLabel(tr("")));
    networkLayout->addRow(new QLabel(tr("Remote Dropping Port")), fPortLine);

    networkLayout->addRow(new QLabel(tr("")));
    networkLayout->addRow(new QLabel(tr("Enable Http Interface Automatically")), fHttpAuto);
#endif
   
#ifdef OSCCTRL
    fOscAuto = new QCheckBox;

    networkLayout->addRow(new QLabel(tr("")));
    networkLayout->addRow(new QLabel(tr("Enable Osc Interface Automatically")), fOscAuto);
#endif
    
    networkTab->setLayout(networkLayout);
    
    //------------------STYLE PREFERENCES
    generalTabStructure->addTab(styleTab, tr("Style"));
    
    QPlainTextEdit* container = new QPlainTextEdit(styleTab);
    container->setReadOnly(true);
    
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
    
    styleLayout->addWidget(defaultColor, 0, 0);    
    styleLayout->addWidget(blue, 1, 0);
    styleLayout->addWidget(grey, 1, 1);
    styleLayout->addWidget(pastel, 0, 1);
    
    container->setLayout(styleLayout);
    
    styleContainerLayout->addWidget(container);
    styleTab->setLayout(styleContainerLayout);
    
    generalTabLayout->addRow(generalTabStructure);
    generalTabLayout->addRow(buttonContainer);
    setLayout(generalTabLayout);
    
    resetVisualObjects();
    
    centerOnPrimaryScreen(this);
}

//Response to save button triggered in preferences
void FLPreferenceWindow::save(){
    
    FLSettings* settings = FLSettings::_Instance();
    
	if(isStringInt(fOptVal->text().toLatin1().data()))
        settings->setValue("General/Compilation/OptValue", atoi(fOptVal->text().toLatin1().data()));
    else
        settings->setValue("General/Compilation/OptValue", 3);
    
    if(settings->value("General/Network/FaustWebUrl", "http://faustservices.grame.fr").toString() != fServerLine->text()){
        settings->setValue("General/Network/FaustWebUrl", fServerLine->text());
        emit urlChanged();
    }
    
    settings->setValue("General/Compilation/FaustOptions", fCompilModes->text());
#ifdef REMOTE
    int portVal;
    
    if(isStringInt(fRemoteServerLine->text().toLatin1().data()))
        portVal = atoi(fRemoteServerLine->text().toLatin1().data());
    else
        portVal = 5555;
    
    if(settings->value("General/Network/RemoteServerPort", 5555).toInt() != portVal){
        settings->setValue("General/Network/RemoteServerPort", portVal);
        emit remoteServerPortChanged();
    }
#endif
#ifdef HTTPCTRL
    int value;
    
    if(isStringInt(fPortLine->text().toLatin1().data()))
        value = atoi(fPortLine->text().toLatin1().data());
    else
        value = 7777;
    
    if(settings->value("General/Network/HttpDropPort", 7777).toInt() != value){
        settings->setValue("General/Network/HttpDropPort", value);
        emit dropPortChange();
    }
    
    settings->setValue("General/Network/HttpDefaultChecked", fHttpAuto->isChecked());
#endif
	
#ifdef OSCCTRL
    settings->setValue("General/Network/OscDefaultChecked", fOscAuto->isChecked());
#endif
    
    hide();
}

void FLPreferenceWindow::resetVisualObjects(){
    
    fCompilModes->setText(FLSettings::_Instance()->value("General/Compilation/FaustOptions", "").toString());
    
    fOptVal->setText(QString::number(FLSettings::_Instance()->value("General/Compilation/OptValue", 3).toInt()));
    
    fServerLine->setText(FLSettings::_Instance()->value("General/Network/FaustWebUrl", "http://faustservice.grame.fr").toString());
    
#ifdef REMOTE
    fRemoteServerLine->setText(QString::number(FLSettings::_Instance()->value("General/Network/RemoteServerPort", 5555).toInt()));
#endif
    
    
#ifdef  HTTPCTRL
    fPortLine->setText(QString::number(FLSettings::_Instance()->value("General/Network/HttpDropPort", 7777).toInt()));

    fHttpAuto->setChecked(FLSettings::_Instance()->value("General/Network/HttpDefaultChecked", false).toBool());
#endif
#ifdef OSCCTRL
     fOscAuto->setChecked(FLSettings::_Instance()->value("General/Network/OscDefaultChecked", false).toBool());
#endif
}

//Response to cancel button triggered in preferences
void FLPreferenceWindow::cancel(){
    
    resetVisualObjects();
    
    fAudioCreator->savedSettingsToVisualSettings();
    hide();
}


//Style clicked in Menu
void FLPreferenceWindow::styleClicked(){
    
    QPushButton* item = (QPushButton*)QObject::sender();
    
    emit newStyle(item->text());
}

