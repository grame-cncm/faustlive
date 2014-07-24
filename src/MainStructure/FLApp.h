//
//  FaustLiveApp.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLApp is the centerpiece of FaustLive. The class controls all the windows, menu and actions of a user. 

#ifndef _FLApp_h
#define _FLApp_h

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
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

#include <map>
using namespace std;

class FLApp : public QApplication
{
    Q_OBJECT
    
    private :
    
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
    
        QProgressBar*       fPBar;   //Artificial progress bar to print a goodbye message
    
    //Appendices Dialogs
        FLHelpWindow*        fHelpWindow;        //Help Dialog
    
        FLErrorWindow*      fErrorWindow;       //Error Dialog
        FLPresentationWindow*            fPresWin;           //Presentation Window
        QDialog*            fCompilingMessage;   //Entertaining the user during long operations
        QDialog*            fVersionWindow;     //Not Active Window containing the versions of all used librairies
    
        FLServerHttp*       fServerHttp;        //Server that embbedes all HttpInterfaces in a droppable environnement
    
    //List of windows currently running in the application
        QList<FLWindow*>     FLW_List;           //Container of the opened windows
    
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
        
    //Save/Recall the recent Files/Sessions 
        void                save_Recent(QList<QString>& recents, const QString& pathToSettings);
    
        void                recall_Recent(QList<QString>& recents, const QString& pathToSettings);
    
    //Recent Files Parameters and functions
        QList<QString>        fRecentFiles;
        void                recall_Recent_Files();
        void                save_Recent_Files();
        void                set_Current_File(const QString& pathName);
        void                update_Recent_File();

    //Recent Sessions Parameters and functions
        QList<QString>         fRecentSessions;
        void                save_Recent_Sessions();
        void                recall_Recent_Sessions(); 
        void                set_Current_Session(const QString& pathName);
        void                update_Recent_Session();
    
    //
        QString             createWindowFolder(const QString& sessionFolder, int index);
        QString             copyWindowFolder(const QString& sessionNewFolder, int newIndex, const QString& sessionFolder, int index, map<int, int> indexChanges);
    
        void                cleanSHAFolder();
    
    //When the application is launched without parameter, this timer will start a initialized window
        QTimer*             fInitTimer;
        QTimer*             fEndTimer;
    
    //Preference Menu Objects and Functions
        FLPreferenceWindow*     fPrefDialog;     //Preference Window
        AudioCreator*           fAudioCreator;
    
        void                update_AudioArchitecture();
    
    //Save and Recall Session actions
		bool				fRecalling;		//True when recalling for the app not to close whith last window
    
    //In case of drops on Application Icon, this event is called
        virtual bool        event(QEvent *ev);
    
    //Functions of rehabilitation if sources disapears
        bool                recall_CurrentSession();
    
    //-----------------Questions about the current State

        FLWindow*           getActiveWin();
        FLWindow*           getWinFromHttp(int port);
    
    private slots :
    
#ifndef _WIN32
//--------Server Response
        FLWindow*           httpPortToWin(int port);
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
        void                connectWindowSignals(FLWindow* win);
        void                create_Empty_Window();
        FLWindow*                createWindow(int index, const QString& mySource, FLWinSettings* windowSettings, QString& error);
        void                open_New_Window();
        void                open_Example_From_FileMenu();
        void                open_Recent_File();
        void                open_Recent_File(const QString& toto);
        void                common_shutAction(FLWindow* win);
        void                display_Progress();
        void                close_Window_Action();

        void                deleteComponent();
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
        void                updateNavigateText();
        void                frontShow();
    
    //--------Window

        void                duplicate(FLWindow* window);
        void                duplicate_Window();
    
    //---------Preferences
        void                styleClicked(const QString& style);
        void                Preferences();
    
    //---------Help
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
    
    public slots :
    
        void                create_New_Window(const QString& name);
    
        virtual void        closeAllWindows();
        void                shut_AllWindows();
        void                update_CurrentSession();
        void                create_Component_Window();

    //--------Error received
        void                errorPrinting(const QString& msg);

};

#endif
