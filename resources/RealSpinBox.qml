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
    id: spinBoxEditor
    anchors.fill: parent

    property real minValue: 0;
    property real maxValue: 0;
    property real step: 0.1;
    property int  decimals: 1;

    property bool selected: propertyView.currentItemRow === model.row
    property bool editorActive: false

    onSelectedChanged: function() {
        if (!selected) {
            editorActive = false
        }
    }

    // Component used to render value in TableView
    Text {
        id: valueRenderer
        anchors.fill: parent
        text: Number(model.display).toLocaleString(Qt.locale(), 'f', decimals)
        elide: Text.ElideRight
        padding: 5
        color: (spinBoxEditor.selected) ? palette.highlightedText : text
        focus: spinBoxEditor.selected && !spinBoxEditor.editorActive

        MouseArea{
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onPressed: function(mouse) {
                spinBoxEditor.editorActive = true;
                mouse.accepted = false;
            }
        }

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                spinBoxEditor.editorActive = true;
                event.accepted = true;
            }
        }
    }

    // Component used as in-place editor in TableView
    // Component is invisible first and rendered only if row becomes active (selected)
    SpinBox {
        id: valueInPlaceEditor
        anchors.fill: parent
        visible: spinBoxEditor.editorActive
        focus: spinBoxEditor.editorActive
        editable: true
        wheelEnabled: true
        leftPadding: 5

        property int decimals: parent.decimals
        property real scalingFactor: 10.0 * decimals
        property bool blockUpdate: true

        from: parent.minValue * scalingFactor
        to: parent.maxValue * scalingFactor
        stepSize: parent.step * scalingFactor

        validator: DoubleValidator {
            bottom: Math.min(spinBoxEditor.from, spinBoxEditor.to)
            top:  Math.max(spinBoxEditor.from, spinBoxEditor.to)
        }

        textFromValue: function(value, locale) {
            return Number(value / scalingFactor).toLocaleString(locale, 'f', decimals)
        }

        valueFromText: function(text, locale) {
            return Number.fromLocaleString(locale, text) * scalingFactor
        }

        Component.onCompleted: function() {
            // we do not use property binding here but just assign the control the current value
            // otherwise there will be a binding loop in onValueChanged handler
            value = Number(model.display) * scalingFactor
            // modification of SpinBox's contentItem
            contentItem.horizontalAlignment = Qt.AlignLeft
            contentItem.selectByMouse = true
            blockUpdate = false
        }

        onValueChanged: function() {
            if (!blockUpdate)
            {
                model.display = Number(value / scalingFactor)
            }
        }

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Escape || event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                spinBoxEditor.editorActive = false;
                event.accepted = true
            }
        }

    }
}
