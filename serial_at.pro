QT += core gui serialport widgets
TARGET = serial_at
TEMPLATE = app

CONFIG += c++11

# 包含目录
INCLUDEPATH += . \
               serial \
               atcommand \
               network \
               ui

# 源文件
SOURCES += main.cpp \
           mainwindow.cpp \
           serial/serialmanager.cpp \
           network/networkmanager.cpp \
           atcommand/atcommandmanager.cpp \
           ui/uiactionhandler.cpp \
           ui/uimanager.cpp

# 头文件
HEADERS += mainwindow.h \
           serial/serialmanager.h \
           atcommand/atcommandmanager.h \
           network/networkmanager.h \
           ui/uiactionhandler.h \
           ui/uimanager.h

FORMS += mainwindow.ui

# 输出目录
DESTDIR = bin
MOC_DIR = temp/moc
RCC_DIR = temp/rcc
UI_DIR = temp/ui
OBJECTS_DIR = temp/obj
