import QtQuick
import QtQuick.Controls

Dialog {
    id: networkDialog

    // Свойство, которое вы будете менять из C++ или логики приложения
    property bool isNetworkAvailable: true

    title: "Ошибка соединения"
    modal: true
    anchors.centerIn: parent
    closePolicy: Dialog.NoAutoClose // Запрещаем закрывать по клику мимо

    // Автоматически открываем диалог, если сеть пропала
    visible: !isNetworkAvailable

    standardButtons: Dialog.Retry | Dialog.Cancel

    Column {
        spacing: 15
        width: parent.width

        Label {
            text: "⚠️ Нет подключения к сети"
            font.bold: true
            font.pixelSize: 16
            horizontalAlignment: Text.AlignHCenter
            width: parent.width
        }

        Label {
            text: "Не удалось установить соединение с сервером. Проверьте подключение к Интернету и повторите попытку."
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            width: parent.width
        }
    }

    // Обработка нажатий на кнопки диалога
    onAccepted: {
        console.log("Пользователь нажал 'Повторить'")
        // Здесь вызываем вашу C++ функцию переподключения
        // Например: myClient.connectToServer()
    }

    onRejected: {
        console.log("Пользователь нажал 'Отмена'")
        // Здесь можно закрыть приложение или оставить интерфейс заблокированным
    }
}
