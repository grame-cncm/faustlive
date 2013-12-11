//
//  NJ_audioSettings.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class describes the Settings of netjack clients : 
//  - Its compression Value
//  - Its IPadress
//  - Its Port
//  - Its Latencty
// All of them are described visually by a QLineEdit

#include "NJ_audioSettings.h"

#include <sstream>

#include "utilities.h"

NJ_audioSettings::NJ_audioSettings(string homeFolder, QGroupBox* parent) : AudioSettings(homeFolder, parent){
    
    fSavingFile = homeFolder + "/" + NJ_SAVINGFILE;
    
    QFormLayout* layout = new QFormLayout;
    
    fCVLine = new QLineEdit;
    layout->addRow(new QLabel(tr("Compression Value")), fCVLine);
    
    fIPLine = new QLineEdit;
    fIPLine->setInputMask("000.000.000.000");
    layout->addRow(new QLabel(tr("Master IP adress")), fIPLine);
    
    fPortLine = new QLineEdit;
    fPortLine->setInputMask("000000");
    layout->addRow(new QLabel(tr("Master Port")), fPortLine);
    
    fLatLine = new QLineEdit;
    layout->addRow(new QLabel(tr("Latency")), fLatLine);
    
    parent->setLayout(layout);
    
    readSettings();
    setVisualSettings();
}

NJ_audioSettings::~NJ_audioSettings(){

    writeSettings();
}

//Real Params TO Saving File
void NJ_audioSettings::writeSettings(){
    
    QFile f(fSavingFile.c_str()); 
    
    if(f.open(QFile::WriteOnly | QIODevice::Truncate)){
        
        QTextStream textWriting(&f);
        
        QString IPaddress(fIP.c_str());
        
        textWriting<<fCompressionValue<<' '<<IPaddress<<' '<<fPort<<' '<<fLatency;
        
        f.close();
    }
}

//Saving File TO Real Params 
void NJ_audioSettings::readSettings(){
    
    QFile f(fSavingFile.c_str());
    
    if(QFileInfo(fSavingFile.c_str()).exists() && f.open(QFile::ReadOnly)){
        
        QTextStream textReading(&f);
        QString IPaddress = "";
        textReading>>fCompressionValue>>IPaddress>>fPort>>fLatency;
        
        fIP = IPaddress.toStdString();
        
        f.close();
    }
    else{
        fCompressionValue = -1;
        fIP = "225.3.19.154";
        fPort = 19000;
        fLatency = 2;
    }
}

//Real Params TO Visual params
void NJ_audioSettings::setVisualSettings(){
    
    stringstream cv, p, l;
    cv << fCompressionValue;
    p << fPort;
    l << fLatency;
    
    fCVLine->setText(cv.str().c_str());
    fIPLine->setText(fIP.c_str());
    fPortLine->setText(p.str().c_str());
    fLatLine->setText(l.str().c_str());
}

//Visual Params TO Real params
void NJ_audioSettings::storeVisualSettings(){
    
//    if(isStringInt(fCVLine->text().toStdString().c_str()))
        fCompressionValue = atoi(fCVLine->text().toStdString().c_str());
    
    fIP = fIPLine->text().toStdString();
    
//    if(isStringInt(fPortLine->text().toStdString().c_str()))
        fPort = atoi(fPortLine->text().toStdString().c_str());
    
//    if(isStringInt(fLatLine->text().toStdString().c_str()))
        fLatency = atoi(fLatLine->text().toStdString().c_str());
}

//Operator== for NetJack Settings
bool NJ_audioSettings::isEqual(AudioSettings* as){

    NJ_audioSettings* settings = dynamic_cast<NJ_audioSettings*>(as);
    
    if(settings != NULL && settings->get_compressionValue() == fCompressionValue && settings->get_IP() == fIP && settings->get_Port() == fPort && settings->get_latency() == fLatency)
        return true;
    else
        return false;

}

//Accessors to the parameters
int NJ_audioSettings::get_compressionValue(){
    return fCompressionValue;
}

string& NJ_audioSettings::get_IP(){
    return fIP;
}

int NJ_audioSettings::get_Port(){
    return fPort;
}

int NJ_audioSettings::get_latency(){
    return fLatency;
}

string NJ_audioSettings::get_ArchiName(){
    return "NetJackAudio";
}