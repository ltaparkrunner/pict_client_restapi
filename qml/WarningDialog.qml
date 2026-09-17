import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root

    // Dialog configuration
    property string dialogTitle: "Warning"
    property string messageText: ""
    property string iconSource: "../icons/warning.png" // Replace with your icon path

    // Button configurations
    property string acceptButtonText: "OK"
    property string rejectButtonText: "Cancel"
    property bool showCancel: true

    // Signals
    signal accepted
    signal rejected

    // Modal settings
    modal: true
    focus: true
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    width: Math.min(parent.width * 0.8, 400)
    implicitHeight: layout.implicitHeight + 40
    closePolicy: Popup.NoAutoClose

    // Styling
    background: Rectangle {
        radius: 12
        color: "#FFFFFF"
        border.color: "#E0E0E0"
    }

    contentItem: ColumnLayout {
        id: layout
        spacing: 16

        // Title and Icon
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Image {
                source: root.iconSource
                sourceSize.width: 24
                sourceSize.height: 24
                Layout.alignment: Qt.AlignVCenter
            }

            Label {
                text: root.dialogTitle
                font.bold: true
                font.pixelSize: 16
                color: "#1A1A1A"
                Layout.fillWidth: true
                wrapMode: Label.Wrap
            }
        }

        // Message
        Label {
            text: root.messageText
            color: "#4A4A4A"
            font.pixelSize: 14
            Layout.fillWidth: true
            wrapMode: Label.Wrap
        }

        // Action Buttons
        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 12

            Button {
                id: cancelButton
                text: root.rejectButtonText
                visible: root.showCancel

                background: Rectangle {
                    radius: 6
                    color: cancelButton.pressed ? "#E8E8E8" : "#F5F5F5"
                }

                onClicked: {
                    root.rejected()
                    root.close()
                }
            }

            Button {
                id: okButton
                text: root.acceptButtonText

                background: Rectangle {
                    radius: 6
                    color: okButton.pressed ? "#D32F2F" : "#F44336" // Red warning color
                }

                contentItem: Text {
                    text: okButton.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    root.accepted()
                    root.close()
                }
            }
        }
    }
}
