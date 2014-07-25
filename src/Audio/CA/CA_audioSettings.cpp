//
//  CA_audioSettings.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "CA_audioSettings.h"
#include "utilities.h"

#include "FLSettings.h"

CA_audioSettings::CA_audioSettings(QGroupBox* parent) : AudioSettings(parent){
    
    QFormLayout* layout = new QFormLayout;
    
    fBufSize = new QLineEdit;

    fsplRate = new QTextBrowser;
    
    string urlText = "To modify the machine sample rate, go to <a href = /Applications/Utilities/Audio\\MIDI\\Setup.app>Audio Configuration</a>";
    
    fsplRate->setOpenExternalLinks(false);
    fsplRate->setHtml(urlText.c_str());
    fsplRate->setFixedHeight(70);
    connect(fsplRate, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(linkClicked(const QUrl&)));
    
    layout->addRow(new QLabel(tr("Audio Buffer Size")), fBufSize);
    layout->addRow(fsplRate);
    
    parent->setLayout(layout);
    
    setVisualSettings();
}

CA_audioSettings::~CA_audioSettings(){
}

//Accessors to the Buffersize
int CA_audioSettings::get_BufferSize(){
    
    if(isStringInt(fBufSize->text().toStdString().c_str()))
        return atoi(fBufSize->text().toStdString().c_str());
    else
        return 512;
}

//Real to Visual
void CA_audioSettings::setVisualSettings(){
    
    fBufSize->setText(QString::number(FLSettings::_Instance()->value("General/Audio/CoreAudio/BufferSize", 512).toInt()));
}

//Visual to Real
void CA_audioSettings::storeVisualSettings(){
    
    int value;
    
    if(isStringInt(fBufSize->text().toStdString().c_str())){
        
        value = atoi(fBufSize->text().toStdString().c_str());
        
        if(value == 0)
            value = 512;
    }
    else
            value = 512;
    
    fBufSize->setText(QString::number(value));
    FLSettings::_Instance()->setValue("General/Audio/CoreAudio/BufferSize", value);
}

//The sample rate cannot be modified internally, it is redirected in Configuration Audio and Midi
void CA_audioSettings::linkClicked(const QUrl& link){
 
    string myLink = link.path().toStdString();
    
    size_t pos = myLink.find("\\");
    
    while(pos != string::npos){
        myLink.insert(pos + 1, 1, ' ');
        pos = myLink.find("\\", pos+2);
    }
    
    string myCmd = "open -a " + myLink;
    system(myCmd.c_str());
    
    fsplRate->reload();
    
}

//Operator== for CoreAudio Settings
bool CA_audioSettings::isEqual(AudioSettings* as){
    
    CA_audioSettings* settings1 = dynamic_cast<CA_audioSettings*>(as);
    
    if(settings1 != NULL && settings1->get_BufferSize() == get_BufferSize())
        return true;
    else
        return false;
        
}

//Accessor to ArchitectureName
QString CA_audioSettings::get_ArchiName(){
    return "CoreAudio";
}

