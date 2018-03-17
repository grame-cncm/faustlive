//
//  FLHelpWindow.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLHelpWindow.h"
#include "utilities.h"
#include "FLErrorWindow.h"

#define LLVM_DSP_FACTORY
#ifdef LLVM_DSP_FACTORY
#include "faust/dsp/llvm-dsp.h"
#else
#include "faust/dsp/interpreter-dsp.h"
#endif

//-----------------------ERRORWINDOW IMPLEMENTATION

FLHelpWindow* FLHelpWindow::_helpWindow = NULL;

FLHelpWindow::FLHelpWindow(const QString& libsFolder, const QString& testDSPPath)
{
    fLibsFolder = libsFolder;
    fTestDSPPath = testDSPPath;
    init();
}

FLHelpWindow::~FLHelpWindow()
{
    delete fToolText;
    delete fTreeLibs;
    delete fLibsText;
    delete fAppText;
    delete fWinText;
}

void FLHelpWindow::createInstance(const QString home, const QString& testDSPPath)
{
    if (_helpWindow == NULL) {
        _helpWindow = new FLHelpWindow(home, testDSPPath);
    }
}

void FLHelpWindow::deleteInstance()
{
    delete _helpWindow;
}

FLHelpWindow* FLHelpWindow::_Instance()
{
    return _helpWindow;
}

//Set Text in Tools Menu of HELP
// ---> this could probably be externalized
void FLHelpWindow::setToolText(const QString & currentText)
{
    if (currentText.compare("FAUST") == 0)
        fToolText->setHtml("<br>\nTo develop your own effects, you will need to learn the Faust language.<br><br>""LEARN MORE ABOUT FAUST : <a href = http://faust.grame.fr>faust.grame.fr</a>");
    else if (currentText.compare("LLVM") == 0)
        fToolText->setHtml("<br>\nThanks to its embedded LLVM compiler, this application allows dynamic compilation of your Faust objects.<br><br>""LEARN MORE ABOUT LLVM : <a href = http://llvm.org>llvm.org</a>");
    else if (currentText.compare("COREAUDIO") == 0)
        fToolText->setHtml("<br>Core Audio is the digital audio infrastructure of MAC OS X.<br><br>""LEARN MORE ABOUT COREAUDIO : <a href = http://developer.apple.com/library/ios/#documentation/MusicAudio/Conceptual/CoreAudioOverview/WhatisCoreAudio/WhatisCoreAudio.html>developer.apple.com </a> ");
    else if (currentText.compare("JACK") == 0)
        fToolText->setHtml("<br>Jack (the Jack Audio Connection Kit) is a low-latency audio server. It can connect any number of different applications to a single hardware audio device.<br><br>YOU CAN DOWNLOAD IT HERE : <a href =http://www.jackosx.com> www.jackosx.com</a>\n");
    else if (currentText.compare("NETJACK") == 0)
        fToolText->setHtml("<br>NetJack (fully integrated in Jack) is a Realtime Audio Transport over a generic IP Network. It allows to send audio signals through the network to a server.<br><br>""LEARN MORE ABOUT NETJACK : <a href = http://netjack.sourceforge.net> netjack.sourceforge.net</a>\n");
    else if (currentText.compare("PORTAUDIO") == 0)
        fToolText->setHtml("<br>PortAudio is a free, cross-platform, open-source, audio I/O library. <br><br>""LEARN MORE ABOUT PORTAUDIO : <a href = http://www.portaudio.com/> portaudio.com</a>\n");
    else if (currentText.compare("LIB QRENCODE") == 0)
        fToolText->setHtml("<br>Libqrencode is a C library for encoding data in a QR Code symbol, a kind of 2D symbology that can be scanned by handy terminals such as a mobile phone with CCD.<br><br>""LEARN MORE ABOUT LIB QRENCODE : <a href = http://fukuchi.org/works/qrencode> fukuchi.org/works/qrencode</a>\n");
    else if (currentText.compare("LIB MICROHTTPD") == 0)
        fToolText->setHtml("<br>GNU libmicrohttpd is a small C library that allows running an HTTP server as part of an application.<br><br>""LEARN MORE ABOUT LIB MICROHTTPD : <a href = http://www.gnu.org/software/libmicrohttpd> gnu.org/software/libmicrohttpd</a>\n");
    else if (currentText.compare("OSC PACK") == 0)
        fToolText->setHtml("<br>Oscpack is simply a set of C++ classes for packing and unpacking OSC packets. Oscpack includes a minimal set of UDP networking classes for Windows and POSIX.<br><br>""LEARN MORE ABOUT OSC PACK : <a href = http://code.google.com/p/oscpack> code.google.com/p/oscpack</a>\n");
}

