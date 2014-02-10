//
//  FaustLiveApp.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// FLApp is the centerpiece of FaustLive. The class controls all the windows, menu and actions of a user. 

#ifndef _FaustLiveApp_h
#define _FaustLiveApp_h

//#include <list>
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

#define FLVERSION   "1.5"
#define DEFAULTNAME "DefaultName"

#define kExportUrlFile "/ServerURL.txt"

#define kMAXRECENTFILES 4
#define kMAXRECENTSESSIONS 3

//Keeping the information of a Window running in the current Session
//It provides an easy way of testing the session's content and writing the session file
struct WinInSession{
    
    int ID;         //Index of the window
    QString source;  //Path of the source file
    QString name;    //Name of the effect contained in the window
    float x;        //Position x on screen
    float y;
    QString compilationOptions; //Compilation options tied to the effect contained in the window
    int opt_level;  //Optimization level for llvm compiler
    int portHttpd;
    int oscPort;
    
    bool isLocal;
    
	QString ipServer;
    int portServer;
    
};

class FLApp : public QApplication
{
    Q_OBJECT
    
    private :
    
    //Menu Bar and it's sub-Menus
    
        QMenuBar *          fMenuBar;
    
        QAction**           fRecentFileAction;
        QAction**           fRrecentSessionAction;
        QAction**           fIrecentSessionAction;
    
        QList<QString>     fFrontWindow;

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
    
        //FLServerHttp*       fServerHttp;        //Server that embbedes all HttpInterfaces in a droppable environnement
    
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
        QList<QString>        get_currentDefault();
        QString              find_smallest_defaultName(QList<QString> currentDefault);
    
    //Application Parameters
        QList<WinInSession*>  fSessionContent;    //Describes the state of the application 
    
        QString              fWindowBaseName; //Name of Application
    
        void                create_Session_Hierarchy();
    
        QString              fSessionFolder; //Path to currentSession Folder
        QString              fSettingsFolder;   //Path to currentSettings Folder
        QString              fSessionFile; //Path to currentSession DescriptionFile
        QString              fSourcesFolder; //Folder with the copy of the sources
        QString              fSVGFolder;   //Folder with the SVG processes
        QString              fIRFolder;    //Folder with the Bitcode files
        QString              fExamplesFolder;    //Folder containing Examples copied from QResources
        
    //Recent Files Parameters and functions
        QString              fRecentsFile;    //Path to  Recent Dsp file
        QList<std::pair<QString, QString> >         fRecentFiles;
        void                recall_Recent_Files(const QString& filename);
        void                save_Recent_Files();
        void                set_Current_File(const QString& pathName, const QString& effName);
        void                update_Recent_File();
    
    //When the application is launched without parameter, this timer will start a initialized window
        QTimer*             fInitTimer;
        QTimer*             fEndTimer;

    //To be stored in a file, the compilation options have to be converted
        QString              convert_compilationOptions(QString compilationOptions);
        QString              restore_compilationOptions(QString   compilationOptions);
    
    //Preference Menu Objects and Functions
        QDialog*            fPrefDialog;     //Preference Window
        AudioCreator*       fAudioCreator;
        QGroupBox*          fAudioBox;
    
        QString              fHomeSettings;       //Path of settings file
    
        QLineEdit*          fCompilModes;
        QLineEdit*          fOptVal;
        QLineEdit*          fServerLine;
        QLineEdit*          fPortLine;
    
        QString              fCompilationMode;
        int                 fOpt_level;
        QString              fServerUrl;
        int                 fPort;
        
        QString              fStyleChoice;
    
        void                init_PreferenceWindow();
        void                save_Settings(const QString& home);
        void                recall_Settings(const QString& home);
    
        void                update_AudioArchitecture();
    
    //Setups of help menu and the presentation interface
        void                init_HelpWindow();
        void                init_presentationWindow();
    //If the user chooses to open an example, it is stored 
        QString              fExampleToOpen;

    //Recent Sessions Parameters and functions
        QString              fHomeRecentSessions;
        QStringList         fRecentSessions;
        void                save_Recent_Sessions();
        void                recall_Recent_Sessions(const QString& filename); 
        void                set_Current_Session(const QString& pathName);
        void                update_Recent_Session();
        
    //Save and Recall Session actions
		bool				fRecalling;		//True when recalling for the app not to close whith last window
        void                recall_Session(const QString& filename);
        void                addWinToSessionFile(FLWindow* win);
        void                deleteWinFromSessionFile(FLWindow* win);
        void                update_CurrentSession();
    
