//
//  audioFader.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _audioFader_Interface_h
#define _audioFader_Interface_h

class audioFader_Interface
{        
    public:
    
        audioFader_Interface(){}
        ~audioFader_Interface(){}
    
        virtual void launch_fadeOut() = 0;
        virtual void launch_fadeIn() = 0;
        virtual bool get_FadeOut() = 0;
};

#endif
