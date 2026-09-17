import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic
import pict_client

Dialog {
    id: authDialog
    title: isLoginMode ? "Login": "Registration"
    // standardButtons: Dialog.Ok | Dialog.Cancel
    objectName: "authDialog"
    anchors.centerIn: parent
    modal: true
    width: 300

    property alias statusTextElement: statusText
    property bool isLoginMode: true

    // Сигналы для C++ части
    signal loginRequested(string email, string password)
    signal registerRequested(string email, string password)

    // onLoginRequested: (email, password) => authHandler.login(email, password)
    // onRegisterRequested: (email, password) => authHandler.registerUser(email, password)

    // Отслеживаем сигналы от C++ объекта loginManager
    // Connections {
    //     target: authHandler

    //     function onSuccAuth(succMsg) {
    //         console.log("function onSuccAuth", succMsg)
    //         statusText.color = "green"
    //         statusText.text = "Вход успешно выполнен!"
    //         // Здесь можно закрыть диалог или переключить экран через Delay
    //         loginTimer.start()
    //         loginButton.enabled = true
    //     }

    //     function onErrAuth(errMsg) {
    //         console.log("function onErrAuth", errMsg)
    //         statusText.color = "red"
    //         statusText.text = errMsg
    //         loginButton.enabled = true
    //     }
    // }

    Connections {
        target: wsClient
        function onAuthConnectionStateChanged(){
            console.log("function authConnectionStateChanged")
            if(wsClient.authConnectionState === WebSocketClient.Authorized) {
                console.log("function authConnectionStateChanged Succ")
                statusText.color = "green"
                statusText.text = "Вход успешно выполнен!"
                // Здесь можно закрыть диалог или переключить экран через Delay
                loginTimer.start()
                loginButton.enabled = true
            }
            if(wsClient.authConnectionState === WebSocketClient.NotAuthorized) {
                console.log("function authConnectionStateChanged Err")
                statusText.color = "red"
                statusText.text = "Authorization error, check your login and password."
                loginButton.enabled = true
            }
            if(wsClient.authConnectionState === WebSocketClient.NoConnection) {
                console.log("function authConnectionStateChanged Err")
                statusText.color = "red"
                statusText.text = "No network connection. Check your network."
                loginTimer.start()
                loginButton.enabled = true
            }
        }
    }

    Timer {
        id: loginTimer
        interval: 1000
        onTriggered: authDialog.accept() // Закрывает диалог ТОЛЬКО при успехе
    }

    ColumnLayout {
        spacing: 15
        anchors.fill: parent
        anchors.margins: 10

        Label {
            text: isLoginMode ? "Log in to your account": "Create a new account"
            font.pixelSize: 16
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        TextField {
            id: emailField
            placeholderText: "Email (name@example.com)"
            Layout.fillWidth: true
            inputMethodHints: Qt.ImhEmailCharactersOnly

            // Разрешает нажимать "ОК" только если введен валидный email
            validator: RegularExpressionValidator {
                regularExpression: /^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$/
            }

            background: Rectangle {
                implicitHeight: 45
                radius: 8
                // Если текст введен, но некорректен — подсвечиваем рамку красным
                border.color: !emailField.acceptableInput && emailField.text !== "" ? "#D32F2F" :
                              (emailField.activeFocus ? "#2196F3" : "#bdbdbd")
                border.width: emailField.activeFocus || (!emailField.acceptableInput && emailField.text !== "") ? 2 : 1
            }
        }

        Label{
            text: "The password must be at least eight characters
    long, contain uppercase and lowercase
    letters, numbers, and at least one of
    the symbols @, _, -, and nothing else."
            font.pixelSize: 10
            font.bold: false
            Layout.alignment: Qt.AlignHCenter
        }

        TextField {
            id: passwordField
            placeholderText: "Password"
            //echoMode: TextInput.Password
            echoMode: showPasswordButton.checked ? TextInput.Normal : TextInput.Password
            Layout.fillWidth: true
            leftPadding: 10
            rightPadding: 40


            validator: RegularExpressionValidator {
                // Проверяет: строчные, прописные, цифры, спецсимволы (_-@), от 8 символов, без пробелов
                regularExpression: /^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)(?=.*[_@\-])[A-Za-z\d_@\-]{8,}$/
            }

            background: Rectangle {
                implicitHeight: 45
                radius: 8
                // Подсветка красным, если пароль начали вводить, но он не соответствует правилам
                border.color: !passwordField.acceptableInput && passwordField.text !== "" ? "#D32F2F" :
                              (passwordField.activeFocus ? "#2196F3" : "#bdbdbd")
                border.width: passwordField.activeFocus || (!passwordField.acceptableInput && passwordField.text !== "") ? 2 : 1
            }
            Button {
                id: showPasswordButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 5
                width: 30
                height: 30
                flat: true
                checkable: true // Кнопка работает как переключатель (вкл/выкл)

                // Текстовая иконка (Unicode символы глаза)
                contentItem: Text {
                    text: showPasswordButton.checked ? "👁️" : "🙈"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle { color: "transparent" }
            }
        }

        // Переключатель между режимами
        Button {
            flat: true
            text: isLoginMode ? "Don't have an account? Sign up" : "Already have an account? Log in"
            Layout.alignment: Qt.AlignRight
            onClicked: isLoginMode = !isLoginMode
        }

        Label {
            id: errorLabel
            color: "red"
            text: "Please fill in the fields."
            visible: false
        }
        Text {
            id: statusText
            text: ""
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
        }
    }

    footer: DialogButtonBox {
        Button {
            id: loginButton
            text: isLoginMode? "Login" : "Register"
            enabled: emailField.acceptableInput && passwordField.acceptableInput

            onClicked: {
                statusText.text = "Проверка..."
                statusText.color = "blue"
                enabled = false // Блокируем кнопку на время проверки
                if (isLoginMode) {
                    wsClient.loginRequested(emailField.text, passwordField.text)
                } else {
                    wsClient.registerRequested(emailField.text, passwordField.text)
                }
            }
        }
        Button {
            text: "Cancel"
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }

    onAboutToShow: {
        emailField.text = ""
        passwordField.text = ""

        // Опционально: убираем фокус с полей, чтобы не подсвечивались рамки
        emailField.focus = false
        passwordField.focus = false
    }
}
