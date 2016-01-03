/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2016  Lars Libuda
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
import QtQuick 2.5
import QtQuick.Controls 1.4

Item {
    SystemPalette { id: palette; colorGroup: SystemPalette.Active }
    id: intComboBoxEditor
    anchors.fill: parent

    property var items: ["Empty"]

    // Component used to render value in TableView
    Text {
        id: valueRenderer
        anchors.fill: parent
        text: parent.items[styleData.value]
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
    ComboBox {
        id: valueInPlaceEditor
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        visible: false
        editable: false
        model: parent.items
        currentIndex: styleData.value
        focus: true
        onActivated: {
            itemProperties.setProperty(styleData.row,"value",index.toString());
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