//Set Text in Application Properties Menu of HELP
void FLHelpWindow::setAppPropertiesText(const QString& currentText){
    
    if (currentText.compare("New Default Window") == 0)
        fAppText->setPlainText("\nCreates a new window containing a simple Faust process.\n\n process = !,!:0,0; ");
    
    else if (currentText.compare("Open") == 0)
        fAppText->setPlainText("\nCreates a new window containing the DSP you choose on disk.\n");
    
    else if (currentText.compare("Take Snapshot") == 0)
        fAppText->setPlainText("\nSaves the actual state of the application in a folder : all the windows, their graphical parameters, their audio connections, their position on the screen, ...\n");
    
    else if (currentText.compare("Recall Snapshot") == 0)
        fAppText->setPlainText("\nRestores the state of the application as saved. All current windows are closed. If one of the source file can't be found, a back up file is used.\n");
    
    else if( currentText.compare("Import Snapshot") == 0)
        fAppText->setPlainText("\nAdds the state of the application as saved to the current state of the application. That way, current windows are not closed. Some audio application/windows may have to be renamed during the importation.\n");
    
    else if (currentText.compare("Navigate") == 0)
        fAppText->setPlainText("\nBrings the chosen running window to front end.\n");
    
    else if (currentText.compare("Preferences") == 0){
        
        QString text = "\nWINDOW PREFERENCES : You can choose default compilation options for new windows. (Faust & LLVM options)\n\nAUDIO PREFERENCES : If this version of FaustLive includes multiple audio architectures, you can switch from one to another in Audio Preferences. All opened windows will try to switch. If the update fails, former architecture will be reloaded.\n\nNETWORK PREFERENCES : The compilation web service URL can be modified.";
#ifdef HTTPCTRL
        text += "The port of remote drop (on html interface) can be modified.";  
#endif
        
        text += "\n\nSTYLE PREFERENCES : You can also choose the graphical style of the application.\n";
        fAppText->setPlainText(text);
    }
    else if (currentText.compare("Error Displaying") == 0)
        fAppText->setPlainText("\nDisplays a window every time the program catches an error : whether it's a error in the edited code, a compilation problem, a lack of memory during saving action, ...");
}

//Set Text in Window Properties Menu of HELP
void FLHelpWindow::setWinPropertiesText(const QString& currentText)
{
    if (currentText.compare("Audio Cnx/Dcnx") == 0)
        fWinText->setPlainText("\nWith JackRouter audio device, you can connect a window to another one or to an external application like iTunes, VLC or directly to the computer input/output.\nYou can choose Jack as the audio architecture in the preferences.");
    
    else if (currentText.compare("Edit Source") == 0)
        fWinText->setPlainText("\nThe Faust code corresponding to the active window is opened in a text editor. When you save your modifications, the window(s) corresponding to this source will be updated. The graphical parameters and the audio connections that can be kept will stay unmodified.");
    
    else if (currentText.compare("Drag and Drop / Paste") == 0)
        fWinText->setPlainText("\nIn a window, you can drop or paste : \n - File.dsp\n - Faust code\n - Faust URL\nAn audio crossfade will be calculated between the outcoming and the incoming audio application. The new application will be connected as the outcoming one.\n");
    
    else if (currentText.compare("Duplicate") == 0)
        fWinText->setPlainText("\nCreates a new window, that has the same characteristics : same Faust code, same graphical parameters, same compilation options, ...\n");
    
    else if (currentText.compare("View QrCode") == 0)
        fWinText->setPlainText("\nYou can display a new window with a QRcode so that you can remotely control the User Interface of the audio application.");
    
    else if (currentText.compare("Window Options") == 0){
        
        QString text = "\nYou can add compilation options for Faust Compiler. You can also change the level of optimization for the LLVM compiler. If several windows correspond to the same audio application, they will load the chosen options.";
#ifdef HTTPCTRL
        text+="\n\nThe HTTPD Port corresponds to the TCP port used for remote HTTP control of the interface.\n";
#endif
#ifdef OSCVAR
        text+="\n\nThe OSC Port corresponds to the UDP port used for OSC control. \nWARNING : a port needs a few seconds to be released once a window is closed. Moreover, only 30 OSC ports can be opened at the same time.";
#endif
        fWinText->setPlainText(text);
    }
    else if (currentText.compare("View SVG") == 0)
        fWinText->setPlainText("\nYou can display the SVG diagram of the active Window. It will be opened in your chosen default navigator.");
    
    else if (currentText.compare("Export") == 0)
        fWinText->setPlainText("\nA web service is available to upload your Faust application for another platform or/and architecture.");
}

