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
#include "faust/audio/portaudio-dsp.h"

#include "FLSettings.h"

PA_audioSettings::PA_audioSettings(QGroupBox* parent) : AudioSettings(parent){
    
    fLayout = new QFormLayout;
    
    fBufSize = new QLineEdit;
    fsplRate = new QLineEdit;
    
 /* fInputDeviceBox = new QComboBox;
    fOutputDeviceBox = new QComboBox;

    fLayout->addRow(new QLabel(tr("Choose Input Device")), fInputDeviceBox);
	fLayout->addRow(new QLabel(tr("Choose Ouput Device")), fOutputDeviceBox);*/
    fLayout->addRow(new QLabel(tr("Audio Sample Rate")), fsplRate);
    fLayout->addRow(new QLabel(tr("Audio Buffer Size")), fBufSize);
    
    parent->setLayout(fLayout);

    setVisualSettings();
//	set_deviceList();
}

PA_audioSettings::~PA_audioSettings(){

//    delete fLayout;
//    delete fBufSize;
//    delete fsplRate;
}

//Accessors to the Buffersize
long PA_audioSettings::get_BufferSize(){
    
    return fBufSize->text().toLong();
}

//Accessors to the Buffersize
long PA_audioSettings::get_SampleRate(){
    
    return fsplRate->text().toLong();
}

//Real to Visual
void PA_audioSettings::setVisualSettings(){
    
    FLSettings* settings = FLSettings::_Instance();
    
    stringstream sr, bs;
    bs << settings->value("General/Audio/PortAudio/BufferSize", 1024).toInt();
    sr << settings->value("General/Audio/PortAudio/SampleRate", 44100).toInt();
    
    fBufSize->setText(bs.str().c_str());
    fsplRate->setText(sr.str().c_str());
}

//Visual to Real
void PA_audioSettings::storeVisualSettings(){
    
    FLSettings* settings = FLSettings::_Instance();
    
    int value;
    
    if(isStringInt(fBufSize->text().toLatin1().data())){
        
        value = atoi(fBufSize->text().toLatin1().data());
        
        if(value == 0)
            value = 1024;
    }
    else
        value = 1024;
    
    settings->setValue("General/Audio/PortAudio/BufferSize", value);
    
    if(isStringInt(fsplRate->text().toLatin1().data())){
        
        value = atoi(fsplRate->text().toLatin1().data());
        
        if(value == 0)
            value = 44100;
    }
    else
        value = 44100;
    
    settings->setValue("General/Audio/PortAudio/SampleRate", value);
}

//Operator== for CoreAudio Settings
bool PA_audioSettings::isEqual(AudioSettings* as){
    
    PA_audioSettings* settings1 = dynamic_cast<PA_audioSettings*>(as);
    
    if(settings1 != NULL && settings1->get_BufferSize() == get_BufferSize() && settings1->get_SampleRate() == get_SampleRate())
        return true;
    else
        return false;
    
}

//Accessor to ArchitectureName
QString PA_audioSettings::get_ArchiName(){
    return "PortAudio";
}

//int PA_audioSettings::get_inputDevice(){
///*	if(fInputDeviceBox->currentText().compare("") != 0){
//		return fInputdevices[fInputDeviceBox->currentText().toStdString()];
//	}
//	else*/
//		return 0;
//}
//
//
//int PA_audioSettings::get_ouputDevice(){
///*    if(fOutputDeviceBox->currentText().compare("") != 0){
//		return fOutputdevices[fOutputDeviceBox->currentText().toStdString()];
//	}
//	else*/
//		return 0;
//}
//
//void PA_audioSettings::set_deviceList(){
//    /*	
//     fInputdevices = portaudio::get_InputDevices();
//     fOutputdevices = portaudio::get_OutputDevices();
//     
//     map<string, int>::iterator it;
//     
//     for(it = fInputdevices.begin(); it != fInputdevices.end(); it++){
//     fInputDeviceBox->addItem(it->first.c_str());
//     }
//     for(it = fOutputdevices.begin(); it != fOutputdevices.end(); it++){
//     fOutputDeviceBox->addItem(it->first.c_str());
//     }*/
//}

