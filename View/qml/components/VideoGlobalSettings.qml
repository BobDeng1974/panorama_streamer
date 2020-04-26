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
    id: item1
    width : 350
    height: 800
    opacity: 1

    property int        hoveredType: 0
    property bool       isHovered : false
    property bool       isSelected: false
    property int        leftMargin: 15
    property int        rightMargin: 15
    property color      textColor: "#ffffff"
    property color      comboTextColor: "#7899ff"
    property color      comboBackColor: "#343434"
    property color      spliterColor: "#555555"
    property int        lblFont: 14
    property int        groupFont: 16
    property int        nItemCount: 6
    property int        itemHeight: 15
    property bool       isHoveredfileOpen : false
    property int        lblWidth: 80
    property int        textWidth: 45
    property int        spacing: 25
    property bool       isOculus
    property bool       isPanoramaType
    property bool       isYoutube: false
    property bool       tempYouTube: false
    property bool       isNvidia: false

    Rectangle {
        id: backgroundRectangle
        x: 0
        y: 0
        width: parent.width
        height: parent.height
        color: "#171717"
        opacity: 0.9
    }

    Rectangle {
        id: titleRectangle
        x: 0
        y: 0
        width: parent.width
        height: 48
        color: "#171717"
        z: 1
        opacity: 1

        Text {
            id: titleText
            x: (350 - width) / 2
            y: (parent.height - height) / 2
            z: 3
            color: "#ffffff"
            font.bold: false
            font.pixelSize: 20
        }
    }
    

    Spliter {
        id: spliterRectangle
        width: parent.width
        height: 2
        z: 3
        anchors.top: titleRectangle.bottom
    }

    ScrollView {
        id: scrollView
        y: titleRectangle.height
        width: parent.width
        height: parent.height - titleRectangle.height - okRectangle.height
        horizontalScrollBarPolicy: 1
        opacity: 0.8
        verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        flickableItem.interactive: true

        style: ScrollViewStyle {
            transientScrollBars: false
            handle: Item {
                implicitWidth: 14
                implicitHeight: 26
                Rectangle {
                    color: "#424246"
                    anchors.fill: parent
                    anchors.topMargin: 6
                    anchors.leftMargin: 4
                    anchors.rightMargin: 4
                    anchors.bottomMargin: 6
                }

            }

            scrollBarBackground: Item {
                implicitWidth: 14
                //implicitHeight: leftMargin
            }

            decrementControl: Rectangle{
                visible: false
            }

            incrementControl: Rectangle{
                visible: false
            }
            corner: Rectangle{
                visible:false
            }
            //handleOverlap: 70

        }
        Item{
            id: groupItems
            width: scrollView.width
            height: itemHeight * 54

            Item {
                id: resolutionItem
                width: parent.width
                height: 30
                anchors.top: parent.top
                anchors.topMargin: itemHeight
                Text {
                    id:xText
                    color: textColor
                    width: lblWidth
                    text: qsTr("Width")
                    horizontalAlignment: Text.AlignLeft
                    anchors.left: parent.left
                    anchors.leftMargin: leftMargin
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: lblFont

                }

                FlatText {
                    id: xRes
                    width: textWidth
                    height: 30
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: xText.right
                    anchors.leftMargin: spacing
                }

                Text {
                    id: yText
                    x: 175
                    color: textColor
                    width: lblWidth
                    text: qsTr("Height")
                    horizontalAlignment: Text.AlignLeft
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: lblFont

                }

                FlatText {
                    id: yRes
                    width: textWidth
                    height: 30
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: rightMargin
                }

                Item {
                    id: youtubeItem
                    width: resolutionItem.width
                    height: 30
                    anchors.top: resolutionItem.bottom
                    anchors.topMargin: itemHeight
                    Text {
                        id: youTubeText
                        width: lblWidth
                        color: textColor
                        text: qsTr("YouTube Rate")
                        horizontalAlignment: Text.AlignLeft
                        anchors.left: parent.left
                        anchors.leftMargin: leftMargin
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: lblFont

                    }

                    RadioButton {
                        id: youtube
                        y: (parent.height - height) / 2
                        width: textWidth
                        anchors.left: youTubeText.right
                        anchors.leftMargin: spacing
                        checked: false
                        onCheckedChanged: {
                            if(checked)
                                isYoutube = true;
                            else
                                isYoutube = false

                        }
                        style: RadioButtonStyle {
                               indicator: Rectangle {
                                   color: "#171717"
                                   implicitWidth: 15
                                   implicitHeight: 15
                                   radius: 9
                                   //border.color: control.activeFocus ? "darkblue" : "gray"
                                   border.color: "#4e8d15"
                                   border.width: 1
                                   Rectangle {
                                       anchors.fill: parent
                                       visible: control.checked
                                       color: "#4e8d15"
                                       radius: 9
                                       anchors.margins: 4
                                   }
                               }

                           }
                    }
                 }

                Item {
                    id: panoResItem
                    width: resolutionItem.width
                    height: 30
                    anchors.top: youtubeItem.bottom
                    anchors.topMargin: itemHeight
                    Text {
                        id:xPanoText
                        width: lblWidth
                        color: textColor
                        text: qsTr("Panorama Width")
                        textFormat: Text.RichText
                        wrapMode: Text.NoWrap
                        horizontalAlignment: Text.AlignLeft
                        anchors.left: parent.left
                        anchors.leftMargin: leftMargin
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: lblFont

                    }

                    FlatText {
                        id: xPano
                        width: textWidth
                        height: 30
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: xPanoText.right
                        anchors.leftMargin: spacing
                        onEditingFinished:  {
                           //if (text === "")	return;
                           if(youtube.checked) return;
                           yPano.text = text / 2;
                        }
                    }

                    Text {
                        id: yPanoText
                        x: 175
                        color: textColor
                        width: lblWidth
                        text: qsTr("Panorama Height")
                        textFormat: Text.AutoText
                        wrapMode: Text.NoWrap
                        horizontalAlignment: Text.AlignLeft
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: lblFont

                    }

                    FlatText {
                        id: yPano
                        width: textWidth
                        height: 30
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: rightMargin

                        onEditingFinished:  {
                            //if (text === "")	return;
                            if(youtube.checked) return;
                            xPano.text = text * 2;
                        }
                    }
                 }

                Item {
                    id: frameItem
                    width: resolutionItem.width
                    height: 30
                    anchors.top: panoResItem.bottom
                    anchors.topMargin: itemHeight
                    Text {
                        id: startText
                        width: lblWidth
                        color: textColor
                        text: qsTr("Start")
                        horizontalAlignment: Text.AlignLeft
                        anchors.left: parent.left
                        anchors.leftMargin: leftMargin
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: lblFont

                    }

                    FlatText {
                        id: startFrame
                        width: textWidth
                        height: 30
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: startText.right
                        anchors.leftMargin: spacing
                    }

                    Text {
                        id: endText
                        x: 175
                        color: textColor
                        width: lblWidth
                        text: qsTr("End ")
                        horizontalAlignment: Text.AlignLeft
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: lblFont

                    }

                    FlatText {
                        id: endFrame
                        width: textWidth
                        height: 30
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: rightMargin
                    }
                 }

                Item {
                    id: cameraCalibItem
                    x: 0
                    width: parent.width
                    height: 30
                    anchors.top: frameItem.bottom
                    anchors.topMargin: itemHeight

                    Text {
                        id: cameraCalibText
                        color: textColor
                        width: lblWidth
                        text: qsTr("Calibration File")
                        horizontalAlignment: Text.AlignLeft
                        font.pixelSize: lblFont
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: leftMargin
                    }

                    FlatText {
                        id: cameraCalib
                        anchors.left: cameraCalibText.right
                        anchors.leftMargin: spacing
                        width: parent.width / 2 - 4
                        height: 30
                        //textFormat: Text.AutoText
                        anchors.verticalCenter: parent.verticalCenter
                        //color: "#4e8d15"
                        //wrapMode: Text.WordWrap

			onEditingFinished: {
				if (text === "") {
					hfov.enabled = true;
					lensTypeCombo.enabled = true;
				} else {
					hfov.enabled = false;
					lensTypeCombo.enabled = false;
				}
			}
                    }
                    Rectangle{
                        width: 30
                        height: 30
                        z: 1
                        anchors.right: parent.right
                        anchors.rightMargin: rightMargin
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#373737"
                        //border.color: "#4e8d15"
                        Text{
                            anchors.fill: parent
                            text: "  ..."
                            color: "#4e8d15"
                            verticalAlignment: Text.AlignVCenter
                            z: 3
                        }

                    }
                    Rectangle{
                        id: fileHoveredRectangle
                        width: 30
                        height: 30
                        z: 1
                        color: "#373737"
                        anchors.right: parent.right
                        anchors.rightMargin: rightMargin
                        anchors.verticalCenter: parent.verticalCenter
                        border.color: "#4e8d15"
                        border.width: 1
                        Text{
                            anchors.fill: parent
                            text: "  ..."
                            color: "#4e8d15"
                            verticalAlignment: Text.AlignVCenter
                            z: 3
                        }

                        visible: false
                    }
                    MouseArea{
                        x: fileHoveredRectangle.x
                        z: 2
                        width: fileHoveredRectangle.width
                        height: fileHoveredRectangle.height
                        anchors.verticalCenter: parent.verticalCenter
                        hoverEnabled: true
                        onHoveredChanged: {
                            isHoveredfileOpen = !isHoveredfileOpen
                            if(isHoveredfileOpen)
                                fileHoveredRectangle.visible = true
                            else
                                fileHoveredRectangle.visible = false
                        }
                        onClicked: fileDialog.visible = true
                    }
                }
                FileDialog{
                    id:fileDialog
                    title: "Select configuration file"
                    nameFilters: [ "Calib File (*.pac *.pts)", "All files (*)" ]
                    selectMultiple: false

                    onSelectionAccepted: {
                        var fileName = fileUrl.toString().substring(8); // Remove "file:///" prefix
                        cameraCalib.text = fileName;
			hfov.enabled = false;
			lensTypeCombo.enabled = false;
                    }
                }

		Item {
			id: autoCalib
			width: parent.width
			height: 30
			anchors.top: cameraCalibItem.bottom
			anchors.topMargin: itemHeight
			Text {
				id: lensTypeLabel
				width: lblWidth
				color: textColor
				text: qsTr("Lens Type")
				horizontalAlignment: Text.AlignLeft
				anchors.left: parent.left
				anchors.leftMargin: leftMargin
				anchors.verticalCenter: parent.verticalCenter
				font.pixelSize: lblFont

			}

			ComboBox {
				id: lensTypeCombo
				width: parent.width / 4
				height: 30
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: lensTypeLabel.right
				anchors.leftMargin: spacing
				model: ["Standard", "Fisheye"]
                onCurrentIndexChanged:
                {
                    if (currentIndex)
                        hfov.text = 240;
                    else
                        hfov.text = 120;
                }
			}

			Text {
				id: hfovLabel
				color: textColor
				text: qsTr("HFOV")
				horizontalAlignment: Text.AlignLeft
				anchors.left: lensTypeCombo.right
				anchors.leftMargin: 10
				anchors.verticalCenter: parent.verticalCenter
				font.pixelSize: lblFont

			}

			FlatText {
			        id: hfov
			        width: textWidth
			        height: 30
			        anchors.verticalCenter: parent.verticalCenter
			        anchors.right: parent.right
			        anchors.rightMargin: rightMargin
			}
		}

                Item {
                    id: fpsItem
                    width: resolutionItem.width
                    height: 30
                    anchors.top: autoCalib.bottom
                    anchors.topMargin: itemHeight
                    Text {
                        id: fpsText
                        width: lblWidth
                        color: textColor
                        text: qsTr("FPS")
                        horizontalAlignment: Text.AlignLeft
                        anchors.left: parent.left
                        anchors.leftMargin: leftMargin
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: lblFont

                    }

                    FlatText {
                        id: fps
                        width: textWidth
                        height: 30
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: fpsText.right
                        anchors.leftMargin: spacing
                    }

                 }

//                Item {
//                    id: urlItem
//                    width: resolutionItem.width
//                    height: 30
//                    anchors.top: fpsItem.bottom
//                    anchors.topMargin: itemHeight
//                    Text {
//                        id: urlText
//                        width: 105
//                        color: textColor
//                        text: qsTr("URL")
//                        horizontalAlignment: Text.AlignLeft
//                        anchors.left: parent.left
//                        anchors.leftMargin: leftMargin
//                        anchors.verticalCenter: parent.verticalCenter
//                        font.pixelSize: lblFont

//                    }

//                    FlatText {
//                        id: streamURL
//                        width: parent.width / 2
//                        height: 30
//                        anchors.verticalCenter: parent.verticalCenter
//                        anchors.left: urlText.right
//                    }

//                 }

                Item {
                    id: fileExtItem
                    width: resolutionItem.width
                    height: 30
                    anchors.top: fpsItem.bottom
                    anchors.topMargin: itemHeight
                    Text {
                        id: fileExtText
                        width: lblWidth
                        color: textColor
                        text: qsTr("File Extension")
                        horizontalAlignment: Text.AlignLeft
                        anchors.left: parent.left
                        anchors.leftMargin: leftMargin
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: lblFont

                    }

                    ComboBox {
                        id: fileExtCombo
                        width: parent.width / 3
                        height: 30
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: fileExtText.right
                        anchors.leftMargin: spacing
                        model: ["dpx","tiff","jpg","png"]
                    }

                 }

                Item {
                    id: panoramaTypeItem
                    width: resolutionItem.width
                    height: 70
                    visible: true
                    anchors.top: fileExtItem.bottom
                    anchors.topMargin: itemHeight

                    Text {
                        id: panoramaTypeLabel
                        x: leftMargin
                        y: 5
                        width: lblWidth
                        color: textColor
                        text: qsTr("Panorama Type")
                        font.bold: true
                        font.pixelSize: 15
                        horizontalAlignment: Text.AlignLeft

                    }



//                    Rectangle {
//                        id: spliter
//                        width: parent.width
//                        height: 2
//                        anchors.top: parent.top
//                        anchors.topMargin: itemHeight + 10
//                        color: "#555555"
//                    }
                    Spliter {
                        id: spliter
                        width: parent.width
                        height: 2
                        anchors.top: parent.top
                        anchors.topMargin: itemHeight + 10
                        //color: "#555555"
                    }

                    Item {
                        id: stereoTypeItem
                        width: parent.width
                        height: 30
                        anchors.top: spliter.bottom
                        anchors.topMargin: 10
                        Text {
                            id:  monoLabel
                            x: lblWidth - 10
                            anchors.verticalCenter: parent.verticalCenter
                            color: "#ffffff"
                            text: qsTr("Mono")
                            font.pixelSize: lblFont
                        }

                        Switch{
                            id: stereoSwitch
                            anchors.left: monoLabel.right
                            anchors.leftMargin: leftMargin
                            width: 50
                            height: 30
                            checked: true
                            onCheckedChanged: {
                                if(stereoSwitch.checked){
                                    root.globalStereoState = true;
                                }else{
                                    root.globalStereoState = false;
                                }
                            }
                        }
                        Text {
                            id:  stereoLabel
                            anchors.left: stereoSwitch.right
                            anchors.leftMargin: leftMargin
                            anchors.verticalCenter: parent.verticalCenter
                            color: "#ffffff"
                            text: qsTr("Stereo")
                            anchors.rightMargin: 5
                            font.pixelSize: lblFont
                        }
                    }
                 }

                Item {
                    id: outItem
                    width: resolutionItem.width
                    height: 70
                    visible: true
                    anchors.top: panoramaTypeItem.bottom
                    anchors.topMargin: itemHeight

                    Text {
                        id: outLabel
                        x: leftMargin
                        y: 5
                        width: lblWidth
                        color: textColor
                        text: qsTr("Output")
                        font.bold: true
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignLeft

                    }



//                    Rectangle {
//                        id: outSpliter
//                        width: parent.width
//                        height: 2
//                        anchors.top: parent.top
//                        anchors.topMargin: itemHeight + 10
//                        color: "#555555"
//                    }

                    Spliter {
                        id: outSpliter
                        width: parent.width
                        height: 2
                        anchors.top: parent.top
                        anchors.topMargin: itemHeight + 10
                    }

                    Item {
                        id: oculusItem
                        width: parent.width
                        height: 30
                        anchors.top: outSpliter.bottom
                        anchors.topMargin: 10
                        Text {
                            id:  oculusLabel
                            x: leftMargin
                            width: lblWidth
                            anchors.verticalCenter: parent.verticalCenter
                            color: "#ffffff"
                            text: qsTr("Oculus Rift")
                            font.pixelSize: lblFont
                        }

                        Switch{
                            id: oculusSwitch
                            anchors.left: oculusLabel.right
                            anchors.leftMargin: spacing
                            width: 50
                            height: 30
                            checked: true
                            onCheckedChanged: {
                                if(oculusSwitch.checked){
                                    isOculus = true;
                                    hddVideoSwitch.checked = false;
                                    rtmpSwitch.checked = false;
                                }else{
                                    isOculus = false;
                                    rtmpSwitch.enabled = true;
                                }
                            }
                        }
                    }

                    Item {
                        id: hddVideoItem
                        width: parent.width
                        height: 30
                        anchors.top: oculusItem.bottom
                        anchors.topMargin: 20
                        Text {
                            id:  hddVideoLabel
                            x: leftMargin
                            width: lblWidth
                            anchors.verticalCenter: parent.verticalCenter
                            color: "#ffffff"
                            text: qsTr("HDD Video")
                            font.pixelSize: lblFont
                        }

                        Switch{
                            id: hddVideoSwitch
                            anchors.left: hddVideoLabel.right
                            anchors.leftMargin: spacing
                            width: 50
                            height: 30
                            checked: true
                            onCheckedChanged: {
                                if(hddVideoSwitch.checked){
                                    oculusSwitch.checked = false
                                    rtmpSwitch.checked = false;
                                    offlineVideoPath.enabled = true;
                                    videoPathMouseArea.enabled = true;
                                    offlineVideoPath.text = qmlMainWindow.getTempOfflineFile();
                                    codecItem.enabled = true;
                                    getOfflineVideoCodec();
                                    getOfflineAudioCodec();
                                }else{
                                    offlineVideoPath.enabled = false;
                                    offlineVideoPath.text = "";
                                    videoPathMouseArea.enabled = false;
                                    if(rtmpSwitch.checked) return;
                                    codecItem.enabled = false;
                                }
                            }
                        }
                    }

                    Item {
                        id: videoPathItem
                        x: 0
                        width: parent.width
                        height: 30
                        anchors.topMargin: 10
                        anchors.top: hddVideoItem.bottom

                        Text {
                            id: videoPathLabel
                             width: lblWidth
                            color: textColor
                            text: qsTr("Path")
                            horizontalAlignment: Text.AlignLeft
                            font.pixelSize: lblFont
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: leftMargin
                        }

                        FlatText {
                            id: offlineVideoPath
                            anchors.left: videoPathLabel.right
                            anchors.leftMargin: spacing
                            width: parent.width / 2 - 4
                            height: 30
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Rectangle{
                            width: 30
                            height: 30
                            z: 1
                            anchors.right: parent.right
                            anchors.rightMargin: rightMargin
                            anchors.verticalCenter: parent.verticalCenter
                            color: "#373737"
                            //border.color: "#4e8d15"
                            Text{
                                anchors.fill: parent
                                text: "  ..."
                                color: "#4e8d15"
                                verticalAlignment: Text.AlignVCenter
                                z: 3
                            }

                        }
                        Rectangle{
                            id: fileHoveredRect
                            width: 30
                            height: 30
                            z: 1
                            color: "#373737"
                            anchors.right: parent.right
                            anchors.rightMargin: rightMargin
                            anchors.verticalCenter: parent.verticalCenter
                            border.color: "#4e8d15"
                            border.width: 1
                            Text{
                                anchors.fill: parent
                                text: "  ..."
                                color: "#4e8d15"
                                verticalAlignment: Text.AlignVCenter
                                z: 3
                            }

                            visible: false
                        }
                        MouseArea{
                            id: videoPathMouseArea
                            x: fileHoveredRect.x
                            z: 2
                            width: fileHoveredRect.width
                            height: fileHoveredRect.height
                            anchors.verticalCenter: parent.verticalCenter
                            hoverEnabled: true
                            onHoveredChanged: {
                                isHoveredfileOpen = !isHoveredfileOpen
                                if(isHoveredfileOpen)
                                    fileHoveredRect.visible = true
                                else
                                    fileHoveredRect.visible = false
                            }
                            onClicked: offlineVideoFileDialog.visible = true
                        }
                    }

                    FileDialog{
                        id: offlineVideoFileDialog
                        selectExisting: false
                        selectFolder: false
                        selectMultiple: false
                        nameFilters: [ "Video File (*.mp4)"]
                        selectedNameFilter: "All files (*)"
                        onAccepted: {
                            var fileName = fileUrl.toString().substring(8);
                            offlineVideoPath.text = fileName;
                        }
                    }

                    Item {
                        id: qualityItem
                        width: resolutionItem.width
                        height: 30
                        anchors.top: videoPathItem.bottom
                        anchors.topMargin: itemHeight
                        Text {
                            id: qualityLabel
                            width: lblWidth
                            color: textColor
                            text: qsTr("Quality")
                            horizontalAlignment: Text.AlignLeft
                            anchors.verticalCenter: parent.verticalCenter
                            font.pixelSize: lblFont
                            anchors.left: parent.left
                            anchors.leftMargin: leftMargin

                        }

                        Text {
                            id: lowText
                            color: textColor
                            horizontalAlignment: Text.AlignLeft
                            anchors.left: qualityLabel.right
                            anchors.leftMargin: spacing
                            text: qsTr("Low")
                            anchors.verticalCenter: parent.verticalCenter
                            font.pixelSize: lblFont
                        }

                        Slider {
                            id: qualitySlider
                            width: parent.width / 2.6
                            minimumValue: 0
                            maximumValue: 51
                            stepSize: 1
                            anchors.left: lowText.right
                            anchors.leftMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            id: highText
                            text: qsTr("High")
                            width: 30
                            horizontalAlignment: Text.AlignLeft
                            color: textColor
                            anchors.verticalCenter: parent.verticalCenter
                            font.pixelSize: lblFont
                            anchors.right: parent.right
                            anchors.rightMargin: rightMargin
                        }

                     }

                    Item {
                        id: rtmpItem
                        width: parent.width
                        height: 30
                        anchors.top: qualityItem.bottom
                        anchors.topMargin: 20
                        Text {
                            id:  rtmpLabel
                            x: leftMargin
                            width: lblWidth
                            anchors.verticalCenter: parent.verticalCenter
                            color: "#ffffff"
                            text: qsTr("RTMP Streaming")
                            font.pixelSize: lblFont
                        }

                        Switch{
                            id: rtmpSwitch
                            anchors.left: rtmpLabel.right
                            anchors.leftMargin: spacing
                            width: 50
                            height: 30
                            checked: false
                            onCheckedChanged: {
                                if(rtmpSwitch.checked){
                                    oculusSwitch.checked = false;
                                    hddVideoSwitch.checked = false;
                                    rtmpURL.enabled = true;
                                    var url = qmlMainWindow.getTempStreamURL();
                                    if (url === ""){
                                        rtmpURL.text = "rtmp://localhost:1935/live/myStream";
                                    }else{
                                        rtmpURL.text = url;
                                    }
                                    codecItem.enabled = true;
                                    getStreamVideoCodec();
                                    getStreamAudioCodec();
                                    ///rtmpVideoCombo.enabled = true;
                                    //rtmpAudioCombo.enabled = true;
                                }else{
                                    rtmpURL.enabled = false;
                                    rtmpURL.text = "";
                                    if(hddVideoSwitch.checked)  return;
                                    codecItem.enabled = false;
                                    //rtmpVideoCombo.enabled = false;
                                    //rtmpAudioCombo.enabled = false;
                                }
                            }
                        }
                    }

                    Item {
                        id: rtmpURLItem
                        x: 0
                        width: parent.width
                        height: 30
                        anchors.topMargin: 10
                        anchors.top: rtmpItem.bottom

                        Text {
                            id: rtmpURLLabel
                            width: lblWidth
                            color: textColor
                            text: qsTr("RTMP URL")
                            horizontalAlignment: Text.AlignLeft
                            font.pixelSize: lblFont
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: leftMargin
                        }

                        FlatText {
                            id: rtmpURL
                            anchors.left: rtmpURLLabel.right
                            anchors.leftMargin: spacing
                            width: parent.width / 2 - 4
                            height: 30
                            enabled: false
                            anchors.verticalCenter: parent.verticalCenter
                        }

                    }

                    Item {
                        id: codecItem
                        width: resolutionItem.width
                        height: 30
                        anchors.top: rtmpURLItem.bottom
                        anchors.topMargin: itemHeight
                        Text {
                            id: rtmpvideoCodecLabel
                            width: lblWidth
                            color: textColor
                            text: qsTr("V-Codec")
                            horizontalAlignment: Text.AlignLeft
                            anchors.left: parent.left
                            anchors.leftMargin: leftMargin
                            anchors.verticalCenter: parent.verticalCenter
                            font.pixelSize: lblFont

                        }

                        ComboBox {
                            id: rtmpVideoCombo
                            width: parent.width / 5
                            height: 30
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: rtmpvideoCodecLabel.right
                            anchors.leftMargin: spacing
                            model: ["H.264","H.265"]
                        }

                        Text {
                            id: rtmpaudioCodecLabel
                            color: textColor
                            text: qsTr("A-Codec")
                            horizontalAlignment: Text.AlignLeft
                            anchors.left: rtmpVideoCombo.right
                            anchors.leftMargin: spacing / 2
                            anchors.verticalCenter: parent.verticalCenter
                            font.pixelSize: lblFont
                        }

                        ComboBox {
                            id: rtmpAudioCombo
                            width: parent.width / 5
                            height: 30
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: rightMargin
                            model: ["AAC"]
                        }
                     }

                    Item {
                        id: nvidiaItem
                        width: parent.width
                        height: 30
                        anchors.top: codecItem.bottom
                        anchors.topMargin: 10
                        Text {
                            id:  nvidiaLabel
                            x: leftMargin
                            width: lblWidth
                            anchors.verticalCenter: parent.verticalCenter
                            color: "#ffffff"
                            text: qsTr("Nvidia")
                            font.pixelSize: lblFont
                        }

                        Switch{
                            id: nvidiaSwitch
                            anchors.left: nvidiaLabel.right
                            anchors.leftMargin: spacing
                            width: 50
                            height: 30
                            checked: false
                            onCheckedChanged: {
                                if(checked){
                                    isNvidia = true;
                                }else{
                                    isNvidia = false
                                }
                            }
                        }
                    }
                 }
            }
        }
    }

    Rectangle {
        id: okRectangle
        width: parent.width * 0.5
        height: 40
        color: "#0e3e64"
        opacity: 1
        z: 1
        anchors {
            left: parent.left
            bottom: parent.bottom
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                bFlag = false;
                globalSettingbox.state = "collapsed";
                setGlobalValues();
            }
        }

        Image {
            id: okImage
            x: (175 - width) / 2
            y: (parent.height - height) / 2
            width: 25
            height: 25
            fillMode: Image.PreserveAspectFit
            source: "../../resources/btn_ok.PNG"
        }
    }

    Rectangle {
        id: cancelRectangle
        width: parent.width * 0.5
        height: 40
        color: "#1f1f1f"
        opacity: 1
        z: 1
        anchors {
            right: parent.right
            bottom: parent.bottom
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                bFlag = false;
                isYoutube = tempYouTube;
                stereoSwitch.checked = true;
                globalSettingbox.state = "collapsed";
            }
        }

        Image {
            id: cancelImage
            x: (175 - width) / 2
            y: (parent.height - height) / 2
            width: 25
            height: 25
            fillMode: Image.PreserveAspectFit
            source: "../../resources/btn_cancel.PNG"
        }
    }

    function getGlobalValues() {
        getPanoramaType();
        getInputSettings();
        getFileExtension();
        getOutputSettings();
        //refreshValues();

    }

    function setGlobalValues(){
        qmlMainWindow.setTempWidth(xRes.text);
        qmlMainWindow.setTempHeight(yRes.text);
        qmlMainWindow.setTempPanoWidth(xPano.text);
        qmlMainWindow.setTempPanoHeight(yPano.text);
        qmlMainWindow.setTempFPS(fps.text);
        qmlMainWindow.setTempStartFrame(startFrame.text);
        qmlMainWindow.setTempEndFrame(endFrame.text);
        qmlMainWindow.setTempCalibFile(cameraCalib.text);
        qmlMainWindow.setTempOfflineFile(offlineVideoPath.text);
        qmlMainWindow.setTempStreamURL(rtmpURL.text);
        setPanoramaType();
        setOculus();
        if(hddVideoSwitch.checked){
            setOfflineVideoCodec();
            setOfflineAudioCodec();
        }
        else if(rtmpSwitch.checked){
            setStreamVideoCodec();
            setStreamAudioCodec();
        }

        // Auto Calibration Parameters
        qmlMainWindow.setTempFov(hfov.text);
        qmlMainWindow.setTempLensType(lensTypeCombo.currentIndex);
        qmlMainWindow.setQuality(qualitySlider.value);
        qmlMainWindow.enableNvidia(isNvidia);
    }

    function changeLiveMode()
    {
        titleText.text = "Live Camera Setting";
        fileExtItem.visible = false;
        panoramaTypeItem.anchors.top = fpsItem.bottom;
        panoramaTypeItem.anchors.topMargin = 20;
    }

    function changeVideoMode()
    {
        titleText.text = "Video Setting";
        fileExtItem.visible = false;
        panoramaTypeItem.anchors.top = fpsItem.bottom;
        panoramaTypeItem.anchors.topMargin = 20;
    }

    function changeImageMode()
    {
        titleText.text = "Frame Sequence Setting";
        fileExtItem.visible = true;
    }

    function clearTitle()
    {
        titleText.text = "";
    }

    function getFileExtension()
    {
        var fileExt = qmlMainWindow.getTempFileExt();
        if (fileExt === "dpx")
        {
            fileExtCombo.currentIndex = 0;
        }
        else if (fileExt === "tiff")
        {
            fileExtCombo.currentIndex = 1;
        }
        else if (fileExt === "jpg")
        {
            fileExtCombo.currentIndex = 2;
        }
        else if (fileExt === "png")
        {
            fileExtCombo.currentIndex = 3
        }
    }

    function setFileExtension()
    {
        switch (fileExtCombo.currentIndex)
        {
            case 0: qmlMainWindow.setTempFileExt("dpx"); break
            case 1: qmlMainWindow.setTempFileExt("tiff"); break;
            case 2: qmlMainWindow.setTempFileExt("jpg"); break;
            case 3: qmlMainWindow.setTempFileExt("png"); break;
        }
    }

    function getOculus()
    {
        isOculus =  qmlMainWindow.getTempOculus();
        if(isOculus)
        {
            oculusSwitch.checked = true;
        }
        else
        {
            oculusSwitch.checked = false;
        }

    }

    function setOculus()
    {
        if(isOculus)
        {
            qmlMainWindow.setTempOculus(isOculus);
        }
        else
        {
             qmlMainWindow.setTempOculus(isOculus);
        }

    }

    function getOfflineVideoCodec()
    {
        var videoCodec = qmlMainWindow.getTempOfflineVideoCodec();
        if (videoCodec === "H.264")
        {
            rtmpVideoCombo.currentIndex = 0;
        }
        else if (videoCodec === "H.265")
        {
            rtmpVideoCombo.currentIndex = 1;
        }

    }

    function setOfflineVideoCodec()
    {
        switch (rtmpVideoCombo.currentIndex)
        {
            case 0: qmlMainWindow.setTempOfflineVideoCodec("H.264"); break
            case 1: qmlMainWindow.setTempOfflineVideoCodec("H.265"); break;
        }

    }

    function getOfflineAudioCodec()
    {
        var audioCodec = qmlMainWindow.getTempOfflineAudioCodec();
        if (!audioCodec === "AAC") return;
            rtmpAudioCombo.currentIndex = 0;
    }

    function setOfflineAudioCodec()
    {
        qmlMainWindow.setTempOfflineAudioCodec("AAC");
    }

    function getStreamVideoCodec()
    {
        var videoCodec = qmlMainWindow.getTempStreamVideoCodec();
        if (videoCodec === "H.264")
        {
            rtmpVideoCombo.currentIndex = 0;
        }
        else if (videoCodec === "H.265")
        {
            rtmpVideoCombo.currentIndex = 1;
        }
    }

    function setStreamVideoCodec()
    {
        switch (rtmpVideoCombo.currentIndex)
        {
            case 0: qmlMainWindow.setTempStreamVideoCodec("H.264"); break
            case 1: qmlMainWindow.setTempStreamVideoCodec("H.265"); break;
        }

    }

    function getStreamAudioCodec()
    {
        var audioCodec = qmlMainWindow.getTempStreamAudioCodec();
        if (!audioCodec === "AAC") return;
            rtmpAudioCombo.currentIndex = 0;
    }

    function setStreamAudioCodec()
    {
        qmlMainWindow.setTempStreamAudioCodec("AAC");
    }

    function refreshValues()
    {
        rtmpURL.enabled = false;
        //rtmpVideoCombo.enabled = false;
        //rtmpAudioCombo.enabled = false;
    }

    function setPanoramaType()
    {
        globalStereoState = stereoSwitch.checked
        isFirst = false
        if(titleText.text === "Live Camera Setting")
        {
            setCameraPanoramaType();
        }
        else if(titleText.text === "Video Setting")
        {
            setVideoPanoramaType();
        }
        else if(titleText.text === "Frame Sequence Setting")
        {
            setImagePanoramaType();
            setFileExtension();
        }
    }

    function getPanoramaType()
    {
        if(root.globalStereoState)
            stereoSwitch.checked = true;
        else
            stereoSwitch.checked = false;
    }

    function getInputSettings()
    {
        if(isTemplate){
            getInputInfo();
        }else {

            xRes.text = qmlMainWindow.getTempWidth();
            yRes.text = qmlMainWindow.getTempHeight();
            var fpsVal = qmlMainWindow.getTempFPS();
            if(fpsVal === 0)
                fps.text = 30;
            else
                fps.text = fpsVal;
        }
        xPano.text = qmlMainWindow.getTempPanoWidth();
        yPano.text = qmlMainWindow.getTempPanoHeight();
        startFrame.text = qmlMainWindow.getTempStartFrame();
        endFrame.text = qmlMainWindow.getTempEndFrame();
        cameraCalib.text = qmlMainWindow.getTempCalibFile();
        // Auto Calibration Parameters
        hfov.text = qmlMainWindow.getTempFov();
        lensTypeCombo.currentIndex = qmlMainWindow.getTempLensType();
        getYoutubeRate();
    }

    function getYoutubeRate()
    {
        tempYouTube = isYoutube;
        if(isYoutube)
            youtube.checked = true;
        else
            youtube.checked = false;
    }

    function getInputInfo()
    {
        var infoStr;
        if(titleText.text === "Live Camera Setting")
        {
            //infoStr = qmlMainWindow.getSlotInfo(camList.get(0).titleText,"",1);
            infoStr = liveSettingbox.infoStr;
        }
        else if(titleText.text === "Video Setting")
        {
            //infoStr = qmlMainWindow.getSlotInfo(videoList.get(0).titleText,"",2);
            infoStr = videoSettingbox.infoStr;
        }
        else if(titleText.text === "Frame Sequence Setting")
        {
            //infoStr = qmlMainWindow.getSlotInfo(imageList.get(0).titleText,"jpg",3);
            infoStr = imageSettingbox.infoStr;
        }
        var strList = infoStr.split(":");
        xRes.text = strList[0];
        yRes.text = strList[1];
        if (strList[2] === "0")
            fps.text = 30;
        else
            fps.text = strList[2];

    }

    function getOutputSettings()
    {
        getOculus();
        refreshStreamState();
        refreshOfflineState();
        getQuality();
        getNvidia();
    }

    function refreshOfflineState()
    {
        offlineVideoPath.text = qmlMainWindow.getTempOfflineFile();
        if(offlineVideoPath.text === "")
        {
            hddVideoSwitch.checked = false;
        }
        else
        {
            hddVideoSwitch.checked = true;
            getOfflineVideoCodec();
            getOfflineAudioCodec();
        }
        if(hddVideoSwitch.checked || rtmpSwitch.checked) return;
        hddVideoSwitch.checked = true;
    }

    function refreshStreamState()
    {
        var url = qmlMainWindow.getTempStreamURL();
        if(url === "")
        {
            rtmpSwitch.checked = false;
        }
        else
        {
            rtmpSwitch.checked = true;
            getStreamVideoCodec();
            getStreamAudioCodec();
        }
    }

    function getQuality()
    {
        qualitySlider.value = qmlMainWindow.getQuality();
    }

    function getNvidia()
    {
        var isNvidia = false;
        isNvidia = qmlMainWindow.isNvidia();
        if(isNvidia)
            nvidiaSwitch.checked = true;
        else
            nvidiaSwitch.checked = false;
    }
}
