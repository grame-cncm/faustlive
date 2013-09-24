//
//  FLWindow.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _FLWindow_h
#define _FLWindow_h

#include <string>
#include <QObject>
#include <QMainWindow>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QContextMenuEvent>

#include "FLToolBar.h"
#include "Effect.h"
#include "faust/gui/FUI.h"
#include "faustqt.h"
#include "FJUI.h"
#include "HTTPWindow.h"

#include "crossfade_jackaudio.h"
#include "crossfade_netjackaudio.h"
#include "crossfade_coreaudio.h"

using namespace std;

enum audioArchi{kCoreaudio, kJackaudio, kNetjackaudio};

class FLWindow : public QMainWindow
{
    Q_OBJECT
    
    private : 

        //General functions for files and directory management
        bool deleteDirectoryAndContent(string& directory);
        string  pathToContent(string path);
    
        bool            shortcut;   //True if ALT is pressed when x button is pressed
    
        string          appHome;        //Folder of currentSession

        FLToolBar*       menu;  
        void            setMenuBar();
    
        Effect*         effect;         //Effect currently running in the window
        
        QTGUI*          interface;      //User control interface
        FUI*            finterface;     //Graphic parameter saving interface
        FJUI*           jinterface;     //Audio Connections saving interface
    
        HTTPWindow*    httpdWindow;    //Supporting QRcode and httpd address
        string          IPaddress;      //IP address for TCP protocol

        int             indexAudio;     //Relationned with the audioArchi enum   
        audio*          audioClient;    //AudioClient depending on audio architecture
    
        bool            clientOpen;     //In case the 
        bool            audioSwitched;
    
        llvm_dsp*       current_DSP;    //DSP instance of the effect factory running
    
    //Position on screen
        int             xPos;
        int             yPos;

        string          nameWindow;     //WindowName = Common Base Name + - + index
        int             windowIndex;    //Unique index corresponding to this window
    
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
    
        FLWindow(string& baseName, int index, Effect* eff, int x, int y, string& appHome, int IDAudio);
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
        bool            init_Window(bool init, bool recall, char* errorMsg, int bufferSize, int cprValue, string masterIP, int masterPort, int latency);
    
    //Udpate the effect running in the window and all its related parameters.
    //Returns false if any allocation was impossible and the error buffer is filled
        bool            update_Window(Effect* newEffect, string compilationOptions, int optVal, char* error, int bufferSize, int compressionValue, string masterIP, int masterPort, int latency);
    //If the audio Architecture is modified during execution, the windows have to be updated. If the change couldn't be done it returns false and the error buffer is filled
        void            stop_Audio();
        bool            update_AudioArchitecture(int newIndexAudio, int bufferSize, int compressionValue, string masterIP, int masterPort, int latency);
    
        bool            update_AudioParameters(char* error, int index, int bufferSize, int compressionValue, string masterIP, int masterPort, int latency);
    
        bool            init_audioClient(int index, int bufferSize, int compressionValue, string masterIP, int masterPort, int latency);
    
        void            reset_audioSwitch();
    
    //Drag and drop operations
        virtual void    dropEvent ( QDropEvent * event );
        virtual void    dragEnterEvent ( QDragEnterEvent * event );
        virtual void    dragLeaveEvent ( QDragLeaveEvent * event );   

    //Save the graphical and audio connections of current DSP
        void            save_Window(int index);
    //Update the FJUI file following the changes table
        void            update_ConnectionFile(list<pair<string,string> > changeTable);
    
    //Recall the parameters (graphical and audio)
        bool            recall_Window();
    
    //Functions to create an httpd interface
        bool            init_Httpd(char* error);
    
    //Accessors to parameters
        string          get_nameWindow();
        int             get_indexWindow();
        Effect*         get_Effect();
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
