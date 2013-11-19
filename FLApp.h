//
//  FaustLiveApp.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLApp is the centerpiece of FaustLive. The class controls all the windows, menu and actions of a user. 

#ifndef _FaustLiveApp_h
#define _FaustLiveApp_h

#include <list>
#include <map>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

class FLServerHttp;
class FLExportManager;
class FLErrorWindow;
class FLWindow;
class FLEffect;
class AudioCreator;

using namespace std;

#define FLVERSION   "1.1"
#define DEFAULTNAME "DefaultName"

#define kExportUrlFile "/ServerURL.txt"

#define kMAXRECENTFILES 4
#define kMAXRECENTSESSIONS 3

//Keeping the information of a Window running in the current Session
//It provides an easy way of testing the session's content and writing the session file
struct WinInSession{
    
    int ID;         //Index of the window
    string source;  //Path of the source file
    string name;    //Name of the effect contained in the window
    float x;        //Position x on screen
    float y;
    string compilationOptions; //Compilation options tied to the effect contained in the window
    int opt_level;  //Optimization level for llvm compiler
    int portHttpd;
};

class FLApp : public QApplication
{
    Q_OBJECT
    
    private :
    
    //Menu Bar and it's sub-Menus
    
        QMenuBar *          fMenuBar;
        QMenu *             fFileMenu;
        QMenu *             fWindowMenu;
        QMenu *             fNavigateMenu;
        QMenu *             fHelpMenu;
    
        QAction*            fServer;
        QAction*            fServerStop;
        QAction*            fNewAction;
        QAction*            fOpenAction;
    
        QAction **          fOpenExamples;
        QMenu *             fMenuOpen_Example;
    
        QMenu*              fOpenRecentAction;
        QAction**           fRecentFileAction;
        
        QAction*            fExportAction;
        QAction*            fShutAction;
        QAction*            fShutAllAction;
        QAction*            fCloseAllAction;
    
        QAction*            fEditAction;
        QAction*            fPasteAction;
        QAction*            fDuplicateAction;
    
        QList<QAction*>     fFrontWindow;
    
        QAction*            fTakeSnapshotAction;
        QAction*            fRecallSnapshotAction;
        QAction*            fImportSnapshotAction;
        QMenu*              fRecallRecentAction;
        QAction**           fRrecentSessionAction;
        QMenu*              fImportRecentAction;
        QAction**           fIrecentSessionAction;
    
        QAction*            fHttpdViewAction;
        QAction*            fSvgViewAction;
    
        QAction*            fAboutQtAction;
        QAction*            fPreferencesAction;
        QAction*            fAboutAction; 
        QAction*            fVersionAction;
        QAction*            fPresentationAction;
    
        void                setup_Menu();
        void                redirectMenuToWindow(FLWindow* win);
    
        QProgressBar*       fPBar;   //Artificial progress bar to print a goodbye message
    
    //Appendices Dialogs
        QMainWindow*        fHelpWindow;        //Help Dialog
        QTextBrowser*       fToolText;          //ToolText in Help Dialog
        QPlainTextEdit*       fAppText;
        QPlainTextEdit*       fWinText;
    
        FLErrorWindow*      fErrorWindow;       //Error Dialog
        QDialog*            fPresWin;           //Presentation Window
        QDialog*            fCompilingMessage;   //Entertaining the user during long operations
        QDialog*            fVersionWindow;     //Not Active Window containing the versions of all used librairies
        FLExportManager*    fExportDialog;      //Manager for web service use
    
        FLServerHttp*       fServerHttp;        //Server that embbedes all HttpInterfaces in a droppable environnement
    
    //List of windows currently running in the application
        list<FLWindow*>     FLW_List;           //Container of the opened windows
        list<FLEffect*>     fExecutedEffects;    //This way, the effects already compiled can be recycled if their used further in the execution
    
    //Screen parameters
        int                 fScreenWidth;
        int                 fScreenHeight;
    
