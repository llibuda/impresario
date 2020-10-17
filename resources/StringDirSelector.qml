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
import Qt.labs.platform 1.1

Item {
    id: dirSelectDialog
    anchors.fill: parent

    property int maxLength: 512;

    property bool selected: propertyView.currentItemRow === model.row
    property bool editorActive: false

    onSelectedChanged: {
        if (!selected) {
            editorActive = false
            dirTextField.text = model.display
        }
    }

    // Component used to render value in TableView
    Text {
        id: valueRenderer
        anchors.fill: parent
        text: model.display
        elide: Text.ElideRight
        padding: 5
        color: (dirSelectDialog.selected) ? palette.highlightedText : text
        focus: dirSelectDialog.selected && !dirSelectDialog.editorActive

        MouseArea{
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onPressed: function(mouse) {
                dirSelectDialog.editorActive = true;
                mouse.accepted = false;
            }
        }

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                dirSelectDialog.editorActive = true;
                event.accepted = true;
            }
        }
    }

    // Component used as in-place editor in TableView
    // Component is invisible first and rendered only if editor is activated
    FocusScope {
        id: valueInPlaceEditor
        anchors.fill: parent
        visible: dirSelectDialog.editorActive
        focus: dirSelectDialog.editorActive

        TextField {
            id: dirTextField
            anchors.fill: parent
            anchors.rightMargin: dirSelectBtn.implicitWidth

            maximumLength: dirSelectDialog.maxLength

            visible: dirSelectDialog.editorActive
            focus: dirSelectDialog.editorActive
            selectByMouse: true;

            Component.onCompleted: function() {
                text = model.display
            }

            Keys.onPressed: function(event) {
                if (event.key === Qt.Key_Escape) {
                    dirSelectDialog.editorActive = false;
                    text = model.display
                }
                else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    dirSelectDialog.editorActive = false;
                    model.display = text
                }
            }
        }

        FolderDialog {
            id: dirDialog
            modality: Qt.ApplicationModal
            title: "Choose a folder"
            options: FolderDialog.ShowDirsOnly | FolderDialog.ReadOnly
            onAccepted: {
                if (folder.toString().match("^file:///[A-Z,a-z]:")) {
                    dirTextField.text = decodeURIComponent(folder.toString()).replace("file:///","") //.replace(/\//g,'\\')
                }
                else {
                    dirTextField.text = decodeURIComponent(folder.toString()).replace("file://","")
                }
                model.display = dirTextField.text
            }
        }

        Button {
            id: dirSelectBtn
            anchors.left: dirTextField.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right

            TextMetrics {
                id: btnText
                text: "..."
            }

            implicitWidth: btnText.width + 16
            text: btnText.text

            onClicked: function() {
                var dir = Qt.resolvedUrl(dirTextField.text)
                if (!dir.match("^file://")) {
                    if (dir.match("^[A-Z,a-z]:")) {
                        dirDialog.folder = "file:///" + encodeURIComponent(dir)
                    }
                    else {
                        dirDialog.folder = "file://" + encodeURIComponent(dir)
                    }
                }
                else {
                   dirDialog.folder = "file://" + encodeURIComponent(dir.replace("file://",""))
                }
                dirDialog.open()
            }
        }
    }
}
