import QtQuick 2.7

/*! \brief Поля для ввода IP порта (TCP или UDP)
  */
IntField{
    id: ipPortField
    minimumValue: 1
    maximumValue: 65536
    placeholderText: qsTr("Порт")
}
