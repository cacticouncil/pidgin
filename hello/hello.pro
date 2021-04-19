## Qmake directives
TEMPLATE = app
TARGET = hello
QT+= gui widgets

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

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
SOURCES += hello.cpp
