# -------------------------------------------------
# Project created by QtCreator 2010-02-03T23:47:18
# -------------------------------------------------
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QHdaRedirector
TEMPLATE = app

SOURCES += main.cpp \
    widget.cpp \
    codec.cpp \
    node.cpp
HEADERS += widget.h \
    codec.h \
    node.h \
    hda.h
FORMS += widget.ui
RESOURCES += images.qrc
TRANSLATIONS += main_ru.ts
