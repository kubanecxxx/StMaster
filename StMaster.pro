#-------------------------------------------------
#
# Project created by QtCreator 2013-01-28T10:07:02
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StMaster
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    kelnetclient.cpp \
    variable.cpp \
    hled.cpp \
    mapfile.cpp \
    variabledialog.cpp \
    qcustomplot.cpp \
    plotproperties.cpp \
    plotconfigurationdialog.cpp \
    dragdropmodel.cpp \
    qtablewidgetkrida.cpp \
    plotwidget.cpp

HEADERS  += mainwindow.h \
    kelnetclient.h \
    variable.h \
    hled.h \
    mapfile.h \
    variabledialog.h \
    qtablewidgetkrida.h \
    qcustomplot.h \
    plotproperties.h \
    plotconfigurationdialog.h \
    dragdropmodel.h \
    plotwidget.h

FORMS    += mainwindow.ui \
    variabledialog.ui \
    plotconfigurationdialog.ui

CONFIG += qxt
QXT += core gui
