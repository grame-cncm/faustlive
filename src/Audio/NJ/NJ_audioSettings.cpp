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

#include "utilities.h"
#include "FLSettings.h"

NJ_audioSettings::NJ_audioSettings(QGroupBox* parent) : AudioSettings(parent){
    
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
    
    fMTULine = new QLineEdit;
    layout->addRow(new QLabel(tr("MTU")), fMTULine);
    
    parent->setLayout(layout);
    
    setVisualSettings();
}

NJ_audioSettings::~NJ_audioSettings(){}

//Real Params TO Visual params
void NJ_audioSettings::setVisualSettings(){
    
    FLSettings* settings = FLSettings::_Instance();
    
    fCVLine->setText(QString::number(settings->value("General/Audio/NetJack/CV", -1).toInt()));
    fIPLine->setText(settings->value("General/Audio/NetJack/IP", "225.3.19.154").toString());
    fPortLine->setText(QString::number(settings->value("General/Audio/NetJack/Port", 19000).toInt()));
    fMTULine->setText(QString::number(settings->value("General/Audio/NetJack/MTU", 1500).toInt()));
    fLatLine->setText(QString::number(settings->value("General/Audio/NetJack/Latency", 2).toInt()));
}

//Visual Params TO Real params
void NJ_audioSettings::storeVisualSettings(){
    
    FLSettings* settings = FLSettings::_Instance();
    
    if(isStringInt(fCVLine->text().toStdString().c_str()))
        settings->setValue("General/Audio/NetJack/CV", atoi(fCVLine->text().toStdString().c_str()));
    else{
        fCVLine->setText(QString::number(-1));
        settings->setValue("General/Audio/NetJack/CV", -1);
    }
    
    settings->setValue("General/Audio/NetJack/IP", fIPLine->text());
    
    if(isStringInt(fPortLine->text().toStdString().c_str()))
        settings->setValue("General/Audio/NetJack/Port", atoi(fPortLine->text().toStdString().c_str()));
    else{
        fPortLine->setText(QString::number(19000));
        settings->setValue("General/Audio/NetJack/Port", 19000);
    }
    
    if(isStringInt(fMTULine->text().toStdString().c_str()))
        settings->setValue("General/Audio/NetJack/MTU", atoi(fMTULine->text().toStdString().c_str()));
    else{
        fMTULine->setText(QString::number(1500));
        settings->setValue("General/Audio/NetJack/MTU", 1500);
    }
    
    if(isStringInt(fLatLine->text().toStdString().c_str()))
        settings->setValue("General/Audio/NetJack/Latency", atoi(fLatLine->text().toStdString().c_str()));
    else{
        fLatLine->setText(QString::number(2));
        settings->setValue("General/Audio/NetJack/Latency", 2);
    }
}

//Operator== for NetJack Settings
bool NJ_audioSettings::isEqual(AudioSettings* as){
    
    NJ_audioSettings* settings = dynamic_cast<NJ_audioSettings*>(as);
    
    if(settings != NULL && settings->get_compressionValue() == get_compressionValue() && settings->get_IP() == get_IP() && settings->get_Port() == get_Port() && settings->get_mtu() == get_mtu() && settings->get_latency() == get_latency())
        return true;
    else
        return false;
    
}

//Accessors to the parameters
int NJ_audioSettings::get_compressionValue(){
    
    if(isStringInt(fCVLine->text().toStdString().c_str()))
        return  atoi(fCVLine->text().toStdString().c_str());
    else
        return -1;
}

QString NJ_audioSettings::get_IP(){

    return fIPLine->text();
}

int NJ_audioSettings::get_Port(){
    
    if(isStringInt(fPortLine->text().toStdString().c_str()))
        return atoi(fPortLine->text().toStdString().c_str());
    else
        return 19000;
}

int NJ_audioSettings::get_mtu(){
    
    if(isStringInt(fMTULine->text().toStdString().c_str()))
        return atoi(fMTULine->text().toStdString().c_str());
    else
        return 1500;
}

int NJ_audioSettings::get_latency(){
    
    if(isStringInt(fLatLine->text().toStdString().c_str()))
        return atoi(fLatLine->text().toStdString().c_str());
    else
        return 2;
}

QString NJ_audioSettings::get_ArchiName(){
    return "NetJackAudio";
}



