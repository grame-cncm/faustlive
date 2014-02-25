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

#ifdef REMOTE
#include "faust/remote-dsp.h"
#endif

//----------CONSTRUCTOR/DESTRUCTOR----------------
//###CONSTRUCTOR
//@param : isEffectRecalled = is Effect created from a session recalling situation
//@param : sourceFile = source of effect
//@param : name = name of the created effect
//@param : isLocal = is it processing on local or remote machine

FLEffect::FLEffect(bool isEffectRecalled, const QString& sourceFile, const QString& name, bool isLocal){
    
    fFactory = NULL;
    fOldFactory = NULL;
    fRemoteFactory = NULL;
    fOldRemoteFactory = NULL;
    fWatcher = NULL;
    fSynchroTimer = NULL;
    fSource = sourceFile;
    fName = name;
    fForceSynchro = false;
    fRecompilation= false;
    fRecalled = isEffectRecalled;
    
    fIsLocal = isLocal;
    fIpMachineRemote = "127.0.0.1";
    fPortMachineRemote = 0;
}

FLEffect::~FLEffect(){
    reset();
}

//In case of lost remote factory
void FLEffect::reset(){
    
    delete fSynchroTimer;
    delete fWatcher;
    
    if (fIsLocal) {
        deleteDSPFactory(fFactory);
    }
#ifdef REMOTE
    else {
        deleteRemoteDSPFactory(fRemoteFactory);
    }
#endif 
}

bool FLEffect::reinit(QString& error){
    return init(fCurrentSVGFolder, fCurrentIRFolder, fCompilationOptions, fOpt_level, error, fIpMachineRemote, fPortMachineRemote);
}

