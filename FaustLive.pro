######################################################################
# FAUSTLIVE.PRO
######################################################################

# THANKS TO QMAKE, THIS FILE WILL PRODUCE THE MAKEFILE OF FAUSTLIVE APPLICATION
# IT DESCRIBES ALL THE LINKED LIBRAIRIES, COMPILATION OPTIONS, THE SOURCES TO BE COMPILED, ...

# APPLICATION SETTINGS
ICON = FaustLiveIcon.icns
QMAKE_INFO_PLIST = FaustLiveInfo.plist
system(cp -R Images Libs Examples FaustLive.app/Contents/Resources/)

TEMPLATE = app
TARGET = FaustLive
DEPENDPATH += /usr/local/include/faust/gui
INCLUDEPATH += .
INCLUDEPATH += /opt/local/include	

QMAKE_CXXFLAGS += -g
CONFIG -= x86_64

OPT = $$system(llvm-config --libs)
LLVMDIR = $$system(llvm-config --ldflags)
LIBS+= $$LLVMDIR $$OPT

LIBS+=-L/opt/local/lib -framework CoreAudio -framework AudioUnit -framework CoreServices
QT+=network

LIBS+=-L/usr/local/lib/faust
LIBS+=-lfaust
LIBS+=-lHTTPDFaust

LIBS+=-L/opt/local/lib -lmicrohttpd
LIBS+=-L/opt/local/lib -lqrencode

CONFIG += JVAR NJVAR CAVAR

$$CAVAR{
	message("COREAUDIO NOT LINKED")
}else{
	message("COREAUDIO LINKED")
	LIBS+= 
	DEFINES += COREAUDIO
	HEADERS += 	CA_audioFactory.h\
				CA_audioSettings.h\
				CA_audioManager.h\
				CA_audioFader.h \
				
	SOURCES += 	CA_audioFactory.cpp \
				CA_audioSettings.cpp \
				CA_audioManager.cpp \
}

$$JVAR{
	message("JACK NOT LINKED")
}else{
	message("JACK LINKED")
	LIBS+= -ljack
	DEFINES += JACK
	HEADERS += 	JA_audioFactory.h \
				JA_audioSettings.h \
				JA_audioManager.h \
				JA_audioFader.h \
	
	SOURCES += 	JA_audioSettings.cpp \
				JA_audioManager.cpp \
				JA_audioFactory.cpp \
				JA_audioFader.cpp \
}	

$$NJVAR{
	message("NETJACK NOT LINKED")
}else{
	message("NETJACK LINKED")
	LIBS += -ljacknet
	DEFINES += NETJACK
	HEADERS += 	NJ_audioFactory.h \
				NJ_audioSettings.h \
				NJ_audioManager.h \
				NJ_audioFader.h \
	
	SOURCES += 	NJ_audioFactory.cpp \
				NJ_audioSettings.cpp \
				NJ_audioManager.cpp \
				NJ_audioFader.cpp \
}		

HEADERS += 	AudioSettings.h \
			AudioManager.h \
			AudioFactory.h \
			AudioCreator.h \
			audioFader_Interface.h \
			audioFader_Implementation.h \
			/usr/local/include/faust/gui/faustqt.h \
			FJUI.h \
			FLToolBar.h \
			HTTPWindow.h \
			FLrenameDialog.h \
			FLErrorWindow.h \
			FLExportManager.h \
			Effect.h \
			FLWindow.h \ 
			FaustLiveApp.h \
							
SOURCES += 	AudioCreator.cpp \
			audioFader_Implementation.cpp \
			FLToolBar.cpp \
			HTTPWindow.cpp \
			FLrenameDialog.cpp \
			FLErrorWindow.cpp \
			FLExportManager.cpp \
			Effect.cpp \
			FLWindow.cpp \ 
			FaustLiveApp.cpp \
			main.cpp \