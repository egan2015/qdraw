#-------------------------------------------------
#
# Project created by QtCreator 2015-06-13T23:45:28
#
#-------------------------------------------------

QT       += core gui xml svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(../qtpropertybrowser/src/qtpropertybrowser.pri)
TARGET = qdraw
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    drawobj.cpp \
    drawscene.cpp \
    drawtool.cpp \
    sizehandle.cpp \
    objectcontroller.cpp \
    customproperty.cpp \
    rulebar.cpp \
    drawview.cpp \
    commands.cpp \
    document.cpp

HEADERS  += mainwindow.h \
    drawobj.h \
    drawscene.h \
    drawtool.h \
    sizehandle.h \
    objectcontroller.h \
    customproperty.h \
    rulebar.h \
    drawview.h \
    commands.h \
    document.h

RESOURCES += \
    app.qrc
