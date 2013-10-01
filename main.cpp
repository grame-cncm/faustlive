//
//  main.cpp
//  
//
//  Created by Sarah Denoux on 30/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

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

