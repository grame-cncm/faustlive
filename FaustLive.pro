######################################################################
# 
######################################################################

OPT = $$system(llvm-config --libs)
LLVMDIR = $$system(llvm-config --ldflags)

LIBS+= -ljack
LIBS += -ljacknet

LIBS+= $$LLVMDIR $$OPT
LIBS+=-L/opt/local/lib -framework CoreAudio -framework AudioUnit -framework CoreServices

LIBS+=-L/usr/local/lib/faust
LIBS+=-lfaust
LIBS+=-lHTTPDFaust

LIBS+=-L/opt/local/lib -lmicrohttpd
LIBS+=-L/opt/local/lib -lqrencode


TEMPLATE = app
TARGET = FaustLive
DEPENDPATH += /usr/local/include/faust/gui
INCLUDEPATH += .
INCLUDEPATH += /opt/local/include

CONFIG -= x86_64

QT+=network

ICON = FaustLiveIcon.icns
QMAKE_INFO_PLIST = FaustLiveInfo.plist

system(cp -R Images Libs Examples FaustLive.app/Contents/Resources/)

# Input

HEADERS += 	audioSettings.h \
			audioManager.h \
			audioFactory.h \
			CA_audioFactory.h\
			JA_audioFactory.h \
			NJ_audioFactory.h \
			CA_audioSettings.h\
			JA_audioSettings.h \
			NJ_audioSettings.h \
			CA_audioManager.h\
			JA_audioManager.h \
			NJ_audioManager.h \
			AudioCreator.h \
			audioFader_Interface.h \
			audioFader_Implementation.h \
			jack-dsp.h netjack-dsp.h \
			coreaudio-dsp.h \
			crossfade_netjackaudio.h \
			crossfade_jackaudio.h \
			crossfade_coreaudio.h \
			faustqt.h \
			FJUI.h \
			FLToolBar.h \
			HTTPWindow.h \
			FLrenameDialog.h \
			FLErrorWindow.h \
			Effect.h \
			FLWindow.h \ 
			FaustLiveApp.h \
			
SOURCES += 	CA_audioFactory.cpp \
			JA_audioFactory.cpp \
			NJ_audioFactory.cpp \
			CA_audioSettings.cpp \
			JA_audioSettings.cpp \
			NJ_audioSettings.cpp \
			CA_audioManager.cpp \
			JA_audioManager.cpp \
			NJ_audioManager.cpp \
			AudioCreator.cpp \
			audioFader_Implementation.cpp \
			jack-dsp.cpp \
			coreaudio-dsp.cpp \
			crossfade_netjackaudio.cpp \
			crossfade_jackaudio.cpp \
			crossfade_coreaudio.cpp \
			FLToolBar.cpp \
			HTTPWindow.cpp \
			FLrenameDialog.cpp \
			FLErrorWindow.cpp \
			Effect.cpp \
			FLWindow.cpp \ 
			FaustLiveApp.cpp \
			main.cpp \



