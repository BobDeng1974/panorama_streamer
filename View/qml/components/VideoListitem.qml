import QtQuick 2.5
import QtQuick.Dialogs 1.2
Item{
    id: videoListitem
    width: root.width
    height: 30
    property bool            checkSelect: false
    property string          leftTextColor
    property string          rightTextColor
    property string          title: "title"
    property string          titleTextColor
    property bool            isHovered: false
    property bool            isSelected: false
    property bool            isSettingHover: false
    property var             selectVideo
    property int             tempAudioType: 0
    property string          stereoLColor
    property string          stereoRColor
    property bool            checkStereoState: true
    property int             tempStereoType: 0
    property bool            isClicked: true
    property bool            isStereoLeft: false
    property bool            isStereoRight: false
    property bool            isAudioLeft: true
    property bool            isAudioRight: false
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
    Item {
        id: videoItem
        x: 20
        y: (parent.height - height) / 2
        z: 5
        width: 30
        height: 30

        Image {
            id: videoImage
            width: 30
            height: 30
            source: "../../resources/icon_video_small.png"
            x:(parent.width - width) / 2
            y: (parent.height - height) / 2
            z: 1
            fillMode: Image.PreserveAspectFit
        }

        MouseArea {
            id: videoArea
            anchors.fill: parent
            z: 2
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
            onClicked: {
                videoIndex = index;
                openVideoFile();
            }
        }
    }

    Text {
        id: titleText
        x: videoItem.x + videoItem.width + 20
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
        anchors.rightMargin: spacing
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
                if(titleText.text === "Empty Slot") return;
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
                isStereoLeft = !isStereoLeft
                if (isStereoLeft)
                {
                    if(isStereoRight)
                    {
                        stereoLColor = "#ffffff"
                        selectVideo = videoList.get(index);
                        selectVideo.stereoType = 3;
                    }
                    else
                    {
                        stereoLColor = "#ffffff"
                        selectVideo = videoList.get(index);
                        selectVideo.stereoType = 1;
                    }
                }
                else
                {
                    if(isStereoRight)
                    {
                        stereoLColor = "#8a8a8a"
                        selectVideo = videoList.get(index);
                        selectVideo.stereoType = 2;
                    }
                    else
                    {
                        selectVideo = videoList.get(index);
                        selectVideo.stereoType = 0;
                        stereoLColor = "#8a8a8a"
                    }
                }
            }
        }
    }

    Item{
        id: stereoRightItem
        anchors.right: audioItem.left
        anchors.rightMargin: spacing
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
                if(titleText.text === "Empty Slot") return;
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
                isStereoRight = !isStereoRight
                if (isStereoRight)
                {
                    if(isStereoLeft)
                    {
                        stereoRColor = "#ffffff"
                        selectVideo = videoList.get(index);
                        selectVideo.stereoType = 3;
                    }
                    else
                    {
                        stereoRColor = "#ffffff"
                        selectVideo = videoList.get(index);
                        selectVideo.stereoType = 2;
                    }
                }
                else
                {
                    if(isStereoLeft)
                    {
                        stereoRColor = "#8a8a8a"
                        selectVideo = videoList.get(index);
                        selectVideo.stereoType = 1;
                    }
                    else
                    {
                        selectVideo = videoList.get(index);
                        selectVideo.stereoType = 0;
                        stereoRColor = "#8a8a8a"
                    }
                }
            }
        }
    }

    Item{
        id: audioItem
        x: parent.width * 0.7
        y: (parent.height  - height) / 2
        anchors.right: leftItem.left
        anchors.rightMargin: spacing / 3
        z: 5
        width: 30
        height: 30
        visible: checkSelect

        Image {
            id: audioImage
            width: 25
            height: 25
            source: "../../resources/volume.png"
            x:(parent.width - width) / 2
            y: (parent.height - height) / 2
            fillMode: Image.PreserveAspectFit
        }
    }

    Item{
        id: leftItem
        x: parent.width * 0.85
        y: (parent.height - height) / 2
        anchors.right: rightItem.left
        anchors.rightMargin: spacing
        width: 40
        height: 30
        z: 1
        visible: checkSelect


        Text{
            id: leftText
            text: "Left"
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            z: 1
            color: leftTextColor
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 15

        }
        MouseArea {
            id: leftMouseArea
            z: 1
            anchors.fill: parent
            hoverEnabled: true
            onHoveredChanged: {
                if(titleText.text === "Empty Slot") return;
                //cursorShape = Qt.PointingHandCursor
                isHovered = !isHovered
                if(isHovered) {
                    hoveredRectangle.visible = true
                }
                else {
                    hoveredRectangle.visible = false
                }
            }
            onClicked: {
                isAudioLeft = !isAudioLeft
                if (isAudioLeft)
                {
                    if(isAudioRight)
                    {
                        leftText.color = "#ffffff"
                        selectVideo = videoList.get(index);
                        selectVideo.audioType = 0;
                    }
                    else
                    {
                        leftText.color = "#ffffff"
                        selectVideo = videoList.get(index);
                        selectVideo.audioType = 1;
                    }
                }
                else
                {
                    if(isAudioRight)
                    {
                        leftText.color = "#8a8a8a"
                        selectVideo = videoList.get(index);
                        selectVideo.audioType = 2;
                    }
                    else
                    {
                        selectVideo = videoList.get(index);
                        selectVideo.audioType = 3;
                        leftText.color = "#8a8a8a"
                    }
                }
            }


        }
    }
    Item{
        id: rightItem
        anchors.right: settingItem.left
        anchors.rightMargin: spacing / 2.5
        width: 40
        height: 30
        z: 1
        visible: checkSelect

        Text{
            id: rightText
            text: "Right"
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            z: 1
            color: rightTextColor
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 15

        }
        MouseArea {
            id: rightMouseArea
            z: 1
            anchors.fill: parent
            hoverEnabled: true
            onHoveredChanged: {
                if(titleText.text === "Empty Slot") return;
                //cursorShape = Qt.PointingHandCursor
                isHovered = !isHovered
                if(isHovered) {
                    hoveredRectangle.visible = true
                }
                else {
                    hoveredRectangle.visible = false
                }
            }
            onClicked: {
                isAudioRight = !isAudioRight
                if (isAudioRight)
                {
                    if(isAudioLeft)
                    {
                        rightText.color = "#ffffff"
                        selectVideo = videoList.get(index);
                        selectVideo.audioType = 0;
                    }
                    else
                    {
                        rightText.color = "#ffffff"
                        selectVideo = videoList.get(index);
                        selectVideo.audioType = 2;
                    }
                }
                else
                {
                    if(isAudioLeft)
                    {
                        rightText.color = "#8a8a8a"
                        selectVideo = videoList.get(index);
                        selectVideo.audioType = 1;
                    }
                    else
                    {
                        selectVideo = videoList.get(index);
                        selectVideo.audioType = 3;
                        rightText.color = "#8a8a8a"
                    }
                }
            }

        }
    }

    Rectangle {
        id: spliterRectangle
        width: 2
        height: parent.height
        color: "#8a8a8a"
        anchors.left: stereoRightItem.right
        anchors.leftMargin: 25
        visible: checkSelect
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
            hoverEnabled: true
            onHoveredChanged: {
//                if (bFlag || bCamFlag || videoLock) return;
//                cursorShape = Qt.PointingHandCursor
//                isSettingHover = !isSettingHover
//                if(isSettingHover) {
//                    settingHoveredRectangle.visible = true
//                }
//                else {
//                   settingHoveredRectangle.visible = false
//                }
            }
            onClicked: {
                globalSettingbox.state = "collapsed";
                var curIndex = index;
                //if (bFlag || bCamFlag )  return;
                cameraSettingsbox.y = titleRectangle.height +  (index + 1)* 50
                if(cameraSettingsbox.state == "expanded"){
                    cameraSettingsbox.state = "collapsed";
                    bCamFlag = false;

                }else if(cameraSettingsbox.state == "collapsed"){
                    cameraSettingsbox.state = "expanded";
                     getCameraSettings(curIndex);
                     setCameraSettings(curIndex);
                    bCamFlag = true;

                }


            }

        }*/
    }

    MouseArea {
        id: mouseArea
        z: 0
        width: parent.width * 0.3
        height: parent.height
        hoverEnabled: true
        onHoveredChanged: {
           // if (bFlag || bCamFlag) return;
            if(titleText.text === "Empty Slot") return;
            isHovered = !isHovered
            if(isHovered) {
                hoveredRectangle.visible = true
            }
            else {
                hoveredRectangle.visible = false
            }
        }
        onClicked:{
            cameraSettingsbox.state = "collapsed"
            globalSettingbox.state = "collapsed";
            isClicked = !isClicked;
            if(titleText.text === "Empty Slot") return;
            if(isClicked){
                selectVideo = videoList.get(index);
                selectVideo.isSelect = true;
                titleText.color = "#ffffff"
                selectRectangle.visible = true
                settingItem.visible = true;
                showAudioSettings();
                showStereoSettings();
            }else{
                selectVideo = videoList.get(index);
                selectVideo.isSelect = false;
                checkSelect = false
                clearVideoSettings()
            }
        }
    }

    function clearVideoSettings(){
        selectRectangle.visible = false
        titleText.color = "#8a8a8a"
        settingItem.visible = false;
        audioItem.visible = false;
        leftItem.visible = false;
        rightItem.visible = false;
        stereoItem.visible = false;
        stereoLeftItem.visible = false;
        stereoRightItem.visible = false;
        spliterRectangle.visible = false;
    }
    function openVideoFile(){
        singleDlg.open();
    }

    function showStereoSettings()
    {
        stereoItem.visible = true;
        stereoLeftItem.visible = true;
        stereoRightItem.visible = true;
        spliterRectangle.visible = true;
    }

    function showAudioSettings()
    {
        audioItem.visible = true;
        leftItem.visible = true;
        rightItem.visible = true;
    }

}
