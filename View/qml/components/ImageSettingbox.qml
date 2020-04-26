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
    id:imageSettingsBox
    width : 1030
    height: 684
    opacity: 1
    z: 3
    property int         hoveredType: 0
    property bool        isHovered : false
    property bool        isSelected: false
    property int         leftMargin: 20
    property int         rightMargin: 20
    property int         spacing: 20
    property color       textColor: "#ffffff"
    property color       comboTextColor: "#7899ff"
    property color       comboBackColor: "#343434"
    property color       spliterColor: "#555555"
    property int         lblFont: 14
    property int         groupFont: 16
    property int         nItemCount: 21
    property int         itemHeight:30
    property bool        isHoveredCheck: false
    property bool        isHoveredUnCheck: false
    property bool        bFlag
    property bool        bCamFlag
    property var         imageUrl
    property bool        flag
    property var         item
    property string      pathList
    //property bool        globalStereoState: true
    property string      imagePath: "EmptySlot"
    property int         imageIndex: -1
    property bool        imageLock: false
    property int        panorama_Mono: 0
    property int        panorama_LeftEye: 1
    property int        panorama_RightEye: 2
    property int        panorama_BothEye: 3
    property string     infoStr: ""

    Rectangle {
        id: titleRectangle
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
            text: "Frame Sequence Template"
            font.bold: false
            font.pixelSize: 20
        }


        Item {
            id: groupControl
            x: root.width - groupControl.width
            width: 230
            height: 48
            z: 1
            //visible: false
            Item {
                id: checkItem
                anchors.right: uncheckItem.left
                width: 68
                height: 48
                z: 2
                Rectangle {
                    id: checkHoveredRectangle
                    //x: 15
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
                        //if (bFlag) return;
                        isHoveredCheck = !isHoveredCheck
                        if(isHoveredCheck)
                            checkHoveredRectangle.visible = true
                        else
                            checkHoveredRectangle.visible = false

                    }
                    onClicked: {
                        imageLock = true;
                        for(var i=0;i<imageList.count;i++){
                             var  item = imageList.get(i);
                            if(!item.isSelect)  continue;
                            else
                            {
                                imageLock = false;
                                break;
                            }

                        }

                        if(imageLock)
                            console.log("Camera is not selected")
                        else
                        {
                            imageSettingbox.state = "collapsed"
                            recent.state = "collapsed"
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
                        //if (bFlag) return;
                        isHoveredUnCheck = !isHoveredUnCheck
                        if(isHoveredUnCheck)
                            unCheckHoveredRectangle.visible = true
                        else
                            unCheckHoveredRectangle.visible = false
                    }
                     onClicked: {
                         imageSettingbox.state = "collapsed";
                         imageTempCamSetting.state = "collapsed";
                         qmlMainWindow.resetTempGlobalSettings();
                         imageList.clear();
                         for (var i = 0; i < 12; i ++)
                         {
                             imageList.append({"titleText": "Empty Slot","isSelect": false,"titleColor": "#8a8a8a",
                                               "stereoLeftColor": "#8a8a8a", "stereoRightColor": "#8a8a8a",
                                               "stereoType": 1, "stereoState": true,
                                               "selectStereoLeft": false,"selectStereoRight": false,});

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
                    //x: 15
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
                        //if (bFlag) return;
                        isHoveredCheck = !isHoveredCheck
                        if(isHoveredCheck)
                            plusHoveredRectangle.visible = true
                        else
                            plusHoveredRectangle.visible = false

                    }
                    onClicked: {
                        imageDialog.open();

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
                     onClicked: {
//                         var imageName = imageList.get(0).titleText;
//                         var infoStr = qmlMainWindow.getSlotInfo(imageName,"jpg",3);
                         if(infoStr === "") return;
                         globalSettings()
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
        id: imageListView
        y: 80
        width: parent.width
        height: 50 * count

        model: ListModel {
            id: imageList
            ListElement {
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                isSelect: false
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                isSelect: false
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false

            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                isSelect: false
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false

            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                isSelect: false
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false

            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                isSelect: false
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false

            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                isSelect: false
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false

            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                isSelect: false
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false

            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                isSelect: false
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false

            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                isSelect: false
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                isSelect: false
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false

            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                isSelect: false
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false

            }
            ListElement{
                titleText: "Empty Slot"
                titleColor: "#8a8a8a"
                isSelect: false
                stereoLeftColor: "#8a8a8a"
                stereoRightColor: "#8a8a8a"
                stereoState: true
                stereoType: 0
                selectStereoLeft: false
                selectStereoRight: false
            }
        }
        delegate: Item {
            x: 5
            width: parent.width
            height: 50
            Row {
                ImageListitem {
                    title: titleText
                    titleTextColor: titleColor
                    checkSelect: isSelect
                    stereoLColor: stereoLeftColor
                    stereoRColor: stereoRightColor
                    checkStereoState: stereoState
                    tempStereoType: stereoType
                    isStereoLeft: selectStereoLeft
                    isStereoRight: selectStereoRight

                }
            }
        }

    }

    ImageTempCamSetting {
        id: imageTempCamSetting
        anchors.right: parent.right
        anchors.rightMargin: 100
        width: 250
        height: 0
        z: 1
        state: "collapsed"

           states: [
               State {
                   name: "collapsed"
                   PropertyChanges { target: imageTempCamSetting; height: 0}
                   PropertyChanges { target:  imageTempCamSetting;width: 0

                   }
               },
               State {
                   name: "expanded"
                   PropertyChanges { target: imageTempCamSetting; height: 140}
                   PropertyChanges {target: imageTempCamSetting;width: 230}
               }
           ]

           transitions: [
               Transition {
                   NumberAnimation { target: imageTempCamSetting; property: "height"; duration: 300 }
                   NumberAnimation { target: imageTempCamSetting;property: "width";duration: 300}
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
                   PropertyChanges { target:  globalSettingbox; x: root.width}
               },
               State {
                   name: "expanded"
                   PropertyChanges { target: globalSettingbox; x: root.width - 350}
               }
           ]

           transitions: [
               Transition {
                   NumberAnimation { target: globalSettingbox;property: "x";duration: 200 }
               }
           ]

    }

    FileDialog {
        id: imageDialog
        title: "Open image file"
        //nameFilters: [ "Image files (*.jpg *.png)", "All files (*)"  ]
        selectMultiple: false
        selectFolder: true
        onSelectionAccepted: {
            imagePath = fileUrl.toString().substring(8); // Remove "file:///" prefix
            pathList = qmlMainWindow.onImageFileDlg(imagePath);
            //var index = 0
            var prevIdx = 0;
            var curIdx = 0;
            imageIndex = -1;
            do  {
                imageIndex++;
                curIdx = pathList.indexOf(",", prevIdx == 0 ? prevIdx : prevIdx + 1);
                if (curIdx < prevIdx || curIdx == -1) break;
                var curPath = pathList.substring(prevIdx == 0 ? prevIdx : prevIdx + 1, curIdx);
                checkExistImage(imagePath + "/" + curPath);
                prevIdx = curIdx;
            } while (curIdx !== -1);

//            imageUrl = fileUrl + "/";
            imageDialog.folder = imagePath;
//            checkExistImage(imagePath);
            getInfoStr();
        }
    }

    FileDialog {
        id: singleImageDialog
        title: "Open image file"
        //nameFilters: [ "Image files (*.jpg *.png)", "All files (*)"  ]
        selectMultiple: false
        selectFolder: true
        onSelectionAccepted: {
            imagePath = fileUrl.toString().substring(8); // Remove "file:///" prefix
            imageUrl = fileUrl + "/";
            imageDialog.folder = imagePath;
            setImageFile(imagePath);
        }
    }

    Timer {
        id: busyTimer
        interval: 100
        running: false
        repeat: false
        onTriggered: checkSelectImage()
    }

    function checkExistImage(imagePath){
        flag = false;
        for(var i=0;i<imageList.count;i++){
              item = imageList.get(i);
            if(item.titleText === imagePath) {
                flag = true;
                break;
            }
        }

        if(flag) return;
            setImageFile(imagePath);

    }

    function setImageFile(imagePath){
        imageList.set(imageIndex,{"titleColor": "#ffffff"})
        imageList.set(imageIndex,{"isSelect": true})
        imageList.set(imageIndex,{"titleText": imagePath})
    }

    function  getInfoStr()
    {
        var imageName = imageList.get(0).titleText;
        infoStr = qmlMainWindow.getSlotInfo(imageName,"jpg",3);
    }

    function checkSelectImage(){
        curMode = 1;
        clearMainSetting();
        toolbox.showStartIamge();
        clearCameras();
        qmlMainWindow.initTemplateImageObject()
        for(var i=0;i<imageList.count;i++){
            var item = imageList.get(i);
            if(!item.isSelect || item.titleText === "Empty slot")  continue;
            qmlMainWindow.sendImagePath(i,item.titleText);
            qmlMainWindow.setTempStereoType(i,item.stereoType);
        }
        qmlMainWindow.openTemplateImageIniFile()
        clearCameraSettings();
        liveView.createTemplateImageWindow()
        sphericalView.createSphericalCameraWindow()
        interactiveView.createInteractiveCameraWindow();
        qmlMainWindow.initProject();
        qmlMainWindow.openProject()
    }

    function globalSettings(){
        globalSettingbox.changeImageMode();
        imageTempCamSetting.state = "collapsed";
        globalSettingbox.getGlobalValues();
        if(globalSettingbox.state == "expanded"){
            globalSettingbox.state = "collapsed";
        }else if(globalSettingbox.state == "collapsed"){
            globalSettingbox.state = "expanded";
        }
    }

    function setCameraSettings(index){
        imageTempCamSetting.setCameraValues(index);
    }

    function getCameraSettings(index)
    {
        imageTempCamSetting.getCameraSetting(index);
    }

    function setImagePanoramaType()
    {
        if (root.globalStereoState)
        {
            for(var i = 0;i<12;i++)
            {
                //imageList.set(i,{"stereoLeftColor": "#8a8a8a"})
                imageList.set(i,{"stereoState": true})
            }
        }
        else
        {
            for(var j = 0;j<12;j++)
            {
                imageList.set(j,{"stereoLeftColor": "#8a8a8a"})
                imageList.set(j,{"stereoRightColor": "#8a8a8a"})
                imageList.set(j,{"stereoState": false})
                qmlMainWindow.setTempStereoType(j,0);
                //getCameraValues(j);
            }

        }
    }

    function getCameraValues(videoIndex)
    {
        var type =  qmlMainWindow.getTempStereoType(videoIndex)
        switch(type){
        case 0:
            imageList.set(videoIndex,{"stereoLeftColor": "#8a8a8a"})
            imageList.set(videoIndex,{"stereoRightColor": "#8a8a8a"})
            break;
        case 1:
            imageList.set(videoIndex,{"stereoLeftColor": "#8a8a8a"})
            imageList.set(videoIndex,{"stereoRightColor": "#8a8a8a"})
            break;
        case 2:
            imageList.set(videoIndex,{"stereoLeftColor": "#8a8a8a"})
            imageList.set(videoIndex,{"stereoRightColor": "#8a8a8a"})
            break;
        case 3:
            imageList.set(videoIndex,{"stereoLeftColor": "#8a8a8a"})
            imageList.set(videoIndex,{"stereoRightColor": "#8a8a8a"})
            break;
        default:
            break;
        }
    }

    function setImageSlot()
    {
        for (var i = 0; i < qmlMainWindow.getImageCount(); ++i)
        {
            imageIndex = i;
            imageList.set(i,{"titleColor": "#ffffff"});
            imageList.set(i,{"isSelect": true});
            imageList.set(i,{"titleText": qmlMainWindow.getImagePath(i)})
            setStereoSettings();
        }
        getInfoStr();

    }

    function setChangeTitle()
    {
        titleText.text = "Frame Sequence Setting";
    }

    function setStereoSettings()
    {
        var stereoType =  qmlMainWindow.getTempStereoType(imageIndex)
        switch(stereoType){
        case panorama_Mono:
            imageList.set(imageIndex,{"stereoLeftColor": "#8a8a8a"})
            imageList.set(imageIndex,{"stereoRightColor": "#8a8a8a"})
            imageList.set(imageIndex,{"selectStereoLeft": false})
            imageList.set(imageIndex ,{"stereoState": true});
            imageList.set(imageIndex,{"selectStereoRight": false})
            imageList.set(imageIndex,{"stereoType": 0});
            break;
        case panorama_LeftEye:
            imageList.set(imageIndex,{"stereoLeftColor": "#ffffff"})
            imageList.set(imageIndex,{"stereoRightColor": "#8a8a8a"})
            imageList.set(imageIndex,{"selectStereoLeft": true})
            imageList.set(imageIndex ,{"stereoState": true});
            imageList.set(imageIndex,{"selectStereoRight": false})
            imageList.set(imageIndex,{"stereoType": 1});
            break;
        case panorama_RightEye:
            imageList.set(imageIndex,{"stereoLeftColor": "#8a8a8a"})
            imageList.set(imageIndex,{"stereoRightColor": "#ffffff"})
            imageList.set(imageIndex,{"selectStereoLeft": false})
            imageList.set(imageIndex ,{"stereoState": true});
            imageList.set(imageIndex,{"selectStereoRight": true})
            imageList.set(imageIndex,{"stereoType": 2});
            break;
        case panorama_BothEye:
            imageList.set(imageIndex,{"stereoLeftColor": "#ffffff"})
            imageList.set(imageIndex,{"stereoRightColor": "#ffffff"})
            imageList.set(imageIndex,{"selectStereoLeft": true})
            imageList.set(imageIndex ,{"stereoState": true});
            imageList.set(imageIndex,{"selectStereoRight": true})
            imageList.set(imageIndex,{"stereoType": 3});
            break;
        default:
            break;
        }

    }

}
