//
//  JA_audioFader.cpp
//  
//
//  Created by Sarah Denoux on 15/07/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

// This class adds new features to jackaudio so that the dsp of the audioClient can be dynamically changed. MoreOver, the two dsp will be switched with a crossfade between them. 

#include "JA_audioFader.h"

//Calculation of sample[i,j] mixing 2 dsp (one fading in/one fading out)
float JA_audioFader::crossfade_calculation(int i, int j){
    bool connectFadeOut = false;
    bool connectFadeIn = false;
    
    list<pair<string, string> >::const_iterator it;
     
    for (it = fConnections.begin(); it != fConnections.end(); it++){
        
        string jackPort(jack_port_name(fOutputPorts[j]));
        
        if(jackPort.compare(it->first) == 0){
            connectFadeOut = true;
            break;
        }
    }
    for (it = fConnectionsIn.begin(); it != fConnectionsIn.end(); it++){
        
        string jackPort(jack_port_name(fOutputPorts[j]));
        
        if(jackPort.compare(it->first) == 0){
            connectFadeIn = true;
            break;
        }
    }
    
    if(connectFadeIn && connectFadeOut)
        return (fIntermediateFadeIn[j][i]*fInCoef) + (fIntermediateFadeOut[j][i]/fOutCoef);
    else if(connectFadeIn)
        return (fIntermediateFadeIn[j][i]*fInCoef);
    else if(connectFadeOut)
        return (fIntermediateFadeOut[j][i]/fOutCoef);
    else
        return 0;
}


JA_audioFader::JA_audioFader(const void* icon_data, size_t icon_size)
{
    fDsp = NULL;
    fClient = NULL;
    fNumInChans = 0;
    fNumOutChans = 0;
    fInputPorts = NULL;
    fOutputPorts = NULL;
    fShutdown = 0;
    fShutdownArg = 0;
    
    fInputPorts = new jack_port_t*[256];
    fOutputPorts = new jack_port_t*[256];
    
    reset_Values();
    
    if (icon_data) {
        fIconData = malloc(icon_size);
        fIconSize = icon_size;
        memcpy(fIconData, icon_data, icon_size);
    } else {
        fIconData = NULL;
        fIconSize = 0;
    }
}

JA_audioFader::~JA_audioFader() 
{ 
    //stop(); 
    if (fIconData) {
        free(fIconData);
    }
}

bool JA_audioFader::init(const char* /*name*/, dsp* /*DSP*/){ return false;} 

bool JA_audioFader::init(const char* name) 
{
    if ((fClient = jack_client_open(name, JackNullOption, NULL)) == 0) {
        fprintf(stderr, "JACK server not running ?\n");
        return false;
    }
#ifdef JACK_IOS
    jack_custom_publish_data(fClient, "icon.png", fIconData, fIconSize);
#endif
    
#ifdef _OPENMP
    jack_set_process_thread(fClient, _jack_thread, this);
#else
    jack_set_process_callback(fClient, _jack_process, this);
#endif
    
    jack_set_sample_rate_callback(fClient, _jack_srate, this);
    jack_set_buffer_size_callback(fClient, _jack_buffersize, this);
    jack_on_info_shutdown(fClient, _jack_info_shutdown, this);
    
    return true;
}  

