import QtQuick 2.5
import	QmlInteractiveView 1.0

Item {
    width: centralItem.width
    height:　centralItem.height
    visible: false
    property int cellWidthVal: 425
    property int cellHeightVal: 425
    Rectangle {
        id: backgroundRectangle
        color: "#000000"
        width: parent.width
        height: parent.height
        GridView {
            id: interactiveGridView
            anchors.fill: parent

            model: ListModel {
                id: gridModel
            }
        }
    }

    Rectangle {
        id: seamView
        width: parent.width
        height: parent.height
        opacity: 0.0
        z: 1

        MouseArea {

            anchors.fill: parent
            onPressed: {
                var clickPos  = mapToItem(parent, mouse.x,mouse.y)
                qmlMainWindow.onPressedInteractive(clickPos.x, clickPos.y);
            }

            onReleased: {
                var releasedPos = mapToItem(parent, mouse.x,mouse.y);
                qmlMainWindow.onReleasedInteractive(releasedPos.x, releasedPos.y);
            }

            onPositionChanged: {
                var movedPos  = mapToItem(parent,mouse.x,mouse.y);
                qmlMainWindow.onMovedInteractive(movedPos.x, movedPos.y);

            }
        }
    }
    function createInteractiveCameraWindow()
    {
            var component = Qt.createComponent("MCInteractCameraView.qml");
            if (component.status == Component.Ready) {
                var stitchViewObject = component.createObject(interactiveGridView, {"x": 0, "y": 0});
                if(stitchViewObject == null) {
                }else{
                    qmlMainWindow.createInteractView(stitchViewObject.camView)
                }
            }
            else
                console.log(component.errorString())
    }
}

