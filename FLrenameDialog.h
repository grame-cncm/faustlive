//
//  FLrenameDialog.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _FLrenameDialog_h
#define _FLrenameDialog_h

#include <string>

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

using namespace std;

//Many times, the user has to be asked for a name modification. 
class FLrenameDialog : public QDialog{
    
    Q_OBJECT
    
    public :
    
    string newName;
    QLineEdit* filenameBox;
    QPushButton* yes_Button;
    
    FLrenameDialog(string& name,QWidget* parent = NULL);
    ~FLrenameDialog();
    string getNewName();
    
    public slots : 
    
    virtual void accept();
    void enable_Button(const QString& text);
    
};
#endif
