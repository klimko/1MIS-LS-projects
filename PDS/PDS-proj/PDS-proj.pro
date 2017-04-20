TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    arppkt.cpp \
    pds-scanner.cpp \
    ipv4addr.cpp \
    ipv6addr.cpp \
    ipaddr.cpp \
    socket.cpp \
    macaddr.cpp \
    netitf.cpp \
    types.cpp \
    neighbordiscovery.cpp \
    packet.cpp \
    hash.cpp

HEADERS += \
    arppkt.h \
    ipv4addr.h \
    ipv6addr.h \
    types.h \
    ipaddr.h \
    socket.h \
    macaddr.h \
    netitf.h \
    neighbordiscovery.h \
    packet.h \
    hash.h

DISTFILES += \
    Makefile
