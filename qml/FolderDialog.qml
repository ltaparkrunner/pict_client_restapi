import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: folderDialog
    title: "Новая папка"

    // Центрируем диалог в окне
    anchors.centerIn: parent
    standardButtons: Dialog.Ok | Dialog.Cancel

    // Сигнал, который передаст имя папки дальше (например, в ваш JS/C++ бэкенд)
    signal folderAccepted(string folderName)

    ColumnLayout {
        spacing: 10
        anchors.fill: parent

        Label {
            text: "Введите имя папки:"
        }

        TextField {
            id: folderInput
            placeholderText: "Назовите папку..."
            Layout.fillWidth: true
            focus: true // Чтобы сразу можно было печатать

            // Нажатие Enter на клавиатуре сработает как кнопка OK
            onAccepted: folderDialog.accept()
        }
    }

    onAccepted: {
        if (folderInput.text.trim() !== "") {
            folderAccepted(folderInput.text);
            folderInput.clear();
        }
    }

    onRejected: {
        folderInput.clear();
    }
}
