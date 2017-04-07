//
//  JA_audioFader.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class adds new features to jackaudio so that the dsp of the audioClient can be dynamically changed. 
// Moreover, the two dsp will be switched with a crossfade between them. 

#include "JA_audioFader.h"
#include "FLSettings.h"

//Calculation of sample[i,j] mixing 2 dsp (one fading in/one fading out)
float JA_audioFader::crossfade_calculation(int i, int j)
{
    bool connectFadeOut = false;
    bool connectFadeIn = false;
    list<pair<string, string> >::const_iterator it;
     
    for (it = fConnections.begin(); it != fConnections.end(); it++) {
        string jackPort(jack_port_name(fOutputPorts[j]));
        if (jackPort.compare(it->first) == 0) {
            connectFadeOut = true;
            break;
        }
    }
    for (it = fConnectionsIn.begin(); it != fConnectionsIn.end(); it++) {
        string jackPort(jack_port_name(fOutputPorts[j]));
        if (jackPort.compare(it->first) == 0) {
            connectFadeIn = true;
            break;
        }
    }
    
    if (connectFadeIn && connectFadeOut) {
        return (fIntermediateFadeIn[j][i] * (1-fInCoef)) + (fIntermediateFadeOut[j][i] * fOutCoef);
    } else if (connectFadeIn) {
        return (fIntermediateFadeIn[j][i] * (1-fInCoef));
    } else if (connectFadeOut) {
        return (fIntermediateFadeOut[j][i] * fOutCoef);
    } else {
        return 0;
    }
}

JA_audioFader::JA_audioFader(const void* icon_data, size_t icon_size) :jackaudio_midi(icon_data, icon_size)
{
    if (FLSettings::_Instance()->value("General/Audio/Jack/AutoStart", true).toBool()) {
        unsetenv("JACK_NO_START_SERVER");
    } else {
        char* val_on = "1";
        setenv("JACK_NO_START_SERVER", val_on, 1);
    }
    reset_Values();
}

JA_audioFader::~JA_audioFader() 
{}

 // Special version that names the JACK ports
bool JA_audioFader::set_dsp(dsp* dsp, const char* portsName)    
{
    fDSP = dsp;
    
    for (int i = 0; i < fDSP->getNumInputs(); i++) {
        char buf[256];
        snprintf(buf, 256, "%s_In_%d", portsName, i);
        fInputPorts.push_back(jack_port_register(fClient, buf, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0));
    }
    for (int i = 0; i < fDSP->getNumOutputs(); i++) {
        char buf[256];
        snprintf(buf, 256, "%s_Out_%d", portsName, i);
        fOutputPorts.push_back(jack_port_register(fClient, buf, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0));
    }
    
    fDSP->init(jack_get_sample_rate(fClient));
    return true;
}

// Redefine jackaudio method
bool JA_audioFader::start()
{
    if (jack_activate(fClient)) {
        fprintf(stderr, "Cannot activate client");
        return false;
    } else {
        return true;
    }
}

//Init second DSP in Jack Client
void JA_audioFader::init_FadeIn_Audio(dsp* DSP, const char* portsName)
{
    fDSPIn = DSP;
    
    //Rename the common ports
    for (int i = 0; i < fDSP->getNumInputs(); i++){
        char buf[256];
        snprintf(buf, 256, "%s_In_%d", portsName, i);
        jack_port_set_name(fInputPorts[i], buf);
    }
    for (int i = 0; i < fDSP->getNumOutputs(); i++){
        char buf[256];
        snprintf(buf, 256, "%s_Out_%d", portsName, i);
        jack_port_set_name(fOutputPorts[i], buf);
    }
    
    //Register the new ports 
    if (fDSP->getNumInputs() < fDSPIn->getNumInputs()) {
        for (int i = fDSP->getNumInputs(); i < fDSPIn->getNumInputs(); i++) {
            char buf[256];
            snprintf(buf, 256, "%s_In_%d", portsName,i);
            fInputPorts.push_back(jack_port_register(fClient, buf, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0));
        }
    }
    
    if (fDSP->getNumOutputs() < fDSPIn->getNumOutputs()) {
        for (int i = fDSP->getNumOutputs(); i < fDSPIn->getNumOutputs(); i++) {
            char buf[256];
            snprintf(buf, 256, "%s_Out_%d",portsName, i);
            fOutputPorts.push_back(jack_port_register(fClient, buf, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0));
        }
    }
    
    fDSPIn->init(jack_get_sample_rate(fClient));
    save_connections();
    fConnectionsIn = fConnections;
}

//Connect Jack port following Connections
int JA_audioFader::reconnect(list<pair<string, string> > Connections)
{        
    list<pair<string, string> >::const_iterator it;
    
    for (it = Connections.begin(); it != Connections.end(); it++) {
        jack_connect(fClient, it->first.c_str(), it->second.c_str());
    }

    return 0;
}

// UpDate the list of ports needed by new DSP
void JA_audioFader::launch_fadeOut()
{
    //Allocation of the intermediate buffers needed for the crossfade
      
    fIntermediateFadeIn = new float*[fDSPIn->getNumOutputs()];
    for(int i = 0; i < fDSPIn->getNumOutputs();i++) {
        fIntermediateFadeIn[i] = new float[jack_get_buffer_size(fClient)];
    }
  
    fIntermediateFadeOut = new float*[fDSP->getNumOutputs()];
    for (int i = 0; i < fDSP->getNumOutputs(); i++) {
        fIntermediateFadeOut[i] = new float[jack_get_buffer_size(fClient)];
    }
    
    set_doWeFadeOut(true); 
}

