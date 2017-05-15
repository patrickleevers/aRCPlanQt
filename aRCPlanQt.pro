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

ICON = Images/Test.icns

SOURCES += Source/main.cpp\
    Source/about.cpp \
    Source/dialog.cpp \
    Source/guimain.cpp \
    Source/qcustomplot.cpp \
    Source/backfill.cpp \
    Source/beammodel.cpp \
    Source/configfile.cpp \
    Source/constants.cpp \
    Source/creep.cpp \
    Source/decompression.cpp \
    Source/fdprofile.cpp \
    Source/file.cpp \
    Source/outflowprocess.cpp \
    Source/parameters.cpp \
    Source/simulation.cpp \
    Source/solution.cpp \
    Source/symdoublematrix.cpp \
    Source/liquidcontent.cpp

HEADERS  += \
    Source/about.h \
    Source/dialog.h \
    Source/guimain.h \
    Source/qcustomplot.h \
    Source/backfill.h \
    Source/beammodel.h \
    Source/configfile.h \
    Source/constants.h \
    Source/creep.h \
    Source/decompression.h \
    Source/fdprofile.h \
    Source/file.h \
    Source/outflowprocess.h \
    Source/parameters.h \
    Source/simulation.h \
    Source/solution.h \
    Source/symdoublematrix.h \
    Source/liquidcontent.h \
    Source/json.hpp


FORMS    += \
    Source/guimain.ui \
    Source/about.ui \
    Source/dialog.ui

OTHER_FILES += \
    ../aRCPlanQt-master/README.md \
    ../aRCPlanQt-master/aRCPlan.dSYM

RESOURCES +=
