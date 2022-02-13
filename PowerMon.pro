#-------------------------------------------------
#
# Project created by QtCreator 2019-02-26T11:05:40
#
#-------------------------------------------------

QT       += core gui network  sql charts multimedia printsupport
CONFIG += thread c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TEMPLATE = app

VERSION=4.5

TARGET = powermon

windows{
 RC_ICONS =   battery.ico
 contains(QT_ARCH, x86_64){
 TARGET = powermon64
 }else{
   TARGET = powermon32
 }
}

DEFINES += MULTI_IODEV_CONFIG_WIDGET

include(../qtshared/crc/crc_unit.pri )
include(../qtshared/multi_iodev/multi_iodev.pri)
include(../qtshared/dev_proto/dev_proto.pri)
include(../qtshared/text_viewer_widget/text_view_widget.pri)


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../qtshared/inc
INCLUDEPATH += $$PWD/zrm
INCLUDEPATH += $$PWD/data
INCLUDEPATH += $$PWD/utils

SOURCES += \
    data/ZrmTabMethodEditor.cpp \
    data/reportcommon.cpp \
    data/zrmdatabase.cpp \
    data/zrmmethodexportimport.cpp \
    data/zrmmethodminmax.cpp \
    data/zrmreportdatabase.cpp \
    data/zrmstageseditor.cpp \
        main.cpp \
        mainwindow.cpp \
    utils/powermon_utils.cpp \
    zrm/ZrmChannel.cpp \
    zrm/ZrmLogerChart.cpp \
    zrm/ZrmLogerChartUI.cpp \
    zrm/ZrmParams.cpp \
    zrm/ZrmReadySlaveWidget.cpp \
    zrm/ZrmReportViewDialog.cpp \
    zrm/ZrmReports.cpp \
    zrm/ZrmTCPSettings.cpp \
    zrm/zrm_connectivity.cpp \
    zrm/zrmcalibrate.cpp \
    zrm/zrmcellview.cpp \
    zrm/zrmdevmethods.cpp \
    zrm/zrmparamsview.cpp \
    zrm/zrmproto.cpp \
    zrm/zrmmodule.cpp \
    data/zrmdatasource.cpp \
    data/zrmmethodchoose.cpp \
    data/zrmmethodeditor.cpp \
    data/zrmmethodstree.cpp \
    zrm/zrmbasewidget.cpp \
    data/zrmmethodeditor_write.cpp \
    zrm/zrmconnectivityparam.cpp \
    zrm/zrmmaindisplay.cpp \
    zrm/zrmmethodbase.cpp \
    zrm/zrmreadyaccum.cpp \
    zrm/zrmreadylayout.cpp \
    zrm/zrmreadywidget.cpp \
    zrm/zrmrelaybase.cpp \
    zrm/zrmwidget.cpp \
    zrm/zrmreport.cpp \
    zrm/zrmmonitor.cpp \
    zrm/zrmchannelmimimal.cpp

HEADERS += \
    data/ZrmTabMethodEditor.h \
    data/reportcommon.h \
    data/zrmdatabase.h \
    data/zrmmethodexportimport.h \
    data/zrmmethodminmax.h \
    data/zrmreportdatabase.h \
    data/zrmstageseditor.h \
        mainwindow.h \
    utils/powermon_utils.h \
    zrm/ZrmChannel.h \
    zrm/ZrmLogerChart.h \
    zrm/ZrmLogerChartUI.h \
    zrm/ZrmParams.h \
    zrm/ZrmReadySlaveWidget.h \
    zrm/ZrmReportViewDialog.h \
    zrm/ZrmReports.h \
    zrm/ZrmTCPSettings.h \
    zrm/ui_constraints.hpp \
        zrm/zrm_connectivity.hpp \
    zrm/zrmcalibrate.h \
    zrm/zrmcellview.h \
    zrm/zrmdevmethods.h \
    zrm/zrmparamsview.h \
        zrm/zrmproto.hpp \
        zrm/zrmmodule.hpp \
    data/zrmdatasource.h \
    data/zrmmethodchoose.h \
    data/zrmmethodeditor.h \
    data/zrmmethodstree.h \
    zrm/zrmbasewidget.h \
    ../qtshared/inc/signal_bloker.hpp \
    zrm/zrmconnectivityparam.h \
    zrm/zrmmaindisplay.h \
    zrm/zrmmethodbase.h \
    zrm/zrmreadyaccum.h \
    zrm/zrmreadylayout.h \
    zrm/zrmreadywidget.h \
    zrm/zrmrelaybase.h \
    zrm/zrmwidget.h \
    zrm/zrmreport.h \
    zrm/zrmmonitor.h \
    zrm/zrmchannelmimimal.h

FORMS += \
    data/ZrmTabMethodEditor.ui \
    data/reportcommon.ui \
    data/zrmmethodexportimport.ui \
    data/zrmmethodminmax.ui \
    data/zrmstageseditor.ui \
        mainwindow.ui \
    data/zrmmethodchoose.ui \
    data/zrmmethodeditor.ui \
    zrm/ZrmLogerChart.ui \
    zrm/ZrmLogerChartUI.ui \
    zrm/ZrmParams.ui \
    zrm/ZrmReportViewDialog.ui \
    zrm/ZrmReports.ui \
    zrm/ZrmTCPSettings.ui \
    zrm/zrmcalibrate.ui \
    zrm/zrmcellview.ui \
    zrm/zrmconnectivityparam.ui \
    zrm/zrmdevmethods.ui \
    zrm/zrmmaindisplay.ui \
    zrm/zrmmethodbase.ui \
    zrm/zrmparamsview.ui \
    zrm/zrmreadywidget.ui \
    zrm/zrmrelaybase.ui \
    zrm/zrmwidget.ui \
    zrm/zrmreport.ui \
    zrm/zrmmonitor.ui \
    zrm/zrmchannelmimimal.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    zrm/zrm_resource.qrc \
    data/zrm_data.qrc \
    powermon.qrc

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
unix:{
OBJECTS_DIR = objs
MOC_DIR     = objs/moc
INSTALLS.CONFIG += executable no_check_exists
#DESTDIR  += ~/NIKTES/powermon/opt/niktes
}

