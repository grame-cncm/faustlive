/******************************************************************************
 *******************************************************************************
 
 IMPLEMENTATION OF THE USER INTERFACE
 (QT 4.3 for FAUST)
 
 *******************************************************************************
 *******************************************************************************/

//-----------------------------------------------------
// name: "Noise"
// version: "1.1"
// author: "Grame"
// license: "BSD"
// copyright: "(c)GRAME 2009"
//
// Code generated with Faust 2.0.a5 (http://faust.grame.fr)
//-----------------------------------------------------

#include <libgen.h>
#include <stdlib.h>
#include <iostream>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <vector>
#include <cstdlib>

#include <sstream>

using namespace std;

#include "FaustLiveApp.h"

#include <QFileInfo>


//-------------------------------------------------------------------------
// 									MAIN
//-------------------------------------------------------------------------

list<GUI*>               GUI::fGuiList;

int main(int argc, char *argv[])
{   
    
    FaustLiveApp* app = new FaustLiveApp(argc, argv);
    
    //If app was executed with DSP as arguments
    if(argc > 1){
        for(int i=1; i < argc; i++){
            string dsp(argv[i]);
            
            if(QFileInfo(dsp.c_str()).exists())
                app->create_New_Window(dsp);
        }
    }
    
    app->exec();
    
    delete app;
    
    return 1;
}

