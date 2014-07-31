//
//  main.cpp
//  
//
//  Created by Sarah Denoux on 30/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// MAIN OF FAUSTLIVE = Starts the FaustLive Qt Application

//#include <libgen.h>

#include <stdlib.h>
#include <iostream>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <vector>
#include <cstdlib>

#include <sstream>

using namespace std;

#include "FLApp.h"

#include <QFileInfo>

#ifdef _WIN32
#include <windows.h>
#else

#include <sys/time.h>
#include <sys/resource.h>
#endif

#include <stdio.h>
#include <stdlib.h>

FLApp* app;

//-------------------------------------------------------------------------
// 									MAIN
//-------------------------------------------------------------------------


#ifndef _WIN32
void myMessageOutput(QtMsgType type, const char *msg)
{
    printf("Message Output = %s\n", msg);
    switch (type) {
        case QtFatalMsg:
            printf("FATAL\n");
            app->update_CurrentSession();
            app->closeAllWindows();
            abort();
    }
}

static bool SetMaximumFiles(long filecount)
{
    rlimit lim;
    lim.rlim_cur = (rlim_t)filecount;
    lim.rlim_max = (rlim_t)filecount;
    if (setrlimit(RLIMIT_NOFILE, &lim) == 0) {
        return true;
    } else {
        return false;
    }
}

static bool GetMaximumFiles(int& filecount) 
{
    rlimit lim;
    if (getrlimit(RLIMIT_NOFILE, &lim) == 0) {
        filecount = lim.rlim_max;
        return true;
    } else {
        return false;
	}
}

#endif


#if defined(WIN32) && !defined(_DEBUG)
# define USEWINMAIN
#endif
//_______________________________________________________________________
#ifdef USEWINMAIN
#include <windows.h>
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdParam, int iCmdShow){
#else
int main( int argc, char **argv ){
#endif
#ifdef USEWINMAIN
	printf("USEWINMAIN\n");
int argc = __argc;
char **argv = __argv;
#endif

#ifndef _WIN32
    //qInstallMessageHandler(myMessageOutput);
    
    int filecount = 0;
    if(GetMaximumFiles(filecount)){
      
        if(SetMaximumFiles(4096)){
            filecount = 0;
            GetMaximumFiles(filecount);
#endif 
            app = new FLApp(argc, argv);
            
            //    If app was executed with DSP as arguments
            for(int i=1; i < argc; i++){
                QString dsp(argv[i]);
                
                if(QFileInfo(argv[i]).exists())
                    app->create_New_Window(dsp);
            }
            
            app->exec();
            
            delete app;
#ifndef _WIN32
        }
    }
#endif
    return 0;
}
