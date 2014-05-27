######################################################################
# FAUSTLIVE.PRO
######################################################################

# THANKS TO QMAKE, THIS FILE WILL PRODUCE THE MAKEFILE OF FAUSTLIVE APPLICATION
# IT DESCRIBES ALL THE LINKED LIBRAIRIES, COMPILATION OPTIONS, THE SOURCES TO BE COMPILED

# APPLICATION SETTINGS

FAUSTDIR = /usr/local/lib/faust
TARGET = FaustLive
VERSION = 1.1
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

OBJECTS_DIR += src
MOC_DIR += src
RCC_DIR += src

# currently no support for httpd and osc on windows version
win32 {

} else {
DEFINES += HTTPDVAR
DEFINES += OSCVAR
}

win32{

RC_FILE = FaustLive.rc

TEMPLATE=vcapp
INCLUDEPATH += C:\Qt\Qt5.2.0\msvc\include
INCLUDEPATH += C:\Users\Sarah\faudiostream-code\architecture
INCLUDEPATH += C:\Qt\Qt5.2.0\msvc\include\QtWidgets
INCLUDEPATH += C:\Users\Sarah\DevTools\portaudio\include
INCLUDEPATH += C:\Users\Sarah\DevTools\qrencode-win32-681f2ea7a41f\qrencode-win32
INCLUDEPATH += C:\Users\Sarah\DevTools\libmicrohttpd\include
INCLUDEPATH += C:\Users\Sarah\DevTools\curl-7.35.0-win32\include\curl

LLVMLIBS = $$system(C:\Users\Sarah\DevTools\llvm-3.4\bin\Release\llvm-config --libs)
LLVMDIR = $$system(C:\Users\Sarah\DevTools\llvm-3.4\bin\Release\llvm-config --ldflags)
CONFIG += console
} else{

ICON = Resources/Images/FaustLiveIcon.icns
TEMPLATE = app
DEPENDPATH += /usr/local/include/faust/gui
INCLUDEPATH += .
INCLUDEPATH += /opt/local/include	
QMAKE_INFO_PLIST = FaustLiveInfo.plist

LLVMLIBS = $$system($$system(which llvm-config) --libs)
LLVMDIR = $$system($$system(which llvm-config) --ldflags)
}

#QMAKE_CXXFLAGS += -Wno-unused-variable -g
QMAKE_EXTRA_TARGETS += all

CONFIG -= x86_64
CONFIG += exceptions rtti

QT+=widgets
QT+=core
QT+=gui
QT+=network


win32{
LIBS+=-LC:\Users\Sarah\faudiostream-code\windows\_output\Win32\faust_vs2012\Debug
LIBS+=-LC:\Users\Sarah\faudiostream-code\windows\faust_httpd_vs2012\_output\Win32\faust_httpd_vs2012\Debug
LIBS+=-LC:\Users\Sarah\DevTools\llvm-3.4\lib\Debug
LIBS+=-LC:\Users\Sarah\DevTools\portaudio\build\msvc\Win32\Debug
LIBS+=-LC:\Users\Sarah\DevTools\qrencode-win32-681f2ea7a41f\qrencode-win32\vc8\.build\Debug-Dll
LIBS+=-LC:\Users\Sarah\DevTools\curl-7.35.0-win32\lib
}
else{
LIBS+=-L/usr/local/lib/faust
LIBS+= $$LLVMDIR
}


LIBS+=-lfaust
LIBS+= $$LLVMLIBS


win32{
equals(HTTPDVAR, 1){
LIBS+=-lHTTPDFaust
LIBS+=-lqrcodelib
LIBS+=-llibcurl
}
}
else{

     equals(static, 1){
	LIBS+=/usr/local/lib/libqrencode.a
	LIBS+=/usr/lib/x86_64-linux-gnu/libmicrohttpd.a
	LIBS+=-lgnutls
	LIBS+=/usr/lib/x86_64-linux-gnu/libcrypto.a
	#LIBS+=/usr/lib/x86_64-linux-gnu/libgnutls.a
	#LIBS+=/lib/x86_64-linux-gnu/libgcrypt.a
	#LIBS+=/usr/lib/x86_64-linux-gnu/libgpg-error.a
	#LIBS+=/usr/lib/x86_64-linux-gnu/libtasn1.a
	#LIBS+=-lp11-kit
     }
     else{
	LIBS+=-lqrencode
	LIBS+=-lmicrohttpd

     }

      LIBS+=-lHTTPDFaust
      LIBS+=-lOSCFaust -loscpack
      LIBS+=-L/opt/local/lib

      LIBS+= $$LLVMDIR

      DEFINES += HTTPCTRL
      DEFINES += QRCODECTRL
}

HEADERS += src/utilities.h 
SOURCES += src/utilities.cpp

equals(REMVAR, 1){
	DEFINES += REMOTE
	LIBS+=-lfaustremote
	LIBS+=-lcurl
	LIBS+=-llo
}

