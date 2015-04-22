//
//  FLrenameDialog.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// Dialog that asks the user to rename its application if the name already exists. 

#ifndef _FLrenameDialog_h
#define _FLrenameDialog_h

#include <string>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

using namespace std;

//Many times, the user has to be asked for a name modification. 
class FLrenameDialog : public QDialog{
    
    private:
    
        Q_OBJECT
        
        QString         fNewName;
        QLineEdit*      fFilenameBox;
        QPushButton*    fYes_Button;
      
    public:
    
        FLrenameDialog(const QString& name,QWidget* parent = NULL);
        virtual  ~FLrenameDialog();
        
        QString         getNewName();
        
    public slots: 
        
        virtual void    accept();
        void            enable_Button(const QString& text);
    
};

#endif
