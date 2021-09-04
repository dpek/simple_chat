QT += core network
QT -= gui

CONFIG += c++11

TARGET = Server
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += *.cpp

HEADERS += *h

DEFINES += QT_DEPRECATED_WARNINGS
