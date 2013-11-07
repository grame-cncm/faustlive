//
//  HTTPWindow.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "HTTPWindow.h"

#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include <qrencode.h>

#include <QtGui>
#include <QtNetwork>

#include "faust/gui/httpdUI.h"




using namespace std;

//---------------------------HTTPWINDOW IMPLEMENTATION

HTTPWindow::HTTPWindow(){
    
    fIPaddress = "localhost";
    fEntireAddress = "";
    fShortcut = false;
    fInterface = NULL;
    fTitle ="";
}

HTTPWindow::~HTTPWindow(){
    if(fInterface){
        delete fInterface;
    }
    
    //    delete fTitle;
}

string HTTPWindow::getUrl(){
    
    string url = "http://" + fEntireAddress;
    
    return url;
}

int HTTPWindow::get_Port(){
    if(fInterface != NULL)
        return fInterface->getTCPPort();
    else
        return 0;
}

void HTTPWindow::displayQRCode(char* url){
    
    string myURL(url);
    QWidget* centralWidget = new QWidget;
    setCentralWidget(centralWidget);
    //    QTextEdit* httpdText = new QTextEdit(centralWidget);
    QTextBrowser* myBro = new QTextBrowser(centralWidget);
    
    //Construction of the flashcode
    const int padding = 5;
    QRcode* qrc = QRcode_encodeString(url, 0, QR_ECLEVEL_H, QR_MODE_8, 1);
    
    //   qDebug() << "QRcode width = " << qrc->width;
    
    // build the QRCode image
    QImage image(qrc->width+2*padding, qrc->width+2*padding, QImage::Format_Mono);
    // clear the image
    for (int y=0; y<qrc->width+2*padding; y++) {
        for (int x=0; x<qrc->width+2*padding; x++) {
            image.setPixel(x, y, 0);
        }
    }
    // copy the qrcode inside
    for (int y=0; y<qrc->width; y++) {
        for (int x=0; x<qrc->width; x++) {
            image.setPixel(x+padding, y+padding, qrc->data[y*qrc->width+x]&1);
        }
    }
    
    QImage big = image.scaledToWidth(qrc->width*8);
    QLabel* myLabel = new QLabel(centralWidget);
    
    fQrCode = QPixmap::fromImage(big);
    
    myLabel->setPixmap(fQrCode);
    
    //----Written Address
    
    QString sheet = QString::fromLatin1("a{ text-decoration: underline; color: white; font: Menlo; font-size: 14px }");
    //    myBro->document()->setDefaultStyleSheet(sheet);
    //    myBro->setStyleSheet("*{color: white; font: Menlo; font-size: 14px }");
    
    string text("<br>Connect You To");
    text += "<br><a href = http://" + myURL + ">"+ myURL+"</a>";
    text += "<br>Or Flash the code below";
    
    myBro->setOpenExternalLinks(true);
    myBro->setHtml(text.c_str());
    myBro->setAlignment(Qt::AlignCenter);
    myBro->setFixedWidth(qrc->width*8);
    //    myBro->setFixedHeight(myBro->minimumHeight());
    
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(myBro, 0, 1);
    mainLayout->addWidget(myLabel, 1, 1);
    centralWidget->setLayout(mainLayout);
    //    centralWidget->show();
    //    centralWidget->adjustSize();
}

void HTTPWindow::frontShow_Httpd(string windowTitle){
    
    setWindowTitle(windowTitle.c_str());
    raise();
    show();
    adjustSize();
}

void HTTPWindow::display_HttpdWindow(int x, int y){
    //Display Message (URL + portNumber)+ QRcode in the window
   
    move(x, y);
    
    char url[256];
    snprintf(url, 255, "%s:7777/%i", fIPaddress.c_str(), fInterface->getTCPPort());
    
    
    stringstream s;
    s<<fInterface->getTCPPort();
    
    fEntireAddress = fIPaddress;
    fEntireAddress += ":";
    fEntireAddress += s.str();
    
    displayQRCode(url);
    frontShow_Httpd(fTitle);   
}

bool HTTPWindow::is_httpdWindow_active(){
    return isActiveWindow();
}

void HTTPWindow::hide_httpdWindow(){
    hide();
}

void HTTPWindow::search_IPadress(){
    
    //If IPadress was already found
    if(strcmp(fIPaddress.c_str(), "localhost")  != 0){}
    
    //If not, looking for it
    else{
        QTcpSocket sock;
        QHostAddress IP;
        sock.connectToHost("8.8.8.8", 53); // google DNS, or something else reliable
        if (sock.waitForConnected(5000)) {
            IP = sock.localAddress();
            fIPaddress = IP.toString().toStdString();
        }
        else {
            fIPaddress = "localhost";
        }
    }
}

bool HTTPWindow::build_httpdInterface(const char* error, string windowTitle, dsp* current_DSP, int port){
    
    //Allocation of HTTPD interface
    if(fInterface != NULL) delete fInterface;
    
    fTitle = windowTitle;
    
    stringstream s;
    s<<port;
    fPort = s.str();
    
    fOptionPort = "-port";
    
    char* argv[3];

    argv[0] = (char*)(fTitle.c_str());
    argv[1] = (char*)(fOptionPort.c_str());
    argv[2] = (char*)(fPort.c_str());

    fInterface = new httpdUI(argv[0], 3, argv);

    if(fInterface){
        
        current_DSP->buildUserInterface(fInterface);
        return true;
    }
    
    else{
        error = "ERROR = Impossible to allocate a HTTPD Interface";
        return false;
    }
}

void HTTPWindow::launch_httpdInterface(){
    fInterface->run();
}

void HTTPWindow::toPNG(){
    
    string fileName = getenv("HOME");
    fileName +="/Desktop/";
    fileName += fTitle;
    fileName += ".png";
    
    QFile file(fileName.c_str());
    file.open(QIODevice::WriteOnly);
    fQrCode.save(&file, "PNG");
}

void HTTPWindow::contextMenuEvent(QContextMenuEvent* ev){
    
    QMenu *menu = new QMenu();
    
    QAction* exportToPNG = new QAction("Export to PNG",this);
    
    menu->addAction(exportToPNG);
    QAction* act = menu->exec(ev->globalPos());
    
    if(act == exportToPNG)
        toPNG();
    
    delete exportToPNG;
    delete menu;
    
}

void HTTPWindow::keyPressEvent(QKeyEvent* event){ 
    
    if(event->key() == Qt::Key_Alt) fShortcut = true;
}

void HTTPWindow::keyReleaseEvent(QKeyEvent* event){
    
    if(event->key() == Qt::Key_Alt) fShortcut = false;
}

void HTTPWindow::closeEvent(QCloseEvent* /*event*/){
    
    this->hide();
    
    if(fShortcut) emit closeAll();
}



