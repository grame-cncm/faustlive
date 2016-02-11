//
//  JA_audioSettings.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "JA_audioSettings.h"
#include "FLSettings.h"

JA_audioSettings::JA_audioSettings(QGroupBox* parent) : AudioSettings(parent)
{
    QFormLayout* layout = new QFormLayout;
    fAutoConnectBox = new QCheckBox(parent);
    layout->addRow(new QLabel(tr("Auto-Connection")), fAutoConnectBox);
    parent->setLayout(layout);
    setVisualSettings();
}

JA_audioSettings::~JA_audioSettings(){}

void JA_audioSettings::setVisualSettings()
{
    fAutoConnectBox->setChecked (FLSettings::_Instance()->value("General/Audio/Jack/AutoConnect", true).toBool());
}
void JA_audioSettings::storeVisualSettings()
{
    FLSettings::_Instance()->setValue("General/Audio/Jack/AutoConnect", get_AutoConnect());
}

bool JA_audioSettings::isEqual(AudioSettings*)
{
    return true;
}

bool JA_audioSettings::get_AutoConnect()
{
    return fAutoConnectBox->isChecked();
}

QString JA_audioSettings::get_ArchiName()
{
    return "JACK";
}