//Set Faust Lib Text in Help Menu
/* We compile a test.dsp which imports all the faust libraries, declaring their characteristics in metadata to allow us to retrieve them */
void FLHelpWindow::parseLibs(map<string, vector<pair<string, string> > >& infoLibs)
{
	int argc = 2;
#ifdef _WIN32
	argc = argc + 2;
#endif
    const char** argv = new const char*[argc + 1];
    
    argv[0] = "-I";
    
    //The library path is where libraries like the scheduler architecture file are = currentSession
    string libPath = fLibsFolder.toStdString();
    argv[1] = libPath.c_str();
    
#ifdef _WIN32
	argv[2] = "-l";
	argv[3] = "llvm_math.ll";
#endif
    argv[argc] = 0; // NULL terminated argv
    string error;
    string file = fTestDSPPath.toStdString();
  
#ifdef LLVM_DSP_FACTORY
    dsp_factory* temp_factory = createDSPFactoryFromFile(file, argc, argv, "", error, 3);
#else
    dsp_factory* temp_factory = createInterpreterDSPFactoryFromFile(file, argc, argv, error);
#endif
    if (!temp_factory) {
        FLErrorWindow::_Instance()->print_Error(error.c_str());
        return;
    }
    dsp* temp_dsp = temp_factory->createDSPInstance();
 
    if (temp_dsp) {

        MyMeta* meta = new MyMeta;
        temp_dsp->metadata(meta);

        for (size_t i = 0; i < meta->datas.size(); i++) {

            string libName, key, value;
            size_t pos = meta->datas[i].first.find("/");

            if (pos != string::npos) {
                libName = meta->datas[i].first.substr(0, pos);
                key = meta->datas[i].first.substr(pos+1);
            } else {
                key = meta->datas[i].first;
            }

            value = meta->datas[i].second;
            infoLibs[libName].push_back(make_pair(key, value));
        }

        delete temp_dsp;
        
    #ifdef LLVM_DSP_FACTORY
        deleteDSPFactory(static_cast<llvm_dsp_factory*>(temp_factory));
    #else
        deleteInterpreterDSPFactory(static_cast<interpreter_dsp_factory*>(temp_factory));
    #endif

    }
}

void FLHelpWindow::setLibText()
{
    if (QFileInfo(fTreeLibs->currentItem()->text(0)).completeSuffix() != "lib") {
        vector<pair<string, string> > libInfos = fInfoLibs[fTreeLibs->currentItem()->parent()->text(0).toStdString()];
        for (size_t i=0; i<libInfos.size(); i++) {
            if (libInfos[i].first.compare(fTreeLibs->currentItem()->text(0).toStdString()) == 0) {
                fLibsText->setPlainText(libInfos[i].second.c_str());
                break;
            }
        }
    } else {
        fLibsText->setPlainText("");
    }
}

