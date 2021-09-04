QT += qml quick network widgets

CONFIG += c++11

SOURCES += *.cpp

HEADERS += *h

RESOURCES += qml.qrc

#QML_IMPORT_PATH =

#QML_DESIGNER_IMPORT_PATH =

DEFINES += QT_DEPRECATED_WARNINGS

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
