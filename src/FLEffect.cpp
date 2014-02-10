//
//  FLEffect.cpp
//  
//
//  Created by Sarah Denoux on 30/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// An Effect takes care of the compilation of a DSP. Moreover, it is notify in case, the DSP has been modified. 

#include "FLEffect.h"

#define LIBRARY_PATH "/Resources/Libs/"

#include "faust/llvm-dsp.h"

/***********************************EFFECT IMPLEMENTATION*********************************/

FLEffect::FLEffect(bool recallVal, QString sourceFile, QString name){
    
    fFactory = NULL;
    fOldFactory = NULL;
    fWatcher = NULL;
    fSynchroTimer = NULL;
    fSource = sourceFile;
    fName = name;
    fForceSynchro = false;
    fRecalled = recallVal; //Effect is build in a recall situation
}

FLEffect::~FLEffect(){
    
    delete fSynchroTimer;
    delete fWatcher;
    //    printf("deleting factory = %p\n", factory);
    deleteDSPFactory(fFactory);
}


//Initialisation of the effect. From a source, it extracts the source file, the name and builds the factory
//currentSVGFolder = where to create the SVG-Folder tied to the factory 
//currentIRFolder = where to save the bitcode tied to the factory
//Compilation Options = needed to build the llvm factory
//Error = if the initialisation fails, the function returns false + the buffer is filled

bool FLEffect::init(QString currentSVGFolder, QString currentIRFolder ,QString compilationMode, int optValue, QString& error){
    
    printf("FICHIER SOURCE = %s\n", fSource.toLatin1().data());
    
    fCompilationOptions = compilationMode;
    fOpt_level = optValue;
    
    bool sucess = buildFactory(&fFactory, error, currentSVGFolder, currentIRFolder);
    
    if(sucess){
        
        //Initializing watcher looking for changes on DSP through text editor 
        fWatcher = new QFileSystemWatcher(this);
        fSynchroTimer = new QTimer(fWatcher);
        connect(fSynchroTimer, SIGNAL(timeout()), this, SLOT(effectModified()));
        
        connect(fWatcher, SIGNAL(fileChanged(const QString)), this, SLOT(reset_Timer(const QString)));
        
        return true;
    }
    
    else
        return false;
}

//---------------FACTORY ACTIONS

//Creating the factory with the specific compilation options, in case of an error the buffer is filled
bool FLEffect::buildFactory(llvm_dsp_factory** factoryToBuild, QString& error, QString currentSVGFolder, QString currentIRFolder){
    
    //+2 = Path to DSP + -svg to build the svg Diagram
//    int argc = 2 + get_numberParameters(fCompilationOptions);
//   
//    char ** argv;
//    argv = new char*[argc];
//    
//    //Parsing the compilationOptions from a string to a char**
//    QString copy = fCompilationOptions;
//    for(int i=2; i<argc; i++){
//        
//        QString parseResult = parse_compilationParams(copy);
//        argv[i] = new char[parseResult.length()+1];
//        strcpy(argv[i], parseResult.toLatin1().data());
//    }
//    
//    argv[0] = new char[fSource.length() + 1];
//    strcpy(argv[0], fSource.toLatin1().data());
//    argv[1] = new char[5];
//    strcpy(argv[1], "-svg");
//    
//    const char** argument = (const char**) argv;

	int argc = 1;
    char ** argv;
    argv = new char*[argc];

	argv[0] = new char[fSource.length() + 1];
    strcpy(argv[0], fSource.toLatin1().data());

	const char** argument = (const char**) argv;


    //The library path is where libraries like the scheduler architecture file are = Application Bundle/Resources
    char libraryPath[256];
    
#ifdef __APPLE__
    snprintf(libraryPath, 255, "%s%s", QFileInfo(QFileInfo( QCoreApplication::applicationFilePath()).absolutePath()).absolutePath().toStdString(), LIBRARY_PATH);
#endif
#ifdef __linux__
    snprintf(libraryPath, 255, "%s%s", QFileInfo( QCoreApplication::applicationFilePath()).absolutePath().toStdString(), LIBRARY_PATH);
#else
	strcpy(libraryPath, ":\Libs");
#endif
    
    QString testtest(libraryPath);
    testtest += "scheduler.ll";
    
    if(QFileInfo(testtest).exists())
        printf("LIBRARY EXISTS\n");
//    printf("libraryPath = %s\n", libraryPath);
    
    QString path = currentIRFolder + "/" + fName;
	printf("PATH TO IR = %s\n", path.toLatin1().data());
    
    //To speed up the process of compilation, when a session is recalled, the DSP are re-compiled from Bitcode 
    if(fRecalled && QFileInfo(path).exists()){        
        *factoryToBuild = readDSPFactoryFromBitcodeFile(path.toLatin1().data(), "");
        
        printf("factory from IR = %p\n", *factoryToBuild);
    }
    
    fRecalled = false;
    
    if(*factoryToBuild == NULL){
        
        printf("ABOUT TO BUILD with = %s\n", libraryPath);
        
        std::string getError;
		std::string pathLibs = "C:\Users\Sarah\faudiostream-faustlive\Resources\Libs";

		*factoryToBuild = createDSPFactory(argc , argument, pathLibs, currentSVGFolder.toStdString(), "", "", "", getError, fOpt_level);
        
        error = getError.c_str();
        
        printf("ERROR OF FACTORY BUILD = %s\n", error.toLatin1().data());
        
        delete [] argv;
        
        //The creation date is nedded in case the text editor sends a message of modification when actually the file has only been opened. It prevents recompilations for bad reasons
        fCreationDate = fCreationDate.currentDateTime();
        
        if(*factoryToBuild != NULL){
            
            if(!QFileInfo(currentIRFolder).exists()){
                QDir direct(currentIRFolder);
                direct.mkdir(currentIRFolder);
            }
            //The Bitcode files are written at each compilation 
			writeDSPFactoryToBitcodeFile(*factoryToBuild, path.toStdString());
            
            return true;
        }
        else
            return false;
        //    printf("NEW FACTORY = %p\n", factory);
    }
    else{
        delete [] argv;
        fCreationDate = fCreationDate.currentDateTime();
        return true;
    }
    
}


