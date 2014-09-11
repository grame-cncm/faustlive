//
//  FLToolBar.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLToolBar.h"
#include "FLSettings.h"

#include "utilities.h"

//--------------------------FLToolBar

FLToolBar::FLToolBar(QSettings* settings, QWidget* parent) : QToolBar(parent){

    fSettings = settings;
    
    fButtonState = fFold;
    
    setAutoFillBackground(true);

    fWindowOptions = new QPushButton(">   Parameters");
    fWindowOptions->setFlat(true);
    fWindowOptions->setDefault(false);
    addWidget(fWindowOptions);
    
    connect(fWindowOptions, SIGNAL(clicked()), this, SLOT(buttonStateChanged()));

    fWindowOptions->setStyleSheet("*{"
                              "background-color: transparent;"
                              "color: black;"
                              "border-color: transparent;"
                              "margin: 8px;"
                                  "}");
    
    fSaveButton = new QPushButton("Apply Changes");
    fSaveButton->setFlat(true);
    fSaveButton->setEnabled(false);
//    fSaveButton->setEnabled(true);
    connect(fSaveButton, SIGNAL(clicked()), this, SLOT(modifiedOptions()));
    
    fSaveButton->setStyleSheet("*:flat{"
                               "background-color: transparent;"
                               "color: black;"
                               "border: 2px solid black;"
                               "border-radius: 6px;"
                               "}"
                               "*:flat:hover:enabled{"
                               "border: 2px solid #FFC0CB;" 
                               "}" 
                               "*:flat:enabled:pressed{"
                               "background-color: #FFC0CB;"
                               "}"
                               "*:disabled{"
                               "background-color: #606060;"
                               "}"
                               );
    
    init();
}

