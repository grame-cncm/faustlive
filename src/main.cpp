//
//  main.cpp
//  
//
//  Created by Sarah Denoux on 30/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// MAIN OF FAUSTLIVE = Starts the FaustLive Qt Application

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

#include "FLApp.h"

#include <QFileInfo>

//-------------------------------------------------------------------------
// 									MAIN
//-------------------------------------------------------------------------

int main(int argc, char *argv[])
{   
    
    FLApp* app = new FLApp(argc, argv);
    
    //If app was executed with DSP as arguments
    for(int i=1; i < argc; i++){
            string dsp(argv[i]);
            
            if(QFileInfo(argv[i]).exists())
                app->create_New_Window(dsp);
    }
    
    app->exec();
    
    delete app;
    
    return 1;
}

