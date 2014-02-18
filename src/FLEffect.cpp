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

/***********************************EFFECT IMPLEMENTATION*********************************/


FLEffect::FLEffect(bool recallVal, QString sourceFile, QString name, bool isLocal){
    
    fFactory = NULL;
    fOldFactory = NULL;
    fRemoteFactory = NULL;
    fOldRemoteFactory = NULL;
    fWatcher = NULL;
    fSynchroTimer = NULL;
    fSource = sourceFile;
    fName = name;
    fForceSynchro = false;
    fRecalled = recallVal; //Effect is build in a recall situation
    
    fIsLocal = isLocal;
    fIpMachineRemote = "localhost";
    fPortMachineRemote = 0;
}

FLEffect::~FLEffect(){
    
    delete fSynchroTimer;
    delete fWatcher;
    //    printf("deleting factory = %p\n", factory);
    
    if (fIsLocal) {
        deleteDSPFactory(fFactory);
#ifdef REMOTE
    } else {
        deleteRemoteDSPFactory(fRemoteFactory);
    }
#endif
}

//Initialisation of the effect. From a source, it extracts the source file, the name and builds the factory
//currentSVGFolder = where to create the SVG-Folder tied to the factory 
//currentIRFolder = where to save the bitcode tied to the factory
//Compilation Options = needed to build the llvm factory
//Error = if the initialisation fails, the function returns false + the buffer is filled

bool FLEffect::init(const QString& currentSVGFolder, const QString& currentIRFolder ,QString compilationMode, int optValue, QString& error, const QString& IPremote, int portremote){
    
    printf("FICHIER SOURCE = %s\n", fSource.toLatin1().data());
    
    fCompilationOptions = compilationMode;
    fOpt_level = optValue;
    fIpMachineRemote = IPremote;
    fPortMachineRemote = portremote;

    if(buildFactory(kCurrentFactory, error, currentSVGFolder, currentIRFolder)){
        
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
bool FLEffect::buildFactory(int factoryToBuild, QString& error, QString currentSVGFolder, QString currentIRFolder){
    
    QString path = currentIRFolder + "/" + fName;

    //To speed up the process of compilation, when a session is recalled, the DSP are re-compiled from Bitcode 
    
        llvm_dsp_factory* buildingFactory = NULL;
        remote_dsp_factory* buildingRemoteFactory = NULL;
    
    if(fRecalled && QFileInfo(path).exists() && fIsLocal){        
        buildingFactory = readDSPFactoryFromBitcodeFile(path.toStdString(), "");
        
        printf("factory from IR = %p\n", buildingFactory);
            
        fRecalled = false;
        
        if(buildingFactory != NULL){
            if(factoryToBuild == kCurrentFactory){
                fFactory = buildingFactory;
                printf("FFACTORY = %p\n", fFactory);
            }
            else
                fOldFactory = buildingFactory;
            
            return true;
        }
    }
    
    if(buildingFactory == NULL && buildingRemoteFactory == NULL){
        
        int numberFixedParams = 7;
        
        //+5 = -I libraryPath -O drawPath -svg
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
        
        //    printf("Source Path = %s\n", sourcePath.toLatin1().data());
        
        argv[3] = sourcePath.c_str();
        argv[4] = "-O";
        
        string svgPath = currentSVGFolder.toStdString();
        
        argv[5] = svgPath.c_str();
        argv[6] = "-svg";
        
        //Parsing the compilationOptions from a string to a char**
        QString copy = fCompilationOptions;
        
        printf("COPY = %s\n", copy.toStdString().c_str());
        
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
        
        if(fIsLocal){      
//            printf("building local factory\n");
            buildingFactory = createDSPFactoryFromFile(fSource.toStdString(), argc, argv, "", getError, fOpt_level);
        }
#ifdef REMOTE
        else{
//            printf("IP = %s\n", fIpMachineRemote.toLatin1().data());
            
            buildingRemoteFactory = createRemoteDSPFactoryFromFile(fSource.toStdString(), argc, argv, fIpMachineRemote.toStdString(), fPortMachineRemote, getError, fOpt_level);
        }
#endif
        error = getError.c_str();
        
        delete[] argv;
        
        //The creation date is nedded in case the text editor sends a message of modification when actually the file has only been opened. It prevents recompilations for bad reasons
        fCreationDate = fCreationDate.currentDateTime();
        
        printf("REMOTE FACTORY = %p\n", buildingRemoteFactory);
        
        if(buildingFactory != NULL || buildingRemoteFactory != NULL){
            
            if(!QFileInfo(currentIRFolder).exists()){
                QDir direct(currentIRFolder);
                direct.mkdir(currentIRFolder);
            }
            
            if(fIsLocal){
                //The Bitcode files are written at each compilation 
                writeDSPFactoryToBitcodeFile(buildingFactory, path.toStdString());
                
                if(factoryToBuild == kCurrentFactory){
                    fFactory = buildingFactory;
                }
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
        else{
            printf("FALSE IS RETURNED\n");
            return false;
        }
    }
    else{
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
string FLEffect::parse_compilationParams(QString& compilOptions){
    
    string returning = "";
    
    int pos = compilOptions.indexOf(" ");
    
    if(pos != -1){
        returning = compilOptions.mid(0, pos).toStdString();
        compilOptions.remove(0, pos+1);
    }
    else if(compilOptions.compare("") != 0)
        returning = compilOptions.mid(0, compilOptions.length()).toStdString();
    
    return returning;
}

//Re-Build of the factory from the source file
bool FLEffect::update_Factory(QString& error, QString currentSVGFolder, QString currentIRFolder){
    
	if(fIsLocal){
		
//		 fOldFactory = fFactory;
    
		llvm_dsp_factory* factory_update = NULL;
    
		if(buildFactory(kChargingFactory, error, currentSVGFolder, currentIRFolder)){
			factory_update = fFactory;
			fFactory = fOldFactory;
			fOldFactory = factory_update;
			return true;
		}
		else
			return false;

	}
	else{

//		fOldRemoteFactory = fRemoteFactory;
    
		remote_dsp_factory* factory_update = NULL;
    
		if(buildFactory(kChargingFactory, error, currentSVGFolder, currentIRFolder)){
			factory_update = fRemoteFactory;
			fRemoteFactory = fOldRemoteFactory;
			fOldRemoteFactory = factory_update;
				return true;
		 }
		else
			return false;
	}
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
