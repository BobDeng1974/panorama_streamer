import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls.Styles.Flat 1.0 as Flat
import QtQuick.Extras 1.4
import QtQuick.Extras.Private 1.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import "../controls"

Item {
    width : 350
    height: 684


    property int        hoveredType: 0
    property bool       isHovered : false
    property bool       isSelected: false
    property int        leftMargin: 20
    property int        rightMargin: 20
    property int        topMargin: 20
    property int        spacing: 20
    property int        nItemCount: 12
    property color      textColor: "#ffffff"
    property color      comboTextColor: "#7899ff"
    property color      comboBackColor: "#343434"
    property int        itemHeight:30
    property int        lblFont: 14
    property int        groupFont: 16
    property var        cameraCnt
    property bool       isBlend: true
    property color      spliterColor: "#555555"
    property int        lblWidth: 80
    property int        textWidth: 45
    property int        cameraIndex: -1
    property string     seamLabelPos
    property bool       isFirst: true
    property bool       isExposure: false
    property int        cropVal



    Rectangle {
        id: titleRectangle
        x: 0
        y: 0
        width: parent.width
        height: 48
        color: "#171717"
        z: 1

        Text {
            id: titleText
            x: (350 - width) / 2
            y: (parent.height - height) / 2
            z: 3
            color: "#ffffff"
            text: qsTr("Spherical")
            font.bold: false
            font.pixelSize: 20
        }
    }

//    Rectangle {
//        id: spliterRectangle
//        width: parent.width
//        height: 2
//        z: 3
//        anchors.top: titleRectangle.bottom
//        color: "#1f1f1f"
//    }
    Spliter {
        id: spliterRectangle
        width: parent.width
        z: 3
        anchors.top: titleRectangle.bottom
    }

    Rectangle {
        id: backgroundRectangle
        width: parent.width
        height: parent.height
        color: "#171717"
        opacity: 1.0
    }

    Rectangle {
        id: okRectangle
        width: parent.width * 0.5
        height: 40
        color: "#0e3e64"
        z: 1
        anchors {
            left: parent.left
            bottom: parent.bottom
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                //toolbox.clearSelected();
                toolbox.clearState();
                sphericalBox.state = "collapsed";
                if(isExposure)
                {
					qmlMainWindow.enableSeam(-1);
                    qmlMainWindow.setCameraExposure(cameraCombo.currentIndex, exposureText.text);
                    return;
                }
                setBlendValues();
                if (!sphericalView.isSeam) return;
                setSeam();
                clearSeam();
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
        z: 1
        anchors {
            right: parent.right
            bottom: parent.bottom
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                //toolbox.clearSelected()
                toolbox.clearState();
                qmlMainWindow.onCancelCameraSettings();
                qmlMainWindow.reStitch();
                sphericalBox.state = "collapsed"
				qmlMainWindow.enableSeam(-1);
                if (!sphericalView.isSeam) return;
                setSeam();
                clearSeam();
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
    ScrollView {
        id: scrollView
        y: titleRectangle.height
        width: parent.width
        height: parent.height - titleRectangle.height - okRectangle.height
        verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
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
        }

        Item{
            id: groupItems
            width: scrollView.width
            //height:root.height - titleRectangle.height - okRectangle.height
            height:  (spacing + itemHeight )* nItemCount + previewItem.height




            Item {
                id: cameraItem
                width: 350
                height: itemHeight
                anchors.topMargin: 20
                anchors.top: parent.top
               Text {
                   id: cameraLabel
                   anchors.left: parent.left
                   anchors.leftMargin: leftMargin
                   anchors.verticalCenter: parent.verticalCenter
                   text: qsTr("Camera")
                   color: "#ffffff"
                   font.pixelSize: lblFont

               }

               ComboBox {
                   id: cameraCombo
                   anchors.left: cameraLabel.right
                   anchors.leftMargin: leftMargin
                   anchors.verticalCenter: parent.verticalCenter
                   width:parent.width / 2
                   height: 30
                   model: ListModel {
                       id: cameraModel
                   }

                   onCurrentTextChanged:{
                       createBlendViewWindow();
//                       leftSlider.value = qmlMainWindow.getLeft(cameraCombo.currentIndex);
//                       rightSlider.value = qmlMainWindow.getRight(cameraCombo.currentIndex);
//                       topSlider.value = qmlMainWindow.getTop(cameraCombo.currentIndex)
//                       bottomSlider.value = qmlMainWindow.getBottom(cameraCombo.currentIndex);
                      refreshCameraValues();
					  qmlMainWindow.enableSeam(cameraCombo.currentIndex + 1);
                   }

               }

            }

            ListView {
                id: previewItem
                x: 30
                anchors.top: cameraItem.bottom
                anchors.topMargin: 12
                width: 300
                height: 250

            }

            Item {
                id: blend
                anchors.top: previewItem.bottom
                anchors.topMargin: 30
                width: parent.width
                height: 350

                Item {
                    id: blendTitle
                    width: 350
                    height: 30
//                    anchors.topMargin: spacing
//                    anchors.top: previewItem.bottom
                    Text {
                        y: 6
                        color: "#ffffff"
                        text: qsTr("Blending ")
                        font.bold: true
                        anchors.leftMargin: 20
                        font.pixelSize: groupFont
                        anchors.left: parent.left
                    }
                }

//                Rectangle {
//                    id: blendSpliter
//                    width: 340
//                    height: 2
//                    color: spliterColor
//                    anchors.topMargin: 0
//                    anchors.top: blendTitle.bottom
//                    anchors.left: parent.left
//                    anchors.leftMargin: 3
//                }
                Spliter {
                    id: blendSpliter
                    width: 340
                    anchors.topMargin: 0
                    anchors.top: blendTitle.bottom
                    anchors.left: parent.left
                    anchors.leftMargin: 3
                }

                Item {
                    id: radioItem
                    width: parent.width
                    height: itemHeight
                    anchors.top: blendSpliter.bottom
                    anchors.topMargin: spacing


                    RowLayout {
                        anchors.left: parent.left
                        anchors.leftMargin: leftMargin
                        ExclusiveGroup { id: tabPositionGroup }
                        RadioButton {
                            id: featherRadio
                            checked: true
                            exclusiveGroup: tabPositionGroup
                            onClicked: setFeatherMode();
                        }
                        Text {
                            id: feather
                            anchors.left: featherRadio.right
                            anchors.leftMargin: 5
                            width: 70
                            height: parent.height
                            color: "#ffffff"
                            text: "Feather"
                            font.pointSize: 10
                        }

                        RadioButton {
                            id: blendRadio
                            anchors.left: feather.right
                            anchors.leftMargin: leftMargin
                            exclusiveGroup: tabPositionGroup
                            onClicked: setBlendMode();
                        }

                        Text {
                            id: blendText
                            anchors.left: blendRadio.right
                            anchors.leftMargin: 5
                            width: 70
                            height: parent.height
                            color: "#ffffff"
                            text: "Multi Blend"
                            font.pointSize: 10
                        }
                    }
                }

                Text {
                       id: leftLabel
                       text: qsTr("Left")
                       verticalAlignment: Text.AlignVCenter
                       color: "#ffffff"
                       font.pixelSize: lblFont
                       anchors.bottom:　leftItem.top
                       anchors.left: parent.left
                       anchors.leftMargin: leftMargin


                   }

                   Item {
                       id: leftItem
                       width: 350
                       height: itemHeight
                       anchors.top: radioItem.bottom
                       anchors.topMargin: spacing

                       Slider {
                           id: leftSlider
                           value: leftText.text
                           width: parent.width * 0.5
                           minimumValue: 0
                           maximumValue: qmlMainWindow.xRes
                           stepSize: 1
                           anchors.left: parent.left
                           anchors.leftMargin: leftMargin
                           anchors.right: leftText.left
                           anchors.rightMargin: rightMargin
                           anchors.verticalCenter: parent.verticalCenter

                       }

                       FlatText {
                           id: leftText
                           anchors.right: parent.right
                           anchors.rightMargin: rightMargin
                           width: parent.width * 0.15
                           text: leftSlider.value
                           maximumLength: 5
                           onTextChanged: {
                               if(!qmlMainWindow.getLensType())
                               {
                                   leftSlider.value = leftText.text;
                                   rightSlider.value = leftText.text;
                               }
                               else
                                   leftSlider.value = leftText.text;
                           }
                       }
                   }

                   Text {
                       id: rightLabel
                       text: qsTr("Right")
                       verticalAlignment: Text.AlignVCenter
                       color: "#ffffff"
                       font.pixelSize: lblFont
                       anchors.bottom:　rightItem.top
                       anchors.left: parent.left
                       anchors.leftMargin: leftMargin
                   }


                   Item {
                       id: rightItem
                       width: 350
                       height: itemHeight
                       anchors.top: leftItem.bottom
                       anchors.topMargin: topMargin

                       Slider {
                           id: rightSlider
                           value: rightText.text
                           minimumValue: 0
                           maximumValue: qmlMainWindow.xRes
                           stepSize: 1
                           anchors.left: parent.left
                           anchors.leftMargin: leftMargin
                           anchors.right: rightText.left
                           anchors.rightMargin: rightMargin
                           anchors.verticalCenter: parent.verticalCenter
                           width: parent.width * 0.5
                       }

                       FlatText {
                           id: rightText
                           anchors.right: parent.right
                           anchors.rightMargin: rightMargin
                           width: parent.width * 0.15
                           text: rightSlider.value
                           maximumLength: 5
                           onTextChanged: {
                               if(!qmlMainWindow.getLensType())
                               {
                                   leftSlider.value = rightText.text;
                                   rightSlider.value = rightText.text;
                               }
                               else
                                   rightSlider.value = rightText.text;
                           }
                       }

                   }

                   Text {
                       id: topLabel
                       text: qsTr("Top")
                       verticalAlignment: Text.AlignVCenter
                       color: "#ffffff"
                       font.pixelSize: lblFont
                       anchors.bottom:　topItem.top
                       anchors.left: parent.left
                       anchors.leftMargin: leftMargin


                   }

                   Item {
                       id: topItem
                       width: 350
                       height: itemHeight
                       anchors.top: rightItem.bottom
                       anchors.topMargin: topMargin

                       Slider {
                           id: topSlider
                           value: topText.text
                           maximumValue: qmlMainWindow.yRes
                           minimumValue: 0
                           stepSize: 1
                           anchors.left: parent.left
                           anchors.leftMargin: leftMargin
                           anchors.right: topText.left
                           anchors.rightMargin: rightMargin
                           anchors.verticalCenter: parent.verticalCenter
                           width: parent.width * 0.5
                       }

                       FlatText {
                           id: topText
                           anchors.right: parent.right
                           anchors.rightMargin: rightMargin
                           width: parent.width * 0.15
                           text: topSlider.value
                           maximumLength: 5
                           onTextChanged: {
                               if(!qmlMainWindow.getLensType())
                               {
                                   topSlider.value = topText.text;
                                   bottomSlider.value = topText.text;
                               }
                               else
                                   topSlider.value = topText.text;
                           }
                       }

                   }

                   Text {
                       id: bottomLabel
                       text: qsTr("Bottom")
                       verticalAlignment: Text.AlignVCenter
                       color: "#ffffff"
                       font.pixelSize: lblFont
                       anchors.bottom:　bottomItem.top
                       anchors.left: parent.left
                       anchors.leftMargin: leftMargin


                   }

                   Item {
                       id: bottomItem
                       width: 350
                       height: itemHeight
                       anchors.top: topItem.bottom
                       anchors.topMargin: topMargin


                       Slider {
                           id: bottomSlider
                           value: bottomText.text
                           minimumValue: 0
                           maximumValue: qmlMainWindow.yRes
                           stepSize: 1
                           anchors.left: parent.left
                           anchors.leftMargin: leftMargin
                           anchors.right: bottomText.left
                           anchors.rightMargin: rightMargin
                           anchors.verticalCenter: parent.verticalCenter
                           width: parent.width * 0.5
                       }

                       FlatText {
                           id: bottomText
                           anchors.right: parent.right
                           anchors.rightMargin: rightMargin
                           width: parent.width * 0.15
                           text: bottomSlider.value
                           maximumLength: 5
                           onTextChanged: {
                               if(!qmlMainWindow.getLensType())
                               {
                                   topSlider.value = bottomText.text;
                                   bottomSlider.value = bottomText.text;
                               }
                               else
                                   bottomSlider.value = bottomText.text;
                           }
                       }

                   }

                Item {
                    id: levelItem
                    width: 350
                    height: itemHeight
                    anchors.top: bottomItem.bottom
                    anchors.topMargin: topMargin
                    visible: true

                    Text {
                        id: levelLabel
                        text: qsTr("Max Level")
                        verticalAlignment: Text.AlignVCenter
                        color: "#ffffff"
                        font.pixelSize: lblFont
                        anchors.bottom: levelItem.top
                        anchors.left:　parent.left
                        anchors.leftMargin: leftMargin
                    }
                    Slider {
                        id: levelSlider
                        value: levelText.text
                        maximumValue: 5
                        minimumValue: 0
                        stepSize: 1
                        anchors.left: parent.left
                        anchors.leftMargin: leftMargin
                        anchors.right: levelText.left
                        anchors.rightMargin: rightMargin
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width * 0.5
                    }

                    FlatText {
                        id: levelText
                        anchors.right: parent.right
                        anchors.rightMargin: rightMargin
                        width: parent.width * 0.15
                        text: levelSlider.value
                        maximumLength: 5
                    }
                }
            }

            Item {
                id: applyItem
                width: 70
                height: 30
                anchors.top: blend.bottom
                anchors.left: parent.left
                anchors.leftMargin: leftMargin

                Rectangle {
                    id: applyHoverRect
                    width: parent.width
                    height: parent.height
                    color: "#373737"
                    visible: false
                    border.color: "#4e8d15"
                    z: 1

                    Text {
                        color: "#ffffff"
                        text: "Apply"
                        font.pointSize: 11
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        anchors.fill: parent
                    }
                }

                Rectangle {
                    id: applyRect
                    width: parent.width
                    height: parent.height
                    color: "#373737"

                    Text {
                        id: applyText
                        color: "#ffffff"
                        text: "Apply"
                        font.pointSize: 11
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        anchors.fill: parent


                    }

                    MouseArea {
                        id: applyMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onHoveredChanged: {
                            isHovered = !isHovered
                            if(isHovered){
                                applyHoverRect.visible = true;
                            }else{
                                applyHoverRect.visible = false;
                            }
                        }

                        onClicked: {
                            if(isExposure)
                            {
                                qmlMainWindow.setCameraExposure(cameraCombo.currentIndex, exposureText.text);
                                return;
                            }
                            setBlendValues();
                        }
                    }
                }
            }


            Item {
                id: exposure
                width: parent.width
                height: 130
                anchors.top: previewItem.bottom
                anchors.topMargin: 30
                visible: false
                 Item {
                    id: exposoureGroup
                    width: 100
                    height: 30
                    //anchors.topMargin: 10
                    //anchors.top: levelItem.bottom
                    Text {
                        color: "#ffffff"
                        text: qsTr("Exposure")
                        font.bold: true
                        anchors.leftMargin: 20
                        font.pixelSize: groupFont
                        anchors.left: parent.left
                    }
                }


//                Rectangle {
//                    id: exposureSpliter
//                    width: 340
//                    height: 2
//                    color: spliterColor
//                    anchors.top:  exposoureGroup.bottom
//                    anchors.left: parent.left
//                    anchors.leftMargin: 3
//                }
                 Spliter {
                     id: exposureSpliter
                     //height: 2
                     width: 340
                     anchors.top:  exposoureGroup.bottom
                     anchors.left: parent.left
                     anchors.leftMargin: 3
                 }

                Item {
                    id: exposureItem
                    width: 350
                    height: itemHeight
                    anchors.top: exposureSpliter.bottom
                    anchors.topMargin: spacing

                    Slider {
                        id: exposureSlider
                        value: exposureText.text
                        width: parent.width * 0.5
                        minimumValue: -1
                        maximumValue: 1
                        stepSize: 0.1
                        anchors.left: parent.left
                        anchors.leftMargin: leftMargin
                        anchors.right: exposureText.left
                        anchors.rightMargin: rightMargin
                        anchors.verticalCenter: parent.verticalCenter

                    }

                    FlatText {
                        id: exposureText
                        anchors.right: parent.right
                        anchors.rightMargin: rightMargin
                        width: parent.width * 0.15
                        text:  exposureSlider.value
                        maximumLength: 5
                        onTextChanged: {
                            exposureSlider.value = exposureText.text;
                        }

                    }

                }

                Item {
                    id: resetItem
                    width: 65
                    anchors.top: exposure.bottom
                    height: 30
                    anchors.left: parent.left
                    anchors.leftMargin: leftMargin * 2 + applyItem.width

                    Rectangle {
                        id: resetHoverRect
                        x: 0
                        width: parent.width
                        height: parent.height
                        anchors.fill: parent
                        color: "#373737"
                        visible: false
                        border.color: "#4e8d15"
                        border.width: 1
                        z: 1
                    }


                    Text {
                        id: resetText
                        z: 1
                        color: "#ffffff"
                        text: "Reset All"
                        font.pointSize: 11
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        anchors.fill: parent
                    }

                    Rectangle {
                        id: resetRect
                        width: parent.width
                        height: parent.height
                       anchors.fill: parent
                        color: "#373737"

                        MouseArea {
                            id: resetMouseArea
                            width: 60
                            anchors.fill: parent
                            hoverEnabled: true
                            onHoveredChanged: {
                                isHovered = !isHovered
                                if(isHovered){
                                    resetHoverRect.visible = true;
                                }else{
                                    resetHoverRect.visible = false;
                                }
                            }

                            onClicked: {
                                qmlMainWindow.onResetGain();
                                refreshCameraValues();
                            }
                        }
                    }
                }
            }

            Timer {
                id: busyTimer
                interval: 100
                running: false
                repeat: false
            }

            Item {
                id: seam
                width: parent.width
                height: 200
                anchors.top: blend.bottom
                anchors.topMargin: 70


                Item {
                    id: seamGroup
                    width: 100
                    height: 30
//                    anchors.topMargin: 10
//                    anchors.top: blendItem.bottom
                    Text {
                        color: "#ffffff"
                        text: qsTr("Seam")
                        font.bold: true
                        anchors.leftMargin: 20
                        font.pixelSize: groupFont
                        anchors.left: parent.left
                    }
                }

//                Rectangle {
//                    id: seamSpliter
//                    width: 340
//                    height: 2
//                    color: spliterColor
//                    anchors.top:  seamGroup.bottom
//                    anchors.left: parent.left
//                    anchors.leftMargin: 3
//                }
                Spliter {
                    id: seamSpliter
                    width: 340
                    anchors.top:  seamGroup.bottom
                    anchors.left: parent.left
                    anchors.leftMargin: 3
                }

                Item {
                    id: seamView
                    width: parent.width
                    height: 30
                    anchors.top: seamSpliter.bottom
                    anchors.topMargin: itemHeight
                    Text {
                        id: seamLabel
                        width: lblWidth
                        color: textColor
                        text: qsTr("View")
                        horizontalAlignment: Text.AlignLeft
                        anchors.left: parent.left
                        anchors.leftMargin: leftMargin
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: lblFont

                    }

                    Switch{
                        id: seamSwitch
                        anchors.left: seamLabel.right
                        anchors.leftMargin: spacing
                        width: 50
                        height: 30
                        checked: false
                        onClicked: {
                            if(seamSwitch.checked){
                                showSeam();
                            }else{
                                hiddenSeam();
                            }

                        }
                    }

                    ComboBox {
                        id: cameraList
                        width: parent.width / 4
                        height: 30
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: spacing
                        enabled: false
                        model: ListModel {
                            id: camListModel
                        }
                        onCurrentTextChanged: {

                            if (isFirst)
                            {
                                cameraIndex = -1
                            }
                            else
                            {
                                clearSeam();
                                cameraIndex = currentIndex;
                                qmlMainWindow.enableSeam(cameraIndex);
                                createSeam();
                            }
                        }
                    }

                }

            }
        }
    }

    function　showSeam()
    {
        sphericalView.isSeam = true;
        isFirst = false;
        cameraList.enabled = true;
        //cameraIndex = 0;
        //cameraList.currentIndex = cameraIndex;
        cameraIndex = cameraList.currentIndex;
        qmlMainWindow.enableSeam(cameraIndex);
        createSeam();
    }

    function hiddenSeam()
    {
        sphericalView.isSeam = false
        isFirst = true;
        cameraList.enabled = false;
        cameraIndex = -1;
        qmlMainWindow.enableSeam(cameraIndex);
        clearSeam();
    }
	
    function clearSeam()
    {
		sphericalView.clearSeamLabels();
    }

    function getBlendValues(){
        cameraCnt = qmlMainWindow.getCameraCount();
        for(var i = 0; i < cameraCnt; i++){
            cameraModel.append({"text": "Camera" + (i + 1)})
        }

     }

    function getCameraList()
    {
        camListModel.clear();
        camListModel.append({"text": "All"});
        for(var i = 0; i < qmlMainWindow.getCameraCount(); i++){
            camListModel.append({"text": "Camera" + (i + 1)})
        }
        cameraIndex = -1;
    }

    function getSeamState()
    {
        var isSeam = qmlMainWindow.getSeamLabelPos(cameraList.currentIndex - 1);
        if(isSeam)
            seamSwitch.checked = true;
        else
            seamSwitch.checked = false;
    }

    function setSeam()
    {
        if(seamSwitch.checked)
        {
            seamSwitch.checked = false;
            cameraList.currentIndex = 0;
            cameraList.enabled = false;
            cameraIndex = -1;
            qmlMainWindow.enableSeam(cameraIndex);
            sphericalView.isSeam = false;
        }
    }


    function setBlendValues(){
        //qmlMainWindow.setCameraExposure(cameraCombo.currentIndex, exposureText.text);
        qmlMainWindow.setLeft(leftText.text, cameraCombo.currentIndex);
        qmlMainWindow.setRight(rightText.text, cameraCombo.currentIndex);
        qmlMainWindow.setTop(topText.text, cameraCombo.currentIndex)
        qmlMainWindow.setBottom(bottomText.text, cameraCombo.currentIndex);
        if(isBlend){
            qmlMainWindow.setBlendMode(2);
            qmlMainWindow.setBlendLevel(levelText.text);
        }else{
            qmlMainWindow.setBlendMode(1);
        }
		qmlMainWindow.reStitch();
    }

    function getBlendMode()
    {
        var blendMode = qmlMainWindow.getBlendMode();
        if(blendMode)
        {
            featherRadio.checked = false;
            blendRadio.checked = true;
        }
        else
        {
            featherRadio.checked = true;
            blendRadio.checked = false;
        }
    }

    function clearCombo(){
        cameraModel.clear();
    }

    function createBlendViewWindow()
	{
        var component = Qt.createComponent("CameraPreview.qml");
        if (component.status === Component.Ready) {
            var cameraViewObject = component.createObject(previewItem, {"x": 0, "y": 0});
            if(cameraViewObject === null) {
                console.log(cameraViewObject);
            }else{
                qmlMainWindow.createCameraView(cameraViewObject.camView,cameraCombo.currentIndex)
            }
        }
        else
            console.log(component.errorString() )
    }

    function setFeatherMode(){
        levelItem.visible = false;
        isBlend = false;
    }

    function setBlendMode(){
        levelItem.visible = true;
        isBlend = true;
    }

    function createPreviewWindow()
    {
        var component = Qt.createComponent("MCStitchCameraView.qml");
        if (component.status === Component.Ready) {
            var cameraViewObject = component.createObject(exposurePreview, {"x": 0, "y": 0});
            if(cameraViewObject === null) {
                console.log(cameraViewObject);
            }else{
               qmlMainWindow.createPreView(cameraViewObject.camView)
            }
        }
        else
            console.log(component.errorString() )
    }

    function refreshCameraValues()
    {
        exposureSlider.value = qmlMainWindow.getCameraExposure(cameraCombo.currentIndex);
        leftSlider.value = qmlMainWindow.getLeft(cameraCombo.currentIndex);
        rightSlider.value = qmlMainWindow.getRight(cameraCombo.currentIndex);
        topSlider.value = qmlMainWindow.getTop(cameraCombo.currentIndex)
        bottomSlider.value = qmlMainWindow.getBottom(cameraCombo.currentIndex);
        // Auto Calibration Parameters
//        hfov.text = qmlMainWindow.getFov();
//        lensTypeCombo.currentIndex = qmlMainWindow.getLensType();
        getBlendMode();

    }

    function setCalibSettings()
    {
        // Auto Calibration Parameters
        qmlMainWindow.setFov(hfov.text);
        qmlMainWindow.setLensType(lensTypeCombo.currentIndex);
    }

    function onDetails()
    {
        exposureDialog.visible = true;
        exposureDialog.getCameraName();
        exposureDialog.createFirstView();
        exposureDialog.createSecondView();

    }

    function createSeam()
    {
        var seamLabelPos;
        if(cameraIndex === 0)
        {
            for (var i = 0; i < cameraCnt; i++)
            {
                seamLabelPos = qmlMainWindow.getSeamLabelPos(i);
				// left eye
				if ((qmlMainWindow.isStereo() && qmlMainWindow.isLeftEye(i)) ||
					qmlMainWindow.isStereo() === false)
					sphericalView.createSeamLabel(seamLabelPos, i, false);
				// right eye
				if (qmlMainWindow.isStereo()  && qmlMainWindow.isRightEye(i))
					sphericalView.createSeamLabel(seamLabelPos, i, true);
            }
        }
        else
        {
			var i = cameraIndex - 1;
            seamLabelPos = qmlMainWindow.getSeamLabelPos(i);
			// left eye
			if ((qmlMainWindow.isStereo() && qmlMainWindow.isLeftEye(i)) ||
					qmlMainWindow.isStereo() === false)
				sphericalView.createSeamLabel(seamLabelPos, i, false);
			// right eye
			if (qmlMainWindow.isStereo()  && qmlMainWindow.isLeftEye(i))
				sphericalView.createSeamLabel(seamLabelPos, i, true);
        }
    }

    function onCalculate()
    {
        refreshCameraValues();
        qmlMainWindow.onCalculatorGain();
    }

    function onReset()
    {
        refreshCameraValues();
        qmlMainWindow.onResetGain();

    }

    function showExposureSetting()
    {
        exposure.visible = true;
        exposureSlider.value = qmlMainWindow.getCameraExposure(cameraCombo.currentIndex)
        blend.visible = false;
        seam.visible = false;
        applyItem.anchors.top = exposure.bottom
        applyItem.anchors.left = groupItems.left;
        applyItem.anchors.leftMargin = leftMargin;
        nItemCount = 8;
		qmlMainWindow.enableSeam(1);
    }

    function showBlendSettings()
    {
        blend.visible = true;
        exposure.visible = false;
        seam.visible = true;
        applyItem.anchors.top = blend.bottom
        applyItem.anchors.topMargin =  topMargin - 5;
        applyItem.anchors.left = groupItems.left;
        applyItem.anchors.leftMargin = leftMargin;
        nItemCount = 12;

    }
}
