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

all.commands += $(shell cp -R Images Examples FaustLive.app/Contents/Resources/)

MYFILES = $$system(ls $$FAUSTDIR/*.lib)

# SUBSTR = .lib

# VAR = $$find(MYFILES, $$SUBSTR)

for(FILE, MYFILES) {
	all.commands += $(shell cp $$FILE FaustLive.app/Contents/Resources/Libs)
}

all.commands += $(shell cp $$FAUSTDIR/scheduler.ll FaustLive.app/Contents/Resources/Libs)

# QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS += -Wno-unused-variable
QMAKE_EXTRA_TARGETS += all

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
LIBS+=-L/opt/local/lib -lboost_system-mt
LIBS+=-L/opt/local/lib -lboost_filesystem-mt
LIBS+=-L/opt/local/lib -lboost_program_options-mt
LIBS+=-L/opt/local/lib -ljson


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
			AudioFader_Interface.h \
			AudioFader_Implementation.h \
			/usr/local/include/faust/gui/faustqt.h \
			FJUI.h \
			FLToolBar.h \
			HTTPWindow.h \
			FLrenameDialog.h \
			FLErrorWindow.h \
			FLExportManager.h \
# 			FLServerHttp.h \
			FLEffect.h \
			FLWindow.h \ 
			FLApp.h \
							
SOURCES += 	AudioCreator.cpp \
			AudioFader_Implementation.cpp \
			FLToolBar.cpp \
			HTTPWindow.cpp \
			FLrenameDialog.cpp \
			FLErrorWindow.cpp \
			FLExportManager.cpp \
# 			FLServerHttp.cpp \
			FLEffect.cpp \
			FLWindow.cpp \ 
			FLApp.cpp \
			main.cpp \