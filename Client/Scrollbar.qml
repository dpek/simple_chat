import QtQuick 2.7
import QtQuick.Controls.Material 2.1

Item {
    id: root

    property Flickable flickableItem
    property int orientation: Qt.Vertical
    property int thickness: 5
    property int minimumScrollLength: thickness
    property bool moving: flickableItem.moving

    property bool alwaysVisible: false

    property bool customDesign: false
    property alias backColor: customBack.color
    property alias scrollColor: customScroll.color


    width: orientation === Qt.Horizontal ? minimumScrollLength : thickness
    height: orientation === Qt.Vertical ? minimumScrollLength : thickness
    clip: true
    smooth: true
    visible: orientation === Qt.Vertical ? flickableItem.contentHeight > flickableItem.height
                                         : flickableItem.contentWidth > flickableItem.width

    anchors {
        top: orientation === Qt.Vertical ? flickableItem.top : undefined
        bottom: flickableItem.bottom
        left: orientation === Qt.Horizontal ? flickableItem.left : undefined
        right: flickableItem.right
        margins: 2
    }

    Component.onCompleted: {
        if (!alwaysVisible)
            hideAnimation.start()
    }

    onMovingChanged: {
        if (!alwaysVisible) {
            if (moving) {
                hideAnimation.stop()
                showAnimation.start()
            } else {
                hideAnimation.start()
                showAnimation.stop()
            }
        }
    }

    NumberAnimation {
        id: showAnimation
        target: scrollBar;
        property: "opacity";
        to: 0.3;
        duration: 200;
        easing.type: Easing.InOutQuad
    }

    SequentialAnimation {
        id: hideAnimation

        NumberAnimation { duration: 500 }
        NumberAnimation {
            target: scrollBar;
            property: "opacity";
            to: 0;
            duration: 500;
            easing.type: Easing.InOutQuad 
        }
    }

    onOrientationChanged: {
        if (orientation == Qt.Vertical) {
            width = thickness
        } else {
            height = thickness
        }
    }

    Rectangle {
        id: scrollBar
        property int length: orientation == Qt.Vertical ? root.height
                                                        : root.width;
        property int targetLength: orientation == Qt.Vertical ? flickableItem.height
                                                              : flickableItem.width;

        property int contentStart: orientation == Qt.Vertical ? flickableItem.contentY - flickableItem.originY
                                                              : flickableItem.contentX - flickableItem.originX;
        property int contentLength: orientation == Qt.Vertical ? flickableItem.contentHeight
                                                               : flickableItem.contentWidth;
        property int start: Math.max(0, length * contentStart/contentLength);
        property int end: Math.min(length, length * (contentStart + targetLength)/contentLength)

        color: Material.theme == Material.Dark ? "white" : "black"
        opacity: 0.3
        radius: thickness/2
        width: Math.max(orientation == Qt.Horizontal ? end - start : 0, orientation == Qt.Vertical ? thickness : minimumScrollLength)
        height: Math.max(orientation == Qt.Vertical ? end - start : 0, orientation == Qt.Horizontal ? thickness : minimumScrollLength)

        // позиция скроллируемого прямоугольника не выходит за область видимости
        x: orientation == Qt.Horizontal ? ( ( start < 0 ) ? 0 :
                                                            ( start > targetLength-minimumScrollLength-4 ) ? targetLength-minimumScrollLength-4 :
                                                                                       start )  : 0
        y: orientation == Qt.Vertical ? ( ( start < 0 ) ? 0 :
                                                          ( start > targetLength-minimumScrollLength-4 ) ? targetLength-minimumScrollLength-4 :
                                                                                     start )  : 0
        /* Отключаем стандартный скролл */
        visible: !customDesign
    }

    Rectangle {
        id: customBack
        anchors.fill: parent
        visible: customDesign
    }
    Rectangle {
        id: customScroll
        anchors.fill: scrollBar
        visible: customDesign
    }
}
