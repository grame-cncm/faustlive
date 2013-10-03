//
//  AudioFader_Interface.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// audioFader_Interface describes the functions that permits the user to start a crossfade and know when this crossfade is over. 
// A new type of audioFader has to inherit this class.

#ifndef _AudioFader_Interface_h
#define _AudioFader_Interface_h

class AudioFader_Interface
{        
    public:
    
        AudioFader_Interface(){}
        ~AudioFader_Interface(){}
    
        virtual void launch_fadeOut() = 0;
        virtual void launch_fadeIn() = 0;
        virtual bool get_FadeOut() = 0;
};

#endif
