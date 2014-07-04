//
//  FaustLiveApp.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLApp is the centerpiece of FaustLive. The class controls all the windows, menu and actions of a user. 

#ifndef _FLApp_h
#define _FLApp_h

#include <map>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#define numberWindows 60

class FLServerHttp;
class FLExportManager;
class FLErrorWindow;
class FLHelpWindow;
class FLPresentationWindow;
class FLPreferenceWindow;
class FLWindow;
class FLWinSettings;
class FLEffect;
class AudioCreator;

using namespace std;

#define FLVERSION APP_VERSION

#define DEFAULTNAME "DefaultName"

#define kMAXRECENTFILES 4
#define kMAXRECENTSESSIONS 3

class FLApp : public QApplication
{
    Q_OBJECT
    
    private :
    
        QString    createWindowFolder(const QString& sessionFolder, int index);
        QString    copyWindowFolder(const QString& sessionNewFolder, int newIndex, const QString& sessionFolder, int index, map<int, int> indexChanges);
    
    
    //Menu Bar and it's sub-Menus
    
        QMenuBar *          fMenuBar;
    
        QMenu*              create_FileMenu();
        QMenu*              create_ExampleMenu();
        QMenu*              create_RecentFileMenu();
    
    //@param recallOrImport : true = Recall ||| false = Import
        QMenu*              create_LoadSessionMenu(bool recallOrImport);
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
        void                redirectMenuToWindow(FLWindow* win);
    
        QProgressBar*       fPBar;   //Artificial progress bar to print a goodbye message
    
    //Appendices Dialogs
        FLHelpWindow*        fHelpWindow;        //Help Dialog
    
        FLErrorWindow*      fErrorWindow;       //Error Dialog
        FLPresentationWindow*            fPresWin;           //Presentation Window
        QDialog*            fCompilingMessage;   //Entertaining the user during long operations
        QDialog*            fVersionWindow;     //Not Active Window containing the versions of all used librairies
        FLExportManager*    fExportDialog;      //Manager for web service use
    
        FLServerHttp*       fServerHttp;        //Server that embbedes all HttpInterfaces in a droppable environnement
    
    //List of windows currently running in the application
        QList<FLWindow*>     FLW_List;           //Container of the opened windows
        QList<FLEffect*>     fExecutedEffects;    //This way, the effects already compiled can be recycled if their used further in the execution
        QList<FLEffect*>     fRemoteEffects;     //List of effects used in remote processing
    
    //Screen parameters
        int                 fScreenWidth;
        int                 fScreenHeight;
    
    //To index the windows, the smallest index not used is given to the window
    //With this index is calculate the place of the window on the screen
        int                 find_smallest_index(QList<int> currentIndexes); 
        QList<int>           get_currentIndexes();
        void                calculate_position(int index, int* x, int* y);
    
    //Application Parameters
    
        QString              fWindowBaseName; //Name of Application
    
        void                create_Session_Hierarchy();
    
        QString              fSessionFolder; //Path to currentSession Folder
        QString              fExamplesFolder;    //Folder containing Examples copied from QResources
        QString              fLibsFolder;   //Folder containing Libs copied from QResources
        
    //Recent Files Parameters and functions
        QList<std::pair<QString, QString> >         fRecentFiles;
        void                recall_Recent_Files();
        void                save_Recent_Files();
        void                set_Current_File(const QString& pathName, const QString& effName);
        void                update_Recent_File();
    
    //When the application is launched without parameter, this timer will start a initialized window
        QTimer*             fInitTimer;
        QTimer*             fEndTimer;
    
    //Preference Menu Objects and Functions
        FLPreferenceWindow* fPrefDialog;     //Preference Window
        AudioCreator*       fAudioCreator;
    
        void                init_PreferenceWindow();
    
        void                update_AudioArchitecture();

    //Recent Sessions Parameters and functions
        QStringList         fRecentSessions;
        void                save_Recent_Sessions();
        void                recall_Recent_Sessions(); 
        void                set_Current_Session(const QString& pathName);
        void                update_Recent_Session();
        
    //Save and Recall Session actions
		bool				fRecalling;		//True when recalling for the app not to close whith last window
    
    //In case of an import, those steps are necessary to modify the session before opening it
//        std::list<std::pair<std::string,std::string> > establish_nameChanges(QList<WinInSession*>* session);
    
    //In case of drops on Application Icon, this event is called
        virtual bool        event(QEvent *ev);
    
    //Functions of read/write of a session description file
        QString             parseNextOption(QString& optionsCompilation);
    
    //Reset of the Folders contained in the current Session Folder
        void                reset_CurrentSession();
    
    //Functions of rehabilitation if sources disapears
        bool                recall_CurrentSession();
    
    //-----------------Questions about the current State

        QList<QString>        getNameRunningEffects();

        FLWindow*           getActiveWin();
        FLWindow*           getWinFromHttp(int port);
    
    private slots :
        
#ifndef _WIN32
//--------Server Response
        void                changeDropPort();
        void                launch_Server();
        void                compile_HttpData(const char* data, int port);
        void                stop_Server();
#endif
    //---------Drop on a window

        void                drop_Action(QList<QString>);
    
    //---------Presentation Window Slots
    
        void                open_Example_Action(QString pathInQResource);
        void                openExampleAction(const QString& exampleName);
    
    //---------File
        void                create_Empty_Window();
        bool                createWindow(int index, const QString& mySource, FLWinSettings* windowSettings, QString& error);
        void                open_New_Window();
        void                open_Example_From_FileMenu();
        void                open_Recent_File();
        void                open_Recent_File(const QString& toto);
        void                common_shutAction(FLWindow* win);
        void                display_Progress();
        void                close_Window_Action();

    //--Session
        void                take_Snapshot();
#ifndef _WIN32
        void                tarFolder(const QString& folder);
        void                untarFolder(const QString& folder);
#endif
        void                recall_Snapshot(const QString& filename, bool importOption);
        void                recallSnapshotFromMenu();
        void                importSnapshotFromMenu();
        void                recall_Recent_Session();
        void                import_Recent_Session();
    
    //--------Navigate
    
        void                frontShow();
    
    //--------Window

        void                duplicate(FLWindow* window);
        void                duplicate_Window();
    
        void                export_Win(FLWindow* Win);
        void                export_Action();
    
    //---------Preferences
        void                styleClicked(const QString& style);
        void                Preferences();
    
    //---------Help
        void                apropos();
        void                version_Action();
        void                show_presentation_Action();
    
    //--------Timers
        void                init_Timer_Action();
        void                update_ProgressBar();
    
    //--------Long operations entertainment
        void                display_CompilingProgress(const QString& msg);
        void                StopProgressSlot(); 
    
    public : 

                            FLApp(int& argc, char** argv);
        virtual             ~FLApp();
    
        void                create_New_Window(const QString& name);
    
    public slots :
    
        virtual void        closeAllWindows();
        void                shut_AllWindows();
        void                shut_Window(); 
        void                update_CurrentSession();

    //--------Error received
        void                errorPrinting(const char* msg);

};

#endif
