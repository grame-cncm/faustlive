//
//  Effect.cpp
//  
//
//  Created by Sarah Denoux on 30/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "Effect.h"

#include <QFileSystemWatcher>
#include <QTimer>
#include <QObject>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

#define LIBRARY_PATH "/Resources/Libs/"


/***********************************EFFECT IMPLEMENTATION*********************************/

Effect::Effect(bool recallVal, string sourceFile, string name = ""){
    
    factory = NULL;
    oldFactory = NULL;
    watcher = NULL;
    synchroTimer = NULL;
    fichierSource = sourceFile;
    nomEffet = name;
    forceSynchro = false;
    isRecalled = recallVal; //Effect is build in a recall situation
}

Effect::~Effect(){
    
    delete synchroTimer;
    delete watcher;
    //    printf("deleting factory = %p\n", factory);
    deleteDSPFactory(factory);
}

//--------------INITIALISATION FUNCTIONS
bool Effect::init(string currentSVGFolder, string currentIRFolder ,string compilationMode, int optValue, char* error){
    
    compilationOptions = compilationMode;
    opt_level = optValue;
    
    bool sucess = buildFactory(&factory, opt_level, error, currentSVGFolder, currentIRFolder);
    
    if(sucess){
        
        //Initializing watcher looking for changes on DSP through text editor 
        watcher = new QFileSystemWatcher(this);
        synchroTimer = new QTimer(watcher);
        connect(synchroTimer, SIGNAL(timeout()), this, SLOT(effectModified()));
        
        connect(watcher, SIGNAL(fileChanged(const QString)), this, SLOT(reset_Timer(const QString)));
        
        return true;
    }
    
    else
        return false;
}

//---------------FACTORY ACTIONS

bool Effect::buildFactory(llvm_dsp_factory** factoryToBuild, int opt_level, char* error, string currentSVGFolder, string currentIRFolder){
    
    //+2 = Path to DSP + -svg to build the svg Diagram
    int argc = 2 + get_numberParameters(compilationOptions);
    
    char ** argv;
    argv = new char*[argc];
    
    //Parsing the compilationOptions from a string to a char**
    string copy = compilationOptions;
    for(int i=2; i<argc; i++){
        
        string parseResult = parse_compilationParams(copy);
        argv[i] = new char[parseResult.length()+1];
        strcpy(argv[i], parseResult.c_str());
    }
    
    argv[0] = new char[fichierSource.length() + 1];
    strcpy(argv[0], fichierSource.c_str());
    argv[1] = new char[5];
    strcpy(argv[1], "-svg");
    
    const char** argument = (const char**) argv;
    
    //The library path is where libraries like the scheduler architecture file are = Application Bundle/Resources
    char libraryPath[256];
    
    snprintf(libraryPath, 255, "%s%s", QFileInfo(QFileInfo( QCoreApplication::applicationFilePath()).absolutePath()).absolutePath().toStdString().c_str(), LIBRARY_PATH);
    
    //    printf("libraryPath = %s\n", libraryPath);
    
    string path = currentIRFolder + "/" + nomEffet;
    printf("PATH TO IR = %s\n", path.c_str());
    
    //To speed up the process of compilation, when a session is recalled, the DSP are re-compiled from Bitcode 
    if(isRecalled && QFileInfo(path.c_str()).exists()){        
        *factoryToBuild = readDSPFactoryFromBitcodeFile(path, "");
        
        printf("factory from IR = %p\n", *factoryToBuild);
    }
    
    isRecalled = false;
    
    if(*factoryToBuild == NULL){
        
        *factoryToBuild = createDSPFactory(argc , argument, libraryPath, currentSVGFolder, "", "", "", error, opt_level);
        
        delete [] argv;
        
        //The creation date is nedded in case the text editor sends a message of modification when actually the file has only been opened. It prevents recompilations for bad reasons
        creationDate = creationDate.currentDateTime();
        
        if(*factoryToBuild != NULL){
            
            if(!QFileInfo(currentIRFolder.c_str()).exists()){
                QDir direct(currentIRFolder.c_str());
                direct.mkdir(currentIRFolder.c_str());
            }
            //The Bitcode files are written at each compilation 
            writeDSPFactoryToBitcodeFile(*factoryToBuild, path);
            
            return true;
        }
        else
            return false;
        //    printf("NEW FACTORY = %p\n", factory);
    }
    else{
        delete [] argv;
        creationDate = creationDate.currentDateTime();
        return true;
    }
    
}

