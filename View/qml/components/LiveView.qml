import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQml 2.2
import QtQuick.Layouts 1.1
import MCQmlCameraView 1.0

Item {
    width: centralItem.width
    height: centralItem.height
    visible: false
    property int cellWidthVal: 425
    property int cellHeightVal: 350
    property int cellCount: Math.floor(width / 350)
    property int  topMargin: 20
    property var colors: ["Red","Green","Blue","Dark cyan","Magenta","#808000","Dark gray","Dark red","Dark green","Dark blue","Dark magenta","Gray","Light gray"]
    property var componentList: []
    property var cpGroupList: []
    property int frameWidth: 400
    property int frameHeight: 300
    Component.onCompleted: {
    }

    Rectangle {
        id: backgroundRectangle
        color: "#000000"
        width: parent.width
        height: parent.height
        GridView {
            id: liveGridView
            cellHeight: cellHeightVal
            cellWidth: cellWidthVal
            contentHeight: parent.height
            flickableDirection: Flickable.VerticalFlick
            flow: GridView.FlowLeftToRight
            highlightRangeMode: GridView.ApplyRange
            boundsBehavior: Flickable.StopAtBounds
            anchors.fill: parent
           // anchors.left: parent.right
            anchors.topMargin: 30
            anchors.leftMargin: (parent.width - cellCount * 350) / (cellCount + 1)
            model: ListModel {
                id: gridModel
            }
            delegate: Item {
                x: 5
                width: cellWidthVal - 30
                height: cellHeightVal - 30
                Column {
                    MCCameraView{
                        deviceNum: index
                    }
                }
            }
        }
    }

    Timer {
        id: cPointTimer
        interval: 100
        running: false
        repeat: false
        onTriggered: createCPoint();
    }

    function createCameraWindow()
    {
        var x, y;
        x = 65;
        y = 40;
        gridModel.clear()
        for(var i = 0; i < qmlMainWindow.getCameraCount(); i++) {
            gridModel.append({"index": i});
        }
    }
    function createTemplateVideoWindow()
    {
        var x, y;
        x = 65;
        y = 40;
        gridModel.clear()
        for(var i = 0; i < qmlMainWindow.getCameraCount(); i++) {
            gridModel.append({"index": i});
        }
    }
    function createTemplateCameraWindow()
    {
        var x, y;
        x = 65;
        y = 40;
        gridModel.clear()
        for(var i = 0; i < qmlMainWindow.getCameraCount(); i++) {
            gridModel.append({"index": i});
        }
    }
    function createTemplateImageWindow()
    {
        var x, y;
        x = 65;
        y = 40;
        gridModel.clear()
        for(var i = 0; i < qmlMainWindow.getCameraCount(); i++) {
            gridModel.append({"index": i});
        }
    }

    function createCPoint()
    {
//        var frameWidth = 400;
//        var frameHeight = 300;
        var curWidth = frameWidth;
        var curHeight = frameHeight;
        var orgWidth = qmlMainWindow.getTempWidth();
        var orgHeight = qmlMainWindow.getTempHeight();
        if(orgHeight/orgWidth > frameHeight/frameWidth)
        {
            curHeight = frameHeight;
            curWidth = curHeight * (orgWidth / orgHeight);
        }
        else
        {
            curWidth = frameWidth;
            curHeight = curWidth * (orgHeight / orgWidth);
        }
        var cPointCount = 0;
        var componentIndex = -1;
        for(var i = 0; i<qmlMainWindow.getCameraCount(); i++)
        {
            cPointCount = qmlMainWindow.getCPointCountEx(i);
            for (var j = 0; j < cPointCount; j++)
            {
                var pos = qmlMainWindow.getCPointEx(j,i);
                var posList = pos.split(":");
                var xPos = (curWidth/orgWidth) * posList[0] + (frameWidth - curWidth) / 2;
                var yPos = (curHeight/orgHeight) * posList[1] + (frameHeight - curHeight) / 2 + 30;
                var cpGroupIndex = posList[2];
                cpGroupList[j] = cpGroupIndex
                var component = Qt.createComponent("ControlPoint.qml");
                liveGridView.currentIndex = i;
                var cameraItem = liveGridView.currentItem
                var cPoint = component.createObject(cameraItem, {"xPos": xPos, "yPos": yPos, "cpIndex": j, "lblColor": colors[cpGroupIndex%colors.length]});
                componentIndex++
                componentList[componentIndex] = cPoint;
            }
        }
    }

    function clearCPoint()
    {
        var cPointCount = 0;
        var componentIndex = -1;
        for(var i = 0; i<qmlMainWindow.getCameraCount(); i++)
        {
            cPointCount = qmlMainWindow.getCPointCountEx(i);
            for (var j = 0; j < cPointCount; j++)
            {
                componentIndex++;
                var component;
                component = componentList[componentIndex];
                component.destroy();
            }
        }
    }

}

