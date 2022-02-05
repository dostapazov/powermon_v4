QT += testlib
QT += gui
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES +=  tst_test_method.cpp \
    ../../qtshared/crc/crc_unit.cpp \
    ../../qtshared/dev_proto/proto_sync.cpp \
    ../zrm/zrmproto.cpp

HEADERS += \
    ../../qtshared/crc/crc_unit.hpp \
    ../../qtshared/dev_proto/dev_proto.hpp \
    ../zrm/zrmproto.hpp

INCLUDEPATH += ../../qtshared/dev_proto \
               ../../qtshared/crc \
               ../zrm
