import QtQuick 2.5
import QtQuick.Controls 1.4

Item {
    width: 1280
    height: 65
    property int minWidth: 1080
    property int minHeight: 720
    property string elapseText: qsTr("00:00:00.000")
    property string fpsText: qsTr("0 fps")
    property int levelText: 0

    Rectangle {
        anchors.fill: parent
        color:"#000000"
    }

    Rectangle {
        id: exampleTimeLineImage
        anchors.centerIn: parent
        color:"#000000"
        width: 526
        height: 60
        Label{
            id:elapsedID
            text: "Elapsed:"
            font.pixelSize: 25
            color: "#dddddd"
            anchors.verticalCenter: parent.verticalCenter
        }
        Label{
            id:elapsedTextID
            anchors.left: elapsedID.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 4
            text: elapseText
            font.pixelSize: 35
            color: "white"
        }
        Label{
            id:rateID
            anchors.left: elapsedTextID.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 150
            text: "Rate:"
            font.pixelSize: 25
            color: "steelblue"
        }
        Label{
            id:rateTextID
            anchors.left: rateID.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 4
            text: fpsText
            font.pixelSize: 35
            color: "steelblue"
        }
//        Label{
//            id:levelID
//            anchors.left: rateTextID.right
//            anchors.verticalCenter: parent.verticalCenter
//            anchors.leftMargin: 250
//            text: "BandLevel:"
//            font.pixelSize: 25
//            color: "#dddddd"
//        }
//        Label{
//            id:levelTextID
//            anchors.left: levelID.right
//            anchors.verticalCenter: parent.verticalCenter
//            anchors.leftMargin: 4
//            text: levelText
//            font.pixelSize: 35
//            color: "#ffffff"
//        }
    }

    Rectangle {
        id: spliter
        x: 50
        width: parent.width - 50
        height: 2
        color: "#2b2b2b"
    }

    MouseArea {
        id: resizeMouseArea
        width: 5
        height: 5
        y: parent.height - height
        x: parent.width - width

        hoverEnabled: true
        onHoveredChanged: {
            if(!isFullScreen)
                cursorShape = Qt.SizeFDiagCursor
        }

        property bool isMoveMode: false
        property var clickPos: "1,1"
        property real movedX: 0
        property real movedY: 0

        onPressed: {
            if(mouse.button == Qt.LeftButton && !isFullScreen) {
                isMoveMode = true
                clickPos  = mapToItem(statusItem.parent, mouse.x,mouse.y)
            }
        }

        onPositionChanged: {
            if(isMoveMode) {
                var x = mouse.x
                var y = mouse.y
                var parentPos = mapToItem(statusItem.parent, x, y)
                var delta = Qt.point(parentPos.x-clickPos.x - movedX, parentPos.y-clickPos.y - movedY)

                if (root.width + delta.x >= minWidth) {
                    movedX += delta.x
                    root.width += delta.x
                }
                if (root.height + delta.y >= minHeight) {
                    movedY += delta.y
                    root.height += delta.y
                }
            }
        }

        onReleased: {
            if(mouse.button == Qt.LeftButton) {
                isMoveMode = false
                movedX = 0
                movedY = 0
            }
        }
    }

    Rectangle {
        id: resizeLine
        width: 30
        height: 2
        x: parent.width - width + 4
        y: parent.height - height - 3
        color: "#1f1f1f"
        rotation: -45
    }

    Rectangle {
        id: resizeLine2
        width: 20
        height: 2
        x: parent.width - width + 4
        y: parent.height - height - 3
        color: "#1f1f1f"
        rotation: -45
    }
}
