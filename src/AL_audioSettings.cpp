//
//  AL_audioSettings.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "AL_audioSettings.h"
#include <sstream>
#include "utilities.h"

AL_audioSettings::AL_audioSettings(string home, QGroupBox* parent) : AudioSettings(home, parent){
    
    fSavingFile = home + "/" + AL_SAVINGFILE;
    
    QFormLayout* layout = new QFormLayout;
    
    fBufLine = new QLineEdit;
    fFreqLine = new QLineEdit;
    fPeriodLine = new QLineEdit;
    fCardLine = new QLineEdit;
    
    layout->addRow(new QLabel(tr("Device Name")), fCardLine);
    layout->addRow(new QLabel(tr("Audio Sample Rate")), fFreqLine);
    layout->addRow(new QLabel(tr("Audio Buffer Size")), fBufLine);
    layout->addRow(new QLabel(tr("Audio Periods")), fPeriodLine);
    
//        printf("SET LAYOUT %p\n", this);
    parent->setLayout(layout);
//        printf("AFTER LAYOUT %p\n", this);
    
    readSettings();
    setVisualSettings();
}

AL_audioSettings::~AL_audioSettings(){

    writeSettings();
//    delete fBufSize;
//    delete fsplRate;
}

//Accessors to the Buffersize
int AL_audioSettings::get_BufferSize(){
    
    return fBufferSize;
}

//Accessors to the Period
int AL_audioSettings::get_Period(){
    
    return fPeriods;
}

//Accessors to the Frequency
int AL_audioSettings::get_Frequency(){
    
    return fFrequency;
}

//Accessors to the Name of the Device
string AL_audioSettings::get_DeviceName(){
    
    return fCardName;
}

//Real to Visual
void AL_audioSettings::setVisualSettings(){
    
//    printf("SET CURRENT LINE EDIT = %i\n", fBufferSize);
    
    stringstream bs;
    bs << fBufferSize;
    
    fBufLine->setText(bs.str().c_str());
    
    stringstream freq;
    freq << fFrequency;
    
    fFreqLine->setText(freq.str().c_str());
    
    stringstream per;
    per << fPeriods;
    
    fPeriodLine->setText(per.str().c_str());
    
    fCardLine->setText(fCardName.c_str());
}

//Visual to Real
void AL_audioSettings::storeVisualSettings(){
    
    if(isStringInt(fBufLine->text().toStdString().c_str())){
    
        fBufferSize = atoi(fBufLine->text().toStdString().c_str());
        
        if(fBufferSize == 0)
            fBufferSize = 512;
    }
    else
            fBufferSize = 512;
    
    if(isStringInt(fFreqLine->text().toStdString().c_str())){
        
        fFrequency = atoi(fFreqLine->text().toStdString().c_str());
        
        if(fFrequency == 0)
            fFrequency = 44100;
    }
    else
        fFrequency = 44100;
    
    if(isStringInt(fPeriodLine->text().toStdString().c_str())){
        
        fPeriods = atoi(fPeriodLine->text().toStdString().c_str());
        
        if(fPeriods == 0)
            fPeriods = 2;
    }
    else
        fPeriods = 2;
    
    fCardName = fCardLine->text().toStdString();
}

//Write or Read Settings in a File
void AL_audioSettings::writeSettings(){
    
    QFile f(fSavingFile.c_str()); 
    
    if(f.open(QFile::WriteOnly | QIODevice::Truncate)){
        
        QTextStream textWriting(&f);
        
		QString interName(fCardName.c_str());

        textWriting<<fFrequency<<' '<<fBufferSize<<' '<<fPeriods<<' '<<interName;
        
        f.close();
    }
}

void AL_audioSettings::readSettings(){
    
    QFile f(fSavingFile.c_str()); 
    
    if(QFileInfo(fSavingFile.c_str()).exists() && f.open(QFile::ReadOnly)){
        
        QTextStream textReading(&f);
		QString interName;
        textReading>>fFrequency>>fBufferSize>>fPeriods>>interName;

		fCardName = interName.toStdString();
        
        f.close();
    }
    else{
        fBufferSize = 512;
        fFrequency = 44100;
        fPeriods = 2;
		fCardName = "hw:0";
    }
}

//Operator== for CoreAudio Settings
bool AL_audioSettings::isEqual(AudioSettings* as){
    
//    printf("Before casting\n");
    
    AL_audioSettings* settings1 = dynamic_cast<AL_audioSettings*>(as);
    
    
    if(settings1 != NULL && settings1->get_BufferSize() == fBufferSize && settings1->get_Frequency() == fFrequency && settings1->get_Period() == fPeriods && settings1->get_DeviceName().compare(fCardName) == 0)
        return true;
    else
        return false;
        
}

//Accessor to ArchitectureName
string AL_audioSettings::get_ArchiName(){
    return "Alsa";
}