#LIBS+=-lcrypto

equals(CAVAR, 1){
	message("COREAUDIO LINKED")
	LIBS+= -L/opt/local/lib -framework CoreAudio -framework AudioUnit -framework CoreServices
	DEFINES += COREAUDIO
	HEADERS += 	src/CA_audioFactory.h\
			src/CA_audioSettings.h\
			src/CA_audioManager.h\
			src/CA_audioFader.h 
				
	SOURCES += 	src/CA_audioFactory.cpp \
			src/CA_audioSettings.cpp \
			src/CA_audioManager.cpp 
}else{
	message("COREAUDIO NOT LINKED")
}

equals(JVAR, 1){
	message("JACK LINKED")
	LIBS+= -ljack
	DEFINES += JACK
	HEADERS += 	src/JA_audioFactory.h \
				src/JA_audioSettings.h \
				src/JA_audioManager.h \
				src/JA_audioFader.h \
	
	SOURCES += 	src/JA_audioSettings.cpp \
				src/JA_audioManager.cpp \
				src/JA_audioFactory.cpp \
				src/JA_audioFader.cpp 
}else{
	message("JACK NOT LINKED")
}	

equals(NJVAR, 1){
	message("NETJACK LINKED")
	LIBS += -ljacknet
	DEFINES += NETJACK
	HEADERS += 	src/NJ_audioFactory.h \
				src/NJ_audioSettings.h \
				src/NJ_audioManager.h \
				src/NJ_audioFader.h \
				src/JsonParser.h \
							
	
	SOURCES += 	src/NJ_audioFactory.cpp \
				src/NJ_audioSettings.cpp \
				src/NJ_audioManager.cpp \
				src/NJ_audioFader.cpp \
				src/JsonParser.cpp 
}else{
	message("NETJACK NOT LINKED")
}		

equals(ALVAR, 1){
	message("ALSA LINKED")
	LIBS += -lasound
	DEFINES += ALSA
	HEADERS += 	src/AL_audioFactory.h \
				src/AL_audioSettings.h \
				src/AL_audioManager.h \
				src/AL_audioFader.h \
	
	SOURCES += 	src/AL_audioFactory.cpp \
				src/AL_audioSettings.cpp \
				src/AL_audioManager.cpp 
}else{
	message("ALSA NOT LINKED")
}		

equals(PAVAR, 1){
	message("PORT AUDIO LINKED")
win32{
	LIBS += -lportaudio_x86
}
else{
	LIBS += -lportaudio
}
	DEFINES += PORTAUDIO
	HEADERS += 	src/PA_audioFactory.h \
				src/PA_audioSettings.h \
				src/PA_audioManager.h \
				src/PA_audioFader.h \
	
	SOURCES += 	src/PA_audioFader.cpp \
				src/PA_audioFactory.cpp \
				src/PA_audioSettings.cpp \
				src/PA_audioManager.cpp 
}else{
	message("PORT AUDIO NOT LINKED")
}		

HEADERS +=              src/AudioSettings.h \
			src/AudioManager.h \
			src/AudioFactory.h \
			src/AudioCreator.h \
			src/AudioFader_Interface.h \
                        src/AudioFader_Implementation.h \
			src/FJUI.h \
			src/FLToolBar.h \
			src/FLrenameDialog.h \
			src/FLErrorWindow.h \
                        src/FLExportManager.h \
                        src/FLEffect.h \
                        src/FLWindow.h \
                        src/FLApp.h \
                        src/SimpleParser.h
win32{
	equals(HTTPDVAR, 1){
			HEADERS +=		src/HTTPWindow.h 
	}
	HEADERS +=      C:\Users\Sarah\faudiostream-code\architecture\faust\gui\faustqt.h
}
else{
	
	HEADERS +=	        src/FLServerHttp.h \
						src/HTTPWindow.h \
#						API_FAUSTWEB/Faust_Exporter.h \
                        /usr/local/include/faust/gui/faustqt.h

}

SOURCES += 	src/AudioCreator.cpp \
                src/AudioFader_Implementation.cpp \
                src/FLToolBar.cpp \
                src/FLrenameDialog.cpp \
                src/FLErrorWindow.cpp \
                src/FLExportManager.cpp \
                src/FLEffect.cpp \
                src/FLWindow.cpp \
                src/FLApp.cpp
win32{
	equals(HTTPDVAR, 1){
		SOURCES +=	src/HTTPWindow.cpp
	}
}
else{
	SOURCES +=	src/FLServerHttp.cpp \
#				API_FAUSTWEB/Faust_Exporter.cpp \
                src/HTTPWindow.cpp 
}

SOURCES +=      src/SimpleParser.cpp \
                src/main.cpp

RESOURCES     = Resources/application.qrc
win32:RESOURCES += Resources/windows.qrc
unix:RESOURCES += Resources/unix.qrc



