import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
// import com.myapp.helpers 1.0
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtQuick.Effects
import QtQuick.Shapes
import QtCore
//import pict_client //1.0
import com.myapp.helpers 1.0
import pict_client_restapi

ApplicationWindow {
    id: rootWnd
    visible: true
    width: 1000
    height: 600
    minimumWidth: 400
    minimumHeight: 300
    title: "Fixed Left, Flexible Right"

    property string parentLocalPath: Qt.platform.os === "windows" ? "C:/Users" : "/home"
    property string parentNetworkPath: "http://minio:9000/"
    property string cleanNetworkPath: parentNetworkPath

    property int parentCustomDlgTb: 0  //  Main.StoreType.Local

    property var fileNames: [
        "../icons/cherry-blossom.png",
        "../icons/pink-cosmos.png",
        "../icons/morning-glory.png",
        "../icons/rose.png"
    ]
    Settings {
        category: "General"
        // Связываем свойство QSettings со свойством окна
        property alias parentLocalPath: rootWnd.parentLocalPath
        property alias parentNetworkPath: rootWnd.parentNetworkPath
        property alias cleanNetworkPath: rootWnd.cleanNetworkPath
        property alias parentCustomDlgTb: rootWnd.parentCustomDlgTb
    }

    Connections {
        target: restClient
        function onLoginSuccess(token) {
            console.log("Logged in successfully! Token: " + token)
        }
        function onErrorOccurred(errorMsg) {
            console.error("Failed: " + errorMsg)
        }
    }

    CustomFileDialog {
        id: customDialog

        Binding {
            target: customDialog
            property: "currentLocalPath"
            value: rootWnd.parentLocalPath
        }
        Binding {
            target: customDialog
            property: "currentNetworkPath"
            value: rootWnd.parentNetworkPath
        }
        Binding {
            target: customDialog
            property: "currentTabIndex"
            value: rootWnd.parentCustomDlgTb
        }
        onOpenIndexSelected:(index) => {
            //  console.log("onOpenIndexSelected: ", index, " rows: ", storageModel.rowCount())
            if(index>=0 && index<storageModel.rowCount()){
                let img = storageModel.get(index)
                let imgPath = img.path;
                let prefix = "file:///";
                if(!img.isMinio && !imgPath.startsWith(prefix)){
                    mainImageSource = prefix + imgPath
                }
                else mainImageSource = imgPath
                //  console.log("onOpenIndexSelected: ", imgPath);
                let data = storageModel.getData(index);
                console.log("onOpenIndexSelected: cleanPath: ", data.cleanPath, " isMinio: ", data.isMinio)
                //  console.log("onOpenIndexSelected: ", index);
                imageModel.insertImage(data);
            }
            else console.log("Путь не распознан или не существует 1");
        }
        onOpenIndicesSelected:(indices) => {
            let maxindx = storageModel.rowCount()
            let arr = []
            let succ = 0
            let dir = 0
            let dirToGo
            for(let indx of indices) {
                //  console.log("onOpenIndicesSelected indx: ", indx)
                if(indx>=0 && indx < maxindx){
                    let img = storageModel.get(indx);
                    if(!img.isDir && !img.isMinioBucket && !img.VirtualDir) {
                        if(!succ){
                            let imgPath = img.path;
                            let prefix = "file:///";
                            if(!img.isMinio && !imgPath.startsWith(prefix)){
                                mainImageSource = prefix + imgPath
                            }
                            else mainImageSource = imgPath
                            succ = 1
                        }
                        let data = storageModel.getData(indx)
                        arr.push(data)
                    }
                    else if((dir===0) && (img.isDir ||  img.isMinioBucket)){
                        dirToGo = {indx:indx, isDir:img.isDir, isMinio:img.isMinio, isBucket:img.isMinioBucket}; dir = 1;
                    }
                }
            }
            imageModel.insertImages(arr);
            if(dirToGo !== null && dirToGo !== undefined)  // TODO:
                if(dirToGo.dirToGo.isDir && !dirToGo.isMinio)   {       //TODO: qrc:/qt/qml/pict_client/qml/Main.qml:117: TypeError: Cannot read property 'isDir' of undefined
                        console.log("storageModel.enterLocal(dirToGo.indx", dirToGo.indx)
                        storageModel.enterLocal(dirToGo.indx)
                    }
                else if(dirToGo.isBucket)     storageModel.enterMinioBucket(dirToGo.indx)
                else if(dirToGo.isDir && dirToGo.isMinio && !dirToGo.isBucket)  storageModel.enterNetStore(dirToGo.indx)
        }
        onWriteImages: (lf, path) => {
            //  console.log("onWritePathsSelected paths: ", path)
            storageModel.writeImagesToFolder(lf, path);
        }
        onDeletePathsSelected:(indices) => {
            if(indices){
                storageModel.deleteIndices(indices)
            }
        }

        onSetParentPaths:(tbIndx, localPath, networkPath, nwCleanPath) => {
            console.log("tbIndx: ", tbIndx, " networkPath: ", networkPath, " nwCleanPath: ", nwCleanPath)
            if(tbIndx === 0) {
                parentCustomDlgTb = 0;
                parentLocalPath = localPath;
                console.log("tf.tfContent = localPath 1")
                tf.tfContent = localPath;
            }
            else {
                parentCustomDlgTb = 1;
                parentNetworkPath = networkPath;
                cleanNetworkPath = nwCleanPath
                // console.log("tf.tfContent = cleanNetworkPath")
                // tf.tfContent = cleanNetworkPath;
                tf.tfContent = nwCleanPath
            }
        }
    }

    MessageDialog {
        id: msgNothingToDo
        title: "Nothing To Do"  // "Подтверждение"
        text: "Path is empty, or smth like this"    // "Вы уверены, что хотите удалить этот файл?"
        informativeText: "Nothing To Do"     //"Это действие нельзя будет отменить."
        buttons: MessageDialog.Ok | MessageDialog.Cancel

        onAccepted: close()     //console.log("Нажата кнопка ОК")
        onRejected: close()     //console.log("Нажата кнопка Отмена")
    }

    MessageDialog {
        id: plug
        title: "Plug"  // "Подтверждение"
        text: "Code is not ready"    // "Вы уверены, что хотите удалить этот файл?"
        informativeText: "It'll be written"     //"Это действие нельзя будет отменить."
        buttons: MessageDialog.Ok | MessageDialog.Cancel

        onAccepted: close()     //console.log("Нажата кнопка ОК")
        onRejected: close()     //console.log("Нажата кнопка Отмена")
    }

    property string mainImageSource: ""
    property list<string> myImages: ["", "", "", "", "", ""]
    property string largeImgPath: ""
    // Основной горизонтальный контейнер
    ColumnLayout{
        anchors.fill: parent
        ColumnLayout {
            Layout.fillHeight: true
            width: parent.width
            RowLayout{
                // Layout.alignment: Qt.AlignTop
                TextField {
                    id: tf
                    property string tfContent: "/home"
                    property string lastSavedText: ""
                    placeholderText: "Open/Write file/folder"
                    Layout.fillWidth: true
                    Layout.preferredWidth: 4
                    text: tfContent
                    background: Rectangle {
                        implicitWidth: 200
                        implicitHeight: 40
                        color: "#ddfbdd"    //tf.enabled ? "transparent" : "#353535"
                        border.color: (tf.activeFocus || tf.hovered)? "#21be2b" : "#bdbebf"
                        border.width: (tf.activeFocus || tf.hovered)? 2 : 1
                        radius: 4
                    }
                    onActiveFocusChanged: {
                        if (activeFocus) {
                            // Store the text exactly as it was when editing started
                            lastSavedText = tf.text
                        }
                    }
                    onAccepted: {
                        console.log("Пользователь нажал Enter. Введенный текст:", tf.text)
                        rootWnd.processTFPath()
                        // Здесь ваша логика (например, отправка сообщения или запуск поиска)
                    }
                    onEditingFinished: {
                        if (tf.text !== lastSavedText) {
//                            console.log("Text actually changed to:", tf.text, "  ", lastSavedText)
                            lastSavedText = tf.text
                        } else {
                            console.log("Finished editing, but no changes were made.")
                        }
                    }
                }
                AbstractButton {
                    id: loginButton
                    Layout.preferredWidth: 25
                    Layout.preferredHeight: 25
                    hoverEnabled: true

                    background: Rectangle {
                        implicitWidth: loginButton.Layout.preferredWidth
                        implicitHeight: loginButton.Layout.preferredHeight
                        radius: width / 2 // Делает рамку идеально круглой
                        color: "#ddfbdd"    //"transparent"
                        // Логика цвета и толщины рамки (отслеживает состояния loginButton)
                        border.color: (loginButton.activeFocus || loginButton.hovered) ? "#21be2b" : "#bdbebf"
                        border.width: (loginButton.activeFocus || loginButton.hovered) ? 2 : 1

                        // Плавный переход для красивого визуального эффекта
                        Behavior on border.color { ColorAnimation { duration: 100 } }
                    }

                    Item {
//                    Rectangle{
                        id: userImageCliped
                        // Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        // width: 25
                        // height: 25
                        anchors.fill: parent
                        anchors.margins: loginButton.hovered ? 2 : 1
                        // Image {
                        //     id: userImage
                        //     anchors.fill: parent
                        //     source: getCurrentUserImage()
                        //     visible: false

                        //     function getCurrentUserImage() {
                        //         if(wsClient.authConnectionState !== WebSocketClient.Authorized)
                        //             return "../icons/user.svg";
                        //         else {//return "../icons/cherry-blossom.png"
                        //             var index = Math.floor(Math.random() * fileNames.length);
                        //             return fileNames[index]
                        //         }
                        //     }
                        // }

                        // Image {
                        //     id: userMask
                        //     source: "../icons/userMask.svg"
                        //     anchors.fill: userImage
                        //     anchors.margins: 4
                        //     visible: false
                        // }

                        // MultiEffect {
                        //     source: userImage
                        //     anchors.fill: userImage
                        //     maskSource: userMask
                        //     maskEnabled: true
                        // }
                    }

                    onClicked: {
//                        if(!authHandler.loggedIn) {
                        if(wsClient.authConnectionState === WebSocketClient.NotAuthorized ||
                            wsClient.authConnectionState === WebSocketClient.LoggedOut){
                            userLogin.statusTextElement.text = ""
                            userLogin.open()
                        }
                        else if(wsClient.authConnectionState === WebSocketClient.Authorized){
                            userLogoff.open()
                        }
                        else {
                            nwDialog.open()
                        }
                    }
                    // Shape {
                    //     id: bubble
                    //     x: -text.width - 25
                    //     anchors.margins: 3
                    //     preferredRendererType: Shape.CurveRenderer
                    //     visible: wsClient.authConnectionState === WebSocketClient.NotAuthorized ||
                    //              wsClient.authConnectionState === WebSocketClient.LoggedOut
                    //     ShapePath {
                    //         strokeWidth: 0
                    //         fillColor: "#667085"
                    //         startX: 5; startY: 0
                    //         PathLine { x: 5 + text.width + 6; y: 0 }
                    //         PathArc { x: 10 + text.width + 6; y: 5; radiusX: 5; radiusY: 5}
                    //         // arrow
                    //         PathLine { x: 10 + text.width + 6; y: 8 + text.height / 2 - 6 }
                    //         PathLine { x: 10 + text.width + 6 + 6; y: 8 + text.height / 2 }
                    //         PathLine { x: 10 + text.width + 6; y: 8 + text.height / 2 + 6}
                    //         PathLine { x: 10 + text.width + 6; y: 5 + text.height + 6 }
                    //         // end arrow
                    //         PathArc { x: 5 + text.width + 6; y: 10 + text.height + 6 ; radiusX: 5; radiusY: 5}
                    //         PathLine { x: 5; y: 10 + text.height + 6 }
                    //         PathArc { x: 0; y: 5 + text.height + 6 ; radiusX: 5; radiusY: 5}
                    //         PathLine { x: 0; y: 5 }
                    //         PathArc { x: 5; y: 0 ; radiusX: 5; radiusY: 5}
                    //     }
                    //     Text {
                    //         x: 8
                    //         y: 8
                    //         id: text
                    //         color: "white"
                    //         //  text: qsTr("Log in to edit")
                    //         text: authHandler ? ( authHandler.loggedIn ? authHandler.username : "Log in to edit") : "Log in to edit"
                    //         font.bold: true
                    //         horizontalAlignment: Qt.AlignHCenter
                    //         verticalAlignment: Qt.AlignVCenter
                    //     }
                    // }
                }
            }

            RowLayout{
                Button {
                    id: fileButton
                    text: "File dialog"
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    // Настройка шрифта
                    font.pixelSize: 14
                    font.weight: Font.Medium

                    // Кастомизация текстового слоя (для управления цветом при наведении)
                    contentItem: Text {
                        text: fileButton.text
                        font: fileButton.font
                        color: "#2c2c2c"        //fileButton.down ? "#ffffff" : (fileButton.hovered ? "#ffffff" : "#2c2c2c")//"#2c3e50")
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight

                        Behavior on color { ColorAnimation { duration: 150 } }
                    }

                    // Кастомизация фоновой подложки кнопки
                    background: Rectangle {
                        implicitHeight: 40 // Комфортная высота для клика
                        radius: 8          // Скругление углов в современном стиле

                        // Плавная смена цвета фона в зависимости от состояния кнопки
                        color: {
                            if (fileButton.down) return "#cafbca" // "#aafbaa" "#bafbba" "#cafbca" "#98fb98"
                            //if (fileButton.hovered) return "#98fb98"    //return "#2980b9"   // Цвет при наведении (акцентный синий)
                            return "#ddfbdd"    //return "#ebf5fb"                           // Цвет в покое (светло-голубой)
                        }

                        // Тонкая рамка для структуры в режиме покоя
                        // border.color: fileButton.hovered ? "transparent" : "#d4e6f1"
                        // border.width: 1
                        border.color: (fileButton.activeFocus || fileButton.hovered) ? "#21be2b" : "#bdbebf"
                        border.width: (fileButton.activeFocus || fileButton.hovered) ? 2 : 1
                        // Плавная анимация перехода между цветами
                        Behavior on color { ColorAnimation { duration: 150 } }
                    }
                    onClicked: {
                        rootWnd.processTFPath(tf.text)
                    }
                }
            }
        }
/*
        RowLayout {
           Layout.fillHeight: true
           width: parent.width
            // ЛЕВАЯ ЧАСТЬ (Поля ввода и сетка)
            ColumnLayout {
                Layout.fillHeight: true
                Layout.preferredWidth: (parent.width - 330 < 300) ? parent.width / 2 : 330
                spacing: 5//10
//                Item{
                Rectangle{
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: "#ddfbdd"
                    GridView {
                        property string lastDoubleClickedPath: ""
                        id: imageGrid
                        anchors.fill: parent
                        anchors.margins: 10
                        clip: true

                        cellWidth: 100
                        cellHeight: 100

                        model: imageModel
//                        delegate: Item {
                        delegate: ItemDelegate {
                            width: imageGrid.cellWidth
                            height: imageGrid.cellHeight
                            id: imageDelegate
                            z: mouseArea.containsMouse ? 100 : 1
                            readonly property GridView parentView: GridView.view
                            readonly property int limit1: 40
                            //  focusPolicy: Qt.ClickFocus
                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: 5 // Отступы между картинками
                                color: "white"
                                border.color: "silver"
                                radius: 4

                                Image {
                                    anchors.fill: parent
                                    anchors.margins: 2
                                    source: model.path // Данные из C++
                                    fillMode: Image.PreserveAspectCrop // Чтобы не искажать пропорции
                                    clip: true
                                }
                            }
                            MouseArea {
                                id: mouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                onDoubleClicked: {
                                    let img = imageModel.get(index)
                                    mainImageSource = img.path
                                    console.log("tf.tfContent = img.cleanPath 3")
                                    tf.tfContent = img.cleanPath
                                }
                                onClicked: (mouse) => {
                                    if (mouse.button === Qt.RightButton) {
                                        contextMenu.popup() // Открываем меню
                                    } else {
                                        //  dlgt.parentView.currentIndex = index;
                                        console.log("imageDelegate.y: ", imageDelegate.y,  "  imageDelegate.height: ", imageDelegate.height , "  imageGrid.height: ", imageGrid.height)
                                    }
                                }
                                onEntered: {
                                    floatingInfo.targetItem = imageDelegate;
                                    infoText.text = model ? model.cleanPath : ""; // Передаем путь из модели
                                    floatingInfo.updatePosition();
                                }

                                onExited: {
                                    floatingInfo.visible = false;
                                    floatingInfo.targetItem = null;
                                }
                            }
                            Menu {
                                id: contextMenu
                                MenuItem {
                                    text: "Скопировать путь"
                                    onTriggered: {
                                        let img = imageModel.get(index)
                                        imageModel.copyToClipboard(model.cleanPath)
                                        console.log("Путь скопирован: " + model.cleanPath)}
                                }
                                MenuItem {
                                    text: "Удалить"
                                    onTriggered: { imageModel.removeItem(index) }
                                }
                            }
                            function shortenPath(path, limit) {
                                if (path.length <= limit) return path;
                                let partSize = Math.floor(limit / 2) - 2;
                                return path.substring(0, partSize) + "..." + path.substring(path.length - partSize);
                            }
                        }
                        ScrollBar.vertical: ScrollBar {}
                        // GridView.onAdd: {

                        // }
                    }

                    Rectangle {
                        id: floatingInfo
                        width: imageGrid.width
                        //height: Math.min(infoText.implicitHeight + 10, infoText.lineHeight * 3 + 10)
                        height: infoText.implicitHeight + 10
                        color: "#E6000000"
                        visible: false
                        z: 999 // Всегда поверх всего

                        property var targetItem: null // Ссылка на делегат, над которым мышь

                        Text {
                            id: infoText
                            anchors.fill: parent; anchors.margins: 5
                            color: "white"; wrapMode: Text.WrapAnywhere
                            maximumLineCount: 3; elide: Text.ElideMiddle
                        }

                        // Функция динамического пересчета позиции
                        function updatePosition() {
                            if (!targetItem) return;

                            // Получаем глобальные координаты делегата относительно окна/сетки
                            let pos = targetItem.mapToItem(imageGrid, 0, 0);

                            // Решаем: сверху или снизу
                            let spaceBelow = imageGrid.height - (pos.y + targetItem.height);
                            if (spaceBelow < height + 40) {
                                y = pos.y - height + 10; // Показываем НАД
                            } else {
                                y = pos.y + targetItem.height + 10; // Показываем ПОД
                            }

                            visible = true;
                        }
                    }
                }
            }

            // ПРАВАЯ ЧАСТЬ (Большое окно изображения)
            ColumnLayout{
                Layout.preferredWidth: (parent.width - 330 < 300) ? parent.width / 2 : parent.width - 330
                Rectangle {
                    id: largeImgRect
                    property string savedString: ""
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredWidth: (parent.width - 330 < 300) ? parent.width / 2 : parent.width - 330

                    color: mainImageSource === "" ? "#808080" : "#222"
                    border.color: "#222"    //"#333"

                    Image {
                        id: largeImg
                        anchors.fill: parent
                        source: mainImageSource //"https://placeholder.com" // Замените на свое фото
                        fillMode: Image.PreserveAspectFit

                        // Плавное появление картинки
                        opacity: status === Image.Ready ? 1 : 0
                        Behavior on opacity { NumberAnimation { duration: 500 } }
                    }

                    Text {
                        //  anchors.bottom: parent.bottom
                        //  anchors.horizontalCenter: parent.horizontalCenter
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                        text: "No Image Loaded"//"Large Image Preview"
                        color: "white"
                        font.pixelSize: 18
                        visible: mainImageSource === ""
                    }
                }
            }
        }
*/
            // Поле ввода логина
            TextField {
                id: usernameField // <--- Этот ID используется в кнопке
                placeholderText: "Введите логин"
                Layout.preferredWidth: 250
            }

            // Поле ввода пароля
            TextField {
                id: passwordField // <--- Этот ID используется в кнопке
                placeholderText: "Введите пароль"
                echoMode: TextInput.Password // Скрывает символы пароля
                Layout.preferredWidth: 250
            }

            // Ваша кнопка
            Button {
                text: "Log In"
                Layout.alignment: Qt.AlignHCenter

                onClicked: {
                    // Теперь свойства .text будут успешно прочитаны из полей выше
                    restClient.login(usernameField.text, passwordField.text)
                }
            }
            // Ваша кнопка 2
            Button {
                text: "Register"
                Layout.alignment: Qt.AlignHCenter

                onClicked: {
                    // Теперь свойства .text будут успешно прочитаны из полей выше
                    restClient.registerUser(usernameField.text, passwordField.text)
                }
            }
    }
    footer: ToolBar {
        id: statusBar

        // Высота панели
        height: 30

        // Меняем цвет фона панели в зависимости от состояния сети/авторизации
        background: Rectangle {
            color: {
                return "#667085" // Цвет по умолчанию (Серый)
            }

            // Плавный переход цвета при смене состояний
            Behavior on color { ColorAnimation { duration: 300 } }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 10
            // verticalAlignment: Qt.AlignVCenter

            // Иконка или индикатор состояния (маленький кружок)
            Rectangle {
                width: 10
                height: 10
                radius: 5
                Layout.alignment: Qt.AlignVCenter
                color: "white"

                // Делаем так, чтобы индикатор мигал во время подключения
                // SequentialAnimation on opacity {
                //     running: wsClient.authConnectionState === WebSocketClient.Connecting ||
                //              wsClient.authConnectionState === WebSocketClient.Authenticating
                //     loops: Animation.Infinite
                //     PropertyAnimation { to: 0.2; duration: 500 }
                //     PropertyAnimation { to: 1.0; duration: 500 }
                //     // Если состояние стабильное, просто горит на 100%
                //     onRunningChanged: if (!running) opacity = 1.0
                // }
            }
            // Текстовое описание текущего
/*
            Label {
                //anchors.verticalCenter: parent.verticalCenter
                Layout.alignment: Qt.AlignVCenter
                color: "white"
                font.pixelSize: 12
                font.bold: true

                text: {
                    if (!wsClient) return "Инициализация..."
                    switch (wsClient.authConnectionState) {
                        case WebSocketClient.Idle: return "Отключено"
                        case WebSocketClient.Connecting: return "Подключение по сети..."
                        case WebSocketClient.Connected: return "Сеть активна, авторизация..."
                        case WebSocketClient.Authenticating: return "Проверка токена безопасности..."
                        case WebSocketClient.Authorized: return "Подключен: " + (authHandler ? authHandler.username : "")
                        case WebSocketClient.NotAuthorized: return "Ни один пользователь не подключен."     //"Ошибка: Токен устарел или испорчен"
                        case WebSocketClient.NoConnection: return "Ошибка: Нет связи с сервером"
                        case WebSocketClient.LoggingOut: return "Пользователь отключается"
                        case WebSocketClient.LoggedOut: return "Ни один пользователь не подключен."
                        case WebSocketClient.AuthorizedNoPingRespond: return "Подключен: " + (authHandler ? authHandler.username : "") + ".  Ping отсутствует."
                        case WebSocketClient.LoggedOutNoPingRespond: return "Ни один пользователь не подключен.  Ping отсутствует."
                        case WebSocketClient.ExternalDisconnecting: return "Ошибка сервера."
                        case WebSocketClient.UserDisconnecting: return "Отключаемся."
                        default: return "Неизвестный статус"
                    }

                }
            }
*/
        }
    }
    function processTFPath(){
        restClient.checkConnection()
        {
            let result = FileHelper.extCheckPathType(tf.text);
            tf.tfContent = result.path
            let type = result.type
            if (type === FileHelperType.LocalFile) {
                console.log("LocalFile")
                imageModel.addImagePath(tf.text)
            } else if (type === FileHelperType.LocalFolder) {
                console.log("LocalFolder")
                storageModel.enterLocal(tf.text)
                parentCustomDlgTb = 0;
                parentLocalPath = tf.text;
                customDialog.show();
            } else if (type === FileHelperType.MinioBucket) {
                parentCustomDlgTb = 1;
                //customDialog.currentTabIndex = 1;
                storageModel.setParent(tf.text, "mb")
                storageModel.getNetPath(tf.text, 98)
            } else if(type === FileHelperType.MinioFolder) {
                parentCustomDlgTb = 1;
                storageModel.setParent(tf.text, "md")
                storageModel.getNetPath(tf.text, 100)
            } else if (type === FileHelperType.MinioFile) {
                storageModel.setParent(tf.text, "mf")
                storageModel.getNetPath(tf.text, 102)
            } else {
                warningDialog.messageText = "Путь не распознан или не существует:  " +  tf.text;
                warningDialog.open()
            }
        }
    }
    Component.onCompleted:{
    }
}
