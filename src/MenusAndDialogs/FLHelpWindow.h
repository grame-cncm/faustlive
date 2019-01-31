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

#include <QMainWindow>
#include <QString>
#include <QTextBrowser>
#include <QTreeWidget>
#include <QPlainTextEdit>

#if defined(WIN32) && !defined(GCC)
# pragma warning (disable: 4100)
#else
# pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "faust/gui/meta.h"

using namespace std;

struct MyMeta: public Meta{
    
    public:
    vector<pair<string, string> > datas;
    
    virtual void declare(const char* key, const char* value){
        datas.push_back(make_pair(key, value));
    }
};

class FLHelpWindow : public QMainWindow{
    
    private:
    
        Q_OBJECT
    
        QTextBrowser* fToolText;
        QTreeWidget* fTreeLibs;
        QPlainTextEdit* fLibsText;
        QPlainTextEdit* fAppText;
        QPlainTextEdit* fWinText;
    
        QString fLibsFolder;
        QString fTestDSPPath;
        map<string, vector<pair<string, string> > > fInfoLibs;
    
        void parseLibs(map<string, vector<pair<string, string> > >& infoLibs);
    
    public:
    
        FLHelpWindow(const QString& libsFodler, const QString& testDSPPath);
        virtual ~FLHelpWindow();
    
        static  FLHelpWindow*   _helpWindow;
        static  FLHelpWindow*   _Instance();
        static  void            createInstance(const QString home, const QString& testDSPPath);
        static  void            deleteInstance();
    
        void init();
    
        virtual void closeEvent(QCloseEvent* event);
    
    public slots:
    
        void    setToolText(const QString & currentText);
        void    setAppPropertiesText(const QString& currentText);  
        void    setWinPropertiesText(const QString& currentText);
        void    setLibText();
    
        void    hideWindow();
    
};

#endif
