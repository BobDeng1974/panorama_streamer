import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.2

ToolWindow{
    id: details
    width: 730
    height: 445
    z: 10
    windowState: qsTr("windowed")
    visible: false
    floatingWindowBorderColor: "#1f1f1f"
    property bool   isHover: false
    property int    camIndex1: 0
    property int    camIndex2: 0
    property int    cPointCount
    property string pos
    property var colors: ["Red","Green","Blue","Dark cyan","Magenta","#808000","Dark gray","Dark red","Dark green","Dark blue","Dark magenta","Gray","Light gray"]


    TabView {
        id: firstTab
        x: 10
        y: 15
        width: 350
        height: 380
        onCurrentIndexChanged: {
            camIndex1 = currentIndex;
            createFirstView();
            createSecondView();
            createCPoint();
        }
        ListView {
            id: firstPreview
            x: 0
            anchors.fill: parent

        }

        style: TabViewStyle {
            frameOverlap: 1
            tab: Rectangle {
                color: styleData.selected ? "#4e8d15" :"#171717"
                border.color:  "#4e8d15"
                implicitWidth: 40
                implicitHeight: 30
                radius: 2
                Text {
                    id: text
                    anchors.centerIn: parent
                    text: styleData.title
                    color: styleData.selected ? "white" : "white"
                }
            }
            frame: Rectangle {
                color: "#171717"
                opacity: 0.9
            }
        }

        Rectangle {
            id: firstBackground
            anchors.fill: parent
            opacity: 0.0
        }
    }

//    Item {
//        id: spliter
//        x: (parent.width - width) / 2
//        y: (parent.height - height) / 2
//        width: 2
//        height: parent.height - 20
//        Rectangle{
//            color: "#1f1f1f"
//            x: 1
//            height: parent.height
//            width: 1

//        }
//        Rectangle{
//            color: "#3f3f3f"
//            x: 2
//            height: parent.height
//            width: 1


//        }

//    }

    TabView {
        id: secondTab
        x: parent.width / 2 + 5
        y: 15
        width: 350
        height: 380
        onCurrentIndexChanged: {
            camIndex2 = currentIndex;
            createSecondView();
            createFirstView();
            createCPoint();
        }

        ListView {
            id: secondPreview
            y: 10
            anchors.fill: parent

        }

        style: TabViewStyle {
            frameOverlap: 1
            tab: Rectangle {
                color: styleData.selected ? "#4e8d15" :"#171717"
                border.color:  "#4e8d15"
                implicitWidth: 40
                implicitHeight: 30
                radius: 2
                Text {
                    anchors.centerIn: parent
                    text: styleData.title
                    color: styleData.selected ? "white" : "white"
                }
            }
            frame: Rectangle {
                color: "#171717"
                opacity: 0.9
            }
        }

        Rectangle {
            id: secondBackground
            anchors.fill: parent
            opacity: 0.0

        }

//        Text {
//            id: cPoint
//            anchors.centerIn: parent
//            width: 15
//            height: 15
//            text: qsTr("+")
//            font.pixelSize: 15
//            color: "white"
//        }

//        Rectangle {
//            id: cpLabel
//            x: cPoint.x + 10
//            y: cPoint.y + 10
//            width: 20
//            height: 15
//            //radius: 4
//            border.color: "#000000"
//            color: "#ff0000"
//            border.width: 1
//        }
    }


    function createPreviewWindow()
    {
        var component = Qt.createComponent("MCStitchCameraView.qml");
        if (component.status === Component.Ready) {
            var cameraViewObject = component.createObject(cameraview, {"x": 0, "y": 0});
            if(cameraViewObject === null) {
                console.log(cameraViewObject);
            }else{
                qmlMainWindow.createPreView(cameraViewObject.camView)
            }
        }
        else
            console.log(component.errorString() )
    }

    function getCameraName(){
        var cameraCnt = qmlMainWindow.getCameraCount();
        for(var i = 0; i < cameraCnt; i++){
            firstTab.addTab(i + 1);
            secondTab.addTab (i + 1);
        }

     }

    function createFirstView()
    {
        var component = Qt.createComponent("CameraPreview.qml");
        if (component.status === Component.Ready) {
            var cameraViewObject = component.createObject(firstPreview, {"x": 0, "y": 0});
            if(cameraViewObject === null) {
                console.log(cameraViewObject);
            }else{
                qmlMainWindow.createCameraView(cameraViewObject.camView,firstTab.currentIndex)
            }
        }
        else
            console.log(component.errorString() )
    }

    function createSecondView()
    {
        var component = Qt.createComponent("CameraPreview.qml");
        if (component.status === Component.Ready) {
            var cameraViewObject = component.createObject(secondPreview, {"x": 0, "y": 0});
            if(cameraViewObject === null) {
                console.log(cameraViewObject);
            }else{
                qmlMainWindow.createCameraView(cameraViewObject.camView,secondTab.currentIndex)
            }
        }
        else
            console.log(component.errorString() )
    }

    function clearTab()
    {
        var cameraCnt = qmlMainWindow.getCameraCount();
        for(var i = cameraCnt; i > 0; i--){
            firstTab.removeTab(i - 1);
            secondTab.removeTab (i -1);
        }
    }

    function getCPointCount()
    {
        cPointCount = qmlMainWindow.getCPointCount(camIndex1,camIndex2);
    }

    function createCPoint()
    {
        var curWidth = 350;
        var curHeight = 350;
        var orgWidth = qmlMainWindow.getTempWidth();
        var orgHeight = qmlMainWindow.getTempHeight();
        if(orgHeight/orgWidth > 1)
        {
            curHeight = 350;
            curWidth = curHeight * (orgWidth / orgHeight);
        }
        else
        {
            curWidth = 350;
            curHeight = curWidth * (orgHeight / orgWidth);
        }

        //var curHeight = curWidth * (orgHeight/orgWidth);
        var cPointCount = 0;
        cPointCount = qmlMainWindow.getCPointCount(firstTab.currentIndex,secondTab.currentIndex);
        for (var i = 0; i < cPointCount; i++)
        {
            pos = qmlMainWindow.getCPoint(i, firstTab.currentIndex, secondTab.currentIndex);
            var posList = pos.split(":");
            var xPos1 = (curWidth/orgWidth) * posList[0] + (350-curWidth)/ 2;
            var yPos1 = (curHeight/orgHeight) * posList[1] + (350 - curHeight) / 2;
            var xPos2 = (curWidth/orgWidth) * posList[2] + (350 - curWidth) / 2 ;
            var yPos2 = (curHeight/orgHeight) * posList[3] + (350 - curHeight) / 2;
            var component = Qt.createComponent("ControlPoint.qml");

            if (component.status !== Component.Ready) continue;
            component.createObject(firstPreview, {"xPos": xPos1, "yPos": yPos1, "cpIndex": i, "lblColor": colors[i%colors.length]});
            component.createObject(secondPreview, {"xPos": xPos2, "yPos": yPos2,"cpIndex": i, "lblColor": colors[i%colors.length]});

        }
    }

}
