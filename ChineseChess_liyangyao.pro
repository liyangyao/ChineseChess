#-------------------------------------------------
#
# Project created by QtCreator 2014-10-23T17:03:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ChineseChess_liyangyao
TEMPLATE = app


SOURCES += main.cpp\
        chessboardform.cpp \
    chessboard.cpp

HEADERS  += chessboardform.h \
    chessboard.h

RESOURCES += \
    res.qrc

android:DEFINES += ANDROID

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += \
    android/AndroidManifest.xml
