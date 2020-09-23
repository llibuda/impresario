import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    width: 300
    height: 300

    ListModel {
        id: itemProperties;

        property variant props: macro.parameters();

        Component.onCompleted: {
            for(var prop in props) {
                //console.log("Name",props[prop].name,"Value",props[prop].value);
                append({"name": props[prop].name, "value": props[prop].value, "component": props[prop].component, "properties": props[prop].properties, "description": props[prop].description });
            }
            // NOTE: There is a nasty bug in the TableView component introduced in Qt 5.6
            // It seems like a racing condition between this method and loading the TableView
            // Assigning the model to the TableView here fixes this problem
            //propertyView.model = this;
        }

        onDataChanged: {
            if (propertyView.currentRow >= 0 && propertyView.currentRow < itemProperties.count) {
                //console.log("Old Property value ",props[propertyView.currentRow].value);
                props[propertyView.currentRow].value = itemProperties.get(propertyView.currentRow).value;
                //console.log("New Property value ",props[propertyView.currentRow].value);
            }
        }
    }

    Component {
        id: propertyDelegate
        Item {
            width: 180; height: 40
            Column {
                Text { text: '<b>Name:</b> ' + property }
            }
            Column {
                Text { text: '<b>Value:</b> ' + value }
            }
        }
    }

    /*
    ListView {
        id: propertyView;
        anchors.fill: parent;
        model: itemProperties;
        delegate: propertyDelegate;
    }
    */

    /*
    ListView {
        id: propertyView;
        anchors.fill: parent;

        contentWidth: headerItem.width;
        flickableDirection: Flickable.HorizontalAndVerticalFlick;

        header: Row {
            spacing: 1
            function itemAt(index) { return repeater.itemAt(index) }
            Repeater {
                id: repeater
                model: ["Quisque", "Posuere", "Curabitur", "Vehicula", "Proin"]
                Label {
                    text: modelData
                    font.bold: true
                    font.pixelSize: 20
                    padding: 10
                    background: Rectangle { color: "silver" }
                }
            }
        }

        model: 100
        delegate: Column {
            id: delegate
            property int row: index
            Row {
                spacing: 1
                Repeater {
                    model: 5
                    ItemDelegate {
                        property int column: index
                        text: qsTr("%1x%2").arg(delegate.row).arg(column)
                        width: propertyView.headerItem.itemAt(column).width
                    }
                }
            }
            Rectangle {
                color: "silver"
                width: parent.width
                height: 1
            }
        }

        ScrollIndicator.horizontal: ScrollIndicator { }
        ScrollIndicator.vertical: ScrollIndicator { }
    }
    */

    TableView {
        id: propertyView;
        anchors.fill: parent;
        columnSpacing: 1;
        rowSpacing: 1;
        clip: true;

        model: itemProperties;

        delegate: Item {
            implicitHeight: 20
            implicitWidth: 50
            Label {
                text: name
            }
        }
    }

}
