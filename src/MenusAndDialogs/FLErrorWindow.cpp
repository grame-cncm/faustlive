//
//  FLErrorWindow.cpp
//  
//
//  Created by Sarah Denoux on 13/05/13.
//  Copyright (c) 2013 GRAME. All rights reserved.
//

#include <QDesktopWidget>
#include <QApplication>
#include <QWidgetList>
#include <QDate>
#include <QTime>

#include "FLErrorWindow.h"

//-----------------------ERRORWINDOW IMPLEMENTATION

FLErrorWindow* FLErrorWindow::_errorWindow = NULL;

FLErrorWindow::FLErrorWindow()
{
    fErrorText = new QTextEdit(this);
    init_Window();
    setWindowTitle("MESSAGE_WINDOW");
}

FLErrorWindow* FLErrorWindow::_Instance()
{
    if (_errorWindow == NULL)
        _errorWindow = new FLErrorWindow;
    return _errorWindow;
}

FLErrorWindow::~FLErrorWindow()
{
    delete fErrorText;
}

void FLErrorWindow::init_Window()
{
    fWidget = new QWidget(this);
    fButton = new QPushButton(tr("OK"), this);
    connect(fButton, SIGNAL(clicked()), this, SLOT(hideWin()));
    connect(fButton, SIGNAL(clicked()), this, SLOT(redirectClose()));
    fLayout = new QVBoxLayout();
    
    QFont font;
    font.setFamily(QString::fromUtf8("Menlo"));
    font.setPointSize(12);
    fErrorText->setFont(font);
    
    fErrorText->setReadOnly(true);
    
    QSize screenSize;
    
    QDesktopWidget *dw = QApplication::desktop();
    
	QWidgetList l = QApplication::topLevelWidgets();
	if (l.empty()) {
    	screenSize = dw->availableGeometry(dw->primaryScreen()).size();
    } else {
    	QWidget* w = l.first();	
    	screenSize = dw->screenGeometry(w).size();
    }
    
    this->setGeometry(screenSize.width()*3/4 , 0,screenSize.width()/4,screenSize.height()/10);
    this->setCentralWidget(fWidget);
    
    fLayout->addWidget(fErrorText);
    fLayout->addWidget(fButton);
    
    fWidget->setLayout(fLayout);
}

void FLErrorWindow::redirectClose()
{
    emit close();
}

void FLErrorWindow::closeEvent(QCloseEvent* /*event*/)
{
    this->hideWin();
    if (QApplication::keyboardModifiers() == Qt::AltModifier)
        emit closeAll();
}

void FLErrorWindow::hideWin()
{
    fErrorText->setPlainText("");
    hide();
}

void FLErrorWindow::print_Error(const QString& text)
{
    QString inter = fErrorText->toPlainText();
    QString wholeText = inter + "\n\n" + QDate::currentDate().toString() + "  " + QTime::currentTime().toString() +":\n" +text;
    
    fErrorText->setPlainText(wholeText);
    this->adjustSize();
    show();
    raise(); 
}