//Fade In is not needed, because the fade in and out are both launched in the same process
void JA_audioFader::launch_fadeIn() {}
bool JA_audioFader::get_FadeOut() { return get_doWeFadeOut(); }

// The inFading DSP becomes the current one
void JA_audioFader::upDate_DSP()
{
    //Erase the extra ports
    if (fDSP->getNumInputs() > fDSPIn->getNumInputs()) {
        for (int i = fDSPIn->getNumInputs(); i < fDSP->getNumInputs(); i++){
            jack_port_unregister(fClient, fInputPorts[i]);
        }
        fInputPorts.resize(fDSPIn->getNumInputs());
    }
    if (fDSP->getNumOutputs() > fDSPIn->getNumOutputs()) {
        for (int i = fDSPIn->getNumOutputs(); i < fDSP->getNumOutputs(); i++){
            jack_port_unregister(fClient, fOutputPorts[i]);
        }
        fOutputPorts.resize(fDSPIn->getNumOutputs());
    }
    
    dsp* DspInt = fDSP;
    fDSP = fDSPIn; 
    fDSPIn = DspInt;
    
    delete [] fIntermediateFadeOut;
    delete [] fIntermediateFadeIn;
}

// JACK callbacks
void JA_audioFader::processAudio(jack_nframes_t nframes) 
{
    AVOIDDENORMALS;
    // Retrieve JACK inputs/output audio buffers
    float** fInChannel = (float**)alloca(fDSP->getNumInputs() * sizeof(float*));
    
    for (int i = 0; i < fDSP->getNumInputs(); i++) {
        fInChannel[i] = (float*)jack_port_get_buffer(fInputPorts[i], nframes);
    }
    
    if (get_doWeFadeOut()) {
        
        //Step 1 : Calculation of intermediate buffers
        
        // By convention timestamp of -1 means 'no timestamp conversion' : events already have a timestamp espressed in frames
        fDSP->compute(-1, nframes, fInChannel, fIntermediateFadeOut);
        float** fInChannelDspIn = (float**)alloca(fDSPIn->getNumInputs() * sizeof(float*));
        
        for (int i = 0; i < fDSPIn->getNumInputs(); i++) {
            fInChannelDspIn[i] = (float*)jack_port_get_buffer(fInputPorts[i], nframes);
        }
        
        // By convention timestamp of -1 means 'no timestamp conversion' : events already have a timestamp espressed in frames
        fDSPIn->compute(-1, nframes, fInChannelDspIn, fIntermediateFadeIn); 
        
        int numOutPorts = max(fDSP->getNumOutputs(), fDSPIn->getNumOutputs());
		float** fOutFinal = (float**)alloca(numOutPorts * sizeof(float*));
        
        //Step 2 : Mixing the 2 DSP in the final output buffer taking into account the number of IN/OUT ports of the in- and out-coming DSP
        
        for (int i = 0; i < numOutPorts; i++) {
            fOutFinal[i] = (float*)jack_port_get_buffer(fOutputPorts[i], nframes); 
        }
        
        if (fDSP->getNumOutputs() < fDSPIn->getNumOutputs()) {
            for (size_t i = 0; i < nframes; i++) {
            
                for (int j = 0; j < fDSP->getNumOutputs(); j++) {
                    fOutFinal[j][i] = crossfade_calculation(i, j);
                }
                
                for (int j = fDSP->getNumOutputs(); j < fDSPIn->getNumOutputs(); j++) {
                    fOutFinal[j][i] = fIntermediateFadeIn[j][i]*(1-fInCoef);
                }
                
                if ((1-fInCoef) < 1) {
                    fInCoef = fInCoef - kFadeCoefficient;
                }
                
                fOutCoef = fInCoef;  
            }
        } else {
            for (size_t i = 0; i < nframes; i++) {
            
                for (int j = 0; j < fDSPIn->getNumOutputs(); j++) {
                    fOutFinal[j][i] = crossfade_calculation(i, j);
                }
                
                for (int j = fDSPIn->getNumOutputs(); j < fDSP->getNumOutputs(); j++) {
                    fOutFinal[j][i] = fIntermediateFadeOut[j][i]*fOutCoef;
                }
                
                if ((1-fInCoef) < 1) {
                    fInCoef = fInCoef - kFadeCoefficient;
                }
                
                fOutCoef = fInCoef;        
            }   
        }
        increment_crossFade();
    } else {
    
        //Normal processing
        float** fOutFinal = (float**)alloca(fDSP->getNumOutputs() * sizeof(float*));
        for (int i = 0; i < fDSP->getNumOutputs(); i++) {
            fOutFinal[i] = (float*)jack_port_get_buffer(fOutputPorts[i], nframes);
        }
        
        // By convention timestamp of -1 means 'no timestamp conversion' : events already have a timestamp espressed in frames
        fDSP->compute(-1, nframes, fInChannel, fOutFinal);   
    }
}

// Access to the fade parameter
list<pair<string, string> > JA_audioFader::get_audio_connections(bool& saved)
{
    saved = save_connections();
    return fConnections;
}  

