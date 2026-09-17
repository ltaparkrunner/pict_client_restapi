import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic

Dialog {
    required property var clientBackend
//    property alias txtMsg: txtMsg.text
    id: warnLoginDialog
    modal: true
    anchors.centerIn: parent
    width: 350

    //  visible: clientBackend.tokenExpired
    Column {
        Text{
            id: txtMsg
            text: "Login or register"
        }

        Button {
            text: "Close"
            onClicked: {
                warnLoginDialog.close()
            }
        }
    }
}
/*

import QtQuick
import QtQuick.Controls

Dialog {
    id: authDialog

    // Требуем ссылку на бэкенд
    required property var clientBackend

    title: "Ошибка авторизации"
    modal: true
    anchors.centerIn: parent
    closePolicy: Dialog.NoAutoClose // Нельзя закрыть кликом мимо

    // Открывается автоматически, если C++ зафиксировал испорченный токен
    visible: clientBackend.tokenExpired

    // Оставляем только одну кнопку действия
    standardButtons: Dialog.Ok

    // Меняем текст на стандартной кнопке Ok (в Qt 6 это делается через Header/Footer)
    Component.onCompleted: {
        var okButton = authDialog.standardButton(Dialog.Ok)
        if (okButton) {
            okButton.text = "Войти заново"
        }
    }

    Column {
        spacing: 15
        width: parent.width

        Label {
            text: "🔑 Сессия устарела"
            font.bold: true
            font.pixelSize: 16
            horizontalAlignment: Text.AlignHCenter
            width: parent.width
        }

        Label {
            text: "Ваш токен безопасности поврежден или срок его действия истек. Пожалуйста, авторизуйтесь в системе заново."
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            width: parent.width
        }
    }

    // Обработка нажатия на кнопку "Войти заново"
    onAccepted: {
        console.log("Перенаправление пользователя на экран логина...")

        // 1. Здесь вы можете вызвать метод C++ для очистки сохраненного токена из памяти/файла
        // clientBackend.clearSavedToken()

        // 2. Переключаем интерфейс QML на экран авторизации
        // (Например, если у вас используется StackView или Loader):
        // mainStackView.replace("LoginScreen.qml")
    }
}
*/
