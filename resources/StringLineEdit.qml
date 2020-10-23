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
    id: lineTextEditor
    anchors.fill: parent

    property int maxLength: 255;
    property bool readOnly: false;

    property bool selected: propertyView.currentItemRow === model.row
    property bool editorActive: false

    onSelectedChanged: function() {
        if (!selected) {
            editorActive = false
            valueInPlaceEditor.text = model.display
        }
    }

    // Component used to render value in TableView
    Text {
        id: valueRenderer
        anchors.fill: parent
        text: model.display
        elide: Text.ElideRight
        padding: 5
        color: (lineTextEditor.selected) ? palette.highlightedText : text
        focus: lineTextEditor.selected && !lineTextEditor.editorActive

        MouseArea{
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onPressed: function(mouse) {
                if (!lineTextEditor.readOnly) {
                    lineTextEditor.editorActive = true;
                }
                mouse.accepted = false;
            }
        }

        Keys.onPressed: function(event) {
            if (!lineTextEditor.readOnly && (event.key === Qt.Key_Enter || event.key === Qt.Key_Return)) {
                lineTextEditor.editorActive = true;
                event.accepted = true;
            }
        }
    }

    // Component used as in-place editor in TableView
    // Component is invisible first and rendered only if editor is activated
    TextField {
        id: valueInPlaceEditor
        anchors.fill: parent

        maximumLength: lineTextEditor.maxLength

        visible: lineTextEditor.editorActive
        focus: lineTextEditor.editorActive
        selectByMouse: true;
        text: model.display
        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Escape) {
                lineTextEditor.editorActive = false;
                text = model.display
            }
            else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                lineTextEditor.editorActive = false;
                model.display = text
            }
        }
    }
}
