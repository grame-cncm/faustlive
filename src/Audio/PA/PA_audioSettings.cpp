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

PA_audioSettings::PA_audioSettings(QGroupBox* parent) : AudioSettings(parent)
{
    QFormLayout* layout = new QFormLayout;
    fBufferSize = new QLineEdit;
    fSampleRate = new QLineEdit;
    
 /* fInputDeviceBox = new QComboBox;
    fOutputDeviceBox = new QComboBox;

    fLayout->addRow(new QLabel(tr("Choose Input Device")), fInputDeviceBox);
	fLayout->addRow(new QLabel(tr("Choose Ouput Device")), fOutputDeviceBox);*/
    
    layout->addRow(new QLabel(tr("Audio Sample Rate")), fSampleRate);
    layout->addRow(new QLabel(tr("Audio Buffer Size")), fBufferSize);
    
    parent->setLayout(layout);

    setVisualSettings();
//	set_deviceList();
}

PA_audioSettings::~PA_audioSettings()
{}

//Accessors to the Buffersize
long PA_audioSettings::get_BufferSize()
{
    return fBufferSize->text().toLong();
}

//Accessors to the Buffersize
long PA_audioSettings::get_SampleRate()
{
    return fSampleRate->text().toLong();
}

//Real to Visual
void PA_audioSettings::setVisualSettings()
{
    FLSettings* settings = FLSettings::_Instance();
    
    std::stringstream sr, bs;
    bs << settings->value("General/Audio/PortAudio/BufferSize", 1024).toInt();
    sr << settings->value("General/Audio/PortAudio/SampleRate", 44100).toInt();
    
    fBufferSize->setText(bs.str().c_str());
    fSampleRate->setText(sr.str().c_str());
}

//Visual to Real
void PA_audioSettings::storeVisualSettings()
{
    FLSettings* settings = FLSettings::_Instance();
    char* sr_str = fSampleRate->text().toLatin1().data();
    char* bs_str = fBufferSize->text().toLatin1().data();
    
    int sample_rate = (isStringInt(sr_str) ? ((atoi(sr_str) == 0) ? 44100 : atoi(sr_str)) : 44100);
    settings->setValue("General/Audio/PortAudio/SampleRate", sample_rate);
    
    int buffer_size = (isStringInt(bs_str) ? ((atoi(bs_str) == 0) ? 1024 : atoi(bs_str)) : 1024);
    settings->setValue("General/Audio/PortAudio/BufferSize", buffer_size);
}

//Operator== for CoreAudio Settings
bool PA_audioSettings::isEqual(AudioSettings* as)
{
    PA_audioSettings* settings = dynamic_cast<PA_audioSettings*>(as);
    return (settings 
            && settings->get_BufferSize() == get_BufferSize() 
            && settings->get_SampleRate() == get_SampleRate());
}

//Accessor to ArchitectureName
QString PA_audioSettings::get_ArchiName()
{
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

