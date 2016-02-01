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

class httpdUI;
class QTGUI;
class FLToolBar;
class FLStatusBar;
class OSCUI;
class MidiUI;
class FLWindow;
class FLWinSettings;
class remote_dsp_factory;
class FUI;
class AudioCreator;
class AudioManager;
class HTTPWindow;
class dsp;

using namespace std;

enum initType{
    kNoInit,
    kInitBlue,
    kInitWhite
};

class FLWindow : public QMainWindow
{
    
    private: 
    
        Q_OBJECT
    
        QDateTime       fLastMigration;
    
        QString         fHome;        //Folder of currentSession
    
//--- Handle toolbar
        FLToolBar*      fToolBar;
        void            set_ToolBar();
    //Set fToolBar with current windows options
        void            setWindowsOptions();
    
//--- Handle status
        FLStatusBar*    fStatusBar;
        void            set_StatusBar();

//--- Handle menus
        QMenu*          fWindowMenu;
        void            set_MenuBar(QList<QMenu*> appMenus);
    
//--- All the window settings
        QString         fWindowName;     //WindowName = Common Base Name + - + index
        int             fWindowIndex;    //Unique index corresponding to this window
        FLWinSettings*  fSettings;
        bool            fIsDefault;
        QString         fSource;
        QString         fWavSource;
        QDateTime       fCreationDate;
        
//--- Interfaces
        QTGUI*          fInterface;         //User control interface
        FUI*            fRCInterface;       //Graphical parameters saving interface

        OSCUI*          fOscInterface;      //OSC interface 
        MidiUI*         fMIDIInterface;     //MIDI interface
        
        httpdUI*        fHttpInterface;     //Httpd interface for distance control      
        HTTPWindow*     fHttpdWindow;       //Supporting QRcode and httpd address

        void            allocateOscInterface();
        void            deleteOscInterface();

		void            allocateHttpInterface();
        void            deleteHttpInterface();
        
        void            allocateMIDIInterface();
        void            deleteMIDIInterface();
    
//--- Audio driver
        AudioManager*   fAudioManager;
        bool            fAudioManagerStopped;
    
        bool            fClientOpen;     //If the client has not be inited, the audio can't be closed when the window is closed
    
//        string          fInstanceKey; /* Though to be used for remote interactions*/
        
//--- CURRENT DSP Instance
        dsp*            fCurrent_DSP;
    
    //Calculate a multiplication coefficient to place the httpdWindow on screen (avoiding overlapping of the windows)
        int             calculate_Coef();

    //Diplays the default interface with Message : Drop a DSP
        void            print_initWindow(int typeInit);
    
//--- Handle recents
        QList<std::pair<QString, QString> > fRecentFiles;
        QStringList     fRecentSessions;
            
//-- Transforms Wav file into faust string
        bool            ifWavToString(const QString& source, QString& newSource);
    
    signals :
        void            drop(QList<QString>);
        void            closeWin();
        void            shut_AllWindows();
        void            duplicate_Action();
        void            windowNameChanged();
        void            remoteCnxLost(int);
        void            audioError(const QString&);
        void            audioPrefChange();
    
    private slots :
        void            edit();
        void            paste();
        void            duplicate();

        void            view_qrcode();

        void            view_svg();
        void            export_file();
        void            redirectSwitch();
    
    public:
    
    //####CONSTRUCTOR
    //@param : baseName = Window name
    //@param : index = Index of the window
    //@param : appHome = current Session folder
    //@param : windowSettings = all the window settings 
    //@param : appMenus = menus that will be displayed in its toolbar

        FLWindow(QString& baseName, int index, const QString& appHome, FLWinSettings* windowSettings, QList<QMenu*> appMenus);
        virtual ~FLWindow();
    
    //To close a window the safe way
        //At the end of application execution
        void            closeWindow();
        //During the execution
        void            shutWindow();  
    
    //Called when the X button of a window is triggered
        virtual void    closeEvent(QCloseEvent* event);
    
    //-- 4 steps in a interface's life
        bool            allocateInterfaces(const QString& nameEffect); 
        void            buildInterfaces(dsp* dsp);
        void            runInterfaces();
        void            deleteInterfaces();
    
    //Returning false if it fails and fills the errorMsg buffer
    //@param : init = if the window created is a default window.
    //@param : source = DSP to be compiled in the window
    //@param : error = in case init fails, the error is filled
        bool            init_Window(int init, const QString& source, QString& errorMsg);
    
    //If the audio Architecture is modified during execution, the windows have to be updated. If the change couldn't be done it returns false and the error buffer is filled
        bool            update_AudioArchitecture(QString& error);
        void            stop_Audio();
        void            start_Audio();
    
        static void*    startAudioSlave(void* arg);
    
//    In case audio architecture collapses
        static void     audioShutDown(const char* msg, void* arg);
        void            audioShutDown_redirect(const char* msg);

        bool            init_audioClient(QString& error);
        bool            setDSP(QString& error);
        void            update_AudioParams();
    //Drag and drop operations
        virtual void    dropEvent(QDropEvent * event);
        virtual void    dragEnterEvent(QDragEnterEvent * event);
        virtual void    dragLeaveEvent(QDragLeaveEvent * event);
                void    pressEvent();
        virtual bool    eventFilter(QObject *obj, QEvent *ev);

    //Start or stop file/dependency watcher
        void            start_stop_watcher(bool on);
    
    //Save the graphical and audio connections of current DSP
        void            saveWindow();
    
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
    
    //Functions to create an httpd interface
        void            viewQRCode();
        QString         get_HttpUrl();
    
    //In case of a right click, it is called
        virtual void    contextMenuEvent(QContextMenuEvent *ev);
    
    public slots:
    
        void            audioShutDown(const QString& msg);
    
    //Modification of the compilation options
        void            modifiedOptions();
        void            generateAuxFiles();
        void            resizingBig();
        void            resizingSmall();

    //Modification of the HTTP interface
        void            updateHttpInterface();
        void            switchHttp(bool);
        void            exportToPNG();    
    
    //Modification of the OSC interface
		void            updateOSCInterface();
        void            switchOsc(bool);
        void            disableOSCInterface();
        
    //Modification of the MIDI interface
        void            updateMIDIInterface();
        void            switchMIDI(bool);
   
        void            shut();

    //Raises and shows the window
        void            frontShow();
    //Error received
        void            errorPrint(const QString& msg);
    
    //The window has to be warned in case its source file is deleted
        void            source_Deleted();
    
        static          int remoteDSPCallback(int error_code, void* arg);
    
    //Udpate the effect running in the window and all its related parameters.
    //@param : source = DSP that reemplaces the current one
        bool            update_Window(const QString& source);
        void            selfUpdate();
        void            selfNameUpdate(const QString& oldSource, const QString& newSource);

#ifdef REMOTE
        void            RemoteCallback(int);
/*This is a never used attempt to implement remote control (checkout Network/FLRemoteServer)*/

//        void            switchRemoteControl(bool);
//        virtual bool    createNJdspInstance(const string& name, const string& key, 
//                                        const string& celt, const string& ip, 
//                                        const string& port, const string& mtu, 
//                                        const string& latency);
//        virtual bool    startNJdspAudio();
//        void            stopNJdspAudio(const char*);
//        virtual void    cleanInactiveNJdspInstance();
//    
//        void            switchRelease(bool);
#endif
    
};

#endif
