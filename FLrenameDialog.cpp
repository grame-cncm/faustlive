//
//  FLrenameDialog.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLrenameDialog.h"

//-----------------------------NEWNAMEDIALOG IMPLEMENTATION


FLrenameDialog::FLrenameDialog(string& name ,QWidget* parent): QDialog(parent, 0){
    fYes_Button = new QPushButton("Ok");
    fYes_Button->setDefault(true);
    fYes_Button->setEnabled(false);
    
    QString tit("Renaming Effect");
    QString msg("");
    QString label1("New name : ");
    string oldname = "Please Rename your effect, "+ name + " is already used!";
    QString label(oldname.c_str());
    
    setWindowFlags(Qt::FramelessWindowHint);
    
    QFormLayout *layout = new QFormLayout;
    
    layout->addRow(new QLabel(label));
    fFilenameBox = new QLineEdit(this);
    fFilenameBox->setReadOnly(false);
    layout->addRow(new QLabel(label1), fFilenameBox);
    layout->addRow(new QLabel(msg),fYes_Button);
    setLayout(layout);
    
    connect(fFilenameBox, SIGNAL(textEdited(const QString& )), this, SLOT(enable_Button(const QString&)));
    connect(fYes_Button, SIGNAL(released()), this, SLOT(accept()));
}

FLrenameDialog::~FLrenameDialog(){
    delete fFilenameBox;
}

void FLrenameDialog::enable_Button(const QString& text){
    
    if(text.toStdString().compare("") == 0)
        fYes_Button->setEnabled(false);
    else
        fYes_Button->setEnabled(true);
    
}

string FLrenameDialog::getNewName(){ return fNewName;}

void FLrenameDialog::accept(){
    hide();
    fNewName = fFilenameBox->text().toStdString();
}
