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

FLToolBar::FLToolBar(QSettings* settings, QWidget* parent) : QToolBar(parent)
{
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

void FLToolBar::init()
{
    fContainer = new QToolBox;
    fContainer->setStyleSheet("*{background-color: transparent}");

    //------- OSC Control
    QWidget* oscBox = new QWidget();
    fOSCCheckBox = new QCheckBox;
    
    connect(fOSCCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableButton(int)));
    QFormLayout* oscLayout = new QFormLayout;
    
    fPortInOscLine = new QLineEdit(tr(""), oscBox);
    fPortInOscLine->setStyleSheet("*{background-color:white;}");
    fPortInOscLine->setMaxLength(4);
    fPortInOscLine->setMaximumWidth(50);
    connect(fPortInOscLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    connect(fPortInOscLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
    fPortOutOscLine = new QLineEdit(tr(""), oscBox);
    fPortOutOscLine->setStyleSheet("*{background-color:white;}");
    fPortOutOscLine->setMaxLength(4);
    fPortOutOscLine->setMaximumWidth(50);
    connect(fPortOutOscLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    connect(fPortOutOscLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
    fDestHostLine = new QLineEdit(tr(""), oscBox);
//    fDestHostLine->setInputMask("000.000.000.000");
    fDestHostLine->setStyleSheet("*{background-color:white;}");
    connect(fDestHostLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    connect(fDestHostLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
    fPortErrOscLine = new QLineEdit(tr(""), oscBox);
    fPortErrOscLine->setStyleSheet("*{background-color:white;}");
    fPortErrOscLine->setMaxLength(4);
    fPortErrOscLine->setMaximumWidth(50);
    connect(fPortErrOscLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    connect(fPortErrOscLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
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
    fContainer->addItem(httpBox, "HTTP Interface");
      
    //------- MIDI Control
    QWidget* midiBox = new QWidget;
    fMIDICheckBox = new QCheckBox();
    
    connect(fMIDICheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableButton(int)));
    QFormLayout* midiLayout = new QFormLayout;
    
    midiLayout->addRow(new QLabel(tr("Enable Interface")), fMIDICheckBox);
    
    midiBox->setLayout(midiLayout);
    fContainer->addItem(midiBox, "MIDI Interface");
 
#ifdef REMOTE
//-------- Remote Control
//    QWidget* remoteControlBox = new QWidget;
//    QFormLayout* remoteControlLayout = new QFormLayout;
//    
//    fRemoteControlCheckBox = new QCheckBox;
//    connect(fRemoteControlCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableButton(int)));
//    
//    fRemoteControlIP = new QLabel(tr(""), remoteControlBox);
//    
//    remoteControlLayout->addRow(new QLabel(tr("Enable Control")), fRemoteControlCheckBox);
//    remoteControlLayout->addRow(new QLabel(tr("Remote IP")), fRemoteControlIP);
//    
//    remoteControlBox->setLayout(remoteControlLayout);
//    fContainer->addItem(remoteControlBox, "Remote Control");
//    
//    

    //-------- Remote Processing
    QWidget* remoteBox = new QWidget;
    QFormLayout* remoteLayout = new QFormLayout;
    
    fCVLine = new QLineEdit;
    fCVLine->setMaxLength(4);
    fCVLine->setMaximumWidth(50);
    fCVLine->setStyleSheet("*{background-color:white;}");
    connect(fCVLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    connect(fCVLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
    fMTULine = new QLineEdit;
    fMTULine->setMaxLength(6);
    fMTULine->setMaximumWidth(50);
    fMTULine->setStyleSheet("*{background-color:white;}");
    connect(fMTULine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    connect(fMTULine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
    fLatLine = new QLineEdit;
    fLatLine->setMaxLength(4);
    fLatLine->setMaximumWidth(50);
    fLatLine->setStyleSheet("*{background-color:white;}");
    connect(fLatLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    connect(fLatLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
    remoteLayout->addRow(new QLabel(tr("Compression")), fCVLine);
    remoteLayout->addRow(new QLabel(tr("MTU")), fMTULine);
    remoteLayout->addRow(new QLabel(tr("Latency")), fLatLine);
    
    remoteBox->setLayout(remoteLayout);
    fContainer->addItem(remoteBox, "Remote Processing");
    
//------- Publish DSP
/*    
    QWidget* publishBox = new QWidget;
    QFormLayout* publishLayout = new QFormLayout;
    
    fPublishBox = new QCheckBox;
    connect(fPublishBox, SIGNAL(stateChanged(int)), this, SLOT(enableButton(int)));
    
    publishLayout->addRow(new QLabel(tr("Publish DSP")), fPublishBox);
    
    publishBox->setLayout(publishLayout);
    fContainer->addItem(publishBox, "Publish");*/
#endif
    
    //------- Compilation Options
    QWidget* compilationOptions = new QWidget();
    QFormLayout* compilationLayout = new QFormLayout;
    
    compilationLayout->addRow(new QLabel("------Code Generation Options------"));
    
    //--Faust
    fOptionLine = new QLineEdit(tr(""), compilationOptions);
    fOptionLine->setStyleSheet("*{background-color:white;}");
    compilationLayout->addRow(new QLabel(tr("FAUST Compiler Options")));
    compilationLayout->addRow(fOptionLine);
    
    connect(fOptionLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    connect(fOptionLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    //--LLVM
    fOptValLine = new QLineEdit(tr(""), compilationOptions);
    fOptValLine->setStyleSheet("*{background-color:white;}");
    
    fOptValLine->setMaxLength(3);
    fOptValLine->adjustSize();
    
    compilationLayout->addRow(new QLabel(tr("LLVM Optimization")));  
    compilationLayout->addRow(fOptValLine);
    
    connect(fOptValLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    connect(fOptValLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
    compilationLayout->addRow(new QLabel(""));
    compilationLayout->addRow(new QLabel("-----Additional Compilation Step-----"));
    
    //------ Automatic Export
    fAutomaticExportLine = new QLineEdit(tr(""), compilationOptions);
    fAutomaticExportLine->setStyleSheet("*{background-color:white;}");
    
    connect(fAutomaticExportLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    connect(fAutomaticExportLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
    compilationLayout->addRow(new QLabel("Faust Compiler Options"));
    compilationLayout->addRow(fAutomaticExportLine);
    compilationLayout->addRow(new QLabel("Ex: -lang ajs -o filename.js"));
    
    compilationLayout->addRow(new QLabel(""));
    
    //------ Post compilation scripting
    compilationLayout->addRow(new QLabel("-------Post-Compilation Script-------"));
    
    fScriptLine = new QLineEdit(tr(""), compilationOptions);
    fScriptLine->setStyleSheet("*{background-color:white;}");
    
    connect(fScriptLine, SIGNAL(textEdited(const QString&)), this, SLOT(enableButton(const QString&)));
    connect(fScriptLine, SIGNAL(returnPressed()), this, SLOT(modifiedOptions()));
    
    compilationLayout->addRow(new QLabel("Command Line"));    
    compilationLayout->addRow(fScriptLine);
    
    compilationOptions->setLayout(compilationLayout);
    fContainer->addItem(compilationOptions, tr("Compilation"));
    
    syncVisualParams();
}

///*item*/ is useless but QT signal forces the slot parameters
void FLToolBar::buttonStateChanged()
{
    if (fButtonState == fFold) {
        fButtonState = fUnFold;
        expansionAction();
        fWindowOptions->setText("V   Parameters");
    } else {
        fButtonState = fFold;
        collapseAction();
        fWindowOptions->setText(">   Parameters");
    }
}

//TRICK to be able to add/remove objects from the toolbar 
void FLToolBar::expansionAction()
{
    fSaveButton->show();
    fContainer->show();
    
    fAction2 = addWidget(fSaveButton);
    fAction1 = addWidget(fContainer);
    
    setOrientation(Qt::Vertical);
    
    adjustSize();
    emit sizeGrowth();
}

void FLToolBar::collapseAction()
{
    removeAction(fAction1);
    removeAction(fAction2);
    
    adjustSize();
    emit sizeReduction();
    
    syncVisualParams();
    
    setOrientation(Qt::Horizontal);
}

FLToolBar::~FLToolBar()
{
    delete fWindowOptions;
    delete fSaveButton;
    delete fOptionLine;
    delete fOptValLine;
    delete fAutomaticExportLine;

    delete fHttpCheckBox;
    delete fHttpPort;
    
    delete fMIDICheckBox;

    delete fOSCCheckBox;
    delete fPortInOscLine;
    delete fPortOutOscLine;
    delete fPortErrOscLine;

#ifdef REMOTE
//  delete fRemoteControlIP;
//  delete fRemoteControlCheckBox;
    delete fCVLine;
    delete fMTULine;
    delete fLatLine;
    delete fDestHostLine;
//  delete fPublishBox;
#endif
    delete fContainer;
}

//Changes in parameters = enable save changes button
void FLToolBar::enableButton(const QString& /*newText*/)
{
    fSaveButton->setEnabled(hasStateChanged());
}

void FLToolBar::enableButton(int /*state*/)
{
    fSaveButton->setEnabled(hasStateChanged());
}

bool FLToolBar::hasStateChanged()
{
    return 
        (hasCompilationOptionsChanged()||
        hasAutomaticExportChanged() ||
        hasScriptChanged() ||
        wasOscSwitched() ||
        hasOscOptionsChanged() ||
        wasHttpSwitched() ||
        wasMIDISwitched() ||
        wasRemoteControlSwitched() ||
        hasRemoteOptionsChanged() ||
        hasReleaseOptionsChanged());    
}

bool FLToolBar::hasCompilationOptionsChanged()
{
    FLSettings* generalSettings = FLSettings::_Instance();
    QString val = fOptValLine->text();
    
    bool ok;
    int value = val.toInt(&ok);
	if (!ok)
        value = 3;
    
    return (fOptionLine->text() != (fSettings->value("Compilation/FaustOptions", generalSettings->value("General/Compilation/FaustOptions", "").toString()).toString()) 
            || value != fSettings->value("Compilation/OptValue", generalSettings->value("General/Compilation/OptValue", 3).toInt()).toInt());
}

bool FLToolBar::hasAutomaticExportChanged()
{
    return (fAutomaticExportLine->text() != (fSettings->value("AutomaticExport/Options", "").toString()));
 }

bool FLToolBar::hasScriptChanged()
{
    return (fScriptLine->text() != (fSettings->value("Script/Options", "").toString()));
}

bool FLToolBar::wasOscSwitched()
{
    return (fSettings->value("Osc/Enabled", FLSettings::_Instance()->value("General/Network/OscDefaultChecked", false)) != fOSCCheckBox->isChecked());
}

bool FLToolBar::hasOscOptionsChanged()
{
    if (fOSCCheckBox->isChecked()) {
    
        if (fPortInOscLine->text() != fSettings->value("Osc/InPort", "5510").toString())
            return true;
        
        if (fPortOutOscLine->text() != fSettings->value("Osc/OutPort", "5511").toString())
            return true;
        
        if (fDestHostLine->text() != fSettings->value("Osc/DestHost", "localhost").toString())
            return true;
        
        if (fPortErrOscLine->text() != fSettings->value("Osc/ErrPort", "5512").toString())
            return true;
    }

    return false;
}

bool FLToolBar::wasHttpSwitched()
{
    return (fSettings->value("Http/Enabled", FLSettings::_Instance()->value("General/Network/HttpDefaultChecked", false)) != fHttpCheckBox->isChecked());
}

bool FLToolBar::wasMIDISwitched()
{
    return (fSettings->value("MIDI/Enabled", FLSettings::_Instance()->value("General/Control/MIDIDefaultChecked", false)) != fMIDICheckBox->isChecked());
}

bool FLToolBar::wasRemoteControlSwitched()
{
//#ifdef REMOTE  
//    if(fSettings->value("RemoteControl/Enabled", false) != fRemoteControlCheckBox->isChecked())
//        return true;
//#endif    
    return false;
}

bool FLToolBar::hasRemoteOptionsChanged()
{
#ifdef REMOTE
    if (fCVLine->text() != fSettings->value("RemoteProcessing/CV", "64").toString())
       return true;

    if (fMTULine->text() != fSettings->value("RemoteProcessing/MTU", "1500").toString())
       return true;
       
    if (fLatLine->text() != fSettings->value("RemoteProcessing/Latency", "10").toString())
       return true;
#endif
    return false;
}

bool FLToolBar::hasReleaseOptionsChanged()
{
#ifdef REMOTE   
//    if(fSettings->value("Release/Enabled", false) != fPublishBox->isChecked())
//        return true;
#endif    
    return false;
}

//Reaction to Apply Changes Button
void FLToolBar::modifiedOptions()
{
 //	It's obliged to pass through variables. Otherwise, while one signal is emitted, some toolbar variables are modified from the outside and change the wanted behavior
    bool automaticExportOpt = false;
//  bool scriptOpt = false;
    bool compilationOpt = false;

    bool oscSwitchOpt = false;
    bool oscSwitchVal = fOSCCheckBox->isChecked();
    bool oscOpt = false;

    bool httpOpt = false;
    bool httpSwitchVal = fHttpCheckBox->isChecked();
    
    bool MIDIOpt = false;
    bool MIDISwitchVal = fMIDICheckBox->isChecked();

#ifdef REMOTE
//    bool remoteControlOpt= false;
//    bool remoteControlVal = fRemoteControlCheckBox->isChecked();
    bool remoteOpt = false;
//    bool releaseOpt= false;
//    bool releaseVal = fPublishBox->isChecked();
#endif
    if (hasAutomaticExportChanged()) {
        fSettings->setValue("AutomaticExport/Options", fAutomaticExportLine->text());
		automaticExportOpt = true;
    }
    
    if (hasCompilationOptionsChanged()) {
        fSettings->setValue("Compilation/OptValue", fOptValLine->text()); 
        fSettings->setValue("Compilation/FaustOptions", fOptionLine->text()); 
		compilationOpt = true;
    }

    if (hasScriptChanged()) {
        fSettings->setValue("Script/Options", fScriptLine->text());
//      scriptOpt = true;
    }
    
    if (hasOscOptionsChanged()) {
        fSettings->setValue("Osc/InPort", fPortInOscLine->text());
        fSettings->setValue("Osc/OutPort", fPortOutOscLine->text());
        fSettings->setValue("Osc/DestHost", fDestHostLine->text());
        fSettings->setValue("Osc/ErrPort", fPortErrOscLine->text());
        
        if (wasOscSwitched()) {
            fSettings->setValue("Osc/Enabled", fOSCCheckBox->isChecked());
            oscSwitchOpt = true;        
        }
//-- Port changes are declared only if osc isn't switched & if osc is on
        else {
			oscOpt = true;
        }
    } else if (wasOscSwitched()) {
        fSettings->setValue("Osc/Enabled", fOSCCheckBox->isChecked());
        oscSwitchOpt = true;
    }

    if (wasHttpSwitched()) {
        fSettings->setValue("Http/Enabled", fHttpCheckBox->isChecked());
        httpOpt = true;
    }
    
    if (wasMIDISwitched()) {
        fSettings->setValue("MIDI/Enabled", fMIDICheckBox->isChecked());
        MIDIOpt = true;
    }
 
#ifdef REMOTE
//    if(wasRemoteControlSwitched()){
//       fSettings->setValue("RemoteControl/Enable", fRemoteControlCheckBox->isChecked());   
//        
//        remoteControlOpt = true;
//    }
    if (hasRemoteOptionsChanged()) {
        fSettings->setValue("RemoteProcessing/CV", fCVLine->text());  
        fSettings->setValue("RemoteProcessing/MTU", fMTULine->text());   
        fSettings->setValue("RemoteProcessing/Latency", fLatLine->text());  
        remoteOpt = true;
    }
    
//    if(hasReleaseOptionsChanged()){
//        fSettings->setValue("Release/Enabled", fPublishBox->isChecked()); 
//        releaseOpt = true;
//    }
#endif

    //	Now emit signals if needed
	if (automaticExportOpt)
        emit generateNewAuxFiles();
//    if(scriptOpt)
//        emit execScript();
	if (compilationOpt)
        emit compilationOptionsChanged();

	if (oscSwitchOpt)
        emit switch_osc(oscSwitchVal);
        
	if (oscOpt)
        emit oscPortChanged();

	if (httpOpt)
        emit switch_http(httpSwitchVal);
        
    if (MIDIOpt)
        emit switch_midi(MIDISwitchVal);

#ifdef REMOTE
//	if(remoteControlOpt)
//        emit switch_remotecontrol(remoteControlVal);
    if (remoteOpt)
        emit compilationOptionsChanged();
//    if(releaseOpt)
//        emit switch_release(releaseVal);
#endif
    fSaveButton->setEnabled(false);
}
                             
void FLToolBar::switchHttp(bool on)
{
	fHttpCheckBox->setChecked(on);
    modifiedOptions();
}

void FLToolBar::switchOsc(bool on)
{
	fOSCCheckBox->setChecked(on);
    modifiedOptions();
}

void FLToolBar::switchMIDI(bool on)
{
	fMIDICheckBox->setChecked(on);
    modifiedOptions();
}

void FLToolBar::syncVisualParams()
{
    FLSettings* generalSettings= FLSettings::_Instance();
    
    //---- Compilation
    fOptionLine->setText(fSettings->value("Compilation/FaustOptions", generalSettings->value("General/Compilation/FaustOptions", "").toString()).toString());
    fOptValLine->setText(QString::number(fSettings->value("Compilation/OptValue", generalSettings->value("General/Compilation/OptValue", 3).toInt()).toInt()));
    
    //---- Automatic Export
    fAutomaticExportLine->setText(fSettings->value("AutomaticExport/Options", "").toString());
    
    //---- Post Compilation Script
    fScriptLine->setText(fSettings->value("Script/Options", "").toString());

    //------ OSC
    fOSCCheckBox->setChecked(fSettings->value("Osc/Enabled", generalSettings->value("General/Network/OscDefaultChecked", false)).toBool());
    
    fPortInOscLine->setText(fSettings->value("Osc/InPort", "5510").toString());
    fPortOutOscLine->setText(fSettings->value("Osc/OutPort", "5511").toString());
    fDestHostLine->setText(fSettings->value("Osc/DestHost", "localhost").toString());
    fPortErrOscLine->setText(fSettings->value("Osc/ErrPort", "5512").toString());

    //------ Http    
    fHttpCheckBox->setChecked(fSettings->value("Http/Enabled", generalSettings->value("General/Network/HttpDefaultChecked", false)).toBool());
    fHttpPort->setText(fSettings->value("Http/Port", "5510").toString());
    
    //------ MIDI    
    fMIDICheckBox->setChecked(fSettings->value("MIDI/Enabled", generalSettings->value("General/Control/MIDIDefaultChecked", false)).toBool());

#ifdef REMOTE
    //------ RemoteProcessing
    fCVLine->setText(fSettings->value("RemoteProcessing/CV", "64").toString());  
    fMTULine->setText(fSettings->value("RemoteProcessing/MTU", "1500").toString());   
    fLatLine->setText(fSettings->value("RemoteProcessing/Latency", "10").toString());  
    
//----- Release
//    bool checked = fSettings->value("Release/Enabled", false).toBool();
//    
//    fPublishBox->setChecked(checked);
#endif
    fSaveButton->setEnabled(false);
}






