/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015  Lars Libuda
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
**   along with Impresario in subdirectory "licenses", file "LICENSE.GPLv3".
**   If not, see <http://www.gnu.org/licenses/>.
******************************************************************************************/
import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2

Item {
    SystemPalette { id: palette; colorGroup: SystemPalette.Active }
    id: dirSelectDialog
    anchors.fill: parent

    property int maxLength: 512;

    // Component used to render value in TableView
    Text {
        id: valueRenderer
        anchors.fill: parent
        text: styleData.value
        color: if (styleData.selected) {
            return palette.highlightedText
        }
        else {
            return palette.text
        }
        elide: styleData.elideMode
        verticalAlignment: Text.AlignVCenter
    }

    // Component used as in-place editor in TableView
    // Component is invisible first and rendered only if row becomes active (selected)
    Item {
        id: valueInPlaceEditor
        anchors.fill: parent
        visible: false

        FileDialog {
            id: dirDialog
            modality: Qt.WindowModal
            title: "Choose a folder"
            selectExisting: true
            selectMultiple: false
            selectFolder: true
            onAccepted: {
                if (folder.toString().match("^file:///[A-Z,a-z]:")) {
                    dirTextField.text = decodeURIComponent(folder.toString()).replace("file:///","");//.replace(/\//g,'\\');
                }
                else {
                    dirTextField.text = decodeURIComponent(folder.toString()).replace("file://","");
                }
            }
        }

        TextField {
            id: dirTextField
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: dirSelectBtn.left
            maximumLength: parent.parent.maxLength
            text: styleData.value
            focus: true
            onTextChanged: {
                itemProperties.setProperty(styleData.row,"value",text);
            }
        }

        Button {
            id: dirSelectBtn
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width: 20
            text: "..."
            isDefault: false
            onClicked: {
                var dir = Qt.resolvedUrl(dirTextField.text);
                if (!dir.match("^file://")) {
                    if (dir.match("^[A-Z,a-z]:")) {
                        dirDialog.folder = "file:///" + encodeURIComponent(dir);
                    }
                    else {
                        dirDialog.folder = "file://" + encodeURIComponent(dir);
                    }
                }
                else {
                   dirDialog.folder = "file://" + encodeURIComponent(dir.replace("file://",""));
                }
                dirDialog.open();
            }
        }

        states: [
            State {
                name: "selected"
                when: styleData.selected && styleData.column === 1
                PropertyChanges {target: valueInPlaceEditor; visible: true}
            }
        ]
    }
}
