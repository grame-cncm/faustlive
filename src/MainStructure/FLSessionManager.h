//
//  FLSessionManager.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//
// The role of the session manager is to be the compilation center and maintain the hierarchy of the current session (save and recall it).


// The hierarchie is as follow:
// - Settings.ini : file containing the saved settings of the application 
// - Examples : folder containing a copy of the example files of the Faust distribution 
// - Libs : folder containing a copy of the libraries of the Faust distribution 
// - Windows : folder containing the window-specific folders 
//      - FLW-i : window-specific folder
//          - Settings.ini : the window specific settings
//          - Graphics.rc : file saving the graphical parameters of the last DSP contained in the window
//          - Connections.jc : file saving the last known Jack connections of the window
//          - SHAKey.dsp : copy of the Faust code of the last DSP contained in the window
// - SHAFolder : folder containing the DSP-specific folders
//      - SHAKey = DSP-specific folder
//          – SHAKey* : LLVM intermediate representation of the DSP 
//          – SHAKey.dsp*: copy of the Faust code corresponding to this SHAKey 
//          – SHAKey-svg* : folder containing the svg diagram resources

#ifndef _FLSessionManager_h
#define _FLSessionManager_h

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include <QtNetwork>
#include <map>

// THI IS UGLY !!! (to fix...)
#ifdef REMOTE
#include "faust/dsp/remote-dsp.h"
#else
typedef int (*remoteDSPErrorCallback) (int error_code, void* arg);
#endif

#if defined(_WIN32) && !defined(GCC)
# pragma warning (disable: 4100 4267)
#else
# pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#define LLVM_DSP_FACTORY

#ifdef LLVM_DSP_FACTORY
#include "faust/dsp/poly-dsp-tools.h"
#include "faust/dsp/llvm-dsp.h"
#else
#include "faust/dsp/interpreter-dsp.h"
#endif

class FLWinSettings;

using namespace std;

enum {
    TYPE_LOCAL, TYPE_REMOTE
};

union factory {
    dsp_poly_factory* fLLVMFactory;
    
#ifdef REMOTE
    remote_dsp_factory* fRemoteFactory;
#endif
    
    factory() {
        fLLVMFactory = NULL;
    #ifdef REMOTE
        fRemoteFactory = NULL;
    #endif
    }
};

struct factorySettings {
    factory*        fFactory;
    QString         fPath;
    QString         fName;
    int             fType;
};

class rtmidi;

class FLSessionManager : public QObject
{
    
    private:

        Q_OBJECT
        
        QString         fSessionFolder;
 
        static FLSessionManager* _sessionManager;
        
//------ Handle name giving 
        QString         getDeclareName(QString text, QString default_name);
        
        QString         getErrorFromCode(int code);
        
        bool            isSourceDSPPath(const QString& source);
        QString         ifFileToName(const QString& source);
        
        //--Transforms into faust string
        QString         ifFileToString(const QString& source);
        QString         ifUrlToString(const QString& source);
        QString         ifGoogleDocToString(const QString& source);
        
        //--Shows restoration warning. 
        //----It returns true, in case "Yes" is chosen | false otherwise
        bool            viewRestorationMsg(const QString& msg, const QString& yesMsg, const QString& noMsg);
        
        void            copySHAFolder(const QString& snapshotFolder);
        
        const char**    getFactoryArgv(const QString& sourcePath, const QString& faustOptions, QSettings* settings, int& argc);
        
        const char**    getRemoteInstanceArgv(QSettings* winSettings, int& argc);
        void            deleteArgv(int argc, const char** argv);
            
        QMap<dsp*, factorySettings*>  fDSPToFactory;
    
    #ifdef REMOTE
        QMap<QString, remote_dsp_factory*>  fPublishedFactories;
    #endif
    
        void cleanSHAFolder();
        
        QVector<QString> getDependencies(dsp_factory* factoryDependency);
        
    private slots:
    
        void receiveDSP();
        void networkError(QNetworkReply::NetworkError);
        
    public:
        
        FLSessionManager(const QString& sessionFolder);
        virtual ~FLSessionManager();
        
        static FLSessionManager* _Instance();
        
        static void createInstance(const QString homePath);
        static void deleteInstance();

        void updateFolderDate(const QString& shaValue);
        
        bool generateAuxFiles(const QString& shaKey, const QString& sourcePath, const QString& faustOptions, const QString& name, QString& error);
        bool generateSVG(const QString& shaKey, const QString& sourcePath, const QString& svgPath, const QString& name, QString& errorMsg);
        
        QPair<QString, void*> createFactory(const QString& source, FLWinSettings* settings, QString& errorMsg);
        
        dsp* createDSP(QPair<QString, void*> factorySetts, 
                        const QString& source, FLWinSettings* settings,
                        remoteDSPErrorCallback error_callback, 
                        void* error_callback_arg, 
                        QString& errorMsg);

        void deleteDSPandFactory(dsp* toDeleteDSP);
        
        QString             getExpandedVersion(QSettings* settings, const QString& source);
        
        QVector<QString>    readDependencies(const QString& shaValue);
        void                writeDependencies(QVector<QString> dependencies, const QString& shaValue);
        
        QString             askForSourceSaving(const QString& sourceContent);
        QString             contentOfShaSource(const QString& shaSource);
        
        void                saveCurrentSources(const QString& sessionFolder);
        map<int, QString>   currentSessionRestoration();
        void                createSnapshot(const QString& snapshotFolder);
        map<int, QString>   snapshotRestoration(const QString& filename);
        
    
/* This is a not finished attempt to implement a publish service of DSP to make them accessible from the outside */
//      #ifdef REMOTE
//          bool addWinToServer(FLWinSettings* settings);
//          void deleteWinFromServer(FLWinSettings* settings);
//      #endif
    
    
    signals:
    
        void                error(const QString&);
};

#endif
