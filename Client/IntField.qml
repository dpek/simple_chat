import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1

/*! @brief Редактор для ввода целаых чисел
  *
  *  Аналогичен TextField, однако если пользователь пытается
  *  ввести не целое число, выдает предупреждение.
  *
  *  Узнать содержит ли поле ошибку вы может с помощью свойства hasError
  */
TextField{
    id: intField

    property var minimumValue: 0
    property var maximumValue: 100

    /*! Для проверок используются именно строковые значения, по умолчанию они забиндены на свойства minimumValue и maximumValue
     * Но для чисел, превышающих возможности javascript, необходимо задавать рамки именно через строковое задание minStr и maxStr
     */
    property string minStr: {
        return parseInt(minimumValue).toString();
    }
    property string maxStr: {
        return parseInt(maximumValue).toString();
    }

    /*! @brief Сообщение показываемое если  errorType == "nanError"
      */
    property string nanErrorMessage: qsTr("Введите целое число")
    /*! @brief Сообщение показываемое если  errorType == "emptyText"
      */
    property string emptyTextErrorMessage: qsTr("Введите целое число")
    /*! @brief Сообщение показываемое если  errorType == "rangeError"
      */
    property string rangeErrorMessage: {
        //var minStr = parseInt(minimumValue).toString()
        //var maxStr = parseInt(maximumValue).toString()
        return qsTr("Диапазон от %1 до %2").arg(minStr).arg(maxStr)
    }

    property bool helperTextVisible: true

    // свойство hasError обновляется позже, чем свойство text, поэтому при использовании компонента intField
    // при обработке сигнала textChanged свойство hasError еще имеет неактуальное значение
    // чтобы это обойти, можно при обработке сигнала textChanged использовать не свойство hasError, а функцию hasErrorFunc(text)
    property var hasErrorFunc: function hasErrorFunction(text) {
        if ( text === "" ){
            return true
        }
        else if( text.match(/^-?\d+$/) ){
            var iValue = parseInt(text, 10)
            if ( (minimumValue > iValue) || (iValue > maximumValue)  ){
                return true
            }
            else{
                return false
            }
        }
        else {
            return true
        }
    }

    /*! @brief Тип ошибки
      *
      *  - "emptyText"  - Пустая строка
      *  - "rangeError" - Значение вне указаного диапазона
      *  - "nanError"   - Не число (пользователь ввел все что угодно, но не целое число)
      *  - "ok"         - Ошибок нет
      */
    property string errorType: {
        if ( text === "" ){
            return "emptyText"
        }
        else if( text.match(/^-?\d+$/) ){
            var iValue = parseInt(text, 10)
            if ( (minimumValue > iValue) || (iValue > maximumValue)  ){
                return "rangeError"
            }
            else{
                return "ok"
            }
        }
        else{
            return "nanError"
        }
    }


    property string helperText:{
        if ( !helperTextVisible ) {
            return ""
        }
        else if ( errorType  === "emptyText" ){
            return ( placeholderText === "" )
                    ? emptyTextErrorMessage
                    : ""
        }
        else if ( errorType === "rangeError" ){
            return rangeErrorMessage
        }
        else if ( errorType === "nanError" ){
            return nanErrorMessage
        }
        else{
            return ""
        }
    }

    property bool hasError: errorType !== "ok" && intField.enabled

    Material.accent: hasError ? Material.Red : Material.Blue
}