//Get number of compilation options
int FLEffect::get_numberParameters(const QString& compilOptions){
    
    QString copy = compilOptions;
    
    int argc = 0;
    int pos = 0;
    
	if(copy.indexOf("-") == -1){
        return 0;
    }
    
    while(copy.indexOf(" ", pos+1) != -1){
        argc++;
        pos = copy.indexOf(" ", pos+1);
    }
    
    return argc+1;
    
}

//Hand Made Parser = a ' ' means a separation between parameters. If there are none and still there are compilation Options = it's the last one but it has to be taken into account anyway.
//Returns : the first option found
//CompilOptions : the rest of the options are kept in
QString FLEffect::parse_compilationParams(QString& compilOptions){
    
    QString returning = "";
    
    size_t pos = compilOptions.indexOf(" ");
    
    if(pos != -1){
        returning = compilOptions.mid(0, pos);
        compilOptions.remove(0, pos+1);
    }
    else if(compilOptions.compare("") != 0)
        returning = compilOptions.mid(0, compilOptions.length());
    
    return returning;
}


//Re-Build of the factory from the source file
bool FLEffect::update_Factory(QString& error, QString currentSVGFolder, QString currentIRFolder){
    
    fOldFactory = fFactory;
    
    llvm_dsp_factory* factory_update = NULL;
    
    if(buildFactory(&factory_update, error, currentSVGFolder, currentIRFolder)){
        fFactory = factory_update;
        return true;
    }
    else
        return false;
}

//Once the rebuild is complete, the former factory has to be deleted
void FLEffect::erase_OldFactory(){
    
    //    printf("delete Factory = %p\n", oldFactory);
    deleteDSPFactory(fOldFactory);
}

//---------------WATCHER & FILE MODIFICATIONS ACTIONS

//When any action on the effect is performed, the watcher has to be stopped (and then re-launched) otherwise the synchronisation is called without good reason

void FLEffect::reset_Timer(const QString /*toto*/){
    
    //    printf("Reseting Timer\n");
    
    //If the signal is triggered multiple times in 2 second, only 1 is taken into account
    if(fSynchroTimer->isActive()){
        fSynchroTimer->stop();
        fSynchroTimer->start(2000);
    }
    else
        fSynchroTimer->start(2000);
}

void FLEffect::effectModified(){
    fSynchroTimer->stop();
    //    printf("Emission FLEffectChanged\n");
    emit effectChanged();
}

void FLEffect::stop_Watcher(){
    printf("PATH STOP WATCHING = %s\n", fSource.toLatin1().data());
    fWatcher->removePath(fSource);
}

void FLEffect::launch_Watcher(){
    
    printf("PATH WATCHED= %s\n", fSource.toLatin1().data());
    
    fWatcher->addPath(fSource);
}

//--------------ACCESSORS

QString FLEffect::getSource(){
    return fSource;
}

void FLEffect::setSource(QString file){
    fSource = file;
}

QDateTime FLEffect::get_creationDate(){
    return fCreationDate;
}

QString FLEffect::getName(){
    
    return fName; 
}

void FLEffect::setName(QString name){
    fName = name;
}

llvm_dsp_factory* FLEffect::getFactory(){
    return fFactory;
}

QString FLEffect::getCompilationOptions(){
    return fCompilationOptions;
}

void FLEffect::update_compilationOptions(QString& compilOptions, int newOptValue){
    if(fCompilationOptions.compare(compilOptions) !=0 || fOpt_level != newOptValue){
        fCompilationOptions = compilOptions;
        fOpt_level = newOptValue;   
        //        printf("opt level = %i\n", opt_level);
        fForceSynchro = true;
        emit effectChanged();
    }
}

int FLEffect::getOptValue(){
    return fOpt_level;
}

bool FLEffect::isSynchroForced(){
    return fForceSynchro;
}

void FLEffect::setForceSynchro(bool val){
    fForceSynchro = val;
}
