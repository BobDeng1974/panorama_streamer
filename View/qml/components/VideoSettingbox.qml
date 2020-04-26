import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls.Styles.Flat 1.0 as Flat
import QtQuick.Extras 1.4
import QtQuick.Extras.Private 1.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import "../controls"

Item {
    width : 1030
    height: 684
    opacity: 1
    z: 3
    property int        hoveredType: 0
    property bool       isHovered : false
    property bool       isSelected: false
    property int        leftMargin: 20
    property int        rightMargin: 20
    property int        spacing: 20
    property color      textColor: "#ffffff"
    property color      comboTextColor: "#7899ff"
    property color      comboBackColor: "#343434"
    property color      spliterColor: "#555555"
    property int        lblFont: 14
    property int        groupFont: 16
    property int        nItemCount: 21
    property int        itemHeight:30
    property bool       isHoveredCheck: false
    property bool       isHoveredUnCheck: false
    property bool       isHoveredMore: false
    property bool       bFlag
    property bool       bCamFlag
    property bool       flag
    property string     videoPath: "EmptySlot"
    property int        videoIndex: 0
    property bool       videoLock: false
    property int        panorama_Mono: 0
    property int        panorama_LeftEye: 1
    property int        panorama_RightEye: 2
    property int        panorama_BothEye: 3
    property int        mixedChannel: 0
    property int        leftChannel: 1
    property int        rightChannel: 2
    property int        noAudio: 3
    property string     infoStr: ""

    Rectangle {
        id: titleRectangle
        x: 0
        y: 0
        width: parent.width
        height: 48
        color: "#1f1f1f"
        z: 1
        opacity: 1

        Text {
            id: titleText
            x: 30
            y: (parent.height - height) / 2
            z: 3
            color: "#ffffff"
            text: qsTr("Video Template")
            font.bold: false
            font.pixelSize: 20
        }
        Item {
            id: groupControl
            x: root.width - groupControl.width
            width: 230
            height: 48
            z: 1
            Item {
                id: checkItem
                anchors.right: uncheckItem.left
                width: 68
                height: 48
                z: 2
                Rectangle {
                    id: checkHoveredRectangle
                    width: parent.width
                    height: 48
                    color: "#0e3e64"
                    visible: false
                }

                Image {
                    x: (parent.width - width) / 2
                    y: (parent.height - height) / 2
                    z: 2
                    fillMode: Image.PreserveAspectFit
                    source: "../../resources/check.png"
                }
                MouseArea {
                    z: 2
                    anchors.fill: parent
                    hoverEnabled: true
                    onHoveredChanged: {
                        if (bFlag) return;
                        isHoveredCheck = !isHoveredCheck
                        if(isHoveredCheck)
                            checkHoveredRectangle.visible = true
                        else
                            checkHoveredRectangle.visible = false

                    }
                    onClicked: {
                        videoLock = true;
                        for(var i=0;i<videoList.count;i++){
                             var  item = videoList.get(i);
                            if(!item.isSelect)  continue;
                            else
                            {
                                videoLock = false;
                                break;
                            }

                        }

                        if(videoLock)
                            console.log("Camera is not selected")
                        else
                        {
                            recent.state = "collapsed"
                            videoSettingbox.state = "collapsed"
                            openCameraViews();
                            busyTimer.restart();
                        }
                    }
                }

            }

            Item {
                id:　uncheckItem
                anchors.right: plusItem.left
                width:　68
                height: 48
                z: 2
                Rectangle {
                    id: unCheckHoveredRectangle
                    width: parent.width
                    height: 48
                    color: "#c75050"
                    visible: false
                }
                Image {
                    x: (parent.width - width) / 2
                    y: (parent.heigth - height) / 2
                    anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectFit
                    source: "../../resources/uncheck.png"
                }
                MouseArea {
                    z: 2
                    anchors.fill: parent
                    hoverEnabled: true
                    onHoveredChanged: {
                        if (bFlag) return;
                        isHoveredUnCheck = !isHoveredUnCheck
                        if(isHoveredUnCheck)
                            unCheckHoveredRectangle.visible = true
                        else
                            unCheckHoveredRectangle.visible = false
                    }
                     onClicked: {
                         cameraSettingsbox.state = "collapsed";
                         videoSettingbox.state = "collapsed";
                         qmlMainWindow.resetTempGlobalSettings();
                         videoList.clear();
                         for (var i = 0; i < 12; i ++)
                         {
                             videoList.append({"titleText": "Empty Slot",
                                                  "leftColor": "#ffffff",
                                                  "rightColor": "#8a8a8a","isSelect": false,
                                                  "selectStereoLeft": false,"selectStereoRight": false,
                                                  "selectAudioLeft": true,"selectAudioRight": false,
                                                  "audioType": 1,"stereoType": 0,"titleColor": "#8a8a8a",
                                                  "stereoLeftColor": "#8a8a8a",
                                                  "stereoRightColor": "#8a8a8a","stereoState": true});
                         }
                         if(isTemplate) return;
                         else {
                             toolbox.clearState();
                             onForward();
                         }

                     }
                }
            }

            Item {
                id: plusItem
                anchors.right: moreItem.left
                width: 48
                height: 48
                z: 2
                Rectangle {
                    id: plusHoveredRectangle
                    width: parent.width
                    height: 48
                    color: "#353535"
                    visible: false
                }

                Image {
                    x: (parent.width - width) / 2
                    y: (parent.height - height) / 2
                    z: 2
                    fillMode: Image.PreserveAspectFit
                    source: "../../resources/btn_plus.png"
                }
                MouseArea {
                    z: 2
                    anchors.fill: parent
                    hoverEnabled: true
                    onHoveredChanged: {
                        isHoveredCheck = !isHoveredCheck
                        if(isHoveredCheck)
                            plusHoveredRectangle.visible = true
                        else
                            plusHoveredRectangle.visible = false

                    }
                    onClicked: {
                        videoDialog.open();
                    }
                }
            }

            Item {
                id:　moreItem
                anchors.right: parent.right
                width:　48
                height: 48
                z: 2
                Rectangle {
                    id: moreHoveredRectangle
                    width: parent.width
                    height: 48
                    color: "#353535"
                    visible: false
                }
                Image {
                    x: (parent.width - width) / 2
                    y: (parent.heigth - height) / 2
                    anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectFit
                    source: "../../resources/more_control.png"
                }

                MouseArea {
                    z: 2
                    anchors.fill: parent
                    hoverEnabled: true
                    onHoveredChanged: {
                        isHoveredMore = !isHoveredMore
                        if(isHoveredMore)
                           moreHoveredRectangle.visible = true
                        else
                            moreHoveredRectangle.visible = false
                    }
                     onClicked:
                     {
                         if(infoStr === "") return;
                         globalSettings();
                     }
                }
            }
        }
    }

    Rectangle {
        id: backgroundRectangle
        width: parent.width
        height: parent.height
        color: "#000000"
        opacity: 1.0
    }

    ListView {
        id: videoListView
        x: 0
        y: 80
        width: parent.width
        height: 50 * count
        spacing: 0
        model: ListModel {
            id: videoList
            ListElement {
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                leftColor: "#ffffff"
                rightColor: "#8a8a8a"
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                isSelect: false
                audioType: 1
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
                selectAudioLeft: true
                selectAudioRight: false
            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                leftColor: "#ffffff"
                rightColor: "#8a8a8a"
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                isSelect: false
                audioType: 1
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
                selectAudioLeft: true
                selectAudioRight: false
            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                leftColor: "#ffffff"
                rightColor: "#8a8a8a"
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                audioType: 1
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
                selectAudioLeft: true
                selectAudioRight: false
            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                leftColor: "#ffffff"
                rightColor: "#8a8a8a"
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                isSelect: false
                audioType: 1
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
                selectAudioLeft: true
                selectAudioRight: false

            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                leftColor: "#ffffff"
                rightColor: "#8a8a8a"
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                isSelect: false
                audioType: 1
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
                selectAudioLeft: true
                selectAudioRight: false
            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                leftColor: "#ffffff"
                rightColor: "#8a8a8a"
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                isSelect: false
                audioType: 1
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
                selectAudioLeft: true
                selectAudioRight: false
            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                leftColor: "#ffffff"
                rightColor: "#8a8a8a"
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                isSelect: false
                audioType: 1
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
                selectAudioLeft: true
                selectAudioRight: false
            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                leftColor: "#ffffff"
                rightColor: "#8a8a8a"
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                isSelect: false
                audioType: 1
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
                selectAudioLeft: true
                selectAudioRight: false
            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                leftColor: "#ffffff"
                rightColor: "#8a8a8a"
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                isSelect: false
                audioType: 1
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
                selectAudioLeft: true
                selectAudioRight: false
            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                leftColor: "#ffffff"
                rightColor: "#8a8a8a"
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                isSelect: false
                audioType: 1
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
                selectAudioLeft: true
                selectAudioRight: false
            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                leftColor: "#ffffff"
                rightColor: "#8a8a8a"
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                isSelect: false
                audioType: 1
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
                selectAudioLeft: true
                selectAudioRight: false
            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                leftColor: "#ffffff"
                rightColor: "#8a8a8a"
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                isSelect: false
                audioType: 1
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
                selectAudioLeft: true
                selectAudioRight: false
            }
        }
        delegate: Item {
            x: 5
            width: parent.width
            height: 50
            Row {
                VideoListitem {
                    title: titleText
                    titleTextColor: titleColor
                    leftTextColor: leftColor
                    rightTextColor: rightColor
                    stereoLColor: stereoLeftColor
                    stereoRColor: stereoRightColor
                    checkSelect: isSelect
                    tempAudioType: audioType
                    tempStereoType: stereoType
                    checkStereoState: stereoState
                    isStereoLeft: selectStereoLeft
                    isStereoRight: selectStereoRight
                    isAudioLeft: selectAudioLeft
                    isAudioRight: selectAudioRight
                }
            }
        }
    }


    CameraSettingbox {
        id: cameraSettingsbox
        anchors.right: parent.right
        anchors.rightMargin: 100
        width: 250
        height: 0
        z: 1
        state: "collapsed"

           states: [
               State {
                   name: "collapsed"
                   PropertyChanges { target: cameraSettingsbox; height: 0}
                   PropertyChanges { target:  cameraSettingsbox;width: 0

                   }
               },
               State {
                   name: "expanded"
                   PropertyChanges { target: cameraSettingsbox; height: 300}
                   PropertyChanges {target: cameraSettingsbox;width: 200}
               }
           ]

           transitions: [
               Transition {
                   NumberAnimation { target: cameraSettingsbox; property: "height"; duration: 300 }
                   NumberAnimation { target: cameraSettingsbox;property: "width";duration: 300}
               }
           ]

    }

    VideoGlobalSettings {
        id: globalSettingbox
        height: root.height - 32
        width: 350
        z: 1
        state: "collapsed"

           states: [
               State {
                   name: "collapsed"
                   PropertyChanges { target:  globalSettingbox;x: root.width}
               },
               State {
                   name: "expanded"
                   PropertyChanges { target:  globalSettingbox;x: root.width - 350}
               }
           ]

           transitions: [
               Transition {
                   NumberAnimation { target: globalSettingbox;property: "x";duration: 200 }
               }
           ]

    }

    FileDialog {
        id: videoDialog
        title: "Open Video file"
        nameFilters: [ "Video file (*.mp4)", "All files (*)" ]
        selectMultiple: true
        onSelectionAccepted: {
            for (var i = 0; i < fileUrls.length; ++i)
            {
                videoIndex = i;
                checkExistVideo(fileUrls[i].toString().substring(8));
            }
            getInfoStr();

        }
    }

    FileDialog {
        id: singleDlg
        title: "Open Video file"
        nameFilters: [ "Video file (*.mp4)", "All files (*)" ]
        selectMultiple: true
        onSelectionAccepted: {
          videoPath = fileUrl.toString().substring(8); // Remove "file:///" prefix
          checkExistVideo(videoPath);
        }
    }

    Timer {
        id: busyTimer
        interval: 100
        running: false
        repeat: false
        onTriggered: checkSelectVideo()
    }

    function setVideoFile(videoPath){
        videoList.set(videoIndex,{"titleColor": "#ffffff"});
        videoList.set(videoIndex,{"isSelect": true});
        videoList.set(videoIndex,{"titleText": videoPath});
        //getInfoStr();
    }

    function getInfoStr()
    {
        var videoName = videoList.get(0).titleText;
        infoStr = qmlMainWindow.getSlotInfo(videoName,"",2);
    }

    function checkSelectVideo(){
        curMode = 1;
        clearMainSetting();
        toolbox.showStartIamge();
        clearCameras();
        qmlMainWindow.initTemplateVideoObject()
        for(var i=0;i<videoList.count;i++){
            var item = videoList.get(i);
            if(!item.isSelect || item.titleText === "Empty slot")  continue;
            qmlMainWindow.sendVideoPath(i,item.titleText);
            qmlMainWindow.setTempStereoType(i,item.stereoType);
            qmlMainWindow.setTempAudioSettings(i,item.audioType);
        }
        qmlMainWindow.openTemplateVideoIniFile();
        clearCameraSettings();
        liveView.createTemplateVideoWindow()
        sphericalView.createSphericalCameraWindow();
        interactiveView.createInteractiveCameraWindow();
        qmlMainWindow.initProject();
        qmlMainWindow.openProject()
    }


    function globalSettings(){
        globalSettingbox.changeVideoMode();
        globalSettingbox.getGlobalValues();
        if(globalSettingbox.state == "expanded"){
            bFlag = false;
            globalSettingbox.state = "collapsed";
        }else if(globalSettingbox.state == "collapsed"){
            bFlag = true;
            globalSettingbox.state = "expanded";
        }
    }

    function setCameraSettings(index){
        cameraSettingsbox.setCameraValues(index);
    }

    function getCameraSettings(index)
    {
        cameraSettingsbox.getCameraValues(index);
    }

    function checkExistVideo(videoPath){
        flag = false;
        for(var i=0;i<videoList.count;i++){
             var  item = videoList.get(i);
            if(item.titleText === videoPath) {
                flag = true;
                break;
            }
        }
        if(flag) return;
        setVideoFile(videoPath)
    }

    function setVideoPanoramaType()
    {
        if (globalStereoState)
        {
            for(var i = 0;i<12;i++)
            {
                videoList.set(i,{"stereoState": true})
            }
        }
        else
        {
            for(var j = 0;j<12;j++)
            {
                videoList.set(j,{"stereoLeftColor": "#8a8a8a"})
                videoList.set(j,{"stereoRightColor": "#8a8a8a"})
                videoList.set(j,{"stereoState": false})
                videoList.set(j,{"stereoType": 0})
                //qmlMainWindow.setTempStereoType(j,0);

            }

        }
    }

    function setVideoSlot()
    {
        for (var i = 0; i < qmlMainWindow.getVideoCount(); ++i)
        {
            videoIndex = i;
            videoList.set(i,{"titleColor": "#ffffff"});
            videoList.set(i,{"isSelect": true});
            videoList.set(i,{"titleText": qmlMainWindow.getVideoPath(i)})
            setStereoSettings();
            setAudioSettings();
        }
        getInfoStr();
    }

    function setChangeTitle()
    {
        titleText.text = "Video Setting";
    }

    function setStereoSettings()
    {
        var stereoType =  qmlMainWindow.getTempStereoType(videoIndex)
        switch(stereoType){
        case panorama_Mono:
            videoList.set(videoIndex,{"stereoLeftColor": "#8a8a8a"})
            videoList.set(videoIndex,{"stereoRightColor": "#8a8a8a"})
            videoList.set(videoIndex,{"selectStereoLeft": false})
            videoList.set(videoIndex ,{"stereoState": true});
            videoList.set(videoIndex,{"selectStereoRight": false})
            videoList.set(videoIndex,{"stereoType": 0});
            break;
        case panorama_LeftEye:
            videoList.set(videoIndex,{"stereoLeftColor": "#ffffff"})
            videoList.set(videoIndex,{"stereoRightColor": "#8a8a8a"})
            videoList.set(videoIndex ,{"stereoState": true});
            videoList.set(videoIndex,{"selectStereoLeft": true})
            videoList.set(videoIndex,{"selectStereoRight": false})
            videoList.set(videoIndex,{"stereoType": 1});
            break;
        case panorama_RightEye:
            videoList.set(videoIndex,{"stereoLeftColor": "#8a8a8a"})
            videoList.set(videoIndex,{"stereoRightColor": "#ffffff"})
            videoList.set(videoIndex ,{"stereoState": true});
            videoList.set(videoIndex,{"selectStereoLeft": false})
            videoList.set(videoIndex,{"selectStereoRight": true})
            videoList.set(videoIndex,{"stereoType": 2});
            break;
        case panorama_BothEye:
            videoList.set(videoIndex,{"stereoLeftColor": "#ffffff"})
            videoList.set(videoIndex,{"stereoRightColor": "#ffffff"})
            videoList.set(videoIndex ,{"stereoState": true});
            videoList.set(videoIndex,{"selectStereoLeft": true})
            videoList.set(videoIndex,{"selectStereoRight": true})
            videoList.set(videoIndex,{"stereoType": 3});
            break;
        default:
            break;
        }
    }

    function setAudioSettings()
    {
        var audioType =  qmlMainWindow.getTempAudioSettings(videoIndex)
        switch(audioType){
        case mixedChannel:
            videoList.set(videoIndex,{"leftColor": "#ffffff"})
            videoList.set(videoIndex,{"rightColor": "#ffffff"})
            videoList.set(videoIndex,{"selectAudioLeft": true})
            videoList.set(videoIndex,{"selectAudioRight": true})
            videoList.set(videoIndex ,{"audioType": 0});
            break;
        case leftChannel:
            videoList.set(videoIndex,{"leftColor": "#ffffff"})
            videoList.set(videoIndex,{"rightColor": "#8a8a8a"})
            videoList.set(videoIndex,{"selectAudioLeft": true})
            videoList.set(videoIndex,{"selectAudioRight": false})
            videoList.set(videoIndex ,{"audioType": 1});
            break;
        case rightChannel:
            videoList.set(videoIndex,{"leftColor": "#8a8a8a"})
            videoList.set(videoIndex,{"rightColor": "#ffffff"})
            videoList.set(videoIndex,{"selectAudioLeft": false})
            videoList.set(videoIndex,{"selectAudioRight": true})
            videoList.set(videoIndex ,{"audioType": 2});
            break;
        case noAudio:
            videoList.set(videoIndex,{"leftColor": "#8a8a8a"})
            videoList.set(videoIndex,{"rightColor": "#8a8a8a"})
            videoList.set(videoIndex,{"selectAudioLeft": false})
            videoList.set(videoIndex,{"selectAudioRight": false})
            videoList.set(videoIndex ,{"audioType": 3});
            break;
        default:
            break;
        }
    }

    function getStereoState()
    {
        for(var i = 0;i<12;i++){
            var stereoType =  qmlMainWindow.getTempStereoType(i);
            if(stereoType === 0)
            {
                root.globalStereoState = false;
                continue;
            }
            else
            {
                root.globalStereoState = true;
                break;
            }
        }
    }
}
