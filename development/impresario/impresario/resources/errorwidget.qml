import QtQuick 2.5
import QtQuick.Controls 1.4

TextArea
{
    readOnly: true;
    selectByMouse: false;
    textFormat: TextEdit.RichText;
    wrapMode: TextEdit.WordWrap;
    Component.onCompleted: {
        append("<b style=\"color: red\">" + propWnd.qmlErrorDlgTitle + "</b><br>");
        for(var msgIndex in propWnd.qmlErrors) {
            append(propWnd.qmlErrors[msgIndex] + "<br>");
        }
    }
}