    //To index the windows, the smallest index not used is given to the window
    //With this index is calculate the place of the window on the screen
        int                 find_smallest_index(list<int> currentIndexes); 
        list<int>           get_currentIndexes();
        void                calculate_position(int index, int* x, int* y);
        list<string>        get_currentDefault();
        string              find_smallest_defaultName(list<string> currentDefault);
    
    //Application Parameters
        list<WinInSession*>  fSessionContent;    //Describes the state of the application 
    
        string              fWindowBaseName; //Name of Application
        string              fSessionFolder; //Path to currentSession Folder
        string              fSettingsFolder;   //Path to currentSettings Folder
        string              fSessionFile; //Path to currentSession DescriptionFile
        string              fSourcesFolder; //Folder with the copy of the sources
        string              fSVGFolder;   //Folder with the SVG processes
        string              fIRFolder;    //Folder with the Bitcode files
        
    //Recent Files Parameters and functions
        string              fRecentsFile;    //Path to  Recent Dsp file
        list<pair<string, string> >         fRecentFiles;
        void                recall_Recent_Files(string& filename);
        void                save_Recent_Files();
        void                set_Current_File(string& pathName, string& effName);
        void                update_Recent_File();
    
    //When the application is launched without parameter, this timer will start a initialized window
        QTimer*             fInitTimer;
        QTimer*             fEndTimer;

    //To be stored in a file, the compilation options have to be converted
        string              convert_compilationOptions(string compilationOptions);
        string              restore_compilationOptions(string   compilationOptions);
    
    //Preference Menu Objects and Functions
        QDialog*            fPrefDialog;     //Preference Window
        AudioCreator*       fAudioCreator;
        QGroupBox*          fAudioBox;
    
        string              fHomeSettings;       //Path of settings file
    
        QLineEdit*          fCompilModes;
        QLineEdit*          fOptVal;
        QLineEdit*          fServerLine;
        QLineEdit*          fPortLine;
    
        string              fCompilationMode;
        int                 fOpt_level;
        string              fServerUrl;
        int                 fPort;
        
        string              fStyleChoice;
    
        void                init_PreferenceWindow();
        void                save_Settings(string& home);
        void                recall_Settings(string& home);
    
        void                update_AudioArchitecture();
    
    //Setups of help menu and the presentation interface
        void                init_HelpWindow();
        void                init_presentationWindow();
    //If the user chooses to open an example, it is stored 
        string              fExampleToOpen;

    //Recent Sessions Parameters and functions
        string              fHomeRecentSessions;
        QStringList         fRecentSessions;
        void                save_Recent_Sessions();
        void                recall_Recent_Sessions(string& filename); 
        void                set_Current_Session(string& pathName);
        void                update_Recent_Session();
        
    //Save and Recall Session actions
        void                recall_Session(string filename);
        void                addWinToSessionFile(FLWindow* win);
        void                deleteWinFromSessionFile(FLWindow* win);
        void                update_CurrentSession();
    
    //In case of an import, those steps are necessary to modify the session before opening it
        list<std::pair<int, int> >  establish_indexChanges(list<WinInSession*>* session);
        void                copy_WindowsFolders(string& srcDir, string& dstDir, list<std::pair<string,string> > indexChanges);
        void                copy_AllSources(string& srcDir, string& dstDir, list<std::pair<string,string> > nameChanges, string extension);
        void                copy_SVGFolders(string& srcDir, string& dstDir, list<std::pair<string,string> > nameChanges);
    
        void                establish_sourceChanges(list<std::pair<string,string> > nameChanges, list<WinInSession*>* session);
    
        list<std::pair<string,string> > establish_nameChanges(list<WinInSession*>* session);
    
        void                deleteLineIndexed(int index);
    
    //In case of drops on Application Icon, this event is called
        virtual bool        event(QEvent *ev);
    
    //Functions of read/write of a session description file
        void                sessionContentToFile(string filename);
        void                fileToSessionContent(string filename, list<WinInSession*>* session);
    
    //Reset of the Folders contained in the current Session Folder
        void                reset_CurrentSession();
    
    //Functions of rehabilitation if sources disapears
        void                currentSessionRestoration(list<WinInSession*>* session);
        void                snapshotRestoration(string& file, list<WinInSession*>* session);
    
