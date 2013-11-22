######################################################################
# FAUSTLIVE.PRO
######################################################################

# THANKS TO QMAKE, THIS FILE WILL PRODUCE THE MAKEFILE OF FAUSTLIVE APPLICATION
# IT DESCRIBES ALL THE LINKED LIBRAIRIES, COMPILATION OPTIONS, THE SOURCES TO BE COMPILED

# APPLICATION SETTINGS

FAUSTDIR = /usr/local/lib/faust

TEMPLATE = app
TARGET = FaustLive
DEPENDPATH += /usr/local/include/faust/gui
INCLUDEPATH += .
INCLUDEPATH += /opt/local/include	

ICON = FaustLiveIcon.icns
QMAKE_INFO_PLIST = FaustLiveInfo.plist

all.commands += $(shell mkdir FaustLive.app/Contents/Resources/Libs)

MYFILES = $$system(ls $$FAUSTDIR/*.lib)

for(FILE, MYFILES) {
	all.commands += $(shell cp $$FILE FaustLive.app/Contents/Resources/Libs)
}

all.commands += $(shell cp $$FAUSTDIR/scheduler.ll FaustLive.app/Contents/Resources/Libs)

QMAKE_CXXFLAGS += -Wno-unused-variable -g
QMAKE_EXTRA_TARGETS += all

CONFIG -= x86_64

LLVMLIBS = $$system(llvm-config --libs)
LLVMDIR = $$system(llvm-config --ldflags)

QT+=network

LIBS+=-L/usr/local/lib/faust

LIBS+=-lfaust
LIBS+= $$LLVMDIR $$LLVMLIBS

LIBS+=-lHTTPDFaust
LIBS+=-lOSCFaust -loscpack

LIBS+=-L/opt/local/lib
LIBS+=-lmicrohttpd
LIBS+=-lqrencode
LIBS+=-lboost_system-mt -lboost_filesystem-mt -lboost_program_options-mt
LIBS+= $$LLVMDIR

HEADERS += utilities.h 
SOURCES += utilities.cpp

equals(CAVAR, 1){
	message("COREAUDIO LINKED")
	LIBS+= -L/opt/local/lib -framework CoreAudio -framework AudioUnit -framework CoreServices
	DEFINES += COREAUDIO
	HEADERS += 	CA_audioFactory.h\
				CA_audioSettings.h\
				CA_audioManager.h\
				CA_audioFader.h 
				
	SOURCES += 	CA_audioFactory.cpp \
				CA_audioSettings.cpp \
				CA_audioManager.cpp 
}else{
	message("COREAUDIO NOT LINKED")
}

equals(JVAR, 1){
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
				JA_audioFader.cpp 
}else{
	message("JACK NOT LINKED")
}	

equals(NJVAR, 1){
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
				NJ_audioFader.cpp 
}else{
	message("NETJACK NOT LINKED")
}		

equals(ALVAR, 1){
	message("ALSA LINKED")
	LIBS += -lasound
	DEFINES += ALSA
	HEADERS += 	AL_audioFactory.h \
				AL_audioSettings.h \
				AL_audioManager.h \
				AL_audioFader.h \
	
	SOURCES += 	AL_audioFactory.cpp \
				AL_audioSettings.cpp \
				AL_audioManager.cpp 
}else{
	message("ALSA NOT LINKED")
}		

HEADERS +=	AudioSettings.h \
			AudioManager.h \
			AudioFactory.h \
			AudioCreator.h \
			AudioFader_Interface.h \
			AudioFader_Implementation.h \
			/usr/local/include/faust/gui/faustqt.h \
			FJUI.h \
			FLToolBar.h \
			HTTPWindow.h \
			FLrenameDialog.h \
			FLErrorWindow.h \
			FLExportManager.h \
			FLServerHttp.h \
			FLEffect.h \
			FLWindow.h \ 
			FLApp.h \
    		SimpleParser.h
							
SOURCES += 	AudioCreator.cpp \
			AudioFader_Implementation.cpp \
			FLToolBar.cpp \
			HTTPWindow.cpp \
			FLrenameDialog.cpp \
			FLErrorWindow.cpp \
			FLExportManager.cpp \
			FLServerHttp.cpp \
			FLEffect.cpp \
			FLWindow.cpp \ 
			FLApp.cpp \
			main.cpp \
    		SimpleParser.cpp

unix:!macx{

	filealias.files = FaustLive.desktop
	filealias.path = /usr/share/applications/
	
	fileapp.files = FaustLive 
	fileapp.path = /usr/bin

	iconXpm.files = Resources/Images/faustlive.xpm 
	iconXpm.path = /usr/share/pixmaps/
	
	iconPng.files = Resources/Images/faustlive.png
	iconPng.path = /usr/share/icons/hicolor/32x32/apps/

	iconSvg.files = Resources/Images/faustlive.svg
	iconSvg.path = /usr/share/icons/hicolor/scalable/apps/

	INSTALLS += filealias fileapp iconXpm iconPng iconSvg
}

 RESOURCES     = Resources/application.qrc




