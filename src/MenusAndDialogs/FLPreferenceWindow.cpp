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

//----------------------CONSTRUCTOR/DESTRUCTOR---------------------------

FLPreferenceWindow::FLPreferenceWindow(QWidget * parent) : QDialog(parent){
    
    setWindowFlags(Qt::FramelessWindowHint);

    init();
    
    QSize screenSize = QApplication::desktop()->geometry().size(); 
    int screenWidth = screenSize.width();
    int screenHeight = screenSize.height();
    
    move((screenWidth-width())/2, (screenHeight-height())/2);
}

FLPreferenceWindow::~FLPreferenceWindow(){}

void FLPreferenceWindow::init(){
    
    setWindowTitle("Preferences");
    
    QTabWidget* generalTabStructure = new QTabWidget(this);
    generalTabStructure->setStyleSheet("*{}""*::tab-bar{}");
    
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
    
    fServerLine = new QLineEdit(styleTab);
    
#ifdef  HTTPCTRL
    fPortLine = new QLineEdit(styleTab);
    fHttpAuto = new QCheckBox;
#endif
    
    generalTabStructure->addTab(styleTab, tr("Network"));
    networkLayout->addRow(new QLabel(tr("")));
    networkLayout->addRow(new QLabel(tr("Compilation Web Service")), fServerLine);
#ifdef HTTPCTRL
    networkLayout->addRow(new QLabel(tr("")));
    networkLayout->addRow(new QLabel(tr("Remote Dropping Port")), fPortLine);

    networkLayout->addRow(new QLabel(tr("")));
    networkLayout->addRow(new QLabel(tr("Enable Http Interface Automatically")), fHttpAuto);
#endif
    
    
    styleTab->setLayout(networkLayout);
    
    //------------------STYLE PREFERENCES
    generalTabStructure->addTab(networkTab, tr("Style"));
    
    QPlainTextEdit* container = new QPlainTextEdit(networkTab);
    container->setReadOnly(true);
    container->setStyleSheet("*{background-color : transparent;}");
    
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
    networkTab->setLayout(styleContainerLayout);
    
    generalTabLayout->addRow(generalTabStructure);
    generalTabLayout->addRow(buttonContainer);
    setLayout(generalTabLayout);
    
    resetVisualObjects();
}

//Response to save button triggered in preferences
void FLPreferenceWindow::save(){
    
    FLSettings* settings = FLSettings::getInstance();
    
	if(isStringInt(fOptVal->text().toLatin1().data()))
        settings->setValue("General/Compilation/OptValue", atoi(fOptVal->text().toLatin1().data()));
    else
        settings->setValue("General/Compilation/OptValue", 3);
    
    if(settings->value("General/Network/FaustWebUrl", "http://faustservices.grame.fr").toString() != fServerLine->text()){
        settings->setValue("General/Network/FaustWebUrl", fServerLine->text());
        emit urlChanged();
    }
    
    settings->setValue("General/Compilation/FaustOptions", fCompilModes->text());
    
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
    
    hide();
}

void FLPreferenceWindow::resetVisualObjects(){
    
    fCompilModes->setText(FLSettings::getInstance()->value("General/Compilation/FaustOptions", "").toString());
    
    fOptVal->setText(QString::number(FLSettings::getInstance()->value("General/Compilation/OptValue", 3).toInt()));
    
    fServerLine->setText(FLSettings::getInstance()->value("General/Network/FaustWebUrl", "http://faustservice.grame.fr").toString());
    
#ifdef  HTTPCTRL
    fPortLine->setText(QString::number(FLSettings::getInstance()->value("General/Network/HttpDropPort", 7777).toInt()));
    
    bool checked = FLSettings::getInstance()->value("General/Network/HttpDefaultChecked", false).toBool();
    
    if(checked)
        fHttpAuto->setCheckState(Qt::Checked);
    else
        fHttpAuto->setCheckState(Qt::Unchecked);
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

