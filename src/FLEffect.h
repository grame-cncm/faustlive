//
//  FLEffect.h
//  
//
//  Created by Sarah Denoux on 30/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// An Effect takes care of the compilation of a DSP. Moreover, it is notify in case, the DSP has been modified. 

#ifndef _FLEffect_h
#define _FLEffect_h

#include <string.h>
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

class llvm_dsp_factory;
class remote_dsp_factory;

using namespace std;

enum{
    kCurrentFactory,
    kChargingFactory
};

class FLEffect : public QObject
{
    Q_OBJECT
    
    private:
    
    //Indicator of a special situation where the effect is build from saved bitcode
        bool                fRecalled;
        
    //Indicator of forced recompilation in case of source changes
        bool                fRecompilation;
    
    //File holding the Faust Code
        QString              fSource;  
    //Name of the effect 
        QString              fName;       
    //Compilation Options of the Faust compiler
        QString              fCompilationOptions; 
    //Optimization value for the llvm compilation
        int                 fOpt_level;      
    
    //Current Session Folders
        QString              fCurrentSVGFolder;
        QString              fCurrentIRFolder;
    
    // IP + Port of remote server for remote Effects
        QString              fIPaddress;
        int                  fPort;
       
    //Llvm Factory corresponding to file
        llvm_dsp_factory*   fFactory;        
    //When an effect factory is updated, the old one has to be kept, so that it can be desallocated    
        llvm_dsp_factory*   fOldFactory;     
    
    //Llvm Factory corresponding to file
        remote_dsp_factory*   fRemoteFactory;        
    //When an effect factory is updated, the old one has to be kept, so that it can be desallocated    
        remote_dsp_factory*   fOldRemoteFactory;     
    
    //Watching the modifications made on the file (so that the factory can be automatically rebuild)
        QFileSystemWatcher* fWatcher;        
        
    //When the event of a modification is sent, it is offen send twice in a row and we want to act only once
        QTimer*             fSynchroTimer;  
    
    //Indicator that ensures that the re-build of the factory is only due to a compilation option change 
        bool                fForceSynchro;

    //The creationDate allows us to ensure that the signal of modification we receive from the watcher is really due to a change and not only the opening of the editor
        QDateTime           fCreationDate;
    
    //Indicates if the effect holds a local or remote factory
        bool                fIsLocal;
    //Attributes of Processing Machine
        QString              fIpMachineRemote;
        int                 fPortMachineRemote;
    
    
    //The compilation options are decomposed in a table
        int        get_numberParameters(const QString& compilOptions);
        //Extract the first sub-string in the compilOptions
        string     parse_compilationParams(QString& compilOptions);
    
    //Creating the factory with the specific compilation options, in case of an error the buffer is filled
    //@param factoryToBuild = is it the transition factory or the current factory ? 
    //@param        
        bool        buildFactory(int factoryToBuild, QString& error, QString currentSVGFolder, QString currentIRFolder); 
    
    public:
    
    //###CONSTRUCTOR
    //@param : isEffectRecalled = is Effect created from a session recalling situation
    //@param : sourceFile = source of effect
    //@param : name = name of the created effect
    //@param : isLocal = is it processing on local or remote machine
        FLEffect(bool recallVal, const QString& sourceFile, const QString& name = "", bool isLocal = true);
        ~FLEffect();
        void reset();
        bool reinit(QString& error);
    
    //Initialisation of the effect. From a source file, it builds the factory
    //@param : currentSVGFolder = where to create the SVG-Folder tied to the factory 
    //@param : currentIRFolder = where to save the bitcode tied to the factory
    //@param : compilation Options = options to create the factory in faust compiler
    //@param : optvalue = optimization value for LLVM compiler
    //@param : error = if the initialisation fails, error is filled
    //@param : ip/port remote = IP/Port of processing machine (Remote Case)
        bool        init(const QString& currentSVGFolder, const QString& currentIRFolder , QString compilationMode, int optVal, QString& error, const QString& IPremote = "localhost", int portremote = 0);

    //Accessors to the Factory
        llvm_dsp_factory*   getFactory();
        remote_dsp_factory*   getRemoteFactory();
    
    //In case source modified outside of session use
        void        forceRecompilation(bool val);
        bool        hasToBeRecompiled();
    //Re-Build of the factory from the source file
        bool        update_Factory(QString& error, QString currentSVGFolder, QString currentIRFolder);
    //Once the rebuild is complete, the former factory has to be deleted
        void        erase_OldFactory();
    
    //Accessors to source, name, watcher, ...
        QString      getSource();
        void        setSource(QString file);
    
        QDateTime   get_creationDate();
        
        QString      getName();
        void        setName(QString name);
    
        QString      getCompilationOptions();
        void        update_compilationOptions(QString& compilOptions, int newOptValue);
    
        void        update_remoteMachine(const QString& ip, int port);
    
        int         getOptValue();
    
        QString      getRemoteIP();
        int         getRemotePort();
    
        bool        isSynchroForced();
        void        setForceSynchro(bool val);
        
        bool        isLocal();
    //When any action on the effect is performed, the watcher has to be stopped (and then re-launched) otherwise the synchronisation is called without good reason
        void        stop_Watcher();
        void        launch_Watcher();
            
    signals :     
    //When the watcher locates a modification on the file, the signal is emited
        void        effectChanged();
    
    public slots :
    //Sends the effectChanged signal 
        void        effectModified();
    //Ensures of that the signal of modification is sent only once when received twice in a row
        void        reset_Timer(const QString toto);
};

#endif
