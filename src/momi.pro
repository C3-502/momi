# Created by and for Qt Creator. This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

#TARGET = momi

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

HEADERS = \
   include/args.hxx \
   connection.h \
   define.h \
   exception.h \
   loader.h \
   momi.h \
   util.h

SOURCES = \
   loader.cpp \
   main.cpp \
   momi.cpp

INCLUDEPATH = \
    $$PWD/.

LIBS += -L /usr/local/lib -lcurl

#DEFINES = 

