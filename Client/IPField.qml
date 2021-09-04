import QtQuick 2.7
import QtQuick.Controls 2.0 as Controls
import QtQuick.Controls.Material 2.1

/*! \brief Редактор для ввода IP адреса (IPv4)
  *
  *  Аналогичен TextField, однако если пользователь пытается
  *  ввести не IP адрес, выдает предупреждение.
  *
  *  Узнать содержит ли поле ошибку вы может с помощью свойства hasError
  */
Controls.TextField{
    id: ipField
    placeholderText: qsTr("IP-адрес")
    property string helperText: hasError && text !== "" && ipField.enabled
                ? qsTr("Введите IP-адрес, например 127.0.0.1")
                : ""

    Material.accent: hasError ? Material.Red : Material.Blue

    property bool hasError:{
        if ( ( text !== "") && ipField.enabled ){
            return !testValidIP(text)
        }
        else{
            return false
        }
    }
    /*! @brief Функция проверяет является ли строка IP адресом
      *
      * @param:in[string] str Строка котороя должна содержать IP адрес
      */
    function testValidIP( str ){
        var pattern = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/
        return pattern.test(str)
    }
}

