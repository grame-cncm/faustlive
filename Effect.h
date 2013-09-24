//
//  Effect.h
//  
//
//  Created by Sarah Denoux on 30/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _Effect_h
#define _Effect_h

#include "faust/llvm-dsp.h"
#include <string.h>

#include <QObject>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QTimer>

using namespace std;

class Effect : public QObject
{
    Q_OBJECT
    
    private:
    
    //Indicator of a special situation where the effect is build from saved bitcode
        bool                isRecalled;
    
    //File holding the Faust Code
        string              fichierSource;  
    //Name of the effect 
        string              nomEffet;       
    //Compilation Options of the Faust compiler
        string              compilationOptions; 
    //Optimization value for the llvm compilation
        int                 opt_level;      
       
    //Llvm Factory corresponding to file
        llvm_dsp_factory*   factory;        
    //When an effect factory is updated, the old one has to be kept, so that it can be desallocated    
        llvm_dsp_factory*   oldFactory;     
    
    //Watching the modifications made on the file (so that the factory can be automatically rebuild)
        QFileSystemWatcher* watcher;        
        
    //When the event of a modification is sent, it is offen send twice in a row and we want to act only once
        QTimer*             synchroTimer;   
    //Indicator the ensure the re-build of the factory due to a compilation  option change
        bool                forceSynchro;

    
    //The compilation options are decomposed in a table
        int         get_numberParameters(string compilOptions);
        //Extract the first sub-string in the compilOptions
        string&     parse_compilationParams(string& compilOptions);
    
    //Creating the factory with the specific compilation options, in case of an error the buffer is filled
        bool        buildFactory(llvm_dsp_factory** factoryToBuild, int opt_level, char* error, string currentSVGFolder, string currentIRFolder); 
        
    //The creationDate allows us to ensure that the signal of modification we receive from the watcher is really due to a change and not only the opening of the editor
        QDateTime   creationDate;
    
    public:
        Effect(bool recallVal, string sourceFile, string name);
        ~Effect();
    
    //Initialisation of the effect. From a source, it extracts the source file, the name and builds the factory
    //currentSVGFolder = where to create the SVG-Folder tied to the factory 
    //currentIRFolder = where to save the bitcode tied to the factory
    //Compilation Options = needed to build the llvm factory
    //Error = if the initialisation fails, the function returns false + the buffer is filled
        bool        init(string currentSVGFolder, string currentIRFolder , string compilationMode, int optVal, char* error);
    
    //Accessors to the Factory
        llvm_dsp_factory*   getFactory();
    //Re-Build of the factory from the source file
        bool        update_Factory(int opt_level, char* error, string currentSVGFolder, string currentIRFolder);
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
