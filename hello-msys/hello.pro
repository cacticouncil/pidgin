## Qmake directives
TEMPLATE = app
TARGET = hello
QT+= gui widgets

## Major directories
PIDGIN_TREE_TOP = ..
PURPLE_TOP = $$PIDGIN_TREE_TOP/libpurple
GTK_TOP = $$PIDGIN_TREE_TOP/../win32-dev/gtk_2_0-2.14

## Include Paths
INCLUDEPATH +=	. $$PURPLE_TOP \
		$$PURPLE_TOP/win32 \
		$$PIDGIN_TREE_TOP \
		$$GTK_TOP/include/glib-2.0 \
		$$GTK_TOP/lib/glib-2.0/include
#		$$GTK_TOP/include <--- Do we need this?

## Libraries
LIBS += -L$$PURPLE_TOP -lglib-2.0 -lpurple

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
SOURCES += hello.cpp
