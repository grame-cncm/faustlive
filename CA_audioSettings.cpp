//
//  CA_audioSettings.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "CA_audioSettings.h"
#include <sstream>
#include <QFileInfo>
#include "utilities.h"

CA_audioSettings::CA_audioSettings(string home, QGroupBox* parent) : AudioSettings(home, parent){
    
    fSavingFile = home + "/" + CA_SAVINGFILE;
    
    QFormLayout* layout = new QFormLayout;
    
    fBufSize = new QLineEdit;
    
//    fBufSize->setInputMask("0000000");
//    bufSize->setText(bf.str().c_str());
    
    fsplRate = new QTextBrowser;
    
    string urlText = "To modify the machine sample rate, go to <a href = /Applications/Utilities/Audio\\MIDI\\Setup.app>Audio Configuration</a>";
//    QString sheet = QString::fromLatin1("*{ text-decoration: underline; color: black; font: Menlo; font-size: 14px }");
//    fsplRate->document()->setDefaultStyleSheet(sheet);
//    fsplRate->setStyleSheet("*{color: black; font: Menlo; font-size: 14px; background-color : white; }");
    
    fsplRate->setOpenExternalLinks(false);
    fsplRate->setHtml(urlText.c_str());
    fsplRate->setFixedHeight(70);
    connect(fsplRate, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(linkClicked(const QUrl&)));
    
    layout->addRow(new QLabel(tr("Audio Buffer Size")), fBufSize);
    layout->addRow(fsplRate);
    
        printf("SETLAYOUT \n");
    parent->setLayout(layout);
        printf("AFTERLAYOUT \n");
    
    readSettings();
    setVisualSettings();
}

CA_audioSettings::~CA_audioSettings(){

    writeSettings();
//    delete fBufSize;
//    delete fsplRate;
}

//Accessors to the Buffersize
int CA_audioSettings::get_BufferSize(){
    
    return fBufferSize;
}

//Real to Visual
void CA_audioSettings::setVisualSettings(){
    
//    printf("SET CURRENT LINE EDIT = %i\n", fBufferSize);
    
    stringstream bs;
    bs << fBufferSize;
    
    fBufSize->setText(bs.str().c_str());
}

//Visual to Real
void CA_audioSettings::storeVisualSettings(){
    
    if(isStringInt(fBufSize->text().toStdString().c_str())){
    
        fBufferSize = atoi(fBufSize->text().toStdString().c_str());
        
        if(fBufferSize == 0)
            fBufferSize = 512;
    }
    else
            fBufferSize = 512;
}

//Write or Read Settings in a File
void CA_audioSettings::writeSettings(){
    
    QFile f(fSavingFile.c_str()); 
    
    if(f.open(QFile::WriteOnly | QIODevice::Truncate)){
        
        QTextStream textWriting(&f);
        
        textWriting<<fBufferSize;
        
        f.close();
    }
}

void CA_audioSettings::readSettings(){
    
    QFile f(fSavingFile.c_str()); 
    
    if(QFileInfo(fSavingFile.c_str()).exists() && f.open(QFile::ReadOnly)){
        
        QTextStream textReading(&f);
        textReading>>fBufferSize;
        
        f.close();
    }
    else{
        fBufferSize = 512;
    }
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
    
//    printf("Before casting\n");
    
    CA_audioSettings* settings1 = dynamic_cast<CA_audioSettings*>(as);
    
    printf("SETTINGS1 = %i\n", settings1->get_BufferSize());
    printf("SETTINGS2 = %i\n", fBufferSize);
    
    if(settings1 != NULL && settings1->get_BufferSize() == fBufferSize)
        return true;
    else
        return false;
        
}

//Accessor to ArchitectureName
string CA_audioSettings::get_ArchiName(){
    return "CoreAudio";
}

