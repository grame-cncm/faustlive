//
//  PA_audioFader.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class adds new features to jackaudio so that the dsp of the audioClient can be dynamically changed. MoreOver, the two dsp will be switched with a crossfade between them. 

#include "PA_audioFader.h"

PA_audioFader::PA_audioFader(long srate, long bsize) : portaudio(srate, bsize){}

PA_audioFader::~PA_audioFader(){}

bool PA_audioFader::set_dsp(dsp* DSP){
    set_dsp_aux(DSP);
    return true;
}

int PA_audioFader::processAudio(float** ibuf, float** obuf, unsigned long frames) 
{
    // process samples
	//printf("Process Audio \n");
    fDsp->compute(frames, ibuf, obuf);
    crossfade_Calcul(fBufferSize, fDevNumOutChans, obuf);
	
/*	for(int i=0; i<fDsp->getNumOutputs(); i++){
		for(int j=0; j<frames; j++){
			printf("Output Buffer = %f\n", obuf[i][j]);
		}
	}*/
	return paContinue;
}

// UpDate the list of ports needed by new DSP
void PA_audioFader::launch_fadeOut(){
    set_doWeFadeOut(true);
}

//Fade In is not needed, because the fade in and out are both launched in the same process
void PA_audioFader::launch_fadeIn(){
    set_doWeFadeIn(true);
}

bool PA_audioFader::get_FadeOut(){
    return get_doWeFadeOut();
}

int PA_audioFader::get_buffer_size() { return fBufferSize; }

int PA_audioFader::get_sample_rate() { return fSampleRate; }