int Effect::get_numberParameters(string compilOptions){
    
    string copy = compilOptions;
    
    int argc = 0;
    int pos = 0;
    
    if(copy.find("-") == string::npos){
        return 0;
    }
    
    while(copy.find(" ", pos+1) != string::npos){
        argc++;
        pos = copy.find(" ", pos+1);
    }
    
    return argc+1;
    
}

string& Effect::parse_compilationParams(string& compilOptions){
    
    //Hand Made Parser = a ' ' means a separation between parameters. If there are none and still there are compilation Options = it's the last one but it has to be taken into account anyway!    
    
    string returning = "";
    
    int pos = compilOptions.find(" ");
    
    if(pos != string::npos){
        returning = compilOptions.substr(0, pos);
        compilOptions.erase(0, pos+1);
    }
    else if(compilOptions.compare("") != 0)
        returning = compilOptions.substr(0, compilOptions.length());
    
    return returning;
}

bool Effect::update_Factory(int opt_level, char* error, string currentSVGFolder, string currentIRFolder){
    
    oldFactory = factory;
    
    llvm_dsp_factory* factory_update = NULL;
    
    if(buildFactory(&factory_update, opt_level, error, currentSVGFolder, currentIRFolder)){
        factory = factory_update;
        return true;
    }
    else
        return false;
}

void Effect::erase_OldFactory(){
    
    //    printf("delete Factory = %p\n", oldFactory);
    deleteDSPFactory(oldFactory);
}

//---------------WATCHER & FILE MODIFICATIONS ACTIONS

void Effect::reset_Timer(const QString toto){
    
    //    printf("Reseting Timer\n");
    
    //If the signal is triggered multiple times in 2 second, only 1 is taken into account
    if(synchroTimer->isActive()){
        synchroTimer->stop();
        synchroTimer->start(2000);
    }
    else
        synchroTimer->start(2000);
}

void Effect::effectModified(){
    synchroTimer->stop();
    //    printf("Emission EffectChanged\n");
    emit effectChanged();
}

void Effect::stop_Watcher(){
    printf("PATH STOP WATCHING = %s\n", fichierSource.c_str());
    watcher->removePath(fichierSource.c_str());
}

void Effect::launch_Watcher(){
    
    printf("PATH WATCHED= %s\n", fichierSource.c_str());
    watcher->addPath(fichierSource.c_str());
}

//--------------ACCESSORS

string Effect::getSource(){
    
    return fichierSource;
}

void Effect::setSource(string file){
    fichierSource = file;
}

QDateTime Effect::get_creationDate(){
    return creationDate;
}

string Effect::getName(){
    
    return nomEffet; 
}

void Effect::setName(string name){
    nomEffet = name;
}

llvm_dsp_factory* Effect::getFactory(){
    return factory;
}

string Effect::getCompilationOptions(){
    return compilationOptions;
}

void Effect::update_compilationOptions(string& compilOptions, int newOptValue){
    if(compilationOptions.compare(compilOptions) !=0 || opt_level != newOptValue){
        compilationOptions = compilOptions;
        opt_level = newOptValue;   
        //        printf("opt level = %i\n", opt_level);
        forceSynchro = true;
        emit effectChanged();
    }
}

int Effect::getOptValue(){
    return opt_level;
}

bool Effect::isSynchroForced(){
    return forceSynchro;
}

void Effect::setForceSynchro(bool val){
    forceSynchro = val;
}
