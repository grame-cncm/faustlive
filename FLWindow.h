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

#include "faust/gui/FUI.h"

#include "FLToolBar.h"
#include "FLEffect.h"
#include "HTTPWindow.h"
#include "AudioCreator.h"
#include "AudioManager.h"

class QTGUI;

using namespace std;

class FLWindow : public QMainWindow
{
    Q_OBJECT
    
    private : 

        //General functions for files and directory management
        bool            deleteDirectoryAndContent(string& directory);
        string          pathToContent(string path);
    
        bool            fShortcut;   //True if ALT is pressed when x button is pressed
    
        string          fHome;        //Folder of currentSession

        FLToolBar*      fMenu;  
        void            setMenuBar();
    
        FLEffect*         fEffect;         //Effect currently running in the window
        
        QTGUI*          fInterface;      //User control interface
        FUI*            fRCInterface;     //Graphical parameters saving interface
    
        HTTPWindow*     fHttpdWindow;    //Supporting QRcode and httpd address

        AudioManager*   fAudioManager;
        bool            fClientOpen;     //If the client has not be inited, the audio can't be closed when the window is closed
    
        llvm_dsp*       fCurrent_DSP;    //DSP instance of the effect factory running
    
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
    
    signals :
    //Informing of a drop, a close event, ...
        void            drop(list<string>);
        void            close();
        void            closeAll();
        void            rightClick(const QPoint&);
        void            error(const char*);
    
    public :
    
    //Constructor
    //baseName = Window name
    //Index = Index of the window
    //Effect = effect that will be contained in the window
    //x,y = position on screen
    //appHome = current Session folder
    //IDAudio = what architecture audio are we running in?
    //bufferSize, cprValue, ... = audio parameters
    
        FLWindow(string& baseName, int index, FLEffect* eff, int x, int y, string& appHome);
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
        bool            init_Window(bool init, bool recall, char* errorMsg);
    
    //Udpate the effect running in the window and all its related parameters.
    //Returns false if any allocation was impossible and the error buffer is filled
        bool            update_Window(FLEffect* newEffect, string compilationOptions, int optVal, char* error);
    
        bool            update_AudioArchitecture(char* error);
    
    //If the audio Architecture is modified during execution, the windows have to be updated. If the change couldn't be done it returns false and the error buffer is filled
        void            stop_Audio();
        void            start_Audio();
    
        bool            init_audioClient(char* error);
    
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
        bool            init_Httpd(char* error);
    
    //Accessors to parameters
        string          get_nameWindow();
        int             get_indexWindow();
        FLEffect*         get_Effect();
        int             get_x();
        int             get_y();
        bool            is_Default();
    
    //Accessors to httpd Window
        bool            is_httpdWindow_active();
        void            hide_httpdWindow();
    
    //In case of a right click, it is called
        virtual void    contextMenuEvent(QContextMenuEvent *ev);
    
    public slots :
    //Modification of the compilation options
        void            modifiedOptions(string text, int value);
        void            resizing();
    
        void            emit_closeAll();
    //Raises and shows the window
        void            frontShow();
    //Error received
        void            errorPrint(const char* msg);

};

#endif
