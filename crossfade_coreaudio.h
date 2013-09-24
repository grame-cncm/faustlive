//
//  crossfade_coreaudio.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _crossfade_coreaudio_h
#define _crossfade_coreaudio_h

#include "coreaudio-dsp.h"
#include "audioFader_Interface.h"
#include "audioFader_Implementation.h"

class crossfade_TCoreAudioRenderer: public TCoreAudioRenderer, public audioFader_Implementation{
    
public:
    virtual OSStatus Render(AudioUnitRenderActionFlags *ioActionFlags,
                    const AudioTimeStamp *inTimeStamp,
                    UInt32 inNumberFrames,
                    AudioBufferList *ioData);
};


class crossfade_coreaudio : public audio, public audioFader_Interface
{
    
protected:
    crossfade_TCoreAudioRenderer fAudioDevice;
	int fSampleRate, fFramesPerBuf;
    
public:
    crossfade_coreaudio(int srate, int fpb);
    crossfade_coreaudio(int fpb);
    
	virtual ~crossfade_coreaudio();
    
	virtual bool init(const char* /*name*/, dsp* DSP);
    
	virtual bool start();
    
	virtual void stop();
    
    virtual void launch_fadeOut();
    virtual void launch_fadeIn();
    virtual bool get_FadeOut();
};

#endif
