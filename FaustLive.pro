######################################################################
# FAUSTLIVE.PRO
######################################################################

# THANKS TO QMAKE, THIS FILE WILL PRODUCE THE MAKEFILE OF FAUSTLIVE APPLICATION
# IT DESCRIBES ALL THE LINKED LIBRAIRIES, COMPILATION OPTIONS, THE SOURCES TO BE COMPILED

# APPLICATION SETTINGS

FAUSTDIR = /usr/local/lib/faust

TEMPLATE = vcapp
TARGET = FaustLive
INCLUDEPATH += C:\Qt\Qt5.2.0\5.2.0\msvc2012\include
INCLUDEPATH += C:\Users\Sarah\faudiostream-code\architecture
INCLUDEPATH += C:\Qt\Qt5.2.0\5.2.0\msvc2012\include\QtWidgets
INCLUDEPATH += C:\Users\Sarah\DevTools\portaudio\include

ICON = Resources/FaustLiveIcon.icns
QMAKE_INFO_PLIST = FaustLiveInfo.plist

OBJECTS_DIR += src
MOC_DIR += src
RCC_DIR += src

CONFIG -= x86_64

LLVMLIBS = $$system(llvm-config --libs)
LLVMDIR = $$system(llvm-config --ldflags)

QT+=gui
QT-=core
QT+=network
QT+=widgets

LIBS+= -LC:\Users\Sarah\faudiostream-code\windows\_output\Win32\faust_vs2012\Debug
LIBS+= -LC:\Users\Sarah\DevTools\llvm-3.4\lib\Debug

LIBS+=-lfaust
LIBS+= $$LLVMDIR $$LLVMLIBS

HEADERS += src/utilities.h 
SOURCES += src/utilities.cpp
	
LIBS+= -LC:\Users\Sarah\DevTools\portaudio\build\msvc\Win32\Debug
LIBS += -lportaudio_x86
DEFINES += PORTAUDIO
HEADERS += 	src/PA_audioFactory.h \
		src/PA_audioSettings.h \
		src/PA_audioManager.h \
		src/PA_audioFader.h \

SOURCES += 	src/PA_audioFader.cpp \
		src/PA_audioFactory.cpp \
		src/PA_audioSettings.cpp \
		src/PA_audioManager.cpp 
		

HEADERS +=	src/AudioSettings.h \
		src/AudioManager.h \
		src/AudioFactory.h \
		src/AudioCreator.h \
		src/AudioFader_Interface.h \
		src/AudioFader_Implementation.h \
		C:\Users\Sarah\faudiostream-code\architecture\faust\gui\faustqt.h \
		src/FJUI.h \
		src/FLToolBar.h \
		#src/HTTPWindow.h \
		src/FLrenameDialog.h \
		src/FLErrorWindow.h \
		src/FLExportManager.h \
		#src/FLServerHttp.h \
		src/FLEffect.h \
		src/FLWindow.h \ 
		src/FLApp.h \
    		src/SimpleParser.h 

SOURCES += 	src/AudioCreator.cpp \
		src/AudioFader_Implementation.cpp \
		src/FLToolBar.cpp \
		#src/HTTPWindow.cpp \
		src/FLrenameDialog.cpp \
		src/FLErrorWindow.cpp \
		src/FLExportManager.cpp \
		#src/FLServerHttp.cpp \
		src/FLEffect.cpp \
		src/FLWindow.cpp \ 
		src/FLApp.cpp \
		src/main.cpp \
    		src/SimpleParser.cpp 

 RESOURCES     = Resources/application.qrc




