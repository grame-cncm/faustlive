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

CA_audioSettings::CA_audioSettings(QGroupBox* parent) : AudioSettings(parent)
{
    QFormLayout* layout = new QFormLayout;
    fBufferSize = new QLineEdit;
    fSampleRate = new QTextBrowser;
    std::string urlText = "To modify the machine sample rate, go to <a href = /Applications/Utilities/Audio\\MIDI\\Setup.app>Audio Configuration</a>";
    
    fSampleRate->setOpenExternalLinks(false);
    fSampleRate->setHtml(urlText.c_str());
    fSampleRate->setFixedHeight(70);
    connect(fSampleRate, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(linkClicked(const QUrl&)));
    
    layout->addRow(new QLabel(tr("Audio Buffer Size")), fBufferSize);
    layout->addRow(fSampleRate);
    
    parent->setLayout(layout);
    setVisualSettings();
}

CA_audioSettings::~CA_audioSettings()
{
    // TO CHECK
    //delete fBufferSize;
    //delete fSampleRate;
}

//Accessors to the Buffersize
int CA_audioSettings::get_BufferSize()
{
    char* bs_str = fBufferSize->text().toLatin1().data();
    return (isStringInt(bs_str) ? atoi(bs_str) : 512);
}

//Real to Visual
void CA_audioSettings::setVisualSettings()
{
    fBufferSize->setText(QString::number(FLSettings::_Instance()->value("General/Audio/CoreAudio/BufferSize", 512).toInt()));
}

//Visual to Real
void CA_audioSettings::storeVisualSettings()
{
    char* bs_str = fBufferSize->text().toLatin1().data();
    int buffer_size = (isStringInt(bs_str) ? ((atoi(bs_str) == 0) ? 512 : atoi(bs_str)) : 512);
    
    fBufferSize->setText(QString::number(buffer_size));
    FLSettings::_Instance()->setValue("General/Audio/CoreAudio/BufferSize", buffer_size);
}

//The sample rate cannot be modified internally, it is redirected in Configuration Audio and Midi
void CA_audioSettings::linkClicked(const QUrl& link)
{
    std::string myLink = link.path().toStdString();
    size_t pos = myLink.find("\\");
    
    while(pos != std::string::npos) {
        myLink.insert(pos + 1, 1, ' ');
        pos = myLink.find("\\", pos+2);
    }
    
    std::string myCmd = "open -a " + myLink;
    system(myCmd.c_str());
    
    fSampleRate->reload();
}

//Operator== for CoreAudio Settings
bool CA_audioSettings::isEqual(AudioSettings* as)
{
    CA_audioSettings* settings = dynamic_cast<CA_audioSettings*>(as);
    return (settings != NULL && settings->get_BufferSize() == get_BufferSize());
}

//Accessor to ArchitectureName
QString CA_audioSettings::get_ArchiName()
{
    return "CoreAudio";
}