//Initialisation of the effect. From a source file, it builds the factory
//@param : currentSVGFolder = where to create the SVG-Folder tied to the factory 
//@param : currentIRFolder = where to save the bitcode tied to the factory
//@param : compilation Options = options to create the factory in faust compiler
//@param : optvalue = optimization value for LLVM compiler
//@param : error = if the initialisation fails, error is filled
//@param : ip/port remote = IP/Port of processing machine (Remote Case)
bool FLEffect::init(const QString& currentSVGFolder, const QString& currentIRFolder ,QString compilationMode, int optValue, QString& error, const QString& IPremote, int portremote){
    
    printf("FICHIER SOURCE = %s\n", fSource.toStdString().c_str());
    
    fCompilationOptions = compilationMode;
    fOpt_level = optValue;
    fIpMachineRemote = IPremote;
    fPortMachineRemote = portremote;

    fCurrentSVGFolder = currentSVGFolder;
    fCurrentIRFolder = currentIRFolder;
    
    if(buildFactory(kCurrentFactory, error, currentSVGFolder, currentIRFolder)){
        
        //Initializing watcher looking for changes on faust source through text editor 
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

void FLEffect::forceRecompilation(bool val){
    
    fRecompilation = val;
}

bool FLEffect::hasToBeRecompiled(){
    
    if(fRecompilation){
        fRecompilation = false;
        return true;
    }
    else
        return false;
}   

//Creating the factory with the specific compilation options, in case of an error the buffer is filled
//@param : factoryToBuild = creating the current or charged factory
//@param : error = buffer to build in case build fails
//@param : currentSVGFolder = where to create the SVG-Folder tied to the factory 
//@param : currentIRFolder = where to save the bitcode tied to the factory
bool FLEffect::buildFactory(int factoryToBuild, QString& error, QString currentSVGFolder, QString currentIRFolder){
    
    llvm_dsp_factory* buildingFactory = NULL;
    remote_dsp_factory* buildingRemoteFactory = NULL;
    
//To speed up the process of compilation, when a session is recalled, the DSP are re-compiled from Bitcode 
    QString IRpath = currentIRFolder + "/" + fName;
    
    if(fRecalled && QFileInfo(IRpath).exists() && fIsLocal){        
        buildingFactory = readDSPFactoryFromBitcodeFile(IRpath.toStdString(), "");
        
        printf("factory from IR\n");
            
        fRecalled = false;
        
        if(buildingFactory != NULL){
            if(factoryToBuild == kCurrentFactory)
                fFactory = buildingFactory;
            else
                fOldFactory = buildingFactory;
            
            return true;
        }    
    }
    
    if(buildingFactory == NULL && buildingRemoteFactory == NULL){
        
        int numberFixedParams = 7;
        
        //+7 = -I libraryPath -I currentFolder -O drawPath -svg
        int argc = 7;
        argc += get_numberParameters(fCompilationOptions);
        
        const char** argv = new const char*[argc];
        
        argv[0] = "-I";
        
//The library path is where libraries like the scheduler architecture file are = Application Bundle/Resources
#ifdef __APPLE__
        string libPath = QFileInfo(QFileInfo( QCoreApplication::applicationFilePath()).absolutePath()).absolutePath().toStdString() + LIBRARY_PATH;
        argv[1] = libPath.c_str();
        printf("ARGV 1 = %s\n", argv[1]);
#elif __linux__
        
        QString libPath = QFileInfo(QCoreApplication::applicationFilePath()).absolutePath() + LIBRARY_PATH;
        argv[1] = libPath.toLatin1().data();
#else    
        QString libPath = "C:/Users/Sarah/faudiostream-faustlive/Resources/Libs";
        argv[1] = libPath.toLatin1().data();
#endif
        
        argv[2] = "-I";    
        string sourcePath = QFileInfo(fSource).absolutePath().toStdString();
        
        
        argv[3] = sourcePath.c_str();
        argv[4] = "-O";
        
        string svgPath = currentSVGFolder.toStdString();
        
        argv[5] = svgPath.c_str();
        argv[6] = "-svg";
        
        //Parsing the compilationOptions from a string to a char**
        QString copy = fCompilationOptions;
        
        for(int i=numberFixedParams; i<argc; i++){
            
            string parseResult(parse_compilationParams(copy));
            
            char* intermediate = new char[parseResult.size()+1];
            
            strcpy(intermediate,parseResult.c_str());
            
//        OPTION DOUBLE HAS TO BE SKIPED, it causes segmentation fault
            if(strcmp(intermediate, "-double") != 0)
                argv[i] = (const char*)intermediate;
            else{
                argc--;
                i--;
                printf("Option -double not taken into account\n");
            }
        }
        
        for(int i=0; i<argc; i++)
            printf("ARGV %i = %s\n", i, argv[i]);
        
        std::string getError("");
        
//        BUILDING THE FACTORY (Local or Remote)
        
        if(fIsLocal){      
            
            buildingFactory = createDSPFactoryFromFile(fSource.toStdString(), argc, argv, "", getError, fOpt_level);
        }
#ifdef REMOTE
        else{
            printf("REMOTE BUILDING FACTORY\n");
             
            buildingRemoteFactory = createRemoteDSPFactoryFromFile(fSource.toStdString(), argc, argv, fIpMachineRemote.toStdString(), fPortMachineRemote, getError, fOpt_level);
        }
#endif
        error = getError.c_str();
        
        delete[] argv;
        
        //The creation date is nedded in case the text editor sends a message of modification when actually the file has only been opened. It prevents recompilations for bad reasons
        fCreationDate = fCreationDate.currentDateTime();
        
        if(buildingFactory != NULL || buildingRemoteFactory != NULL){
            
//          KEEPING BUILT FACTORY IN RIGHT CLASS MEMBER
            if(fIsLocal){
                //The Bitcode files are written at each compilation 
                writeDSPFactoryToBitcodeFile(buildingFactory, IRpath.toStdString());
                
                if(factoryToBuild == kCurrentFactory)
                    fFactory = buildingFactory;
                else
                    fOldFactory = buildingFactory;
            }
            else{
                if(factoryToBuild == kCurrentFactory)
                    fRemoteFactory = buildingRemoteFactory;
                else
                    fOldRemoteFactory = buildingRemoteFactory;
            }
            
            return true;
        }
        else
            return false;
    }
    else{
        fCreationDate = fCreationDate.currentDateTime();
        return false;
    }
} 

//Re-Build of the factory from the source file
//@param : error = buffer to build in case build fails
//@param : currentSVGFolder = where to create the SVG-Folder tied to the factory 
//@param : currentIRFolder = where to save the bitcode tied to the factory
bool FLEffect::update_Factory(QString& error, QString currentSVGFolder, QString currentIRFolder){
    
    llvm_dsp_factory* local_factory_update = NULL;
    remote_dsp_factory* remote_factory_update = NULL;
    
	if(buildFactory(kChargingFactory, error, currentSVGFolder, currentIRFolder)){
        
        if(fIsLocal){
			local_factory_update = fFactory;
			fFactory = fOldFactory;
			fOldFactory = local_factory_update;
		}
		else{
            remote_factory_update = fRemoteFactory;
			fRemoteFactory = fOldRemoteFactory;
			fOldRemoteFactory = remote_factory_update;
        }
        return true;
	}
	else
        return false;
}

//Once the rebuild is complete, the former factory has to be deleted
void FLEffect::erase_OldFactory(){
    
	if(fIsLocal) {
	    deleteDSPFactory(fOldFactory);
    }
#ifdef REMOTE
	else{
        printf("DELETE REMOTE OLD FACTORY\n");
        deleteRemoteDSPFactory(fOldRemoteFactory);
    }
#endif
}

//---------------COMPILATION OPTIONS

//Get number of compilation options
int FLEffect::get_numberParameters(const QString& compilOptions){
    
    int argc = 0;
    int pos = compilOptions.indexOf("-");
    
    while(pos != -1 && pos < compilOptions.size()){
        
        argc++;
        
        pos = compilOptions.indexOf(" ", pos);
        
        while(pos != -1 && pos < compilOptions.size()){
            
            if(compilOptions[pos] == ' ')
                pos++;
            else if(compilOptions[pos] == '-')
                break;
            else{
                argc++;
                pos = compilOptions.indexOf(" ", pos);
            }
        }
    }
    
    printf("ARGC = %i\n", argc);
    
    return argc;
    
}

//Hand Made Parser
//Returns : the first option found, skipping the ' '
//CompilOptions : the rest of the options are kept in
string FLEffect::parse_compilationParams(QString& compilOptions){
    
    QString returning = "";
    
    int pos = 0;
    
    while(pos != -1 && pos < compilOptions.size()){
        if(compilOptions[pos] == ' ')
            pos++;
        else{
            int pos2 = compilOptions.indexOf(" ", pos);
            
            returning = compilOptions.mid(pos, pos2-pos);
            
            if(pos2 == -1)
                pos2 = compilOptions.size();
            compilOptions.remove(0, pos2);
            
            break;
        }
    }
    
    string returnin(returning.toStdString());
    
    return returnin;
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

remote_dsp_factory* FLEffect::getRemoteFactory(){
    return fRemoteFactory;
}

QString FLEffect::getCompilationOptions(){
    return fCompilationOptions;
}

void FLEffect::update_compilationOptions(QString& compilOptions, int newOptValue){
    if(fCompilationOptions.compare(compilOptions) !=0 || fOpt_level != newOptValue){
        
        printf("EFFECT CHANGED IS LOCAL = %i\n", fIsLocal);
        
        fCompilationOptions = compilOptions;
        fOpt_level = newOptValue;   
        //        printf("opt level = %i\n", opt_level);
        fForceSynchro = true;
        emit effectChanged();
    }
}

void FLEffect::update_remoteMachine(const QString& ip, int port){
    
    fIpMachineRemote = ip;
    fPortMachineRemote = port;
    fForceSynchro = true;
    emit effectChanged();
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

bool FLEffect::isLocal(){
    return fIsLocal;
}

QString FLEffect::getRemoteIP(){
    return fIpMachineRemote;
}

int FLEffect::getRemotePort(){
    return fPortMachineRemote;
}
