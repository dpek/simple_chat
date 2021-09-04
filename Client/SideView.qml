import QtQuick 2.7
import QtQuick.Controls.Material 2.1
import QtQuick.Controls 2.1
import QtGraphicalEffects 1.0

Pane {
    id: sideView

    padding: 0
    /*! \brief Иконка в свернутом состоянии
     */
    property string hiddenIconName: "info_outline.svg"
    property alias hiddenIcon: __showIcon

    /*! \brief Иконка в развернутом состоянии
     */
    property string showenIconName: "exit_to_app.svg"
    property alias showenIcon: __hideIcon

    /*! \brief Заголовок
     */
    property alias headerLabel: __headerLabel

    /*! \brief Является ли шторка правосторонней
     */
    property bool rightSide: true

    /*! \brief Ширина в свернутом состоянии
     */
    property int hiddenWidth: 32

    /*! \brief Ширина в раскрытом состоянии
     */
    property int showenWidth: 200

    property real maxWidth: 1.5*showenWidth

    property bool resizeable: false

    default property alias content: __contentItem.data

    anchors {
        left: (!rightSide && !resizeable) ? parent.left : undefined
        right: (rightSide && !resizeable) ? parent.right : undefined
    }

    Material.elevation: 1

    state: "hidden"
    states: [
        State {
            name: "hidden"
            PropertyChanges {
                id: hiddenChange
                target: sideView
                width: hiddenWidth
                x: rightSide ? ( sideView.parent.width - hiddenWidth ) : 0
            }
        },
        State {
            name: "showen"
            PropertyChanges {
                id: showenChange
                target: sideView
                width: !resizeable ? showenWidth :
                                     rightSide ? (sideView.parent.width - x) : (dragArea.x + dragArea.width)
                x: rightSide ? ( sideView.parent.width - showenWidth ) : 0
            }
            PropertyChanges {
                target: dragArea
                x: !rightSide ? showenWidth : 0
            }
        }
    ]
    Behavior on x {
        enabled: resizeable && rightSide && ( x >= dragObj.minimumX )
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on width {
        enabled: (!rightSide && !mouseArea.pressed) || !resizeable
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    function onHideClicked() {
        sideView.state = "hidden"
    }

    Rectangle {
        id: dragArea
        anchors {
            top: parent.top
            left: sideView.rightSide ? parent.left : undefined
            right: sideView.rightSide || resizeable ? undefined : parent.right
            bottom: parent.bottom
        }
        width: hiddenWidth
        color: "transparent"

        ThinDivider {
            horizontal: false
            anchors {
                left: parent.left
            }
            visible: resizeable
        }
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            enabled: (sideView.state == "showen") && resizeable
            drag {
                id: dragObj
                axis: Drag.XAxis
                target: rightSide ? sideView : dragArea
                minimumX: rightSide ? ( sideView.parent.width - maxWidth ) : showenWidth
                maximumX: rightSide ? ( sideView.parent.width - showenWidth ) : maxWidth
            }
        }

        IconButton {
            id: __showIcon
            onClicked: sideView.state = "showen"
            opacity: sideView.state == "hidden" ? 1 : 0
            visible: opacity > 0
            source: hiddenIconName
            anchors {
                top: parent.top
                topMargin: 10
                left: sideView.rightSide ? parent.left : undefined
                leftMargin: 8
                right: sideView.rightSide ? undefined : parent.right
                rightMargin: 8
            }

            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }
        }

        IconButton {
            id: __hideIcon
            onClicked: onHideClicked()
            opacity: sideView.state == "showen" ? 1 : 0
            visible: opacity > 0
            source: showenIconName
            rotation: rightSide ? 0 : 180
            anchors {
                top: parent.top
                topMargin: 10
                left: sideView.rightSide ? parent.left : undefined
                leftMargin: 8
                right: sideView.rightSide ? undefined : parent.right
                rightMargin: 8
            }
            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }
        }
        ThinDivider {
            horizontal: false
            anchors {
                right: parent.right
            }
            visible: resizeable
        }
    }

    Label {
        id: __headerLabel
        anchors {
            top: parent.top
            topMargin: 16
            left: sideView.rightSide ? dragArea.right : parent.left
            leftMargin: 16
            right: sideView.rightSide ? parent.right :  dragArea.left
            rightMargin: 16
        }
        clip: true
        visible: opacity > 0
        opacity: sideView.state == "showen" ? 1 : 0
        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }
    }

    Item {
        id: __contentItem

        anchors {
            top: __headerLabel.bottom
            bottom: parent.bottom
            left:  (sideView.rightSide && resizeable) ? dragArea.right : parent.left
            right: (!sideView.rightSide && resizeable) ? dragArea.left : parent.right
        }
        width: parent.width
        visible: opacity > 0
        opacity: sideView.state == "showen" ? 1 : 0
        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }
    }
}