void FLHelpWindow::init()
{
    //----------------------Global Help Window
    
    QGroupBox* winGroup = new QGroupBox(this);
    QVBoxLayout* winLayout = new QVBoxLayout;
    
    QPushButton* pushButton = new QPushButton("OK", winGroup);
    pushButton->connect(pushButton, SIGNAL(clicked()), this, SLOT(hideWindow()));
    
    QTabWidget *myTabWidget = new QTabWidget(winGroup);
    
    //---------------------General
    
    QWidget* tab_general = new QWidget;
    
    QVBoxLayout* generalLayout = new QVBoxLayout;
    
    QPlainTextEdit* generalText = new QPlainTextEdit(tr("\nFaustLive is a dynamic compiler for audio DSP programs coded with Faust. It embeds Faust & LLVM compiler.\n\nEvery window of the application corresponds to an audio application, which parameters you can adjust."));
    
    QLineEdit* lineEdit = new QLineEdit(tr(" Distributed by GRAME - Centre de Creation Musicale"));
    
    generalText->setReadOnly(true);
    lineEdit->setReadOnly(true);
    
    myTabWidget->addTab(tab_general, QString(tr("General")));
    
    generalLayout->addWidget(generalText);
    generalLayout->addWidget(lineEdit);
    
    tab_general->setLayout(generalLayout);
    
    //----------------------Tools
    
    QWidget* tab_tool = new QWidget;
    
    QGridLayout* toolLayout = new QGridLayout;
    
    QListWidget *vue = new QListWidget;
    
    vue->addItem(QString(tr("FAUST")));
    vue->addItem(QString(tr("LLVM")));
    
#ifdef COREAUDIO
    vue->addItem(QString(tr("COREAUDIO")));
#endif
    
#ifdef JACK
    vue->addItem(QString(tr("JACK")));
#endif
    
#ifdef NETJACK
    vue->addItem(QString(tr("NETJACK")));
#endif
    
#ifdef PORTAUDIO
    vue->addItem(QString(tr("PORTAUDIO")));
#endif
    
#ifdef HTTPCTRL
    vue->addItem(QString(tr("LIB MICROHTTPD")));
    vue->addItem(QString(tr("LIB QRENCODE")));
    vue->addItem(QString(tr("OSC PACK")));
#endif
    
    vue->setMaximumWidth(150);
    connect(vue, SIGNAL(currentTextChanged(const QString&)), this, SLOT(setToolText(const QString &)));
    
    toolLayout->addWidget(vue, 0, 0, 1, 1);
    
    fToolText = new QTextBrowser;
    fToolText->setOpenExternalLinks(true);
    fToolText->setReadOnly(true);
    fToolText->setMinimumWidth(300);
    
    myTabWidget->addTab(tab_tool, QString(tr("Tools")));
    
    toolLayout->addWidget(fToolText, 0, 1, 1, 2);
    tab_tool->setLayout(toolLayout);
    
    vue->setCurrentRow(0);
    
    //-----------------------Faust Libraries
    
    QWidget* tab_app1 = new QWidget();
    
    QGridLayout* appLayout1 = new QGridLayout;
    
    fTreeLibs = new QTreeWidget;
    fTreeLibs->setHeaderLabel("Available Librairies");
    
    //    Mettre en route d'ajouter les librairies présentes dans le dossier Libs
    
    parseLibs(fInfoLibs);
    
    QDir libsDir(fLibsFolder);
    
    QFileInfoList children = libsDir.entryInfoList(QDir::Files);
    
    QFileInfoList::iterator it;
    
    for(it = children.begin(); it != children.end(); it++){
        if(it->completeSuffix().compare("ll") != 0 && it->completeSuffix().compare("dsp") != 0){
            
            
            QString completeName = it->baseName() + "." + it->completeSuffix();
            QTreeWidgetItem* newItem = new QTreeWidgetItem(fTreeLibs, QStringList(completeName));
            
            for(size_t i=0; i<fInfoLibs[completeName.toStdString()].size(); i++){
                
                QTreeWidgetItem* childItem = new QTreeWidgetItem(newItem, QStringList(QString(fInfoLibs[completeName.toStdString()][i].first.c_str())));
                
                newItem->addChild(childItem);
            }
            
            fTreeLibs->addTopLevelItem(newItem);
        }
    }
    
    connect(fTreeLibs, SIGNAL(itemSelectionChanged()), this, SLOT(setLibText()));
    fTreeLibs->setMaximumWidth(150);
    
    appLayout1->addWidget(fTreeLibs, 0, 0, 1, 1);
    
    fLibsText = new QPlainTextEdit;
    fLibsText->setReadOnly(true);
    fLibsText->setMinimumWidth(300);

    appLayout1->addWidget(fLibsText, 0, 1, 1, 2);
    
    myTabWidget->addTab(tab_app1, QString(tr("Faust Libraries")));
    tab_app1->setLayout(appLayout1);
    
    //-----------------------Faust Live Menu
    
    QWidget* tab_app = new QWidget();
    
    QGridLayout* appLayout = new QGridLayout;
    
    QListWidget *vue2 = new QListWidget;
    
    vue2->addItem(QString(tr("New Default Window")));
    vue2->addItem(QString(tr("Open")));    
    vue2->addItem(QString(tr("Take Snapshot")));
    vue2->addItem(QString(tr("Recall Snapshot")));
    vue2->addItem(QString(tr("Import Snapshot")));
    vue2->addItem(QString(tr("Navigate")));
    vue2->addItem(QString(tr("Preferences")));
    vue2->addItem(QString(tr("Error Displaying")));
    
    vue2->setMaximumWidth(150);
    connect(vue2, SIGNAL(currentTextChanged(const QString&)), this, SLOT(setAppPropertiesText(const QString &)));
    
    appLayout->addWidget(vue2, 0, 0, 1, 1);
    
    fAppText = new QPlainTextEdit;
    fAppText->setReadOnly(true);
    fAppText->setMinimumWidth(300);
    
    vue2->setCurrentRow(0);
    
    appLayout->addWidget(fAppText, 0, 1, 1, 2);
    
    myTabWidget->addTab(tab_app, QString(tr("Application Properties")));
    
    tab_app->setLayout(appLayout);
    
    //-----------------------Window Properties
    
    QWidget* tab_win = new QWidget();
    
    QGridLayout* winPropLayout = new QGridLayout;
    
    QListWidget *vue3 = new QListWidget;
    
#ifdef JACK
    vue3->addItem(QString(tr("Audio Cnx/Dcnx")));
#endif
    
    vue3->addItem(QString(tr("Edit Source")));    
    vue3->addItem(QString(tr("Drag and Drop / Paste")));
    vue3->addItem(QString(tr("Duplicate")));
#ifdef HTTPCTRL
    vue3->addItem(QString(tr("View QrCode")));
#endif
    
    vue3->addItem(QString(tr("Window Options")));
    vue3->addItem(QString(tr("View SVG")));
    vue3->addItem(QString(tr("Export")));
    
    vue3->setMaximumWidth(150);
    connect(vue3, SIGNAL(currentTextChanged(const QString&)), this, SLOT(setWinPropertiesText(const QString &)));
    
    winPropLayout->addWidget(vue3, 0, 0, 1, 1);
    
    fWinText = new QPlainTextEdit;
    fWinText->setReadOnly(true);
    fWinText->setMinimumWidth(300);
    
    vue3->setCurrentRow(0);
    
    winPropLayout->addWidget(fWinText, 0, 1, 1, 2);
    
    tab_win->setLayout(winPropLayout);
    
    myTabWidget->addTab(tab_win, QString(tr("Window Properties")));
    
    //Help Window Layout
    
    winLayout->addWidget(myTabWidget);
    winLayout->addWidget(new QLabel(""));
    winLayout->addWidget(pushButton);
    winGroup->setLayout(winLayout);
    setCentralWidget(winGroup);
    
    myTabWidget->setCurrentIndex(0);
    
    
    centerOnPrimaryScreen(this);
}

void FLHelpWindow::hideWindow(){
    emit close();
}

void FLHelpWindow::closeEvent(QCloseEvent* /*event*/){
    hide();
}


