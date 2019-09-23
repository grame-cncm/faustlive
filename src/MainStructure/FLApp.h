//
//  FLApp.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLApp is the centerpiece of FaustLive. 
// Its role is to be a communication center between the menus, dialogs, windows, etc.
// It handles the main event loop

#ifndef _FLApp_h
#define _FLApp_h

#include <map>

#include <QApplication>
#include <QtGui>
#include <QMenu>
#include <QMenuBar>
#include <QProgressBar>

#ifdef REMOTE
class remote_dsp_server;
#endif

#define numberWindows 60
#define kMAXRECENT 4
#define kMaxSHAFolders 100

class FLServerHttp;
class FLErrorWindow;
class FLHelpWindow;
class FLPresentationWindow;
class FLPreferenceWindow;
class FLWindow;
class FLWinSettings;
class AudioCreator;

using namespace std;

class FLApp : public QApplication
{
     
    private:
    
        Q_OBJECT
    
//-------Menu Bar and it's sub-Menus------------------------
    
        QMenuBar*           fMenuBar;
    
        QMenu*              create_FileMenu();
        QMenu*              create_ExampleMenu();
        QMenu*              create_RecentFileMenu();
    
        QMenu*              create_LoadSessionMenu(bool recallOrImport);     //@param recallOrImport : true = Recall ||| false = Import
        QMenu*              create_NavigateMenu();
        QMenu*              create_HelpMenu();
    
        QAction**           fRecentFileAction;
        QAction**           fRrecentSessionAction;
        QAction**           fIrecentSessionAction;
    
        QMenu*                      fNavigateMenu;
        QMap<FLWindow*, QMenu*>     fNavigateMenus;
        QMap<QAction*, FLWindow*>   fFrontWindow;
        void                updateNavigateMenus();

        void                setup_Menu();
    
//--------Artificial progress bar to print a goodbye message
        QProgressBar*       fPBar;  
    
//--------Server that embbedes all HttpInterfaces in a droppable environnement
        FLServerHttp*       fServerHttp;        
    
//--------List of windows currently running in the application
        QList<FLWindow*>     FLW_List;       
    
//--------Screen parameters
        int                 fScreenWidth;
        int                 fScreenHeight;
    
//--------Handling window indexes
    //To index the windows, the smallest index not used is given to the window
        int                 find_smallest_index(QList<int> currentIndexes); 
        QList<int>          get_currentIndexes();
    //With this index is calculate the place of the window on the screen
        void                calculate_position(int index, int* x, int* y);
    
//-----------------Application Parameters--------------------------
    
        QString              fWindowBaseName; //Name of Application
    
//--------------- Current Session Management ------------------
        void                 create_Session_Hierarchy();
    
        QString              fSessionFolder;    //Path to currentSession Folder
        QString              fExamplesFolder;   //Folder containing Examples copied from QResources
        QString              fHtmlFolder;       //Folder containing the HTML pages copied from QResources
        QString              fDocFolder;        //Folder containing the documentation copied from QResources
        QString              fLibsFolder;       //Folder containing Libs copied from QResources
        
    //Save/Recall the recent Files/Sessions in settings 
        void                save_Recent(QList<QString>& recents, const QString& pathToSettings);
    
        void                recall_Recent(QList<QString>& recents, const QString& pathToSettings);
    
    //Recent Files Parameters and functions
        QList<QString>      fRecentFiles;
        void                recall_Recent_Files();
        void                save_Recent_Files();
        void                set_Current_File(const QString& pathName);
        void                update_Recent_File();

    //Recent Sessions Parameters and functions
        QList<QString>      fRecentSessions;
        void                save_Recent_Sessions();
        void                recall_Recent_Sessions(); 
        void                set_Current_Session(const QString& pathName);
        void                update_Recent_Session();
    
//---------------------- FLWindow creation ----------------------
        QString             createWindowFolder(const QString& sessionFolder, int index);
        QString             copyWindowFolder(const QString& sessionNewFolder, int newIndex, const QString& sessionFolder, int index, map<int, int> indexChanges);
    
    //When the application is launched without parameter, this timer will start a initialized window
        QTimer*             fInitTimer;
        QTimer*             fEndTimer;
    
    //Preference Menu Objects and Functions
        AudioCreator*       fAudioCreator;
    
        void                update_AudioArchitecture();
    
    //Save and Recall Session actions
		bool				fRecalling;		//True when recalling for the app not to close whith last window
    
    //In case of drops on Application Icon, this event is called
        virtual bool        event(QEvent *ev);
    
    //Functions of rehabilitation if sources disapears
        bool                recall_CurrentSession();
		void				restoreSession(map<int, QString>);
    
    //-----------------Questions about the current State

        FLWindow*           getActiveWin();
    
    //------- Remembering the last folder in which a file was openened
		QString				fLastOpened;
  
    #ifdef REMOTE
        // This server is a remote compilation service included in FaustLive
        remote_dsp_server*  fDSPServer;
    #endif
     
    private slots:
    
    #ifdef REMOTE
        void                changeRemoteServerPort();
    #endif
   
//--------Http Server Response
        FLWindow*           httpPortToWin(int port);
        void                changeDropPort();
        void                launch_Server();
        void                compile_HttpData(const char* data, int port);
        void                stop_Server();

    //---------Drop on a window

        void                drop_Action(QList<QString>);
    
    //---------Presentation Window Slots
    
        void                open_Example_Action(QString pathInQResource);
        void                openExampleAction(const QString& exampleName);
    
    //---------File
        void                connectWindowSignals(FLWindow* win);
        void                create_Empty_Window();
        FLWindow*           createWindow(int index, const QString& mySource, FLWinSettings* windowSettings, QString& error);
        void                open_New_Window();
        void                open_Example_From_FileMenu();
        void                open_Recent_File();
        void                open_Recent_File(const QString& toto);
        void                open_Remote_Window(); // Not really implemented (checkout FLRemoteDSPScanner)
        void                common_shutAction(FLWindow* win);
        void                display_Progress();
        void                close_Window_Action();

        void                deleteComponent();
    //--Session
        void                take_Snapshot();
        void                recall_Snapshot(const QString& filename, bool importOption);
        void                recallSnapshotFromMenu();
        void                importSnapshotFromMenu();
        void                recall_Recent_Session();
        void                import_Recent_Session();
    
    //--------Navigate
        void                updateNavigateText();
        void                frontShow();
    
    //--------Window

        void                duplicate(FLWindow* window);
        void                duplicate_Window();
    
    //---------Preferences
        void                styleClicked(const QString& style);
        void                Preferences();
        void                audioPrefChanged();
    
    //---------Help
        void                open_FL_doc();
        void                open_F_doc();
        void                version_Action();
        void                show_presentation_Action();
    
    //--------Timers
        void                init_Timer_Action();
        void                update_ProgressBar();
        void                updateGuis();
    
    //--------Long operations entertainment
        void                display_CompilingProgress(const QString& msg);
        void                StopProgressSlot(); 
    
    public : 

                            FLApp(int& argc, char** argv);
        virtual             ~FLApp();
    
    public slots:
    
        void                create_New_Window(const QString& name);
    
        virtual void        closeAllWindows();
        void                shut_AllWindows_FromMenu();
		void                shut_AllWindows_FromWindow();

        void                update_CurrentSession();
        void                create_Component_Window();

    //--------Error received
        void                errorPrinting(const QString& msg);

};

#endif
