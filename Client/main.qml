import QtQuick 2.7
import QtQuick.Controls 2.1 as Controls
//import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1
import QtGraphicalEffects 1.0
import Qt.labs.platform 1.0
import QtQuick.Window 2.2
import Chat 1.0

Controls.ApplicationWindow {
    id: mainWindow
    visible: true
    width: 800
    height: 500
    title: appName

    property bool dark: true
    property string appName: qsTr("Local network chat")

    Material.theme: dark ? Material.Dark : Material.Light
    Material.accent: Material.Blue
    Material.primary: Material.Blue

    property color textColor: dark ? shade(1) : shade(0.87)
    property color subTextColor: dark ? shade(0.70) : shade(0.54)

    function shade(alpha) {
        return dark ? Qt.rgba(1,1,1,alpha) : Qt.rgba(0,0,0,alpha)
    }

    minimumHeight: 500
    minimumWidth: 800
    maximumWidth: 1200

    property int newCount: 0

    Connections {
        target: dialogModel
        onNewTextMessage: {
            mainWindow.requestActivate();
            if (mainWindow.visibility == Window.Minimized) {
                ++newCount;
                tray.showMessage(qsTr("New messages"), qsTr("%1: %2").arg(login).arg(message))
            }
        }
        onConnectionStateChanged: {
            switch (dialogModel.connectionState) {
            case ChatDialogListModel.STATE_CONNECTED:
                console.log("connected");
                connectPopup.error = false;
                connectPopup.connecting = false;
                connectPopup.close();
                break;
            case ChatDialogListModel.STATE_UNCONNECTED:
                console.log("unconnected");
                connectPopup.error = true;
                connectPopup.connecting = false;
                connectPopup.open();
                break;
            case ChatDialogListModel.STATE_CONNECTING:
                console.log("connecting");
                connectPopup.error = false;
                connectPopup.connecting = true;
                break;
            case ChatDialogListModel.STATE_CLOSING:
                console.log("closing");
                break;
            default:
                console.log("undefined");
                break;
            }
        }
    }

    onVisibilityChanged: {
        if (mainWindow.visibility != Window.Minimized) {
            newCount = 0;
        }
    }

    SystemTrayIcon {
        id: tray
        visible: true
        iconSource: "chat.svg"
        tooltip: appName
        onActivated: {
            if (mainWindow.visibility == Window.Minimized) {
                mainWindow.show();
                mainWindow.raise();
                mainWindow.requestActivate();
                messageClicked();
            }
            else {
                mainWindow.showMinimized();
            }
        }
    }

    Item {
        id: header
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: 60
        Rectangle {
            id: headerRectangle
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: 60
            opacity: 0.6
            color: Material.primary
        }
        Item {
            id: titleLabelItem
            width: header.width - actionsRow.implicitWidth
            anchors {
                top: parent.top
                bottom: parent.bottom
                right: parent.right
            }

            Controls.Label {
                id: titleLabel
                anchors {
                    centerIn: parent
                }
                color: "white"
                text: dialogModel.chatters.length < 2 ? qsTr("Launch several instances of this program and start chatting!") : qsTr("Chat")
                font.pixelSize: 14
            }
        }
        Row {
            id: actionsRow
            spacing: 10
            anchors {
                verticalCenter: titleLabelItem.verticalCenter
                left: parent.left
                leftMargin: 20
            }
            IconButton {
                id: settingsButton
                anchors.verticalCenter: parent.verticalCenter
                source: "settings.svg"
                hoverAnimation: true
                onClicked: settingsPopup.open()
            }
            IconButton {
                id: searchButton
                anchors.verticalCenter: parent.verticalCenter
                source: "search.svg"
                onClicked: searchPopup.open()
            }
        }

        ThinDivider {
            anchors {
                bottom: parent.bottom
            }
        }
    }

    Component.onCompleted: {
        connectPopup.open()
    }

    Controls.Popup {
        id: connectPopup
        modal: true
        width: properties.width + padding*2
        height: properties.height + padding*2
        x: parent.width/2 - width/2
        y: parent.height/2 - height/2
        focus: true
        closePolicy: Controls.Popup.NoAutoClose
        property alias error: properties.error
        property alias connecting: properties.connecting
        PropertiesPage {
            id: properties
            nameError: dialogModel.nameError
            onConnect: {
                dialogModel.connectToServer(ip, port, name);
            }
        }
    }

    Controls.Popup {
        id: settingsPopup
        x: actionsRow.mapFromItem(settingsButton, 0, settingsButton.x).x + settingsButton.width
        y: actionsRow.mapFromItem(settingsButton, 0, settingsButton.y).y + settingsButton.height
        width: showServiceMessages.implicitWidth + 30
        height: showServiceMessages.implicitHeight + 30
        focus: true
        transformOrigin: Controls.Popup.TopLeft
        Controls.CheckBox {
            id: showServiceMessages
            anchors.centerIn: parent
            text: qsTr("Show service messages")
            checked: true
        }
    }

    Controls.Popup {
        id: searchPopup
        x: actionsRow.mapFromItem(searchButton, 0, searchButton.x).x + searchButton.width
        y: actionsRow.mapFromItem(searchButton, 0, searchButton.y).y + searchButton.height
        width: searchField.width + 30
        height: searchField.implicitHeight + 30
        focus: true
        transformOrigin: Controls.Popup.TopLeft
        Controls.TextField {
            id: searchField
            anchors.centerIn: parent
            width: 200
            placeholderText: qsTr("Search")
        }
        onClosed: {
            searchField.text = ""
        }
    }


    Scrollbar {
        flickableItem: messagesListView
    }

    ListView {
        id: messagesListView
        interactive: height < contentHeight
        clip: true
        anchors {
            left: parent.left
            top: header.bottom
            topMargin: 16
            bottom: messageDivider.top
            bottomMargin: 10
            right: chattersView.left
        }

        Controls.Label {
            anchors.centerIn: parent
            text: searchField.text != "" ? qsTr("Sorry, no results found") : qsTr("No messages yet")
            visible: proxy.stateContent == SortFilterProxyModel.STATECONTENT_EMPTY_BAD_FILTER
            color: subTextColor
        }

        onCountChanged: {
            messagesListView.positionViewAtEnd()
        }
        spacing: 10

        model: SortFilterProxyModel {
            id: proxy
            filter: searchField.text
            showServiceMessages: showServiceMessages.checked
            sourceModel: ChatDialogListModel {
                id: dialogModel
                accent: "#00B0FF"
            }
        }
        delegate: Loader {
            id: delegateLoader
            property bool showRight: chat_mine && chat_message_type == ChatDialogListModel.MESSAGETYPE_TEXT
            anchors {
                left: (parent && !showRight) ? parent.left : undefined
                leftMargin: 20
                right: (parent && showRight) ? parent.right : undefined
                rightMargin: 20
            }

            sourceComponent: chat_message_type == ChatDialogListModel.MESSAGETYPE_TEXT ? messageViewComponent :
                                                                                         notificationViewComponent
            Component {
                id: messageViewComponent
                Item {
                    width: contentRect.width
                    height: contentRect.height
                    Rectangle {
                        id: contentRect
                        width: Math.min( Math.max(nameLabel.implicitWidth + 20, contentLabel.implicitWidth + 20), messagesListView.width*2/3 )
                        height: contentColumn.implicitHeight + 20
                        Material.elevation: 6
                        radius: 4
                        color: chat_mine ? Material.color(Material.Green) : "#B0BEC5"
                        opacity: 0.6
                    }
                    Column {
                        id: contentColumn
                        width: parent.width - 20
                        spacing: 5
                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            top: parent.top
                            topMargin: 10
                        }
                        Controls.Label {
                            id: nameLabel
                            text: qsTr("%1 [%2]").arg(chat_login).arg(chat_date_time)
                            font.pixelSize: 11
                            color: subTextColor

                            MouseArea {
                                anchors.fill: parent
                                onDoubleClicked: {
                                    if (!chat_mine) {
                                        messageField.addName(chat_login)
                                    }
                                }
                            }
                        }

                        Controls.Label {
                            id: contentLabel
                            width: parent.width
                            text: chat_message
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            color: textColor
                            textFormat: dialogModel.isEmptyHtml(chat_message) ? Text.PlainText : Text.AutoText
                        }
                    }
                }
            }

            Component {
                id: notificationViewComponent
                Controls.Label {
                    width: parent ? parent.width: undefined
                    text: qsTr("%1 [%2]").arg(chat_message).arg(chat_date_time)
                    color: subTextColor
                    font.pixelSize: 12
                }
            }
        }
        add: Transition {
            NumberAnimation {
                properties: "y";
                from: messagesListView.mapFromItem(messageField, 0, messageField.y).y
                duration: 200
                easing.type: Easing.OutCubic
            }
        }

        addDisplaced: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: 200
                easing.type: Easing.OutCubic
            }
        }
    }

    SideView {
        id: chattersView
        rightSide: true
        resizeable: true
        state: "showen"
        hiddenIconName: "group.svg"
        headerLabel {
            text: qsTr("Participants")
            color: subTextColor
            font.pixelSize: 12
        }
        anchors {
            top: header.bottom
            bottom: messageDivider.top
        }
        ListView {
            id: chattersList
            anchors {
                fill: parent
                topMargin: 10
            }
            clip: true
            width: mainWindow.width/3
            model: dialogModel.chatters
            interactive: (height < contentHeight)
            delegate: Item {
                width: Math.max( ( chatterLabel.implicitWidth + 40 ), parent.width)
                implicitWidth: width
                height: chatterLabel.implicitHeight + 10

                Controls.Label {
                    id: chatterLabel
                    text: modelData.name
                    property bool isMine: dialogModel.isMine(modelData.name, modelData.ip, modelData.port)
                    color: isMine ? Material.accent : textColor
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: 16
                    }
                    font.pixelSize: 11
                    MouseArea {
                        anchors.fill: parent
                        onDoubleClicked: {
                            if (!chatterLabel.isMine) {
                                messageField.addName(modelData.name)
                            }
                        }
                    }
                }
            }
        }
    }

    ThinDivider {
        id: messageDivider
        anchors {
            bottom: messageField.top
            bottomMargin: 12
        }
    }

    Controls.TextField {
        id: messageField
        anchors {
            left: parent.left
            leftMargin: 12
            right: parent.right
            rightMargin: 12
            bottom: parent.bottom
            bottomMargin: 12
        }
        placeholderText: qsTr("Message")
        function addName(name) {
            messageField.text = messageField.text + qsTr("%1%2,").arg(messageField.text == "" || messageField.text[messageField.text.length-1] == " " ? "" : " ").arg(name)
        }

        onAccepted: {
            dialogModel.sendMessage(messageField.text)
            messageField.text = ""
        }
        maximumLength: 1024
    }
}