    //-----------------Creation of Effects

    string                  ifUrlToText(string& source);
    //When the source of the effect is not a file but text, it has to be stored in a file
    void                    createSourceFile(string& sourceName, string& content);
    //Updating the content of the backup file 
    void                    update_Source(string& oldSource, string& newSource);
    
    string                  getDeclareName(string text);
    
    string                  renameEffect(string source, string nomEffet);
    
    FLEffect*               getEffectFromSource(string& source, string& nameEffect, string& sourceFolder, string compilationOptions, int optVal, string& error, bool init);
    
    //-----------------Questions about the current State
    
        bool                isIndexUsed(int index, list<int> currentIndexes);
        bool                isEffectInCurrentSession(string sourceToCompare);
        string              getNameEffectFromSource(string sourceToCompare);
        bool                isEffectNameInCurrentSession(string& sourceToCompare, string& name);
        list<string>        getNameRunningEffects();
        list<int>           WindowCorrespondingToEffect(FLEffect* eff);
        void                removeFilesOfWin(string sourceName, string effName);

        FLWindow*           getActiveWin();
        FLWindow*           getWinFromHttp(int port);
    
    private slots :
        
        void                launch_Server();
    
    //---------Drop on a window
    
        void                update_SourceInWin(FLWindow* win, string source);
        void                drop_Action(list<string>);
    
    //---------Presentation Window Slots
    
        void                open_Example_Action();
        void                new_Window_pres();
        void                open_Window_pres();
        void                open_Session_pres();

    //---------Click On an example
        void                itemClick(QListWidgetItem *item);
        void                itemDblClick(QListWidgetItem* item);
    
    //---------File
        FLWindow*           new_Window(string& source, string& error);
        void                create_Empty_Window();
        void                open_New_Window();
        void                open_Recent_File();
        void                open_Recent_File(string toto);
        void                shut_Window(); 
        void                shut_AllWindows();
        virtual void        closeAllWindows();
        void                common_shutAction(FLWindow* win);
        void                display_Progress();
        void                close_Window_Action();

    //--Session
        void                take_Snapshot();
        void                recall_Snapshot(string filename, bool importOption);
        void                recallSnapshotFromMenu();
        void                importSnapshotFromMenu();
        void                recall_Recent_Session();
        void                import_Recent_Session();
    
    //--------Navigate
    
        void                frontShow(QString name);
    
    //--------Window
        void                edit(FLWindow* win);
        void                edit_Action();
    
        void                synchronize_Window();
    
        void                paste(FLWindow* win);
        void                paste_Text();
        void                duplicate(FLWindow* window);
        void                duplicate_Window();
    
        void                viewHttpd(FLWindow* win);
        void                httpd_View_Window();
        void                viewSvg(FLWindow* win);
        void                svg_View_Action();
    
        void                export_Win(FLWindow* Win);
        void                export_Action();
    
    //--------RightClickEvent
        void                redirect_RCAction(const QPoint & p);
    
    //---------Preferences
        void                styleClicked();
        void                styleClicked(string style);
        void                Preferences();
        void                save_Mode();
        void                cancelPref();
    
    //---------Help
        void                setToolText(const QString&);
        void                setAppPropertiesText(const QString& currentText);
        void                setWinPropertiesText(const QString& currentText);
        void                apropos();
        void                end_apropos();
        void                version_Action();
        void                show_presentation_Action();
    
    //--------Timers
        void                init_Timer_Action();
        void                update_ProgressBar();
    
    //--------Long operations entertainment
        void                display_CompilingProgress(string msg);
        void                StopProgressSlot(); 
    
    //--------Error received
        void                errorPrinting(const char* msg);
    
    //--------Server Response
//        void                close_Window_FormHttp(const char* nameEffect);
        void                compile_HttpData(const char* data, int port);
        void                stop_Server();
    
    public : 

                            FLApp(int& argc, char** argv);
        virtual             ~FLApp();
    
        void                create_New_Window(string& name);

};

#endif
