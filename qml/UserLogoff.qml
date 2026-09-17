import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.15
import pict_client

Dialog {
    id: logoffDialog
    title: "Выход из системы"
    modal: true

    // Центрируем диалог относительно главного окна приложения
    anchors.centerIn: parent

    // Настраиваем стандартные кнопки Qt
    standardButtons: Dialog.Yes | Dialog.No

    // Изменяем текст на кнопках, чтобы они выглядели нативно
    Component.onCompleted: {
        let yesButton = logoffDialog.standardButton(Dialog.Yes)
        if (yesButton) yesButton.text = "Выйти"

        let noButton = logoffDialog.standardButton(Dialog.No)
        if (noButton) noButton.text = "Отмена"
    }

    // Содержимое диалога (Иконка предупреждения + Текст)
    RowLayout {
        spacing: 15
        // anchors.fill: parent
        // anchors.margins: 15

        // Простая текстовая иконка предупреждения (можно заменить на Image)
        Text {
            text: "⚠"
            font.pixelSize: 28
            color: "#f39c12" // Оранжевый цвет предупреждения
//            anchors.verticalCenter: parent.verticalCenter
            Layout.alignment: Qt.AlignVCenter
        }

        Label {
            text: "Вы действительно хотите выйти из своей учетной записи?\nВсе несохраненные данные будут утеряны."
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            width: 250 // Ограничиваем ширину для красивого переноса текста
//            anchors.verticalCenter: parent.verticalCenter
            Layout.alignment: Qt.AlignVCenter
        }
    }

    // Обработка нажатия на кнопку "Выйти" (Dialog.Yes)
    onAccepted: {
        //  console.log("Пользователь подтвердил выход")
        // Вызываем ваш C++ метод логаута, который очистит токен
        //  authHandler.logout()
        //  console.log(JSON.stringify(wsClient, null, 2))
        wsClient.logout()
    }

    // Обработка нажатия на кнопку "Отмена" (Dialog.No или закрытие окна)
    onRejected: {
        console.log("Выход отменен")
        logoffDialog.close()
    }
}
