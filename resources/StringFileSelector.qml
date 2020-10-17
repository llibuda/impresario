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
    id: fileSelectDialog
    anchors.fill: parent

    property int maxLength: 512;
    property bool mustExists : true;
    property string title: "Select file"
    property variant filters: [ "All files (*)" ]

    property bool selected: propertyView.currentItemRow === model.row
    property bool editorActive: false

    onSelectedChanged: {
        if (!selected) {
            editorActive = false
            fileTextField.text = model.display
        }
    }

    // Component used to render value in TableView
    Text {
        id: valueRenderer
        anchors.fill: parent
        text: model.display
        elide: Text.ElideRight
        padding: 5
        color: (fileSelectDialog.selected) ? palette.highlightedText : text
        focus: fileSelectDialog.selected && !fileSelectDialog.editorActive

        MouseArea{
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onPressed: function(mouse) {
                fileSelectDialog.editorActive = true;
                mouse.accepted = false;
            }
        }

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                fileSelectDialog.editorActive = true;
                event.accepted = true;
            }
        }
    }

    // Component used as in-place editor in TableView
    // Component is invisible first and rendered only if editor is activated
    FocusScope {
        id: valueInPlaceEditor
        anchors.fill: parent
        visible: fileSelectDialog.editorActive
        focus: fileSelectDialog.editorActive

        TextField {
            id: fileTextField
            anchors.fill: parent
            anchors.rightMargin: dirSelectBtn.implicitWidth

            maximumLength: fileSelectDialog.maxLength

            visible: fileSelectDialog.editorActive
            focus: fileSelectDialog.editorActive
            selectByMouse: true;

            Component.onCompleted: function() {
                text = model.display
            }

            Keys.onPressed: function(event) {
                if (event.key === Qt.Key_Escape) {
                    fileSelectDialog.editorActive = false;
                    text = model.display
                }
                else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    fileSelectDialog.editorActive = false;
                    model.display = text
                }
            }
        }

        FileDialog {
            id: fileDialog
            modality: Qt.ApplicationModal
            title: fileSelectDialog.title
            nameFilters: fileSelectDialog.filters
            options: FolderDialog.ShowDirsOnly | FolderDialog.ReadOnly
            onAccepted: {
                if (file.toString().match("^file:///[A-Z,a-z]:")) {
                    fileTextField.text = decodeURIComponent(file.toString()).replace("file:///","") //.replace(/\//g,'\\')
                }
                else {
                    fileTextField.text = decodeURIComponent(file.toString()).replace("file://","")
                }
                model.display = fileTextField.text
            }
        }

        Button {
            id: dirSelectBtn
            anchors.left: fileTextField.right
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
                var file = Qt.resolvedUrl(fileTextField.text);
                var folder = file.substring(0,file.lastIndexOf("/"));
                if (!folder.match("^file://")) {
                    if (folder.match("^[A-Z,a-z]:")) {
                        fileDialog.folder = "file:///" + encodeURIComponent(folder);
                    }
                    else {
                        fileDialog.folder = "file://" + encodeURIComponent(folder);
                    }
                }
                else {
                   fileDialog.folder = "file://" + encodeURIComponent(folder.replace("file://",""));
                }
                fileDialog.open();
            }
        }
    }
}
