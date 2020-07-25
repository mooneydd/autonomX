import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import "qrc:/stylesheet"

Field {
    id: areaField

    property string placeholder: "Area Field"
    property string defaultText: "Area Field"

    Layout.fillHeight: true

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

        FieldFrame {
            enabled: !deactivated
            isHovered: textArea.hovered
            isFocused: textArea.activeFocus
        }

        Flickable {
            id: flickable
            anchors.fill: parent
            maximumFlickVelocity: 350

            enabled: !deactivated

            TextArea.flickable: TextArea {
                id: textArea

                enabled: !deactivated

                // root settings
                text: defaultText
                placeholderText: placeholder
                wrapMode: TextArea.Wrap

                // alignments
                width: fieldWidth
                leftPadding: 0
                rightPadding: 0
                topPadding: Stylesheet.field.padding
                bottomPadding: Stylesheet.field.padding

                // background
                background: Rectangle { opacity: 0 }

                // font & color
                font.pixelSize: 14

                // mouse interaction
                selectByMouse: true

                // signal hooks
                onEditingFinished: valueChanged(text)
            }

            ScrollBar.vertical: ScrollBar {}
        }
    }
}