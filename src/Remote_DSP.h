


#ifndef Remote_DSP__h
#define Remote_DSP__h

#include <string>
#include <vector>
#include <map>
#include <assert.h>

#include <sstream>
#include <iostream>
#include <fstream> 

#include <curl/curl.h>

#include "JsonParser.h"
#include "Remote_DSP_Factory.h"
#include "faust/gui/ControlUI.h"
#include "faust/audio/dsp.h"
#include "jack/net.h"
#include "faust/gui/meta.h"


using namespace std;

class Remote_DSP : public dsp{

    private:
    
    
        Remote_DSP_Factory*      fFactory;
        
        jack_net_master_t* fNetJack;
        float**     fInputs;
        float**     fOutputs;
    
        int         fBufferSize;
    
        int         fNumInputs;
        int         fNumOutputs;
    
        string         fFactoryIndex;
    
        map<string,string>  fMetadatas; //Metadatas extracted from json
        vector<itemInfo*>   fUiItems;   //Items extracted from json
        int         fNumItem;           //Number of real items (!= group or close group)

    
        FAUSTFLOAT*         fOutControl;        //Buffer containing the values of controls
        FAUSTFLOAT*         fInControl;        //Buffer containing the values of controls

        int         countItem();    
    
        static size_t      store_Response(void *buf, size_t size, size_t nmemb, void* userp);
    
    public: 
    
        Remote_DSP();
        virtual ~Remote_DSP();
        
        void            fillBufferWithZeros(int size1, int size2, FAUSTFLOAT** buffer);
    
        virtual int     getNumInputs();
        virtual int     getNumOutputs();

        virtual void    init(int samplingFreq);
        bool            init(int samplingFreq, int buffer_size, string& error);
    
        virtual void    buildUserInterface(UI* ui);
    
        virtual void    compute(int count, FAUSTFLOAT** input, FAUSTFLOAT** output);
    
        void            metadata(Meta* m);
    
        void        decodeJson(const string& json);
        void        setFactory(Remote_DSP_Factory* factory){fFactory = factory;}
};

Remote_DSP*  createRemoteInstance(Remote_DSP_Factory* factory, int samplingRate, int bufferSize, string& error);

void        deleteRemoteInstance(Remote_DSP* dsp);

#endif



