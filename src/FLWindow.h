//
//  FLWindow.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FAUSTLIVE WINDOW. This class describes the behavior of a window that contains a DSP. 
// Its principal characteristics are : 
//      - to accept drag'n drop
//      - to accept right click
//      - to enable a control within distance of its interface through http protocol (see HTTPDWindow)

#ifndef _FLWindow_h
#define _FLWindow_h

#include <string>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#define kMAXRECENTFILES 4
#define kMAXRECENTSESSIONS 3

#include "faust/gui/FUI.h"

#include "FLEffect.h"
#include "HTTPWindow.h"
#include "AudioCreator.h"
#include "AudioManager.h"

class QTGUI;
class FLToolBar;
class OSCUI;
class Remote_DSP_Factory;

using namespace std;

enum updateType{
    kCrossFade,
    kGetLocal,
    kGetRemote
};


class FLWindow : public QMainWindow
{
    Q_OBJECT
    
    private : 
    
        bool            fShortcut;   //True if ALT is pressed when x button is pressed
    
        string          fHome;        //Folder of currentSession
        string          fSettingsFolder;
    
        FLToolBar*      fMenu;  
        void            setMenu();
        void            set_MenuBar();
        
        QMenu*          fWindowMenu;
        QMenu*          fNavigateMenu;
        QAction**       fRecentFileAction;
        QAction**       fRrecentSessionAction;
        QAction**       fIrecentSessionAction;
        QList<QAction*>     fFrontWindow;
    
        FLEffect*         fEffect;         //Effect currently running in the window
        
        QTGUI*          fInterface;      //User control interface
        FUI*            fRCInterface;     //Graphical parameters saving interface
        OSCUI*          fOscInterface;      //OSC interface 
    
        HTTPWindow*     fHttpdWindow;    //Supporting QRcode and httpd address
        int             fPortHttp;
        int             fGeneralPort;   //FaustLive specific port for droppable httpInterface

        AudioManager*   fAudioManager;
        bool            fClientOpen;     //If the client has not be inited, the audio can't be closed when the window is closed
    
        Remote_DSP_Factory*     fRemoteFactory;
        dsp*            fCurrent_DSP;    //DSP instance of the effect factory running
        bool            fIsLocal;      //True = llvm | False = remote
    
        string          fIpRemoteServer;
    
    //Position on screen
        int             fXPos;
        int             fYPos;

        string          fWindowName;     //WindowName = Common Base Name + - + index
        int             fWindowIndex;    //Unique index corresponding to this window
    
    //Calculate a multiplication coefficient to place the window (and httpdWindow) on screen (avoiding overlapping of the windows)
        int             calculate_Coef();

    //Delete user interface + savings interfaces (FUI, FJUI)
        void            deleteInterfaces();
        
    //Diplays the default interface with Message : Drop a DSP or Edit Me
        void            print_initWindow();
    
    
        list<pair<string, string> > fRecentFiles;
        QStringList                 fRecentSessions;
    
    signals :
    //Informing of a drop, a close event, ...
        void            drop(list<string>);
        void            error(const char*);
    
        void            create_Empty_Window();
        void            open_New_Window();
        void            open_File(string);
        void            takeSnapshot();
        void            recallSnapshotFromMenu();
        void            importSnapshotFromMenu();
        void            closeWin();
        void            shut_AllWindows();
        void            closeAllWindows();
        void            edit_Action();
        void            paste_Action();
        void            duplicate_Action();
        void            httpd_View_Window();
        void            svg_View_Action();
        void            export_Win();
        void            show_aboutQt();
        void            show_preferences();
        void            apropos();
        void            show_presentation_Action();
        void            recall_Snapshot(string, bool);
        void            front(QString);
        void            open_Ex(QString);
    
