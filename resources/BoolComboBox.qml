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
    id: boolComboBoxEditor
    anchors.fill: parent

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
        text: (cbIndex === 0) ? "False" : "True"
        elide: Text.ElideRight
        padding: 5
        color: (boolComboBoxEditor.selected) ? palette.highlightedText : text
        focus: boolComboBoxEditor.selected && !boolComboBoxEditor.editorActive

        MouseArea{
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onPressed: function(mouse) {
                boolComboBoxEditor.editorActive = true;
                mouse.accepted = false;
            }
        }

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                boolComboBoxEditor.editorActive = true;
                event.accepted = true;
            }
        }
    }

    // Component used as in-place editor in TableView
    // Component is invisible first and rendered only if row becomes active (selected)
    ComboBox {
        id: valueInPlaceEditor
        anchors.fill: parent
        visible: boolComboBoxEditor.editorActive
        focus: boolComboBoxEditor.editorActive
        editable: false

        model: ListModel {
            id: cbItems
            ListElement { text: "False"}
            ListElement { text: "True"}
        }

        Component.onCompleted: function() {
            currentIndex = parent.cbIndex
        }

        onActivated: function(index) {
            parent.cbIndex = index.toString()
        }

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Escape || event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                boolComboBoxEditor.editorActive = false;
                event.accepted = true
            }
        }
    }
}