void FLToolBar::init(){
    
    fContainer = new QToolBox;
    fContainer->setStyleSheet("*{background-color: transparent}");
    
//------- Compilation Options
    QWidget* compilationOptions = new QWidget();
    QFormLayout* compilationLayout = new QFormLayout;
    
    //--Faust
    fOptionLine = new QLineEdit(tr(""), compilationOptions);
    fOptionLine->setStyleSheet("*{background-color:white;}");
    compilationLayout->addRow(new QLabel(tr("FAUST Options")));
    compilationLayout->addRow(fOptionLine);
    
    connect(fOptionLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    //--LLVM
    fOptValLine = new QLineEdit(tr(""), compilationOptions);
    fOptValLine->setStyleSheet("*{background-color:white;}");
    
    fOptValLine->setMaxLength(3);
    fOptValLine->adjustSize();
                              
    compilationLayout->addRow(new QLabel(tr("LLVM Optimization")));  
    compilationLayout->addRow(fOptValLine);
    
    connect(fOptValLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
                              
    compilationOptions->setLayout(compilationLayout);
    fContainer->addItem(compilationOptions, tr("Compilation"));

//------ Automatic Export
    QWidget* automaticExportBox = new QWidget();
    QFormLayout* automaticExportLayout = new QFormLayout;
    
    fAutomaticExportLine = new QLineEdit(tr(""), automaticExportBox);
    fAutomaticExportLine->setStyleSheet("*{background-color:white;}");
    
    connect(fAutomaticExportLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    
    automaticExportLayout->addRow(new QLabel("Compiler Options"));
    automaticExportLayout->addRow(fAutomaticExportLine);
    
    automaticExportBox->setLayout(automaticExportLayout);
    fContainer->addItem(automaticExportBox, tr("Automatic Export"));
    
//------- OSC Control
#ifdef HTTPCTRL
    
    QWidget* oscBox = new QWidget();
    
    fOSCCheckBox = new QCheckBox;
    
    connect(fOSCCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableButton(int)));
    
    QFormLayout* oscLayout = new QFormLayout;
    
    fPortInOscLine = new QLineEdit(tr(""), oscBox);
    fPortInOscLine->setStyleSheet("*{background-color:white;}");
    fPortInOscLine->setMaxLength(4);
    fPortInOscLine->setMaximumWidth(50);
    connect(fPortInOscLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    
    fPortOutOscLine = new QLineEdit(tr(""), oscBox);
    fPortOutOscLine->setStyleSheet("*{background-color:white;}");
    fPortOutOscLine->setMaxLength(4);
    fPortOutOscLine->setMaximumWidth(50);
    connect(fPortOutOscLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    
    fDestHostLine = new QLineEdit(tr(""), oscBox);
//    fDestHostLine->setInputMask("000.000.000.000");
    fDestHostLine->setStyleSheet("*{background-color:white;}");
    connect(fDestHostLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    
    fPortErrOscLine = new QLineEdit(tr(""), oscBox);
    fPortErrOscLine->setStyleSheet("*{background-color:white;}");
    fPortErrOscLine->setMaxLength(4);
    fPortErrOscLine->setMaximumWidth(50);
    connect(fPortErrOscLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    
    oscLayout->addRow("Enable Interface", fOSCCheckBox);    
    oscLayout->addRow(new QLabel(tr("In Port")), fPortInOscLine);
    oscLayout->addRow(new QLabel(tr("Out Port")), fPortOutOscLine);
    oscLayout->addRow(new QLabel(tr("Destination Host")), fDestHostLine);
    oscLayout->addRow(new QLabel(tr("Error Port")), fPortErrOscLine);
    
    oscBox->setLayout(oscLayout);
    
    fContainer->addItem(oscBox, "OSC Interface");
    
//------- HTTP Control
    
    QWidget* httpBox = new QWidget;

    fHttpCheckBox = new QCheckBox();
    
    connect(fHttpCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableButton(int)));
    
    QFormLayout* httpLayout = new QFormLayout;
    
    fHttpPort = new QLabel(tr(""), httpBox);
    
    httpLayout->addRow(new QLabel(tr("Enable Interface")), fHttpCheckBox);
    httpLayout->addRow(new QLabel(tr("Port")), fHttpPort);
    
    httpBox->setLayout(httpLayout);
    fContainer->addItem(httpBox, "Http Interface");
    
#endif
 
#ifdef REMOTE
//-------- Remote Control
    QWidget* remoteControlBox = new QWidget;
    QFormLayout* remoteControlLayout = new QFormLayout;
    
    fRemoteControlCheckBox = new QCheckBox;
    connect(fRemoteControlCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableButton(int)));
    
    fRemoteControlIP = new QLabel(tr(""), remoteControlBox);
    
    remoteControlLayout->addRow(new QLabel(tr("Enable Control")), fRemoteControlCheckBox);
    remoteControlLayout->addRow(new QLabel(tr("Remote IP")), fRemoteControlIP);
    
    remoteControlBox->setLayout(remoteControlLayout);
    fContainer->addItem(remoteControlBox, "Remote Control");
    
    
//-------- Remote Processing
    
    QWidget* remoteBox = new QWidget;
    QFormLayout* remoteLayout = new QFormLayout;
    
    fCVLine = new QLineEdit;
    fCVLine->setMaxLength(4);
    fCVLine->setMaximumWidth(50);
    fCVLine->setStyleSheet("*{background-color:white;}");
    connect(fCVLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    
    fMTULine = new QLineEdit;
    fMTULine->setMaxLength(6);
    fMTULine->setMaximumWidth(50);
    fMTULine->setStyleSheet("*{background-color:white;}");
    connect(fMTULine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    
    fLatLine = new QLineEdit;
    fLatLine->setMaxLength(4);
    fLatLine->setMaximumWidth(50);
    fLatLine->setStyleSheet("*{background-color:white;}");
    connect(fLatLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    
    remoteLayout->addRow(new QLabel(tr("Compression")), fCVLine);
    remoteLayout->addRow(new QLabel(tr("MTU")), fMTULine);
    remoteLayout->addRow(new QLabel(tr("Latency")), fLatLine);
    
    remoteBox->setLayout(remoteLayout);
    fContainer->addItem(remoteBox, "Remote Processing");
    
//------- Publish DSP
    
    QWidget* publishBox = new QWidget;
    QFormLayout* publishLayout = new QFormLayout;
    
    fPublishBox = new QCheckBox;
    connect(fPublishBox, SIGNAL(stateChanged(int)), this, SLOT(enableButton(int)));
    
    publishLayout->addRow(new QLabel(tr("Release DSP")), fPublishBox);
    
    publishBox->setLayout(publishLayout);
    fContainer->addItem(publishBox, "Release");
#endif
    syncVisualParams();
}

///*item*/ is useless but QT signal forces the slot parameters
void FLToolBar::buttonStateChanged(){
    
    if(fButtonState == fFold){
        fButtonState = fUnFold;
        expansionAction();
        fWindowOptions->setText("V   Parameters");
    } 
    else{
        fButtonState = fFold;
        collapseAction();
        fWindowOptions->setText(">   Parameters");
    }
}

//TRICK to be able to add/remove objects from the toolbar 
void FLToolBar::expansionAction(){
    
    fSaveButton->show();
    fContainer->show();
    
    fAction2 = addWidget(fSaveButton);
    fAction1 = addWidget(fContainer);
    
    setOrientation(Qt::Vertical);
    
    adjustSize();
    emit sizeGrowth();
}

void FLToolBar::collapseAction(){
    
    removeAction(fAction1);
    removeAction(fAction2);
    
    adjustSize();
    emit sizeReduction();
    
    syncVisualParams();
    
    setOrientation(Qt::Horizontal);
}

FLToolBar::~FLToolBar(){

printf("delete window options \n");
    delete fWindowOptions;
    printf("save button\n");
    delete fSaveButton;
    delete fOptionLine;
    delete fOptValLine;
    delete fAutomaticExportLine;
#ifndef _WIN32
    delete fHttpCheckBox;
    delete fHttpPort;
    delete fOSCCheckBox;
    delete fPortInOscLine;
    delete fPortOutOscLine;
    delete fPortErrOscLine;
#endif

#ifdef REMOTE
    delete fRemoteControlIP;
    delete fRemoteControlCheckBox;
    delete fCVLine;
    delete fMTULine;
    delete fLatLine;
    delete fDestHostLine;
    delete fPublishBox;
#endif
    delete fContainer;
}

//Changes in parameters = enable save changes button
void FLToolBar::enableButton(const QString& /*newText*/){

    fSaveButton->setEnabled(hasStateChanged());
}

void FLToolBar::enableButton(int /*state*/){
    fSaveButton->setEnabled(hasStateChanged());
}

bool FLToolBar::hasStateChanged(){
    return 
        hasCompilationOptionsChanged()||
        hasAutomaticExportChanged() ||
        wasOscSwitched() ||
        hasOscOptionsChanged() ||
        wasHttpSwitched() ||
        wasRemoteControlSwitched() ||
        hasRemoteOptionsChanged() ||
        hasReleaseOptionsChanged();    
}

bool FLToolBar::hasCompilationOptionsChanged(){
    
    FLSettings* generalSettings = FLSettings::_Instance();
    
    QString val = fOptValLine->text();
    
    bool ok;
    int value = val.toInt(&ok);
	if(!ok)
        value = 3;
    
    if(fOptionLine->text() != (fSettings->value("Compilation/FaustOptions", generalSettings->value("General/Compilation/FaustOptions", "").toString()).toString()) || 
       value != fSettings->value("Compilation/OptValue", generalSettings->value("General/Compilation/OptValue", 3).toInt()).toInt())
            return true;
    else
        return false;
}

bool FLToolBar::hasAutomaticExportChanged(){
    if(fAutomaticExportLine->text() != (fSettings->value("AutomaticExport/Options", "").toString()))
        return true;
    else
        return false;
}

bool FLToolBar::wasOscSwitched(){
#ifdef OSCVAR
    
    if(fSettings->value("Osc/Enabled", FLSettings::_Instance()->value("General/Network/OscDefaultChecked", false)) != fOSCCheckBox->isChecked())
        return true;
#endif
    return false;
}

bool FLToolBar::hasOscOptionsChanged(){
    
#ifdef OSCVAR

    if(fOSCCheckBox->isChecked()){
        if(fPortInOscLine->text() != fSettings->value("Osc/InPort", "5510").toString())
            return true;
        
        if(fPortOutOscLine->text() != fSettings->value("Osc/OutPort", "5511").toString())
            return true;
        
        if(fDestHostLine->text() != fSettings->value("Osc/DestHost", "localhost").toString())
            return true;
        
        if(fPortErrOscLine->text() != fSettings->value("Osc/ErrPort", "5512").toString())
            return true;
    }
#endif 
    return false;
}

bool FLToolBar::wasHttpSwitched(){

#ifdef HTTPCTRL   
    if(fSettings->value("Http/Enabled", FLSettings::_Instance()->value("General/Network/HttpDefaultChecked", false)) != fHttpCheckBox->isChecked())
        return true;
 #endif    
    return false;
}

bool FLToolBar::wasRemoteControlSwitched(){
#ifdef REMOTE  
    if(fSettings->value("RemoteControl/Enabled", false) != fRemoteControlCheckBox->isChecked())
        return true;
#endif    
    return false;
}

bool FLToolBar::hasRemoteOptionsChanged(){
#ifdef REMOTE
    if(fCVLine->text() != fSettings->value("RemoteProcessing/CV", "64").toString())
       return true;

    if(fMTULine->text() != fSettings->value("RemoteProcessing/MTU", "1500").toString())
       return true;
       
    if(fLatLine->text() != fSettings->value("RemoteProcessing/Latency", "10").toString())
       return true;
#endif
    return false;
}

bool FLToolBar::hasReleaseOptionsChanged(){
    
#ifdef REMOTE   
    if(fSettings->value("Release/Enabled", false) != fPublishBox->isChecked())
        return true;
#endif    
    return false;
}

//Reaction to Apply Changes Button
void FLToolBar::modifiedOptions(){
    
//	It's obliged to pass through variables. Otherwise, while one signal is emitted, some toolbar variables are modified from the outside and change the wanted behavior
    bool automaticExportOpt = false;
    bool compilationOpt= false;
#ifdef OSCVAR
    bool oscSwitchOpt = false;
    bool oscSwitchVal = fOSCCheckBox->isChecked();
    bool oscOpt= false;
#endif
#ifdef HTTPCTRL
    bool httpOpt= false;
    bool httpSwitchVal = fHttpCheckBox->isChecked();
#endif
#ifdef REMOTE
    bool remoteControlOpt= false;
    bool remoteControlVal = fRemoteControlCheckBox->isChecked();
    bool remoteOpt= false;
    bool releaseOpt= false;
    bool releaseVal = fPublishBox->isChecked();
#endif
    if(hasAutomaticExportChanged()){
        fSettings->setValue("AutomaticExport/Options", fAutomaticExportLine->text());

		automaticExportOpt = true;
    }
    
    if(hasCompilationOptionsChanged()){
        
        fSettings->setValue("Compilation/OptValue", fOptValLine->text()); 
        fSettings->setValue("Compilation/FaustOptions", fOptionLine->text()); 

		compilationOpt = true;
    }

#ifdef OSCVAR   

    if(hasOscOptionsChanged()){
        
        fSettings->setValue("Osc/InPort", fPortInOscLine->text());
        fSettings->setValue("Osc/OutPort", fPortOutOscLine->text());
        fSettings->setValue("Osc/DestHost", fDestHostLine->text());
        fSettings->setValue("Osc/ErrPort", fPortErrOscLine->text());
        
        if(wasOscSwitched()){
            
            fSettings->setValue("Osc/Enabled", fOSCCheckBox->isChecked());
            
            oscSwitchOpt = true;        
        }
//-- Port changes are declared only if osc isn't switched & if osc is on
        else
			oscOpt = true;
    }
    else if(wasOscSwitched()){
        fSettings->setValue("Osc/Enabled", fOSCCheckBox->isChecked());
        
        oscSwitchOpt = true;
    }

#endif

#ifdef HTTPCTRL
    if(wasHttpSwitched()){
        fSettings->setValue("Http/Enabled", fHttpCheckBox->isChecked());
        
        httpOpt = true;
    }
#endif
#ifdef REMOTE
    if(wasRemoteControlSwitched()){
       fSettings->setValue("RemoteControl/Enable", fRemoteControlCheckBox->isChecked());   
        
        remoteControlOpt = true;
    }
    if(hasRemoteOptionsChanged()){
        
        fSettings->setValue("RemoteProcessing/CV", fCVLine->text());  
        fSettings->setValue("RemoteProcessing/MTU", fMTULine->text());   
        fSettings->setValue("RemoteProcessing/Latency", fLatLine->text());  
        
        remoteOpt = true;
    }
    
    if(hasReleaseOptionsChanged()){
        fSettings->setValue("Release/Enabled", fPublishBox->isChecked()); 
        releaseOpt = true;
    }
#endif

    //	Now emit signals if needed
	if(automaticExportOpt)
        emit generateNewAuxFiles();
	if(compilationOpt)
        emit compilationOptionsChanged();
#ifdef OSCVAR
	if(oscSwitchOpt)
        emit switch_osc(oscSwitchVal);
	if(oscOpt)
        emit oscPortChanged();
#endif
#ifdef HTTPCTRL
	if(httpOpt)
        emit switch_http(httpSwitchVal);
#endif
#ifdef REMOTE
	if(remoteControlOpt)
        emit switch_remotecontrol(remoteControlVal);
    if(remoteOpt)
        emit compilationOptionsChanged();
    if(releaseOpt)
        emit switch_release(releaseVal);
#endif

    fSaveButton->setEnabled(false);
}
                             
void FLToolBar::switchHttp(bool on){
//#ifndef _WIN32 || HTTPDVAR
#ifdef HTTPCTRL
        fHttpCheckBox->setChecked(on);
        modifiedOptions();
#endif
}

void FLToolBar::switchOsc(bool on){
//#ifndef _WIN32 || OSCVAR
#ifdef OSCVAR
        fOSCCheckBox->setChecked(on);
        modifiedOptions();
#endif
}

void FLToolBar::syncVisualParams(){
    
    FLSettings* generalSettings= FLSettings::_Instance();
    
//---- Compilation
    fOptionLine->setText(fSettings->value("Compilation/FaustOptions", generalSettings->value("General/Compilation/FaustOptions", "").toString()).toString());
    fOptValLine->setText(QString::number(fSettings->value("Compilation/OptValue", generalSettings->value("General/Compilation/OptValue", 3).toInt()).toInt()));
    
//---- Automatic Export
    fAutomaticExportLine->setText(fSettings->value("AutomaticExport/Options", "").toString());
    
#ifdef HTTPCTRL
//------ OSC
    bool checked = fSettings->value("Osc/Enabled", generalSettings->value("General/Network/OscDefaultChecked", false)).toBool();
    
    fOSCCheckBox->setChecked(checked);
    
    fPortInOscLine->setText(fSettings->value("Osc/InPort", "5510").toString());
    fPortOutOscLine->setText(fSettings->value("Osc/OutPort", "5511").toString());
    fDestHostLine->setText(fSettings->value("Osc/DestHost", "localhost").toString());
    fPortErrOscLine->setText(fSettings->value("Osc/ErrPort", "5512").toString());
    
//------ Http    
    checked = fSettings->value("Http/Enabled", generalSettings->value("General/Network/HttpDefaultChecked", false)).toBool();
    
    fHttpCheckBox->setChecked(checked);
    
    fHttpPort->setText(fSettings->value("Http/Port", "5510").toString());
#endif
#ifdef REMOTE
//------ RemoteProcessing
    fCVLine->setText(fSettings->value("RemoteProcessing/CV", "64").toString());  
    fMTULine->setText(fSettings->value("RemoteProcessing/MTU", "1500").toString());   
    fLatLine->setText(fSettings->value("RemoteProcessing/Latency", "10").toString());  
    
//----- Release
    checked = fSettings->value("Release/Enabled", false).toBool();
    
    fPublishBox->setChecked(checked);
#endif
    fSaveButton->setEnabled(false);
}