    private slots :
        void            create_Empty();
        void            open_New();
        void            open_Example();
        void            take_Snapshot();
        void            recallSnapshot();
        void            importSnapshot();
        void            shut();
        void            shut_All();
        void            closeAll();
        void            edit();
        void            paste();
        void            duplicate();
        void            httpd_View();
        void            svg_View();
        void            exportManage();
        void            aboutQt();
        void            preferences();
        void            aboutFaustLive();
        void            show_presentation();
        void            open_Recent_File();
        void            recall_Recent_Session();
        void            import_Recent_Session();
        void            frontShowFromMenu();
        
    public :
    
    //Constructor
    //baseName = Window name
    //Index = Index of the window
    //Effect = effect that will be contained in the window
    //x,y = position on screen
    //appHome = current Session folder
    //GeneralPort = what 
    //bufferSize, cprValue, ... = audio parameters
    
        FLWindow(string& baseName, int index, FLEffect* eff, int x, int y, string& appHome, int generalPort = 5510, int port = 5510, string ipRemoteServer = "127.0.0.1");
        virtual ~FLWindow();
    
    //To close a window the safe way
        //At the end of application execution
        void            close_Window();
        //During the execution
        void            shut_Window();  
    
    //Called when the X button of a window is triggered
        virtual void    closeEvent ( QCloseEvent * event );
        void            keyPressEvent(QKeyEvent* event);
        void            keyReleaseEvent(QKeyEvent* event);
    
    //Creates dsp and interface corresponding to effect
    //Init = 1 --> if the window is created with default process
    //Init = 0 --> if the window is created with other dsp
    //Recalled = 1 --> the window is recalled from a session and needs its parameter
    //Recalled = 0 --> the window is a new one without parameters

    //Returning false if it fails and fills the errorMsg buffer
        bool            init_Window(bool init, bool recall, string& errorMsg);
    
    //Udpate the effect running in the window and all its related parameters.
    //Returns false if any allocation was impossible and the error buffer is filled
        bool            update_Window(int becomeRemote, FLEffect* newEffect, string& error);
    
        bool            update_AudioArchitecture(string& error);
    
    //If the audio Architecture is modified during execution, the windows have to be updated. If the change couldn't be done it returns false and the error buffer is filled
        void            stop_Audio();
        void            start_Audio();
    
        bool            init_audioClient(string& error);
    
    //Drag and drop operations
        virtual void    dropEvent ( QDropEvent * event );
        virtual void    dragEnterEvent ( QDragEnterEvent * event );
        virtual void    dragLeaveEvent ( QDragLeaveEvent * event );   

    //Save the graphical and audio connections of current DSP
        void            save_Window();
    //Update the FJUI file following the changes table
        void            update_ConnectionFile(list<pair<string,string> > changeTable);
    
    //Recall the parameters (graphical and audio)
        void            recall_Window();
    
    //Functions to create an httpd interface
        bool            init_Httpd(string& error);
    
    //Accessors to parameters
        string          get_nameWindow();
        int             get_indexWindow();
        FLEffect*       get_Effect();
        int             get_x();
        int             get_y();
        int             get_Port();
        string             get_RemoteServerIP();
        bool            is_Default();
    
    //Accessors to httpd Window
        bool            is_httpdWindow_active();
        void            hide_httpdWindow();
        string          get_HttpUrl();
        void            set_generalPort(int port);
    
    //In case of a right click, it is called
        virtual void    contextMenuEvent(QContextMenuEvent *ev);
    
    //Menu action
        void            set_RecentFile(list<pair<string, string> > recents);
        void            update_RecentFileMenu();
    
        void            set_RecentSession(QStringList recents);
        void            update_RecentSessionMenu();
    
        void            addWinInMenu(QString name);
        void            deleteWinInMenu(QString name);
        void            initNavigateMenu(QList<QString> wins);
    
    public slots :
    //Modification of the compilation options
        void            modifiedOptions(string text, int value, int port, const string& ipServer);
        void            resizingBig();
        void            resizingSmall();
        void            changeRemoteState(int state);
    
    //Raises and shows the window
        void            frontShow();
    //Error received
        void            errorPrint(const char* msg);

};

#endif