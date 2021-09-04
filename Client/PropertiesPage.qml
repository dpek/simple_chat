import QtQuick 2.7
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1

Item {
    id: item
    signal connect(string ip, int port, string name)
    property bool error: false
    property bool connecting: false
    property bool nameError: false

    width: contentColumn.implicitWidth + 40
    height: contentColumn.implicitHeight + 40

    Column {
        id: contentColumn
        spacing: 20
        anchors.centerIn: parent
        Controls.Label {
            id: headerLabel
            width: serverRow.implicitWidth
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            text: nameError ? qsTr("Choose another nickname, this one is already in use") :
                              error ? qsTr("The server is unavailable") :
                                      connecting ? qsTr("Trying to connect...") : qsTr("Write connection parameters")
            color: (error || nameError) ? Material.color(Material.Red) : textColor
        }

        Row {
            id: serverRow
            spacing: 20
            IPField {
                id: ipField
                placeholderText: qsTr("Server IP")
            }
            IPPortField {
                id: portField
                placeholderText: qsTr("Server port")
                width: 100
            }
        }
        Controls.TextField {
            id: nameField
            placeholderText: qsTr("Nickname")
            maximumLength: 12
        }
        RowLayout {
            width: serverRow.implicitWidth
            Item {
                Layout.fillWidth: true
            }
            Controls.Button {
                id: connectButton
                text: qsTr("Connect")
                enabled: !ipField.hasError && !portField.hasError && nameField.text != ""
                onClicked: {
                    item.connect(ipField.text, portField.text, nameField.text)
                }
            }
        }
    }
}
