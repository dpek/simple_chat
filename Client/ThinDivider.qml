import QtQuick 2.7
import QtQuick.Controls.Material 2.1

Rectangle {
    anchors {
        left: horizontal ? parent.left : undefined
        right: horizontal ? parent.right : undefined
        top: horizontal ? undefined : parent.top
        bottom: horizontal ? undefined : parent.bottom
    }
    property bool horizontal: true

    color: Material.theme == Material.Dark ? "#616161" : Qt.rgba(0,0,0,0.1)
    height: horizontal ? 1 : undefined
    width: horizontal ? undefined : 1
}