//Set DSP
bool JA_audioFader::set_dsp(dsp* DSP, const char* portsName)    {
    fDsp = DSP;
    
    fNumInChans  = fDsp->getNumInputs();
    fNumOutChans = fDsp->getNumOutputs();
    
    fBufferSize = jack_get_buffer_size(fClient);
    
//        fInput_ports = new jack_port_t*[fNumInChans];
//      fOutput_ports = new jack_port_t*[fNumOutChans];
    
    for (int i = 0; i < fNumInChans; i++) {
        char buf[256];
        snprintf(buf, 256, "%s_In_%d",portsName, i);
        fInputPorts[i] = jack_port_register(fClient, buf, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    }
    for (int i = 0; i < fNumOutChans; i++) {
        char buf[256];
        snprintf(buf, 256, "%s_Out_%d",portsName, i);
        fOutputPorts[i] = jack_port_register(fClient, buf, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    }
    
    fDsp->init(jack_get_sample_rate(fClient));
    return true;
}

bool JA_audioFader::start(){
    if (jack_activate(fClient)) {
        fprintf(stderr, "Cannot activate client");
        return false;
    }
    return true;
}

//Init second DSP in Jack Client
void JA_audioFader::init_FadeIn_Audio(dsp* DSP, const char* portsName){
    fDspIn = DSP;
    
    fNumInDspFade  = fDspIn->getNumInputs();
    fNumOutDspFade = fDspIn->getNumOutputs();  
    
    //Rename the common ports
    for(int i = 0; i<fNumInChans; i++){
        char buf[256];
        snprintf(buf, 256, "%s_In_%d",portsName, i);
        
        jack_port_set_name (fInputPorts[i], buf);
    }
    for(int i = 0; i<fNumOutChans; i++){
        char buf[256];
        snprintf(buf, 256, "%s_Out_%d",portsName, i);
        jack_port_set_name (fOutputPorts[i], buf);
    }
    
    //Register the new ports 
    if(fNumInChans<fNumInDspFade){
        
        for (int i = fNumInChans; i < fNumInDspFade; i++) {
            char buf[256];
            snprintf(buf, 256, "%s_In_%d", portsName,i);
            
            fInputPorts[i] = jack_port_register(fClient, buf, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        }
    }
    
    if(fNumOutChans<fNumOutDspFade){
        for (int i = fNumOutChans; i < fNumOutDspFade; i++) {
            char buf[256];
            snprintf(buf, 256, "%s_Out_%d",portsName, i);
            fOutputPorts[i] = jack_port_register(fClient, buf, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        }
    }
    
    fDspIn->init(jack_get_sample_rate(fClient));
    save_connections();
    fConnectionsIn = fConnections;
}

//Connect Jack port following Connections
int JA_audioFader::reconnect(list<pair<string, string> > Connections)
{        
    list<pair<string, string> >::const_iterator it;
    
    for (it = Connections.begin(); it != Connections.end(); it++){
     
//        printf("CONNECT = %s TO %s\n", it->first.c_str(), it->second.c_str());
        jack_connect(fClient, it->first.c_str(), it->second.c_str());
    }
    return 0;
}

// UpDate the list of ports needed by new DSP
void JA_audioFader::launch_fadeOut()
{
    //Allocation of the intermediate buffers needed for the crossfade
    fIntermediateFadeOut = new float*[fNumOutChans];
    for(int i=0; i<fNumOutChans; i++)
        fIntermediateFadeOut[i] = new float[fBufferSize];
    
    fIntermediateFadeIn = new float*[fNumOutDspFade];
    for(int i=0; i<fNumOutDspFade;i++)
        fIntermediateFadeIn[i] = new float[fBufferSize];
    
    set_doWeFadeOut(true); 
}

//Fade In is not needed, because the fade in and out are both launched in the same process
void JA_audioFader::launch_fadeIn(){}
bool JA_audioFader::get_FadeOut(){return get_doWeFadeOut();}

// The inFading DSP becomes the current one
void JA_audioFader::upDate_DSP(){
    
    //Erase the extra ports
    if(fNumInChans>fNumInDspFade)
    {
        for (int i = fNumInDspFade; i < fNumInChans; i++){
            jack_port_unregister(fClient, fInputPorts[i]);
        }
    }
    if(fNumOutChans>fNumOutDspFade)
    {
        for (int i = fNumOutDspFade; i < fNumOutChans; i++){
            jack_port_unregister(fClient, fOutputPorts[i]);
        }
    }
    
    fNumInChans = fNumInDspFade;
    fNumOutChans = fNumOutDspFade;
    
    dsp* DspInt;
    
    DspInt = fDsp;
    fDsp = fDspIn; 
    fDspIn = DspInt;
    
    delete [] fIntermediateFadeOut;
    delete [] fIntermediateFadeIn;
}

// jack callbacks
int	JA_audioFader::process(jack_nframes_t nframes) 
{
    AVOIDDENORMALS;
    // Retrieve JACK inputs/output audio buffers
    
    float* fInChannel[fNumInChans];
    
    for (int i = 0; i < fNumInChans; i++) {
        fInChannel[i] = (float*)jack_port_get_buffer(fInputPorts[i], nframes);
    }
    
    if(get_doWeFadeOut()){
        
        //Step 1 : Calculation of intermediate buffers
        
        fDsp->compute(nframes, fInChannel, fIntermediateFadeOut);
        
        float* fInChannelDspIn[fNumInDspFade];
        
        for (int i = 0; i < fNumInDspFade; i++) {
            fInChannelDspIn[i] = (float*)jack_port_get_buffer(fInputPorts[i], nframes);
        }
        fDspIn->compute(nframes, fInChannelDspIn, fIntermediateFadeIn); 
        
        int numOutPorts = max(fNumOutChans, fNumOutDspFade);
        float* fOutFinal[numOutPorts];
        
        //Step 2 : Mixing the 2 DSP in the final output buffer taking into account the number of IN/OUT ports of the in- and out-coming DSP
        
        for (int i = 0; i < numOutPorts; i++) 
            fOutFinal[i] = (float*)jack_port_get_buffer(fOutputPorts[i], nframes); 
        
        if(fNumOutChans<fNumOutDspFade){
            for (size_t i = 0; i < nframes; i++) {
                for (int j = 0; j < fNumOutChans; j++)
                    fOutFinal[j][i] = crossfade_calculation(i, j);
                
                for (int j = fNumOutChans; j < fNumOutDspFade; j++)
                    fOutFinal[j][i] = fIntermediateFadeIn[j][i]*fInCoef;
                
                if(fInCoef < 1)
                    fInCoef = fInCoef*kFadeInCoefficient;  
                fOutCoef = fOutCoef*kFadeOutCoefficient;  
            }
        } 
        
        else{
            for (size_t i = 0; i < nframes; i++) {
                for (int j = 0; j < fNumOutDspFade; j++)
                    fOutFinal[j][i] = crossfade_calculation(i, j);
                
                for (int j = fNumOutDspFade; j < fNumOutChans; j++)
                    fOutFinal[j][i] = fIntermediateFadeOut[j][i]/fOutCoef;
                
                if(fInCoef < 1)   
                    fInCoef = fInCoef*kFadeInCoefficient;  
                fOutCoef = fOutCoef*kFadeOutCoefficient;        
            }   
        }
        increment_crossFade();
    }
    
    //Normal processing
    else{
        float* fOutFinal[fNumOutChans];
        
        for (int i = 0; i < fNumOutChans; i++)
            fOutFinal[i] = (float*)jack_port_get_buffer(fOutputPorts[i], nframes);
        
        fDsp->compute(nframes, fInChannel, fOutFinal);   
    }
    
    
    return 0;
}

// Access to the fade parameter
list<pair<string, string> > JA_audioFader::get_audio_connections()
{
    save_connections();
    return fConnections;
}  

