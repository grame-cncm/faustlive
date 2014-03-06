//
//  PA_audioSettings.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "PA_audioSettings.h"
#include <sstream>
#include "utilities.h"

PA_audioSettings::PA_audioSettings(QString home, QGroupBox* parent) : AudioSettings(home, parent){
    
    fSavingFile = home + "/" + PA_SAVINGFILE;
    
    fLayout = new QFormLayout;
    
    fBufSize = new QLineEdit;
    fsplRate = new QLineEdit;
    
    fDeviceBox = new QComboBox;
    
    fLayout->addRow(new QLabel(tr("Choose Audio Device")), fDeviceBox);
    fLayout->addRow(new QLabel(tr("Audio Sample Rate")), fsplRate);
    fLayout->addRow(new QLabel(tr("Audio Buffer Size")), fBufSize);
    
    parent->setLayout(fLayout);
    
    readSettings();
    setVisualSettings();
}

PA_audioSettings::~PA_audioSettings(){
    
    writeSettings();
    delete fLayout;
    delete fBufSize;
    delete fsplRate;
}

//Accessors to the Buffersize
long PA_audioSettings::get_BufferSize(){
    
    return fBufferSize;
}

//Accessors to the Buffersize
long PA_audioSettings::get_SampleRate(){
    
    return fSampleRate;
}

//Real to Visual
void PA_audioSettings::setVisualSettings(){
    
    stringstream sr, bs;
    bs << fBufferSize;
    sr << fSampleRate;
    
    fBufSize->setText(bs.str().c_str());
    fsplRate->setText(sr.str().c_str());
}

//Visual to Real
void PA_audioSettings::storeVisualSettings(){
    
    if(isStringInt(fBufSize->text().toLatin1().data())){
        
        fBufferSize = atoi(fBufSize->text().toLatin1().data());
        
        if(fBufferSize == 0)
            fBufferSize = 512;
    }
    else
        fBufferSize = 512;
    
    if(isStringInt(fsplRate->text().toLatin1().data())){
        
        fSampleRate = atoi(fsplRate->text().toLatin1().data());
        
        if(fSampleRate == 0)
            fSampleRate = 44100;
    }
    else
        fSampleRate = 44100;
}

//Write or Read Settings in a File
void PA_audioSettings::writeSettings(){
    
    QFile f(fSavingFile); 
    
    if(f.open(QFile::WriteOnly | QIODevice::Truncate)){
        
        QTextStream textWriting(&f);
        
        textWriting<<fSampleRate<<' '<<fBufferSize;
        
        f.close();
        printf("FILE WAS WROTE\n");
    }
}

void PA_audioSettings::readSettings(){
    
    QFile f(fSavingFile); 
    
    if(QFileInfo(fSavingFile).exists() && f.open(QFile::ReadOnly)){
        
        QTextStream textReading(&f);
        textReading>>fSampleRate>>fBufferSize;
        
        f.close();
    }
    else{
        fBufferSize = 512;
        fSampleRate = 44100;
    }
}

//Operator== for CoreAudio Settings
bool PA_audioSettings::isEqual(AudioSettings* as){
    
    //    printf("Before casting\n");
    
    PA_audioSettings* settings1 = dynamic_cast<PA_audioSettings*>(as);
    
    if(settings1 != NULL && settings1->get_BufferSize() == fBufferSize && settings1->get_SampleRate() == fSampleRate)
        return true;
    else
        return false;
    
}

//Accessor to ArchitectureName
QString PA_audioSettings::get_ArchiName(){
    return "PortAudio";
}

void PA_audioSettings::add_audioDevice(const QString& deviceName){
    fDeviceBox->addItem(deviceName);
}

QString PA_audioSettings::get_audioDevice(){
    return fDeviceBox->currentText();
}




