//
//  FLrenameDialog.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLrenameDialog.h"

#include <QFormLayout>
#include <QLabel>

//-----------------------------NEWNAMEDIALOG IMPLEMENTATION


FLrenameDialog::FLrenameDialog(string& name ,QWidget* parent = NULL): QDialog(parent, 0){
    yes_Button = new QPushButton("Ok");
    yes_Button->setDefault(true);
    yes_Button->setEnabled(false);
    
    QString tit("Renaming Effect");
    QString msg("");
    QString label1("New name : ");
    string oldname = "Please Rename your effect, "+ name + " is already used!";
    QString label(oldname.c_str());
    
    setWindowFlags(Qt::FramelessWindowHint);
    
    QFormLayout *layout = new QFormLayout;
    
    layout->addRow(new QLabel(label));
    filenameBox = new QLineEdit(this);
    filenameBox->setReadOnly(false);
    layout->addRow(new QLabel(label1), filenameBox);
    layout->addRow(new QLabel(msg),yes_Button);
    setLayout(layout);
    
    connect(filenameBox, SIGNAL(textEdited(const QString& )), this, SLOT(enable_Button(const QString&)));
    connect(yes_Button, SIGNAL(released()), this, SLOT(accept()));
}

FLrenameDialog::~FLrenameDialog(){
    delete filenameBox;
}

void FLrenameDialog::enable_Button(const QString& text){
    
    if(text.toStdString().compare("") == 0)
        yes_Button->setEnabled(false);
    else
        yes_Button->setEnabled(true);
    
}

string FLrenameDialog::getNewName(){ return newName;}

void FLrenameDialog::accept(){
    hide();
    newName = filenameBox->text().toStdString();
}
