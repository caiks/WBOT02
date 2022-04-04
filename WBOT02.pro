#-------------------------------------------------
#
# Project created by QtCreator 2022-04-02T14:00:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WBOT02
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        win001.cpp \
    actor.cpp

HEADERS += \
        win001.h \
    dev.h \
    actor.h

FORMS += \
        win001.ui

INCLUDEPATH += $$PWD/../rapidjson/include

unix:!macx: LIBS += -L$$PWD/../AlignmentActive_build/ -lAlignmentActive

INCLUDEPATH += $$PWD/../AlignmentActive
DEPENDPATH += $$PWD/../AlignmentActive

unix:!macx: PRE_TARGETDEPS += $$PWD/../AlignmentActive_build/libAlignmentActive.a

unix:!macx: LIBS += -L$$PWD/../AlignmentRepaC_build/ -lAlignmentRepaC

INCLUDEPATH += $$PWD/../AlignmentRepaC
DEPENDPATH += $$PWD/../AlignmentRepaC

unix:!macx: PRE_TARGETDEPS += $$PWD/../AlignmentRepaC_build/libAlignmentRepaC.a

unix:!macx: LIBS += -L$$PWD/../AlignmentC_build/ -lAlignmentC

INCLUDEPATH += $$PWD/../AlignmentC
DEPENDPATH += $$PWD/../AlignmentC

unix:!macx: PRE_TARGETDEPS += $$PWD/../AlignmentC_build/libAlignmentC.a
