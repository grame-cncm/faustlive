//
//  FLSessionManager.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _FLSessionManager_h
#define _FLSessionManager_h

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include <map>

typedef int (*RemoteDSPErrorCallback) (int error_code, void* arg);

class dsp;
class llvm_dsp_factory;
class remote_dsp_factory;
class FLWinSettings;

using namespace std;

enum
{
    TYPE_LOCAL, TYPE_REMOTE
};

union factory{
    llvm_dsp_factory*   fLLVMFactory;
    remote_dsp_factory* fRemoteFactory;
    
    factory(){
        fLLVMFactory = NULL;
        fRemoteFactory = NULL;
    }
};

struct factorySettings{
    factory*         fFactory;
    QString         fPath;
    QString         fName;
    int             fType;
};

class FLSessionManager : public QObject
{
    Q_OBJECT
    
private:

    QString             fSessionFolder;

    static FLSessionManager*       _sessionManager;
    
    QString             nameToUniqueName(const QString& name, const QString& path);
    QList<QString>      get_currentDefault();
    QString             find_smallest_defaultName();
    QString             getDeclareName(QString text);
    
    QString         getErrorFromCode(int code);
    
    bool            isSourceDSPPath(const QString& source);
    QString         ifFileToName(const QString& source);
    
    //--Transforms DSP file into faust string
    QString         ifFileToString(const QString& source);
    
    //--Transforms an Url into faust string
    QString         ifUrlToString(const QString& source);
    
    //--Transforms Wav file into faust string
    QString         ifWavToString(const QString& source);
    
    //--Shows restoration warning. 
    //----It returns true, in case "Yes" is chosen | false otherwise
    bool            viewRestorationMsg(const QString& msg, const QString& yesMsg, const QString& noMsg);
    
    void            copySHAFolder(const QString& snapshotFolder);
    
    const char**           getFactoryArgv(const QString& sourcePath, const QString& destPath, const QString& faustOptions, int& argc);
    
    const char**           getRemoteInstanceArgv(QSettings* winSettings, int& argc);
    
    QMap<dsp*, factorySettings*>  fDSPToFactory;
    
public:
    
    FLSessionManager(const QString& sessionFolder);
    ~FLSessionManager();
    
    static FLSessionManager* _Instance();
    static void createInstance(const QString homePath);
    static void deleteInstance();
    
    void            updateFolderDate(const QString& shaValue);
    void            cleanSHAFolder();
    
    QPair<QString, void*> createFactory(const QString& source, FLWinSettings* settings, QString& errorMsg);
    
    dsp* createDSP(QPair<QString, void*> factorySetts, const QString& source, FLWinSettings* settings, RemoteDSPErrorCallback error_callback, void* error_callback_arg, QString& errorMsg);

    void deleteDSPandFactory(dsp* toDeleteDSP);
    
    QString                 get_expandedVersion(QSettings* settings, const QString& source);
    
    QString                askForSourceSaving(const QString& sourceContent);
    QString                contentOfShaSource(const QString& shaSource);
    QString                saveTempFile(const QString& shaSource);
    
    void                saveCurrentSources(const QString& sessionFolder);
    map<int, QString>   currentSessionRestoration();
    void                createSnapshot(const QString& snapshotFolder);
    map<int, QString>   snapshotRestoration(const QString& snapshotFolder);
    
signals:
    void                error(const QString&);
};

#endif
