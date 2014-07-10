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

#ifndef _WIN32
#include "HTTPWindow.h"
#endif
#include "AudioCreator.h"
#include "AudioManager.h"

class httpdUI;
class QTGUI;
class FLToolBar;
class OSCUI;
class FLWindow;
class FLWinSettings;
class remote_dsp_factory;

using namespace std;

enum initType{
    kNoInit,
    kInitBlue,
    kInitWhite
};

class FLWindow : public QMainWindow
{
    Q_OBJECT
    
    private : 
    
        QDateTime        fLastMigration;
    
        QString          fHome;        //Folder of currentSession
    
        FLToolBar*      fToolBar;
        void            setToolBar();
        QMenu*          fWindowMenu;
        void            set_MenuBar(QList<QMenu*> appMenus);
    
        FLWinSettings*      fSettings;       //All the window settings
        bool                fIsDefault;
        QString             fSource;
        QDateTime           fCreationDate;
        
        QTGUI*          fInterface;      //User control interface
        FUI*            fRCInterface;     //Graphical parameters saving interface
        
        OSCUI*          fOscInterface;      //OSC interface 
        void            allocateOscInterface();
    
#ifndef _WIN32
        httpdUI*        fHttpInterface;     //Httpd interface for distance control      
        HTTPWindow*     fHttpdWindow;    //Supporting QRcode and httpd address
#endif
		void            allocateHttpInterface();
    
        AudioManager*   fAudioManager;
        bool            fClientOpen;     //If the client has not be inited, the audio can't be closed when the window is closed
    
        dsp*            fCurrent_DSP;    //DSP instance of the effect factory running

        map<QString, std::pair<QString, int> >* fIPToHostName;  //Correspondance of remote machine IP to its name

        QString         fWindowName;     //WindowName = Common Base Name + - + index
        int             fWindowIndex;    //Unique index corresponding to this window
    
    //Calculate a multiplication coefficient to place the window (and httpdWindow) on screen (avoiding overlapping of the windows)
        int             calculate_Coef();

    //Delete user interface + savings interfaces (FUI, FJUI)
        void            deleteInterfaces();
        
    //Diplays the default interface with Message : Drop a DSP or Edit Me
        void            print_initWindow(int typeInit);
    
    
        QList<std::pair<QString, QString> > fRecentFiles;
        QStringList                 fRecentSessions;
    
//    Set fToolBar with current windows options
        void            setWindowsOptions();
    
    signals :
    //Informing of a drop, a close event, ...
        void            drop(QList<QString>);
        void            error(const QString&);
    
        void            closeWin();
        void            shut_AllWindows();
        void            edit_Action();
        void            duplicate_Action();
        void            export_Win();
        void            migrate(const QString& ip, int port);
        void            windowNameChanged();
    
    private slots :
        void            edit();
        void            paste();
        void            duplicate();
#ifndef _WIN32
        void            httpd_View();
#endif
        void            svg_View();
        void            exportManage();
        void            redirectSwitch();
    
    public :
    
    //####CONSTRUCTOR
    //@param : baseName = Window name
    //@param : index = Index of the window
    //@param : effect = effect that will be contained in the window
    //@param : x,y = position on screen
    //@param : home = current Session folder
    //@param : osc/httpd port = port on which remote interface will be built 
    //@param : machineName = in case of remote processing, the name of remote machine

        FLWindow(QString& baseName, int index, const QString& appHome, FLWinSettings* windowSettings, QList<QMenu*> appMenus);
        virtual ~FLWindow();
    
    //To close a window the safe way
        //At the end of application execution
        void            close_Window();
        //During the execution
        void            shut_Window();  
    
    //Called when the X button of a window is triggered
        virtual void    closeEvent ( QCloseEvent * event );
    
    //Creates dsp and interface corresponding to effect
    //Init = 1 --> if the window is created with default process
    //Init = 0 --> if the window is created with other dsp
    //Recalled = 1 --> the window is recalled from a session and needs its parameter
    //Recalled = 0 --> the window is a new one without parameters

        bool           buildInterfaces(dsp* dsp, const QString& nameEffect);
    
    //Returning false if it fails and fills the errorMsg buffer
    //@param : init = if the window created is a default window.
    //@param : error = in case init fails, the error is filled
        bool            init_Window(int init, const QString& source, QString& errorMsg);
    
        bool            update_AudioArchitecture(QString& error);
    
    //If the audio Architecture is modified during execution, the windows have to be updated. If the change couldn't be done it returns false and the error buffer is filled
        void            stop_Audio();
        void            start_Audio();
    
//    In case audio architecture collapses
        static void     audioShutDown(const char* msg, void* arg);
        void            audioShutDown(const char* msg);
    
        bool            init_audioClient(QString& error);
        bool            setDSP(QString& error);
    
    //Drag and drop operations
        virtual void    dropEvent ( QDropEvent * event );
        virtual void    dragEnterEvent ( QDragEnterEvent * event );
        virtual void    dragLeaveEvent ( QDragLeaveEvent * event );
                void    pressEvent();
        virtual bool    eventFilter( QObject *obj, QEvent *ev );

    //Save the graphical and audio connections of current DSP
        void            save_Window();
    
    //Recall the parameters (graphical and audio)
        void            recall_Window();
    
    //Accessors to parameters
        QString         get_nameWindow();
        int             get_indexWindow();
        QString         getPath();
        QString         getName();
        QString         getSHA();
        QString         get_source();
        int             get_Port();
        bool            is_Default();
    
    //Accessors to httpd Window
#ifndef _WIN32    
    
    //Functions to create an httpd interface
        void            viewQrCode();
    
        bool            is_httpdWindow_active();
        void            hide_httpdWindow();
        QString         get_HttpUrl();
        void            resetHttpInterface();
#endif
    
    //In case of a right click, it is called
        virtual void    contextMenuEvent(QContextMenuEvent *ev);
    
    public slots :
    //Modification of the compilation options
        void            modifiedOptions();
        void            resizingBig();
        void            resizingSmall();
#ifndef _WIN32
        void            switchHttp(bool on);
        void            exportToPNG();    
        void            updateHTTPInterface();
        void            updateOSCInterface();
#endif
        void            switchOsc(bool on);
        void            disableOSCInterface();
        void            shut();

    
    //Raises and shows the window
        void            frontShow();
    //Error received
        void            errorPrint(const QString& msg);

        static          int RemoteDSPCallback(int error_code, void* arg);
    
    
    //Udpate the effect running in the window and all its related parameters.
    //@param : effect = effect that reemplaces the current one
        bool            update_Window(const QString& source);
};

#endif
