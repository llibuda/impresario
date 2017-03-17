/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2017  Lars Libuda
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
import QtQuick.Controls.Styles 1.4

Item {
    width: 300
    height: 300

    ListModel {
        id: itemProperties;

        property variant props: macro.parameters();

        Component.onCompleted: {
            for(var prop in props) {
                //console.log("Name",props[prop].name,"Value",props[prop].value);
                append({"property": props[prop].name, "value": props[prop].value, "component": props[prop].component, "properties": props[prop].properties, "description": props[prop].description });
            }
            // NOTE: There is a nasty bug in the TableView component introduced in Qt 5.6
            // It seems like a racing condition between this method and loading the TableView
            // Assigning the model to the TableView here fixes this problem
            propertyView.model = this;
        }

        onDataChanged: {
            if (propertyView.currentRow >= 0 && propertyView.currentRow < itemProperties.count) {
                //console.log("Old Property value ",props[propertyView.currentRow].value);
                props[propertyView.currentRow].value = itemProperties.get(propertyView.currentRow).value;
                //console.log("New Property value ",props[propertyView.currentRow].value);
            }
        }
    }

    Connections {
        target: macro;
        onParameterUpdated: {
            itemProperties.set(index,{"value": itemProperties.props[index].value });
        }
    }

    Component {
        id: propertyDelegate

        Item {
            SystemPalette { id: palette; colorGroup: SystemPalette.Active }
            id: propertyDelegateItem
            //anchors.fill: parent
            Component.onCompleted: {
                if (styleData.column === 0) {
                    Qt.createQmlObject(
                        'import QtQuick 2.5
                         Text {
                            id: propertyDelegateText
                            anchors.fill: parent
                            anchors.leftMargin: 3.0
                            text: styleData.value
                            elide: styleData.elideMode
                            verticalAlignment: Text.AlignVCenter
                            renderType: Text.NativeRendering
                            color: if (styleData.selected) {
                                return palette.highlightedText
                            }
                            else {
                                return palette.text
                            }
                         }',propertyDelegateItem,"propertyNameItem");
                }
                else if (styleData.column === 1) {
                    var component = Qt.createComponent(itemProperties.props[styleData.row].component + ".qml");
                    if (component.status === Component.Error) {
                        component = Qt.createComponent("StringLineEdit.qml");
                    }
                    if (component.status === Component.Error) {
                        throw "\nFailed to load custom component '" + itemProperties.props[styleData.row].component + ".qml'" +
                              "and default component 'StringLineEdit.qml' as backup for parameter '" + itemProperties.props[styleData.row].name + "'";
                    }

                    if (itemProperties.props[styleData.row].properties.length > 0) {
                        var errorCaught = false;
                        var props;
                        try {
                            props = eval(itemProperties.props[styleData.row].properties);
                        }
                        catch(error) {
                            errorCaught = true;
                            throw "\nFailed to correctly parse custom properties '" + itemProperties.props[styleData.row].properties + "'\n" +
                                  "for parameter '" + itemProperties.props[styleData.row].name + "':" +
                                  error.message +
                                  "\nProperties are ignored.";
                        }
                        finally {
                            if (errorCaught) {
                                component.createObject(propertyDelegateItem);
                            }
                            else {
                                component.createObject(propertyDelegateItem,props);
                            }
                        }
                    }
                    else {
                        component.createObject(propertyDelegateItem);
                    }
                }
            }

            Rectangle {
                anchors.fill: parent
                color: palette.midlight

                Rectangle {
                    anchors.fill: parent
                    anchors.bottomMargin: 1.0
                    anchors.rightMargin: 1.0
                    color: {
                        if (styleData.selected) {
                            return palette.highlight;
                        }
                        else if ((propertyView.rowCount - styleData.row) % 2 === 1) {
                            return palette.alternateBase
                        }
                        else {
                            return palette.base;
                        }
                    }
                }
            }
        }
    }

    Component {
        id: propertyRowDelegate

        Rectangle {
            SystemPalette { id: palette; colorGroup: SystemPalette.Active }
            FontMetrics { id: defaultFont; }
            id: rowRectangle
            height: defaultFont.height + 7
            color: palette.base
        }
    }

    TableView {
        id : propertyView;
        // NOTE: No model assignment here any more, see also comment in ListModel:Component.onCompleted
        //model: itemProperties;
        anchors.fill: parent;
        itemDelegate: propertyDelegate
        rowDelegate: propertyRowDelegate

        onClicked: {
            macro.showDescription(row);
        }

        TableViewColumn {
            id: colPropertyName
            role: "property"
            title: "Property"
            width: 120
            resizable: true
            movable: false
        }
        TableViewColumn {
            id: colPropertyValue
            role: "value"
            title: "Value"
            width: propertyView.width - colPropertyName.width - 2
            resizable: false
            movable: false
        }

    }

}
