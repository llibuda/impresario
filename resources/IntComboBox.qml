/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2020  Lars Libuda
**
**   This file is part of Impresario.
**
**   Impresario is free software: you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation, either version 3 of the License, or
**   (at your option) any later version.
**
**   Impresario is distributed in the hope that it will be useful,
**   but WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**   GNU General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with Impresario in subdirectory "licenses", file "LICENSE_Impresario.GPLv3".
**   If not, see <http://www.gnu.org/licenses/>.
******************************************************************************************/
import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: intComboBoxEditor
    anchors.fill: parent

    property var items: ["Empty"]

    property bool selected: propertyView.currentItemRow === model.row
    property bool editorActive: false

    property int cbIndex: 0

    Component.onCompleted: function() {
        cbIndex = model.display
    }

    onCbIndexChanged: function() {
        model.display = cbIndex.toString()
    }

    onSelectedChanged: function() {
        if (!selected) {
            editorActive = false
        }
    }

    // Component used to render value in TableView
    Text {
        id: valueRenderer
        anchors.fill: parent
        text: parent.items[parent.cbIndex]
        elide: Text.ElideRight
        padding: 5
        color: (intComboBoxEditor.selected) ? palette.highlightedText : text
        focus: intComboBoxEditor.selected && !intComboBoxEditor.editorActive

        MouseArea{
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onPressed: function(mouse) {
                intComboBoxEditor.editorActive = true;
                mouse.accepted = false;
            }
        }

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                intComboBoxEditor.editorActive = true;
                event.accepted = true;
            }
        }
    }

    // Component used as in-place editor in TableView
    // Component is invisible first and rendered only if row becomes active (selected)
    ComboBox {
        id: valueInPlaceEditor
        anchors.fill: parent
        visible: intComboBoxEditor.editorActive
        focus: intComboBoxEditor.editorActive
        editable: false

        model: parent.items

        Component.onCompleted: function() {
            currentIndex = parent.cbIndex
        }

        onActivated: function(index) {
            parent.cbIndex = index
        }

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Escape || event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                intComboBoxEditor.editorActive = false;
                event.accepted = true
            }
        }

        // On Linux ComboBox causes a segfault when using default popup and this popup was activated once.
        // Defining a custom popup mitigates this segfault. On Windows default popup works.
        popup: Popup {
            y: valueInPlaceEditor.height - 1
            width: valueInPlaceEditor.width
            implicitHeight: contentItem.implicitHeight
            padding: 1

            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: valueInPlaceEditor.popup.visible ? valueInPlaceEditor.delegateModel : null
                currentIndex: valueInPlaceEditor.highlightedIndex

                ScrollIndicator.vertical: ScrollIndicator { }
            }

            background: Rectangle {
                color: palette.window
                border.color: palette.dark
                radius: 2
            }
        }
    }
}
