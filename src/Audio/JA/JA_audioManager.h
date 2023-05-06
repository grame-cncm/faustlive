//
//  JA_audioManager.h
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// JA_audioManager controls 1 JA_audioFader. It can switch from one DSP to another with a crossfade or it can act like a simple jackaudio-dsp
// JA_audioManager also controls the jack connections of the audio. 

#ifndef _JA_audioManager_h
#define _JA_audioManager_h

#include "AudioManager.h"
#include "FJUI.h"

class JA_audioFader;

class JA_audioManager : public AudioManager {

    private:
    
        Q_OBJECT

        JA_audioFader* fCurrentAudio;
        
        virtual bool init(const char*, dsp* DSP);
        static void shutdown_message(const char * msg, void* arg);
    
    public:
        
        JA_audioManager(shutdown_callback cb, void* arg);
        virtual ~JA_audioManager();
       
        virtual bool initAudio(QString& error, const char* name, bool midi);
        
        virtual bool setDSP(QString& error, dsp* DSP, const char* port_name);
        virtual bool start();
        virtual void stop();
        
        virtual bool init_FadeAudio(QString& error, const char* name, dsp* DSP);
        virtual void start_Fade();
        virtual void wait_EndFade();
        
        virtual void connect_Audio(std::string homeDir);
        virtual void save_Connections(std::string homeDir);

        virtual int getBufferSize();
        virtual int getSampleRate();
        
        // Needed to give 'jackaudio_midi = midi_handler' object to MidiUI interface
        JA_audioFader* getAudioFader() { return fCurrentAudio; }
};

#endif
