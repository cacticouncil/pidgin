## Qmake directives
TEMPLATE = app
TARGET = qtnullclient

##Added this
QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
##QT+= gui widgets


CONFIG += c++11

## Major directories
PIDGIN_TREE_TOP = ..
PURPLE_TOP = $$PIDGIN_TREE_TOP/libpurple

## Include Paths
INCLUDEPATH +=	. $$PURPLE_TOP \
		$$PIDGIN_TREE_TOP \
		/usr/include/glib-2.0 \
		/usr/lib/x86_64-linux-gnu/glib-2.0/include
#		/usr/include/gtk-2.0 \ <--- Do we need this?
#		/usr/lib/x86_64-linux-gnu/gtk-2.0/include <--- Do we need this?
		
## Libraries
LIBS += -L$$PURPLE_TOP -lpurple
## I believe we need the below statement here but def need in Makefile otherwise
LIBS += /lib/x86_64-linux-gnu/libglib-2.0.so

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
SOURCES += \
	main.cpp \
	MainWindow.cpp \
	qtnullclient.cpp \
	qtlibpurple.cpp
	

HEADERS += \
	MainWindow.h \
	qtnullclient.h \
	qtlibpurple.h \
	../libpurple/purple.h
	# need to fix the above <--------------
	
	
FORMS += \
	mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target