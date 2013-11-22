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
#include "faust/llvm-dsp.h"

using namespace std;

class FLEffect : public QObject
{
    Q_OBJECT
    
    private:
    
    //Indicator of a special situation where the effect is build from saved bitcode
        bool                fRecalled;
    
    //File holding the Faust Code
        string              fSource;  
    //Name of the effect 
        string              fName;       
    //Compilation Options of the Faust compiler
        string              fCompilationOptions; 
    //Optimization value for the llvm compilation
        int                 fOpt_level;      
       
    //Llvm Factory corresponding to file
        llvm_dsp_factory*   fFactory;        
    //When an effect factory is updated, the old one has to be kept, so that it can be desallocated    
        llvm_dsp_factory*   fOldFactory;     
    
    //Watching the modifications made on the file (so that the factory can be automatically rebuild)
        QFileSystemWatcher* fWatcher;        
        
    //When the event of a modification is sent, it is offen send twice in a row and we want to act only once
        QTimer*             fSynchroTimer;  
    
    //Indicator that ensures that the re-build of the factory is only due to a compilation option change 
        bool                fForceSynchro;

    //The creationDate allows us to ensure that the signal of modification we receive from the watcher is really due to a change and not only the opening of the editor
        QDateTime           fCreationDate;
    
    
    //The compilation options are decomposed in a table
        int         get_numberParameters(const string& compilOptions);
        //Extract the first sub-string in the compilOptions
        string     parse_compilationParams(string& compilOptions);
    
    //Creating the factory with the specific compilation options, in case of an error the buffer is filled
        bool        buildFactory(llvm_dsp_factory** factoryToBuild, string& error, string currentSVGFolder, string currentIRFolder); 
    
    public:
        FLEffect(bool recallVal, string sourceFile, string name = "");
        ~FLEffect();
    
    //Initialisation of the effect. From a source, it extracts the source file, the name and builds the factory
    //currentSVGFolder = where to create the SVG-Folder tied to the factory 
    //currentIRFolder = where to save the bitcode tied to the factory
    //Compilation Options = needed to build the llvm factory
    //Error = if the initialisation fails, the function returns false + the buffer is filled
        bool        init(string currentSVGFolder, string currentIRFolder , string compilationMode, int optVal, string& error);
    
    //Accessors to the Factory
        llvm_dsp_factory*   getFactory();
    //Re-Build of the factory from the source file
        bool        update_Factory(string& error, string currentSVGFolder, string currentIRFolder);
    //Once the rebuild is complete, the former factory has to be deleted
        void        erase_OldFactory();
    
    //Accessors to source, name, watcher, ...
        string      getSource();
        void        setSource(string file);
    
        QDateTime   get_creationDate();
        
        string      getName();
        void        setName(string name);
    
        string      getCompilationOptions();
        void        update_compilationOptions(string& compilOptions, int newOptValue);
    
        int         getOptValue();
    
        bool        isSynchroForced();
        void        setForceSynchro(bool val);
    //When any action on the effect is performed, the watcher has to be stopped (and then re-launched) otherwise the synchronisation is called without good reason
        void        stop_Watcher();
        void        launch_Watcher();
    
    //The source file is copied in the sourceFolder in case of a source loss
//        void        copySourceFile(string& sourceFolder);
            
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