    //In case of an import, those steps are necessary to modify the session before opening it
        QList<std::pair<int, int> >  establish_indexChanges(QList<WinInSession*>* session);
        void                copy_WindowsFolders(const QString& srcDir, const QString& dstDir, QList<std::pair<QString,QString> > indexChanges);
        void                copy_AllSources(const QString& srcDir, const QString& dstDir, QList<std::pair<QString,QString> > nameChanges, const QString extension);
        void                copy_SVGFolders(const QString& srcDir, const QString& dstDir, QList<std::pair<QString,QString> > nameChanges);
    
        void                establish_sourceChanges(QList<std::pair<QString,QString> > nameChanges, QList<WinInSession*>* session);
    
        QList<std::pair<QString,QString> > establish_nameChanges(QList<WinInSession*>* session);
    
        void                deleteLineIndexed(int index);
    
    //In case of drops on Application Icon, this event is called
        virtual bool        event(QEvent *ev);
    
    //Functions of read/write of a session description file
        void                sessionContentToFile(const QString& filename);
        void                fileToSessionContent(const QString& filename, QList<WinInSession*>* session);
    
    //Reset of the Folders contained in the current Session Folder
        void                reset_CurrentSession();
    
    //Functions of rehabilitation if sources disapears
        void                currentSessionRestoration(QList<WinInSession*>* session);
        void                snapshotRestoration(const QString& file, QList<WinInSession*>* session);
    
    //-----------------Creation of Effects

    QString                  ifUrlToText(const QString& source);
    //When the source of the effect is not a file but text, it has to be stored in a file
    void                    createSourceFile(const QString& sourceName, const QString& content);
    //Updating the content of the backup file 
    void                    update_Source(const QString& oldSource, const QString& newSource);
    
    QString                  getDeclareName(QString text);
    QString                  renameEffect(const QString& source, const QString& nomEffet, bool isRecalledEffect);
    
    FLEffect*               getEffectFromSource(QString source, QString nameEffect, const QString& sourceFolder, QString& compilationOptions, int optVal, QString& error, bool init, bool isLocal, const QString& ip = "localhost", int port= 0);
    
    //-----------------Questions about the current State
    
        bool                isIndexUsed(int index, QList<int> currentIndexes);
        bool                isLocalEffectInCurrentSession(const QString& sourceToCompare);
        bool                isRemoteEffectInCurrentSession(const QString& sourceToCompare, const QString& ip, int port);
        bool                isSourceInCurrentSession(const QString& sourceToCompare);

        QString              getNameEffectFromSource(const QString& sourceToCompare);
        bool                isEffectNameInCurrentSession(const QString& sourceToCompare, const QString& name, bool isRecalledEffect);
        QList<QString>        getNameRunningEffects();
        QList<int>           WindowCorrespondingToEffect(FLEffect* effect);
        void                removeFilesOfWin(const QString& sourceName, const QString& effName);

        FLWindow*           getActiveWin();
        FLWindow*           getWinFromHttp(int port);
    
    private slots :
        
        void                launch_Server();
    
    //---------Drop on a window
    
        void                update_SourceInWin(FLWindow* win, const QString& source);
        void                drop_Action(QList<QString>);
    
    
    //--------Switch to remote processing
        bool    migrate_ProcessingInWin(QString ip, int port);
    
    //---------Presentation Window Slots
    
        void                open_Example_Action(QString pathInQResource);
        void                open_Example_Action();
        void                new_Window_pres();
        void                open_Window_pres();
        void                open_Session_pres();

    //---------Click On an example
        void                itemClick(QListWidgetItem *item);
        void                itemDblClick(QListWidgetItem* item);
    
    //---------File
        FLWindow*           new_Window(const QString& mySource, QString& error);
        void                create_Empty_Window();
        void                open_New_Window();
        void                open_Recent_File();
        void                open_Recent_File(const QString& toto);
        void                shut_Window(); 
        void                shut_AllWindows();
        virtual void        closeAllWindows();
        void                common_shutAction(FLWindow* win);
        void                display_Progress();
        void                close_Window_Action();

    //--Session
        void                take_Snapshot();
        void                recall_Snapshot(const QString& filename, bool importOption);
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
    
    //---------Preferences
        void                styleClicked();
        void                styleClicked(const QString& style);
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
        void                display_CompilingProgress(const QString& msg);
        void                StopProgressSlot(); 
    
    //--------Error received
        void                errorPrinting(const char* msg);
    
    //--------Server Response
        void                compile_HttpData(const char* data, int port);
        void                stop_Server();
    
    public : 

                            FLApp(int& argc, char** argv);
        virtual             ~FLApp();
    
        void                create_New_Window(const QString& name);

};

#endif
