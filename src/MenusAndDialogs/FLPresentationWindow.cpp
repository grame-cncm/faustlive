//
//  FLPresentationWindow.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "FLPresentationWindow.h"
#include "utilities.h"
#include "faust/dsp/llvm-dsp.h"

FLPresentationWindow* FLPresentationWindow::_presWindow = NULL;

//-----------------------PRESENTATION WINDOW IMPLEMENTATION

FLPresentationWindow::FLPresentationWindow(){
    fExampleToOpen = "";
    init();
}

FLPresentationWindow::~FLPresentationWindow(){}

FLPresentationWindow* FLPresentationWindow::_Instance(){
    if(_presWindow == NULL)
        _presWindow = new FLPresentationWindow;
    
    return _presWindow;
}

void FLPresentationWindow::init(){
    
    QDir ImagesDir(":/");
    
    ImagesDir.cd("Images");
    
    QFileInfoList child = ImagesDir.entryInfoList(QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
    
    QGroupBox*      iconeBox = new QGroupBox;
    QGroupBox*      buttonBox = new QGroupBox;
    QGroupBox*      gridBox = new QGroupBox;
    QGroupBox*      textBox = new QGroupBox;
    QGroupBox*      openExamples = new QGroupBox;
    
    QPushButton*    new_Window;
    QPushButton*    open_Window;
    QPushButton*    open_Session;
    QPushButton*    preferences;
    QPushButton*    help;
    
    QPushButton*    ok;
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(iconeBox);
    mainLayout->addWidget(gridBox);
    mainLayout->addWidget(textBox);
    
    setLayout(mainLayout);
    
    //------------------------ICON
    
    QHBoxLayout *layout = new QHBoxLayout;
    
    QLabel *image = new QLabel();
    //Path of the executable + the presentation Image
    QPixmap presImg(ImagesDir.absoluteFilePath("Presentation_Image.png"));
    presImg.scaledToWidth(100, Qt::SmoothTransformation);
    presImg.scaledToHeight(100, Qt::SmoothTransformation);
    image->setPixmap(presImg);
    image->setAlignment(Qt::AlignCenter);
    layout->addWidget(image);
    iconeBox->setLayout(layout);
    
    string welcome = "<h2>WELCOME TO FAUSTLIVE</h2>\nDynamic Faust Compiler version " + string(getCLibFaustVersion());
    QLabel* text = new QLabel(welcome.c_str());
    text->setAlignment(Qt::AlignCenter);
    
    QLabel* subTitle = new QLabel("");
    
    layout->addWidget(text);
    layout->addWidget(subTitle);
    
    QHBoxLayout* layout3 = new QHBoxLayout;
    QGridLayout* layout2 = new QGridLayout;
    
    
    //------------------------GRID BUTTONS
    
    QLabel *new_Image = new QLabel(gridBox);
    //Path of the executable + the presentation Image
    QPixmap newPix(ImagesDir.absoluteFilePath("InitWin.png"));
    newPix = newPix.scaledToWidth(60, Qt::SmoothTransformation);
    new_Image->setPixmap(newPix);
    new_Image->setAlignment(Qt::AlignCenter);
    layout2->addWidget(new_Image, 0, 0);
    
    new_Window = new QPushButton("New Default Window\nOpen a window with simple process");
    new_Window->setToolTip("Open a window containing a default process.");
    new_Window->setFlat(true);
    new_Window->setDefault(false);
    
    layout2->addWidget(new_Window, 0, 1);
    connect(new_Window, SIGNAL(clicked()), this, SLOT(newWindow()));
    
    QLabel *open_Image = new QLabel(gridBox);
    QPixmap openPix(ImagesDir.absoluteFilePath("OpenWin.png"));
    openPix = openPix.scaledToWidth(60, Qt::SmoothTransformation);
    open_Image->setPixmap(openPix);
    layout2->addWidget(open_Image, 1, 0);
    
    open_Window = new QPushButton("Open your File.dsp\n");
    open_Window->setToolTip("Open the DSP you choose.");
    open_Window->setFlat(true);
    open_Window->setDefault(false);
    
    layout2->addWidget(open_Window, 1, 1);
    connect(open_Window, SIGNAL(clicked()), this, SLOT(openWindow()));
    
    QLabel *snap_Image = new QLabel(gridBox);
    QPixmap snapPix(ImagesDir.absoluteFilePath("RecallMenu.png"));
    snapPix = snapPix.scaledToWidth(60, Qt::SmoothTransformation);
    snap_Image->setPixmap(snapPix);
    layout2->addWidget(snap_Image, 2, 0);
    
    open_Session = new QPushButton("Import your Snapshot\n");
    open_Session->setToolTip("Open a saved snapshot.");
    open_Session->setFlat(true);
    open_Session->setDefault(false);
    
    layout2->addWidget(open_Session, 2, 1);
    connect(open_Session, SIGNAL(clicked()), this, SLOT(session()));
    
    QLabel *pref_Image = new QLabel(gridBox);
    QPixmap prefPix(ImagesDir.absoluteFilePath("Preferences.png"));
    prefPix = prefPix.scaledToWidth(60, Qt::SmoothTransformation);
    pref_Image->setPixmap(prefPix);
    layout2->addWidget(pref_Image, 3, 0);
    
    preferences = new QPushButton("Open Preferences Menu\n");
    preferences->setToolTip("Modify audio & compilation preferences");
    preferences->setFlat(true);
    preferences->setDefault(false);
    
    layout2->addWidget(preferences, 3, 1);
    connect(preferences, SIGNAL(clicked()), this, SLOT(pref()));
    
    
    QLabel *help_Image = new QLabel(gridBox);
    QPixmap helpPix(ImagesDir.absoluteFilePath("HelpMenu.png"));
    helpPix = helpPix.scaledToWidth(60, Qt::SmoothTransformation);
    help_Image->setPixmap(helpPix);
    layout2->addWidget(help_Image, 4, 0);
    
    help = new QPushButton("About Faust Live\n Learn all about FaustLive charateristics");
    help->setToolTip("Help Menu.");
    
    layout2->addWidget(help, 4, 1);
    connect(help, SIGNAL(clicked()), this, SLOT(help()));
    help->setFlat(true);
    help->setDefault(false);
    
    
    //-------------------------OPEN EXAMPLES
    
    
    QVBoxLayout *layout5 = new QVBoxLayout;
    
    QListWidget *vue = new QListWidget(openExamples);
    
    QDir examplesDir(":/");
    
    if(examplesDir.cd("Examples")){
        
        QFileInfoList children = examplesDir.entryInfoList(QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
        
        fExampleToOpen = (children.begin())->baseName();
        
        QFileInfoList::iterator it;
        
        for(it = children.begin(); it != children.end(); it++)
            vue->addItem(QString(it->baseName()));
        
        connect(vue, SIGNAL(itemDoubleClicked( QListWidgetItem *)), this, SLOT(itemDblClick(QListWidgetItem *)));
        connect(vue, SIGNAL(itemClicked( QListWidgetItem *)), this, SLOT(itemClick(QListWidgetItem *)));
    }
    
    layout5->addWidget(new QLabel(tr("<h2>Try Out an Example</h2>")));
    layout5->addWidget(vue);
    
    ok = new QPushButton("Open");
    connect(ok, SIGNAL(clicked()), this, SLOT(itemChosen()));
    ok->setDefault(true);
    
    layout5->addWidget(ok);
    
    buttonBox->setLayout(layout2);
    openExamples->setLayout(layout5);
    
    layout3->addWidget(buttonBox);
    layout3->addWidget(openExamples); 
    
    gridBox->setLayout(layout3);
    
    QHBoxLayout *layout4 = new QHBoxLayout;
    
    QPushButton* cancel = new QPushButton("Cancel");
    
    connect(cancel, SIGNAL(clicked()), this, SLOT(hideWindow()));
    layout4->addWidget(new QLabel(""));
    layout4->addWidget(cancel);
    layout4->addWidget(new QLabel(""));    
    QString grame = "<p>Version ";
    grame   += readFile(":/distVersion.txt");
    grame   += " by GRAME, Centre de Creation Musicale";
    QLabel* more = new QLabel(grame);  
    more->setAlignment(Qt::AlignRight);
    
    layout4->addWidget(more);
    textBox->setLayout(layout4);
    
    
    new_Window->setStyleSheet("QPushButton:flat{"
                              "background-color: lightGray;"
                              "color: black;"
                              "border: 2px solid gray;"
                              "border-radius: 6px;"
                              "}"
                              "QPushButton:flat:hover{"
                              "background-color: darkGray;"                         
                              "}" );
    open_Window->setStyleSheet("QPushButton:flat{"
                               "background-color: lightGray;"
                               "color: black;"
                               "border: 2px solid gray;"
                               "border-radius: 6px;"
                               "}"
                               "QPushButton:flat:hover{"
                               "background-color: darkGray;"                         
                               "}" );
    open_Session->setStyleSheet("QPushButton:flat{"
                                "background-color: lightGray;"
                                "color: black;"
                                "border: 2px solid gray;"
                                "border-radius: 6px;"
                                "}"
                                "QPushButton:flat:hover{"
                                "background-color: darkGray;"                         
                                "}" );
    
    preferences->setStyleSheet("QPushButton:flat{"
                               "background-color: lightGray;"
                               "color: black;"
                               "border: 2px solid gray;"
                               "border-radius: 6px;"
                               "}"
                               "QPushButton:flat:hover{"
                               "background-color: darkGray;"                         
                               "}" );
    
    help->setStyleSheet("QPushButton:flat{"
                        "background-color: lightGray;"
                        "color: black;"
                        "border: 2px solid gray;"
                        "border-radius: 6px;"
                        "}"
                        "QPushButton:flat:hover{"
                        "background-color: darkGray;"                         
                        "}" );
    
    centerOnPrimaryScreen(this);
}

//Store the item clicked to open it when the open button is pressed
void FLPresentationWindow::itemClick(QListWidgetItem *item){
    fExampleToOpen = item->text();
}

void FLPresentationWindow::itemChosen(){
    hide();
    emit openExample(fExampleToOpen);
}

//Opens directly a double clicked item
void FLPresentationWindow::itemDblClick(QListWidgetItem* item){
    fExampleToOpen = item->text();
    itemChosen();
}

void FLPresentationWindow::newWindow(){
    emit newWin();
	hide();
}

void FLPresentationWindow::openWindow(){

    emit openWin();
	hide();
}

void FLPresentationWindow::session(){
    emit openSession();
	hide();
}

void FLPresentationWindow::pref(){
    emit openPref();
}

void FLPresentationWindow::help(){
    emit openHelp();
}

void FLPresentationWindow::hideWindow(){
    emit close();
}

void FLPresentationWindow::closeEvent(QCloseEvent* /*event*/){
    hide();
}


