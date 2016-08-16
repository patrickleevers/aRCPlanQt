#-------------------------------------------------
#
# Project created by QtCreator 2014-03-26T13:48:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = aRCPlanQt
TEMPLATE = app
CONFIG-= app_lib

ICON = ../aRCPlanQt-master/Images/Test.icns

SOURCES += ../aRCPlanQt-master/Source/main.cpp\
    ../aRCPlanQt-master/Source/about.cpp \
    ../aRCPlanQt-master/Source/dialog.cpp \
    ../aRCPlanQt-master/Source/guimain.cpp \
    ../aRCPlanQt-master/Source/qcustomplot.cpp \
    ../aRCPlanQt-master/Source/backfill.cpp \
    ../aRCPlanQt-master/Source/beammodel.cpp \
    ../aRCPlanQt-master/Source/configfile.cpp \
    ../aRCPlanQt-master/Source/constants.cpp \
    ../aRCPlanQt-master/Source/creep.cpp \
    ../aRCPlanQt-master/Source/decompression.cpp \
    ../aRCPlanQt-master/Source/fdprofile.cpp \
    ../aRCPlanQt-master/Source/file.cpp \
    ../aRCPlanQt-master/Source/outflowprocess.cpp \
    ../aRCPlanQt-master/Source/parameters.cpp \
    ../aRCPlanQt-master/Source/simulation.cpp \
    ../aRCPlanQt-master/Source/solution.cpp \
    ../aRCPlanQt-master/Source/symdoublematrix.cpp \
    ../aRCPlanQt-master/Source/liquidcontent.cpp

HEADERS  += \
    ../aRCPlanQt-master/Source/about.h \
    ../aRCPlanQt-master/Source/dialog.h \
    ../aRCPlanQt-master/Source/guimain.h \
    ../aRCPlanQt-master/Source/qcustomplot.h \
    ../aRCPlanQt-master/Source/backfill.h \
    ../aRCPlanQt-master/Source/beammodel.h \
    ../aRCPlanQt-master/Source/configfile.h \
    ../aRCPlanQt-master/Source/constants.h \
    ../aRCPlanQt-master/Source/creep.h \
    ../aRCPlanQt-master/Source/decompression.h \
    ../aRCPlanQt-master/Source/fdprofile.h \
    ../aRCPlanQt-master/Source/file.h \
    ../aRCPlanQt-master/Source/outflowprocess.h \
    ../aRCPlanQt-master/Source/parameters.h \
    ../aRCPlanQt-master/Source/simulation.h \
    ../aRCPlanQt-master/Source/solution.h \
    ../aRCPlanQt-master/Source/symdoublematrix.h \
    ../aRCPlanQt-master/Source/liquidcontent.h


FORMS    += \
    ../aRCPlanQt-master/Source/guimain.ui \
    ../aRCPlanQt-master/Source/about.ui \
    ../aRCPlanQt-master/Source/dialog.ui

OTHER_FILES += \
    ../aRCPlanQt-master/README.md \
    ../aRCPlanQt-master/aRCPlan.dSYM

RESOURCES +=
