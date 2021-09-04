import QtQuick 2.7
import QtQuick.Controls.Material 2.1
import QtGraphicalEffects 1.0

Item {
    id: iconButton

    property alias color: overlay.color
    property alias source: icon.source
    property bool hoverAnimation: false
    signal clicked()
    property real size: 18
    width: iconButton.size
    height: iconButton.size

    Ink {
        id: ink
        anchors.centerIn: icon
        centered: true
        circular: true
        width: icon.width + 20
        height: icon.height + 20
        z: 0
        onClicked: iconButton.clicked()
    }
    Image {
        id: icon
        anchors {
            top: parent.top
            left: parent.left
        }
        visible: source != ""
        width: iconButton.width
        height: iconButton.height
        ColorOverlay {
            id: overlay
            anchors.fill: parent
            source: icon
            cached: true
            color: Material.theme == Material.Dark ? Qt.rgba(1,1,1,0.70) : Qt.rgba(0,0,0,0.54)
            visible: icon.source != ""
            opacity: 0.9
        }
        rotation: iconButton.hoverAnimation ? ink.containsMouse ? 90 : 0
                                            : 0

        Behavior on rotation {
            NumberAnimation { duration: 200 }
        }
    }
}
