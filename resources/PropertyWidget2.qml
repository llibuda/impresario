import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import Qt.labs.qmlmodels 1.0

Item {
    // "macro" is passed as QML Context by Impresario

    width: 300
    height: 300

    onWidthChanged: propertyView.forceLayout()

    SystemPalette {
        id: palette
        colorGroup: SystemPalette.Active
    }
    FontMetrics {
        id: defaultFont
    }

    TableModel {
        id: itemModel;

        property variant props: macro.parameters();

        TableModelColumn {
            display: "name"
        }
        TableModelColumn {
            display: "value"
        }

        Component.onCompleted: {
            for(var prop in props) {
                appendRow({"name": props[prop].name, "value": props[prop].value, "component": props[prop].component, "properties": props[prop].properties, "description": props[prop].description });
            }
        }

        onDataChanged: {
            if (propertyView.currentRow >= 0 && propertyView.currentRow < rowCount) {
                //console.log("Old Property value ",props[propertyView.currentRow].value);
                props[propertyView.currentRow].value = data(index(propertyView.currentRow,1),"display")
                //console.log("New Property value ",props[propertyView.currentRow].value);
            }
        }
    }

    Connections {
        target: macro
        function onParameterUpdated(index) {
            itemModel.setData(itemModel.index(index,1), "display", itemModel.props[index].value)
        }
    }

    HorizontalHeaderView {
        id: headerView
        boundsBehavior: Flickable.StopAtBounds
        syncView: propertyView
        anchors.left: propertyView.left
        width: parent.width
        model: ["Property","Value"]
    }

    TableView {
        id: propertyView
        model: itemModel;
        boundsBehavior: Flickable.StopAtBounds
        anchors.fill: parent
        anchors.topMargin: headerView.height
        clip: true
        focus: true

        columnWidthProvider: function(column) {
            if (column === 0)
                return parent.width / 3
            else
                return parent.width / 3 * 2
        }

        property int currentRow: -1

        onActiveFocusChanged: function(hasFocus) {
            if (!hasFocus) propertyView.currentRow = -1
        }

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Up) {
                if (propertyView.currentRow < 0) propertyView.currentRow = 0
                else if (propertyView.currentRow > 0) propertyView.currentRow--
                macro.showDescription(propertyView.currentRow)
                event.accepted = true;
            }
            else if (event.key === Qt.Key_Down) {
                if (propertyView.currentRow < 0) propertyView.currentRow = 0
                else if (propertyView.currentRow < propertyView.rows - 1) propertyView.currentRow++
                macro.showDescription(propertyView.currentRow)
                event.accepted = true;
            }
        }

        MouseArea{
            id: propertyViewMouseArea
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onPressed: function(mouse) {
                // the following formular implies that all rows have the same height ...
                propertyView.currentRow = Math.min(mouse.y / propertyView.contentHeight * propertyView.rows, propertyView.rows - 1)
                macro.showDescription(propertyView.currentRow)
                mouse.accepted = true
            }
        }

        delegate: DelegateChooser {
            DelegateChoice {
                column: 0
                delegate: Text {
                    text: model.display
                    elide: Text.ElideRight
                    font: defaultFont.font
                    color: (propertyView.currentRow === model.row) ? palette.highlightedText : text
                    padding: 5
                    Rectangle {
                        anchors.fill: parent
                        z: -1
                        border.width: 1
                        border.color: palette.midlight
                        color: {
                            if (propertyView.currentRow === model.row) {
                                return palette.highlight
                            }
                            else if ((model.row) % 2 === 0) {
                                return palette.alternateBase
                            }
                            else {
                                return palette.base;
                            }
                        }
                    }
                }
            }
            DelegateChoice {
                column: 1
                delegate: Text {
                    text: model.display
                    color: (propertyView.currentRow === model.row) ? palette.highlightedText : text
                    elide: Text.ElideRight
                    padding: 5
                    //selectByMouse: true

                    //onAccepted: model.display = text
                    Rectangle {
                        anchors.fill: parent
                        z: -1
                        border.width: 1
                        border.color: palette.midlight
                        color: {
                            if (propertyView.currentRow === model.row) {
                                return palette.highlight
                            }
                            else if ((model.row) % 2 === 0) {
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
    }
}
