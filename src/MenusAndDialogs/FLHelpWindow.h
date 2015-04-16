//
//  FLErrorWindow.h
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//
// This window is the help menu

#ifndef _FLHelpWindow_h
#define _FLHelpWindow_h

#include <string>
#include <map>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

using namespace std;

#include "faust/gui/meta.h"

struct MyMeta: public Meta{
    
    public :
    vector<pair<string, string> > datas;
    
    virtual void declare(const char* key, const char* value){
        datas.push_back(make_pair(key, value));
    }
};

class FLHelpWindow : public QMainWindow{
    
    private :
    
        Q_OBJECT
    
        QTextBrowser* fToolText;
        QTreeWidget*  fTreeLibs;
        QPlainTextEdit*   fLibsText;
        QPlainTextEdit*   fAppText; 
        QPlainTextEdit*   fWinText;
    
        QString fLibsFolder;
        map<string, vector<pair<string, string> > > fInfoLibs;
    
        void    parseLibs(map<string, vector<pair<string, string> > >& infoLibs);
    
    public :
    
        FLHelpWindow(const QString& libsFodler);
        virtual ~FLHelpWindow();
    
        static  FLHelpWindow*   _helpWindow;
        static  FLHelpWindow*   _Instance();
        static  void            createInstance(const QString home);
        static  void            deleteInstance();
    
        void    init();
    
        virtual void closeEvent(QCloseEvent* event);
    
    public slots:
        void    setToolText(const QString & currentText);
        void    setAppPropertiesText(const QString& currentText);  
        void    setWinPropertiesText(const QString& currentText);
        void    setLibText();
    
        void    hideWindow();
    
};

#endif
