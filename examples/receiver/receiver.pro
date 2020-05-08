TEMPLATE = app
TARGET = receiver
CONFIG += link_pkgconfig
PKGCONFIG += Capture_mjpeg_stream
QT += widgets network quick multimedia qtmultimediaquicktools-private
# Input
SOURCES += main.cpp
RESOURCES += qmlvideo.qrc
target.path = /data/user/qt/$$TARGET
INSTALLS   += target
