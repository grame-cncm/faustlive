######################################################################
# FAUSTLIVE.PRO
######################################################################

# THANKS TO QMAKE, THIS FILE WILL PRODUCE THE MAKEFILE OF FAUSTLIVE APPLICATION
# IT DESCRIBES ALL THE LINKED LIBRAIRIES, COMPILATION OPTIONS, THE SOURCES TO BE COMPILED

## Application Settings
OBJECTS_DIR += src/objectsFolder

MOC_DIR += src/objectsFolder

RCC_DIR += src/Utilities

QMAKE_EXTRA_TARGETS += all

CONFIG -= x86_64
CONFIG += exceptions rtti

## QT libraries needed
QT+=widgets
QT+=core
QT+=gui
QT+=network

TARGET = FaustLive
#VERSION = 1.1
DEFINES += APP_VERSION=\\\"1.1\\\"

## Images/Examples and other needed resources
RESOURCES     = Resources/application.qrc
win32:RESOURCES += Resources/windows.qrc
macx:RESOURCES += Resources/unix.qrc

####### INCLUDES PATHS && LIBS PATHS

win32{

	RC_FILE = FaustLive.rc

## VISUAL STUDIO PROJECT
	TEMPLATE = vcapp
	INCLUDEPATH += C:\Qt\Qt5.2.0\msvc\include
	INCLUDEPATH += C:\Users\Sarah\faudiostream-code\architecture
	INCLUDEPATH += C:\Qt\Qt5.2.0\msvc\include\QtWidgets
	INCLUDEPATH += C:\Users\Sarah\DevTools\portaudio\include
	INCLUDEPATH += C:\Users\Sarah\DevTools\qrencode-win32-681f2ea7a41f\qrencode-win32
	INCLUDEPATH += C:\Users\Sarah\DevTools\libmicrohttpd\include
	INCLUDEPATH += C:\Users\Sarah\DevTools\curl-7.35.0-win32\include\curl

	LLVMLIBS = $$system(C:\Users\Sarah\DevTools\llvm-3.4\bin\Release\llvm-config --libs)
	#LLVMDIR = $$system(C:\Users\Sarah\DevTools\llvm-3.4\bin\Release\llvm-config --ldflags)

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

INCLUDEPATH += src/Audio
INCLUDEPATH += src/MenusAndDialogs
INCLUDEPATH += src/MainStructure
INCLUDEPATH += src/Network
INCLUDEPATH += src/Utilities

win32{
	Debug{
		LIBS+=-LC:\Users\Sarah\faudiostream-code\windows\_output\Win32\faust_vs2012\Debug
		LIBS+=-LC:\Users\Sarah\faudiostream-code\windows\faust_httpd_vs2012\_output\Win32\faust_httpd_vs2012\Debug
		LIBS+=-LC:\Users\Sarah\DevTools\llvm-3.4\lib\Debug
		LIBS+=-LC:\Users\Sarah\DevTools\portaudio\build\msvc\Win32\Debug
		LIBS+=-LC:\Users\Sarah\DevTools\qrencode-win32-681f2ea7a41f\qrencode-win32\vc8\.build\Debug-Dll
		LIBS+=-LC:\Users\Sarah\DevTools\curl-7.35.0-win32\lib
		CONFIG += console
}
	Release{
		LIBS+=-LC:\Users\Sarah\faudiostream-code\windows\_output\Win32\libfaust_vs2012\Release
		LIBS+=-LC:\Users\Sarah\faudiostream-code\windows\faust_httpd_vs2012\_output\Win32\faust_httpd_vs2012\Release
		LIBS+=-LC:\Users\Sarah\DevTools\llvm-3.4\lib\Release
		LIBS+=-LC:\Users\Sarah\DevTools\portaudio\build\msvc\Win32\Release
		LIBS+=-LC:\Users\Sarah\DevTools\qrencode-win32-681f2ea7a41f\qrencode-win32\vc8\.build\Debug-Dll
		LIBS+=-LC:\Users\Sarah\DevTools\curl-7.35.0-win32\lib
	}
}
else{
	LIBS+=-L/usr/local/lib/faust -L/usr/lib/faust
	LIBS+= $$LLVMDIR
}


LIBS+=-lfaust
LIBS+= $$LLVMLIBS

win32{
	equals(HTTPDVAR, 1){
		LIBS+=-lHTTPDFaust
		LIBS+=-lqrcodelib
		LIBS+=-llibcurl
		DEFINES += HTTPCTRL
		#DEFINES += QRCODECTRL
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
		LIBS+=-lcrypto
     }
	  
	  LIBS+=-lHTTPDFaust
      LIBS+=-lOSCFaust -loscpack
      LIBS+=-L/opt/local/lib

      LIBS+= $$LLVMDIR

      DEFINES += HTTPCTRL
      DEFINES += QRCODECTRL
      DEFINES += OSCVAR
}

HEADERS += src/Utilities/utilities.h 
SOURCES += src/Utilities/utilities.cpp

########## DEFINES/LIBS/SOURCES/... depending on audio drivers | remote processing feature

equals(REMVAR, 1){
	DEFINES += REMOTE
	LIBS+=-lfaustremote
	LIBS+=-lcurl
	LIBS+=-llo
}

equals(CAVAR, 1){
	message("COREAUDIO LINKED")
	LIBS+= -L/opt/local/lib -framework CoreAudio -framework AudioUnit -framework CoreServices
	DEFINES += COREAUDIO
	INCLUDEPATH += src/Audio/CA
	
	HEADERS += 	src/Audio/CA/CA_audioFactory.h\
				src/Audio/CA/CA_audioSettings.h\
				src/Audio/CA/CA_audioManager.h\
				src/Audio/CA/CA_audioFader.h 
				
	SOURCES += 	src/Audio/CA/CA_audioFactory.cpp \
				src/Audio/CA/CA_audioSettings.cpp \
				src/Audio/CA/CA_audioManager.cpp 
}else{
	message("COREAUDIO NOT LINKED")
}

equals(JVAR, 1){
	message("JACK LINKED")
	LIBS+= -ljack
	DEFINES += JACK
	
	INCLUDEPATH += src/Audio/JA
		
	HEADERS += 	src/Audio/JA/JA_audioFactory.h \
				src/Audio/JA/JA_audioSettings.h \
				src/Audio/JA/JA_audioManager.h \
				src/Audio/JA/JA_audioFader.h \
	
	SOURCES += 	src/Audio/JA/JA_audioSettings.cpp \
				src/Audio/JA/JA_audioManager.cpp \
				src/Audio/JA/JA_audioFactory.cpp \
				src/Audio/JA/JA_audioFader.cpp 
}else{
	message("JACK NOT LINKED")
}	

equals(NJVAR, 1){
	message("NETJACK LINKED")
	LIBS += -ljacknet
	DEFINES += NETJACK
	
	INCLUDEPATH += src/Audio/NJ
	
	HEADERS += 	src/Audio/NJ/NJ_audioFactory.h \
				src/Audio/NJ/NJ_audioSettings.h \
				src/Audio/NJ/NJ_audioManager.h \
				src/Audio/NJ/NJ_audioFader.h 
	
	SOURCES += 	src/Audio/NJ/NJ_audioFactory.cpp \
				src/Audio/NJ/NJ_audioSettings.cpp \
				src/Audio/NJ/NJ_audioManager.cpp \
				src/Audio/NJ/NJ_audioFader.cpp 
}else{
	message("NETJACK NOT LINKED")
}		

equals(ALVAR, 1){
	message("ALSA LINKED")
	LIBS += -lasound
	DEFINES += ALSA
	
	INCLUDEPATH += src/Audio/AL
	
	HEADERS += 	src/Audio/AL/AL_audioFactory.h \
				src/Audio/AL/AL_audioSettings.h \
				src/Audio/AL/AL_audioManager.h \
				src/Audio/AL/AL_audioFader.h \
	
	SOURCES += 	src/Audio/AL/AL_audioFactory.cpp \
				src/Audio/AL/AL_audioSettings.cpp \
				src/Audio/AL/AL_audioManager.cpp 
}else{
	message("ALSA NOT LINKED")
}		

win32{
	PAVAR = 1
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
	
	INCLUDEPATH += src/Audio/PA
	
	HEADERS += 	src/Audio/PA/PA_audioFactory.h \
				src/Audio/PA/PA_audioSettings.h \
				src/Audio/PA/PA_audioManager.h \
				src/Audio/PA/PA_audioFader.h \
	
	SOURCES += 	src/Audio/PA/PA_audioFader.cpp \
				src/Audio/PA/PA_audioFactory.cpp \
				src/Audio/PA/PA_audioSettings.cpp \
				src/Audio/PA/PA_audioManager.cpp 
}else{
	message("PORT AUDIO NOT LINKED")
}		

########## HEADERS AND SOURCES OF PROJECT

HEADERS +=  src/Audio/AudioSettings.h \
			src/Audio/AudioManager.h \
			src/Audio/AudioFactory.h \
			src/Audio/AudioCreator.h \
			src/Audio/AudioFader_Interface.h \
            src/Audio/AudioFader_Implementation.h \
			src/Audio/JA/FJUI.h \
			src/MenusAndDialogs/FLToolBar.h \
			src/MenusAndDialogs/FLrenameDialog.h \
			src/MenusAndDialogs/FLHelpWindow.h \
			src/MenusAndDialogs/FLPresentationWindow.h \
			src/MenusAndDialogs/FLErrorWindow.h \
            src/MenusAndDialogs/FLExportManager.h \
            src/MenusAndDialogs/FLPreferenceWindow.h \
            src/MainStructure/FLEffect.h \
            src/MainStructure/FLWindow.h \
            src/MainStructure/FLSettings.h \
            src/MainStructure/FLApp.h \
            src/MenusAndDialogs/SimpleParser.h
win32{
	equals(HTTPDVAR, 1){
			HEADERS +=		src/HTTPWindow.h 
	}
	HEADERS +=      C:\Users\Sarah\faudiostream-code\architecture\faust\gui\faustqt.h
}
else{
	
	HEADERS +=	        src/Network/FLServerHttp.h \
						src/Network/HTTPWindow.h \
#						API_FAUSTWEB/Faust_Exporter.h \
                        /usr/local/include/faust/gui/faustqt.h

}

<<<<<<< HEAD
SOURCES += 	src/Audio/AudioCreator.cpp \
            src/Audio/AudioFader_Implementation.cpp \
        	src/MenusAndDialogs/FLToolBar.cpp \
            src/MenusAndDialogs/FLrenameDialog.cpp \
			src/MenusAndDialogs/FLHelpWindow.cpp \
			src/MenusAndDialogs/FLPresentationWindow.cpp \
            src/MenusAndDialogs/FLErrorWindow.cpp \
            src/MenusAndDialogs/FLExportManager.cpp \
            src/MainStructure/FLEffect.cpp \
            src/MainStructure/FLWindow.cpp \
            src/MainStructure/FLSettings.cpp \
            src/MenusAndDialogs/FLPreferenceWindow.cpp \
            src/MainStructure/FLApp.cpp
=======
SOURCES += 	src/AudioCreator.cpp \
            src/AudioFader_Implementation.cpp \
        	src/FLToolBar.cpp \
            src/FLrenameDialog.cpp \
            src/FLErrorWindow.cpp \
            src/FLExportManager.cpp \
            src/FLEffect.cpp \
            src/FLWindow.cpp \
            src/FLComponentWindow.cpp \
            src/FLSettings.cpp \
            src/FLApp.cpp
>>>>>>> 996e52e2db956117aa98f01297c6de6116dc56cf
win32{
	equals(HTTPDVAR, 1){
		SOURCES +=	src/Network/HTTPWindow.cpp
	}
}
else{
	SOURCES +=	src/Network/FLServerHttp.cpp \
#				API_FAUSTWEB/Faust_Exporter.cpp \
                src/Network/HTTPWindow.cpp 
}

SOURCES +=      src/MenusAndDialogs/SimpleParser.cpp \
                src/Utilities/main.cpp



