import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls.Styles.Flat 1.0 as Flat
import QtQuick.Extras 1.4
import QtQuick.Extras.Private 1.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import "../controls"

Item {
    width : 1030
    height: 684
    opacity: 1

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
    property bool       isEmpty: false
    property bool       isHoveredMore: false
    property bool       bFlag
    property bool       bCamFlag
    property int        camCount
    property int        audioCount
    property int        camIndex: 0
    property bool       camLock: false
    property int        audioIndex: 0
    property bool       isCameraSelected: false
    property bool       isAudioSelected: false
    property int        panorama_Mono: 0
    property int        panorama_LeftEye: 1
    property int        panorama_RightEye: 2
    property int        panorama_BothEye: 3
    property int        mixedChannel: 0
    property int        leftChannel: 1
    property int        rightChannel: 2
    property int        noAudio: 3
    property var        audioName
    property string     infoStr: ""
    Rectangle {
        id: titleRectangle
        width: parent.width
        height: 48
        color: "#1f1f1f"
        opacity: 1
        z: 1
        Text {
            id: titleText
            x: 50
            y: (parent.height - height) / 2
            z: 0
            color: "#ffffff"
            text: "Live Camera Template"
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
                        //if (bFlag) return;
                        isHoveredCheck = !isHoveredCheck
                        if(isHoveredCheck)
                            checkHoveredRectangle.visible = true
                        else
                            checkHoveredRectangle.visible = false

                    }
                    onClicked: {
                        camLock = true;
                        for(var i=0;i<camList.count;i++){
                             var  item = camList.get(i);
                            if(!item.isSelect)  continue;
                            else
                            {
                                camLock = false;
                                break;
                            }

                        }

                        if(camLock)
                            console.log("Camera is not Selected")
                        else
                        {

                            liveSettingbox.state = "collapsed"
                            recent.state = "collapsed"
                            openCameraViews();
                            busyTimer.restart();
                        }
                    }
                }

            }

            Item {
                id:　uncheckItem
                anchors.right: moreItem.left
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
                         //if (bFlag) return;
                         liveTempCamSetting.state = "collapsed";
                         liveSettingbox.state = "collapsed";
                         qmlMainWindow.resetTempGlobalSettings();
                         camList.clear();
                         audioList.clear();
                         if(isTemplate) return;
                         else {
                             toolbox.clearState();
                             onForward();
                         }
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
                        if(camList.count === 0) return;
//                        var camName = camList.get(0).titleText;
//                        var infoStr = qmlMainWindow.getSlotInfo(camName,"",1);
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
        z: 0
        opacity: 1.0
    }

    ListView {
        id: cameraListView
        y: 80
        width: parent.width
        height: 50 * count
        z: 1
        //snapMode: ListView.SnapOneItem
        spacing: 0
        model: ListModel {
            id: camList
        }
        delegate: Item {
            x: 5
            width: parent.width
            height: 50
            Row {
                LiveCamListitem {
                    title: titleText
                    titleTextColor: titleColor
                    checkSelect: isSelect
                    stereoLColor: stereoLeftColor
                    stereoRColor: stereoRightColor
                    checkStereoState: stereoState
                    liveStereoType: stereoType
                    isStereoLeft: selectStereoLeft
                    isStereoRight: selectStereoRight
                }
            }
        }
    }
    Spliter{
        id: split
        width: root.width - 50
        height: 1
        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.top: cameraListView.bottom
        anchors.topMargin: 5
        anchors.right: parent.right
        anchors.rightMargin: 30

    }
    ListView {
        id: audioListView
        y: cameraListView.height + 90
        anchors.top: split.bottom
        anchors.topMargin: 20
        width: parent.width
        height: 50 * count
        z: 1
        spacing: 0
        model: ListModel {
            id: audioList
        }
        delegate: Item {
            x: 5
            width: parent.width
            height: 50
            Row {
                LiveAudioitem {
                    title: audioText
                    titleTextColor: titleColor
                    checkSelect: isSelect
                    liveAudioType: audioType
                    isAudioLeft: selectAudioLeft
                    isAudioRight: selectAudioRight
                    audioLeftColor: leftColor
                    audioRightColor: rightColor
                }
            }
        }
   }

    LiveTempCamSetting {
        id: liveTempCamSetting
        anchors.right: parent.right
        anchors.rightMargin: 100
        width: 200
        height: 0
        z: 1
        state: "collapsed"

           states: [
               State {
                   name: "collapsed"
                   PropertyChanges { target: liveTempCamSetting; height: 0}
                   PropertyChanges { target:  liveTempCamSetting;width: 0

                   }
               },
               State {
                   name: "expanded"
                   PropertyChanges { target: liveTempCamSetting; height: 300}
                   PropertyChanges {target: liveTempCamSetting;width: 200}
               }
           ]

           transitions: [
               Transition {
                   NumberAnimation { target: liveTempCamSetting; property: "height"; duration: 300 }
                   NumberAnimation { target: liveTempCamSetting;property: "width";duration: 300}
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

    Timer {
        id: busyTimer
        interval: 100
        running: false
        repeat: false
        onTriggered: checkSelectCamera()
    }

   function setCurrentCamera() {
     camListModel.set(videoIndex,{"titleText": videoPath})
   }

   function checkSelectCamera(){
       curMode = 1;
       clearMainSetting()
       toolbox.showStartIamge();
       clearCameras();
       qmlMainWindow.initTemplateCameraObject()
       for(var camIndex=0;camIndex<camList.count; camIndex++){
           var cameraItem = camList.get(camIndex);
           if(!cameraItem.isSelect) continue;
           qmlMainWindow.sendCameraName(camIndex,cameraItem.titleText);
           qmlMainWindow.setTempStereoType(camIndex,cameraItem.stereoType);
       }

       for(var audioIndex=0, selCount=0; audioIndex < audioList.count; audioIndex++) {
           var audioItem = audioList.get(audioIndex);
           if(!(audioItem.isSelect && selCount+1 <= camList.count)) continue;
           qmlMainWindow.sendAudioName(audioIndex,audioItem.audioText);
           qmlMainWindow.setTempAudioSettings(audioIndex,audioItem.audioType);
           selCount++;
       }
       qmlMainWindow.openTemplateCameraIniFile()
       clearCameraSettings();
       liveView.createTemplateCameraWindow()
       sphericalView.createSphericalCameraWindow();
       interactiveView.createInteractiveCameraWindow();
       qmlMainWindow.initProject();
       qmlMainWindow.openProject()
   }

   function globalSettings(){
       globalSettingbox.changeLiveMode();
       liveTempCamSetting.state = "collapsed";
       globalSettingbox.getGlobalValues();
       if(globalSettingbox.state == "expanded"){
           globalSettingbox.state = "collapsed";
       }else if(globalSettingbox.state == "collapsed"){
           globalSettingbox.state = "expanded";
       }
   }

   function setCameraSettings(index){
       liveTempCamSetting.setCameraValues(index);
   }

   function getCameraSettings(index)
   {
       liveTempCamSetting.getCameraValues(index);
   }

   function setCameraPanoramaType()
   {
       if (root.globalStereoState)
       {
           for(var i = 0;i<camCount;i++)
           {
               camList.set(i,{"stereoState": true})
           }
       }
       else
       {
           for(var j = 0;j<camCount;j++)
           {
               camList.set(j,{"stereoLeftColor": "#8a8a8a"})
               camList.set(j,{"stereoRightColor": "#8a8a8a"})
               camList.set(j,{"stereoState": false})
               camList.set(j,{"stereoType": 0});
           }
       }
   }

   function getCameraValues(camIndex)
   {
       var type =  qmlMainWindow.getTempStereoType(camIndex)
       switch(type){
       case 0:
           camList.set(camIndex,{"stereoLeftColor": "#8a8a8a"})
           camList.set(camIndex,{"stereoRightColor": "#8a8a8a"})
           break;
       case 1:
           camList.set(camIndex,{"stereoLeftColor": "#8a8a8a"})
           camList.set(camIndex,{"stereoRightColor": "#8a8a8a"})
           break;
       case 2:
           camList.set(camIndex,{"stereoLeftColor": "#8a8a8a"})
           camList.set(camIndex,{"stereoRightColor": "#8a8a8a"})
           break;
       case 3:
           camList.set(camIndex,{"stereoLeftColor": "#8a8a8a"})
           camList.set(camIndex,{"stereoRightColor": "#8a8a8a"})
           break;
       default:
           break;
       }
   }

   function setCameraList()
   {
       camCount = qmlMainWindow.getCameraCnt();
       camList.clear();
       for(var i = 0;i<camCount;i++){
           camList.set(i ,{"titleText": qmlMainWindow.getDeviceName(i,0)});
           camList.set(i,{"titleColor": "#ffffff"})
           camList.set(i ,{"isSelect": true});
           camList.set(i,{"stereoLeftColor": "#8a8a8a"})
           camList.set(i,{"stereoRightColor": "#8a8a8a"})
           camList.set(i,{"selectStereoLeft": false})
           camList.set(i,{"selectStereoRight": false})
           camList.set(i ,{"stereoState": true});
           camList.set(i,{"stereoType": 0});
       }
       audioCount = qmlMainWindow.getAudioCnt();
       if( audioCount === 0) {
           split.visible = false;
       }
       else if( audioCount > 0){
           split.visible = true;
       }
       audioList.clear();
       for(var j = 0; j<audioCount;j++){
           audioList.set(j ,{"audioText": qmlMainWindow.getDeviceName(j,1)});
           audioList.set(j,{"titleColor": "#8a8a8a"})
           audioList.set(j ,{"audioType": 1});
           audioList.set(j ,{"selectAudioLeft": true});
           audioList.set(j ,{"selectAudioRight": false});
           audioList.set(j ,{"leftColor": "#ffffff"});
           audioList.set(j ,{"rightColor": "#8a8a8a"});
           audioList.set(j ,{"isSelect": false})
       }
       if(camList.count === 0) return;
       getInfoStr();
   }

   function getInfoStr()
   {
       var camName = camList.get(0).titleText;
       infoStr = qmlMainWindow.getSlotInfo(camName,"",1);
   }

   function setChangeTitle()
   {
       titleText.text = "Live Camera Setting"
   }

   function setLiveSlot()
   {
       camList.clear();
       var cameraName;
       camCount = qmlMainWindow.getCameraCnt();
		var camSelList = [];
		for(var i = 0; i < camCount; i++) {
			camSelList[i] = [];
			var count = 0;
			var name = qmlMainWindow.getDeviceName(i, 0);
			for(var j = 0; j < i; j++) {
				if (camSelList[j][0] == name)
					count += camSelList[j][1];
			}
			count++;
			camSelList[i][0] = name;
			camSelList[i][1] = count;
		}
        getInfoStr();
       for(var i = 0;i<camCount;i++){
           camIndex = i;
           cameraName = qmlMainWindow.getDeviceName(i,0);
           camList.set(i ,{"titleText": cameraName});
           isCameraSelected = qmlMainWindow.isSelectedCamera(cameraName);
           if (isCameraSelected && camSelList[i][1] == 1)
                camList.set(i,{"titleColor": "#ffffff"})
           else {
               camList.set(i,{"titleColor": "#8a8a8a"})
			   isCameraSelected = false;
			}
           camList.set(i ,{"isSelect": isCameraSelected});
           setStereoSettings();
       }

       audioList.clear();
       //var audioName;
       for (var j = 0; j<qmlMainWindow.getAudioCnt(); j++)
       {
           audioIndex = j;
           audioName = qmlMainWindow.getDeviceName(j,1);
           audioList.set(j ,{"audioText": audioName});
           isAudioSelected = qmlMainWindow.isSelectedAudio(audioName);
           if(isAudioSelected)
               audioList.set(j,{"titleColor": "#ffffff"});
           else
              audioList.set(j,{"titleColor": "#8a8a8a"});
           audioList.set(j ,{"isSelect": isAudioSelected});
           setAudioSettings();
       }


   }

   function setStereoSettings()
   {
       var stereoType =  qmlMainWindow.getTempStereoType(camIndex)
       switch(stereoType){
       case panorama_Mono:
           camList.set(camIndex,{"stereoLeftColor": "#8a8a8a"})
           camList.set(camIndex,{"stereoRightColor": "#8a8a8a"})
           camList.set(camIndex ,{"stereoState": true});
           camList.set(camIndex,{"selectStereoLeft": false})
           camList.set(camIndex,{"selectStereoRight": false})
           camList.set(camIndex,{"stereoType": 0});
           break;
       case panorama_LeftEye:
           camList.set(camIndex,{"stereoLeftColor": "#ffffff"})
           camList.set(camIndex,{"stereoRightColor": "#8a8a8a"})
           camList.set(camIndex ,{"stereoState": true});
           camList.set(camIndex,{"stereoType": 1});
           camList.set(camIndex,{"selectStereoLeft": true})
           camList.set(camIndex,{"selectStereoRight": false})
           break;
       case panorama_RightEye:
           camList.set(camIndex,{"stereoLeftColor": "#8a8a8a"})
           camList.set(camIndex,{"stereoRightColor": "#ffffff"})
           camList.set(camIndex ,{"stereoState": true});
           camList.set(camIndex,{"stereoType": 2});
           camList.set(camIndex,{"selectStereoLeft": false})
           camList.set(camIndex,{"selectStereoRight": true})
           break;
       case panorama_BothEye:
           camList.set(camIndex,{"stereoLeftColor": "#ffffff"})
           camList.set(camIndex,{"stereoRightColor": "#ffffff"})
           camList.set(camIndex ,{"stereoState": true});
           camList.set(camIndex,{"stereoType": 3});
           camList.set(camIndex,{"selectStereoLeft": true})
           camList.set(camIndex,{"selectStereoRight": true})
           break;
       default:
           break;
       }
   }

  function setAudioSettings()
  {
      //var audioType =  qmlMainWindow.getTempAudioSettings(audioIndex)
      var audioType =  qmlMainWindow.getTempAudioSettingsEx(audioName)
      switch(audioType){
      case mixedChannel:
          audioList.set(audioIndex,{"leftColor": "#ffffff"})
          audioList.set(audioIndex,{"rightColor": "#ffffff"})
          audioList.set(audioIndex ,{"audioType": 0});
          audioList.set(audioIndex,{"selectAudioLeft": true})
          audioList.set(audioIndex,{"selectAudioRight": true})
          break;
      case leftChannel:
          audioList.set(audioIndex,{"leftColor": "#ffffff"})
          audioList.set(audioIndex,{"rightColor": "#8a8a8a"})
          audioList.set(audioIndex ,{"audioType": 1});
          audioList.set(audioIndex,{"selectAudioLeft": true})
          audioList.set(audioIndex,{"selectAudioRight": false})
          break;
      case rightChannel:
          audioList.set(audioIndex,{"leftColor": "#8a8a8a"})
          audioList.set(audioIndex,{"rightColor": "#ffffff"})
          audioList.set(audioIndex ,{"audioType": 2});
          audioList.set(audioIndex,{"selectAudioLeft": false})
          audioList.set(audioIndex,{"selectAudioRight": true})
          break;
      case noAudio:
          audioList.set(audioIndex,{"leftColor": "#8a8a8a"})
          audioList.set(audioIndex,{"rightColor": "#8a8a8a"})
          audioList.set(audioIndex ,{"audioType": 3});
          audioList.set(audioIndex,{"selectAudioLeft": false})
          audioList.set(audioIndex,{"selectAudioRight": false})
          break;
      default:
          break;
      }
  }

  function getStereoState()
  {
      for(var i = 0;i<camCount;i++){
          var stereoType =  qmlMainWindow.getTempStereoType(i);
          if(stereoType === 0)
          {
              globalStereoState = false;
              continue;
          }
          else
          {
              globalStereoState = true;
              break;
          }
      }
  }
}
