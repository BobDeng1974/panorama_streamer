import QtQuick 2.5

Item{
    width:  root.width
    height: 30

    property string          title: "title"
    property string          titleTextColor
    property bool            isHovered: false
    property bool            isSelected: false
    property int             clickCnt: 0
    property string          mix: ""
    property bool            checkSelect: false
    property var             selectCamera
    property int             stereoLeftCnt: 0
    property int             stereoRightCnt: 0
    property string          stereoLColor
    property string          stereoRColor
    property bool            checkStereoState: true
    property int             liveStereoType: 0
    property bool            isClicked: checkSelect
    property bool            isStereoLeft: false
    property bool            isStereoRight: false
    property int             spacing: 50

    Rectangle {
        id: hoveredRectangle
        x: 20
        y: -5
        width: root.width - 50
        height: parent.height + 10
        color: "#1f1f1f"
        visible: false

    }
    Rectangle {
        id: selectRectangle
        x: 20
        y: -5
        width: root.width - 50
        height: parent.height + 10
        color: "#0e3e64"
        visible: checkSelect
    }

    Image {
        id: iconImage
        width: 30
        height: 30
        source: "../../resources/icon_camera_small.png"
        x: 50
        y: 0
        z: 1
        fillMode: Image.PreserveAspectFit
    }

    Text {
        id: titleText
        x: iconImage.x + iconImage.width + 20
        y: (parent.height - height) / 2
        z: 1
        color: titleTextColor
        text: qsTr(title)
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        font.pixelSize: 15
    }

    Item{
        id: stereoItem
        x: parent.width * 0.7
        y: (parent.height  - height) / 2
        anchors.right: stereoLeftItem.left
        anchors.rightMargin: spacing / 3
        z: 5
        width: 30
        height: 30
        visible: checkSelect

        Image {
            id: stereoImage
            width: 25
            height: 25
            source: "../../resources/ico_camera.png"
            x:(parent.width - width) / 2
            y: (parent.height - height) / 2
            fillMode: Image.PreserveAspectFit
        }
    }

    Item{
        id: stereoLeftItem
        x: parent.width * 0.85
        y: (parent.height - height) / 2
        anchors.right: stereoRightItem.left
        anchors.rightMargin: 50
        width: 40
        height: 30
        z: 1
        visible: checkSelect

        Text{
            id: stereoLeft
            text: "Left"
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            z: 1
            color: stereoLColor
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 15
            visible: checkStereoState
        }

        Text {
            id: otherLeft
            text: "Left"
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            z: 1
            color: "#8a8a8a"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 15
            visible: !checkStereoState
        }

        MouseArea {
            id: stereoLeftMouseArea
            z: 1
            anchors.fill: parent
            enabled: checkStereoState
            hoverEnabled: true
            onHoveredChanged: {

                isHovered = !isHovered
                if(isHovered) {
                    hoveredRectangle.visible = true
                    //cursorShape = Qt.PointingHandCursor
                }
                else {
                    hoveredRectangle.visible = false
                }
            }
            onClicked: {
               stereoLeftCnt=stereoLeftCnt + 1
                isStereoLeft = !isStereoLeft
                if (isStereoLeft)
                {
                    if(isStereoRight)
                    {
                        stereoLColor = "#ffffff"
                        selectCamera = camList.get(index);
                        selectCamera.stereoType = 3;
                        //qmlMainWindow.setTempStereoType(index, stereoType)
                    }
                    else
                    {
                        //stereoLeft.color ="#ffffff";
                        stereoLColor = "#ffffff"
                        selectCamera = camList.get(index);
                        selectCamera.stereoType = 1;
                        //qmlMainWindow.setTempStereoType(index, stereoType)
                    }
                }
                else
                {
                    if(isStereoRight)
                    {
                        //stereoLeft.color = "#8a8a8a";
                        stereoLColor = "#8a8a8a"
                        stereoType = 2;
                        selectCamera = camList.get(index);
                        selectCamera.stereoType = 2;
                        //qmlMainWindow.setTempStereoType(index, stereoType)
                    }
                    else
                    {
                        stereoType = 0;
                        selectCamera = camList.get(index);
                        selectCamera.stereoType = 0;
                        //stereoLeft.color = "#8a8a8a";
                        stereoLColor = "#8a8a8a"
                        //qmlMainWindow.setTempStereoType(index, stereoType)
                    }
                }
            }
        }
    }

    Item{
        id: stereoRightItem
        anchors.right: settingItem.left
        anchors.rightMargin: spacing / 2.5
        width: 40
        height: 30
        z: 1
        visible: checkSelect

        Text{
            id: stereoRight
            text: "Right"
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            z: 1
            color: stereoRColor
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 15
            visible: checkStereoState

        }

        Text {
            id: otherRight
            text: "Right"
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            z: 1
            color: "#8a8a8a"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 15
            visible: !checkStereoState
        }
        MouseArea {
            id: stereoRightMouseArea
            z: 1
            anchors.fill: parent
            enabled: checkStereoState
            hoverEnabled: true
            onHoveredChanged: {
                isHovered = !isHovered
                if(isHovered) {
                    hoveredRectangle.visible = true
                    //cursorShape = Qt.PointingHandCursor
                }
                else {
                    hoveredRectangle.visible = false
                }
            }
            onClicked: {
                stereoRightCnt=stereoRightCnt + 1
                isStereoRight = !isStereoRight
                if (isStereoRight)
                {
                    if(isStereoLeft)
                    {
                      // stereoRight.color = "#ffffff";
                        stereoRColor = "#ffffff"
                        stereoType = 3;
                        selectCamera = camList.get(index);
                        selectCamera.stereoType = 3;
                        //qmlMainWindow.setTempStereoType(index, stereoType)
                    }
                    else
                    {
                        //stereoRight.color ="#ffffff";
                        stereoRColor = "#ffffff"
                        stereoType = 2;
                        selectCamera = camList.get(index);
                        selectCamera.stereoType = 2;
                        //qmlMainWindow.setTempStereoType(index, stereoType);
                    }
                }
                else
                {
                    if(isStereoLeft)
                    {
                        //stereoRight.color = "#8a8a8a";
                        stereoRColor = "#8a8a8a"
                        stereoType = 1;
                        selectCamera = camList.get(index);
                        selectCamera.stereoType = 1;
                        //qmlMainWindow.setTempStereoType(index, stereoType);
                    }
                    else
                    {
                        stereoType = 0;
                        selectCamera = camList.get(index);
                        selectCamera.stereoType = 0;
                        stereoRColor = "#8a8a8a"
                        //qmlMainWindow.setTempStereoType(index, stereoType);
                    }
                }
            }
        }
    }

    Item{
        id: settingItem
        anchors.right: parent.right
        anchors.rightMargin: 60
        z: 5
        width: 40
        height: 30
        visible: checkSelect

        Rectangle {
            id: settingHoveredRectangle
            anchors.fill: parent
            color:"#c75050"
            z: 0
            visible: false
        }

        Image {
            id: settingImage
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            source: "../../resources/setting.png"
            z: 1
            fillMode: Image.PreserveAspectFit


        }

        /*MouseArea {
            id: settingMouseArea
            z: 0
            anchors.fill: parent
            onClicked: {
                var curIndex = index;
              // if (bFlag || bCamFlag )  return;
                globalSettingbox.state = "collapsed";
                liveTempCamSetting.y = titleRectangle.height +  (index + 1)* 50
                if(liveTempCamSetting.state === "expanded"){
                    liveTempCamSetting.state = "collapsed";
                    //bCamFlag = false;

                }else if(liveTempCamSetting.state === "collapsed"){
                    liveTempCamSetting.state = "expanded";
                     getCameraSettings(curIndex);
                     setCameraSettings(curIndex);
                    //bCamFlag = true;

                }
            }
        }*/
    }




    MouseArea {
        id: mouseArea
        x: 0
        z: 2
        width: parent.width * 0.6
        height: parent.height
        hoverEnabled: true
        onHoveredChanged: {
            cursorShape = Qt.PointingHandCursor
            isHovered = !isHovered
            if(isHovered) {
                hoveredRectangle.visible = true
            }
            else {
                hoveredRectangle.visible = false
            }
        }
        onClicked:{
            globalSettingbox.state = "collapsed";
            liveTempCamSetting.state = "collapsed";
            isClicked = !isClicked;
            if(isClicked){
                titleText.color = "#ffffff"
                selectCamera = camList.get(index);
                settingItem.visible = true
                selectCamera.isSelect = true;
                selectRectangle.visible = true;
            }else{
                selectCamera = camList.get(index);
                selectCamera.isSelect = false;
                selectCamera.stereoType = 0;
                isStereoLeft = false;
                isStereoRight = false;
                stereoLColor = "#8a8a8a"
                stereoRColor = "#8a8a8a";
                titleText.color = "#8a8a8a";
                selectRectangle.visible = false;
                settingItem.visible = false;
            }
        }
    }
}
