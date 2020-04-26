import QtQuick 2.4
import QmlInteractiveView 1.0

MCInteractVideoWindow {
    id: stitchView
    destroyOnClose: true
    property alias camView : interact

    QmlInteractiveView {
        id: interact
        anchors.fill: parent
        visible: true
    }

    function closeView()
    {
        close()
    }
}

