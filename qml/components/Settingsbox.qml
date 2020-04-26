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
    width : 350
    height: 684
    opacity: 1
    property int hoveredType: 0
    property bool isHovered : false
    property bool isSelected: false
    property int  leftMargin: 20
    property int rightMargin: 20
    property int spacing: 20
    property color textColor: "#ffffff"
    property color comboTextColor: "#7899ff"
    property color comboBackColor: "#343434"
    property color spliterColor: "#555555"
    property int lblFont: 14
    property int groupFont: 16
    property int nItemCount: 21
    property int itemHeight:30
    property bool isHoveredfileOpen : false
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
            text: qsTr("Settings")
            font.bold: false
            font.pixelSize: 20
        }
    }

    Rectangle {
        id: spliterRectangle
        width: parent.width
        height: 2
        z: 3
        anchors.top: titleRectangle.bottom
        color: "#1f1f1f"
    }

    Rectangle {
        id: backgroundRectangle
        width: parent.width
        height: parent.height
        color: "#171717"
        opacity: 0.9
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
                qmlMainWindow.setExposure(exposureFeild.text);
                toolbox.clearSelected()
                settingsBox.state = "collapsed"
                setGlobalValues()
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
                toolbox.clearSelected();
                refreshValues();
                settingsBox.state = "collapsed"
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
            transientScrollBars: true
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
            width: parent.width
            height: (spacing + itemHeight )* nItemCount + 80

            Item {
                id: groupInputItem
                x: 0
                width: 350
                height: 30
                anchors.top: parent.top
                anchors.topMargin: spacing

                Text {
                    id: groupText
                    y: 6
                    color: "#ffffff"
                    text: qsTr("Input")
                    font.bold: true
                    anchors.leftMargin: 20
                    font.pixelSize: groupFont
                    anchors.left: parent.left
                }
            }
            Rectangle {
                id: spliterGroupinput
                width: 340
                height: 2
                color: spliterColor
                anchors.topMargin: 0
                anchors.top: groupInputItem.bottom
                anchors.left: parent.left
                anchors.leftMargin: 3
            }

            Item {
                id: captureTypeItem
                y: 0
                width: 350
                height: 30
                anchors.top: spliterGroupinput.top
                anchors.topMargin: spacing
                Text {
                    id: cmaeraClibText
                    y: 12
                    color: textColor
                    text: qsTr("Capture Type")
                    font.pixelSize: lblFont
                    anchors.left: parent.left
                    anchors.leftMargin: leftMargin
                    anchors.verticalCenter: parent.verticalCenter
                }

                ComboBox {
                    id: captureTypeCombo
                    anchors.right: parent.right
                    anchors.rightMargin: rightMargin
                    anchors.verticalCenter: parent.verticalCenter
                    width:parent.width / 2 -20
                    height: 30
                    model: ["dshow", "ximea","video","image"]
                }
            }

            Item {
                id: deviceCountItem
                width: 350
                height: 30
                anchors.top: captureTypeItem.bottom
                anchors.topMargin: spacing
                Text {
                    id: deviceCountText
                    y: 12
                    color: textColor
                    text: qsTr("Device Count")
                    anchors.left: parent.left
                    anchors.leftMargin: leftMargin
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: lblFont

                }

                FlatText {
                    id: deviceCountFeild
                    width: parent.width * 0.5 - 20
                    height: 30
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: rightMargin
                    readOnly: true
                }
            }

            Item {
                id: framePerSecondItem
                width: 350
                height: 30
                anchors.top: deviceCountItem.bottom
                anchors.topMargin: spacing
                Text {
                    id: framePerSecondText
                    y: 12
                    color: textColor
                    text: qsTr("Frame per Second (fps)")
                    anchors.left: parent.left
                    anchors.leftMargin: leftMargin
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: lblFont

                }

                FlatText {
                    id: framePerSecondFeild
                    width: parent.width * 0.5 - 20
                    height: 30
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: rightMargin
                    readOnly: true
                }
            }

            Item {
                id: resolutionItem
                width: 350
                height: 30
                anchors.top: framePerSecondItem.bottom
                anchors.topMargin: itemHeight
                Text {
                    id:xText
                    width: 60
                    color: textColor
                    text: qsTr("X-Resolution")
                    anchors.left: parent.left
                    anchors.leftMargin: leftMargin
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: lblFont

                }

                FlatText {
                    id: xRes
                    width: 50
                    height: 30
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: xText.right
                    anchors.leftMargin: 30
                }

                Text {
                    color: textColor
                    width: 60
                    text: qsTr("Y-Resolution")
                    anchors.right: yRes.left
                    anchors.rightMargin: 30
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: lblFont

                }

                FlatText {
                    id: yRes
                    width: 50
                    height: 30
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: rightMargin
                }
            }

            Item {
                id: groupOutputItem
                x: 0
                width: 350
                height: 30
                anchors.top: resolutionItem.bottom
                anchors.topMargin: spacing
                Text {
                    y: 6
                    color: "#ffffff"
                    text: qsTr("Output")
                    font.bold: true
                    anchors.leftMargin: 20
                    font.pixelSize: groupFont
                    anchors.left: parent.left
                }
            }
            Rectangle {
                id: spliterGroupoutput
                width: 340
                height: 2
                color: spliterColor
                anchors.topMargin: 0
                anchors.top: groupOutputItem.bottom
                anchors.left: parent.left
                anchors.leftMargin: 3
            }


            Item {
                id: cameraCalibItem
                x: 0
                width: 350
                height: 30
                anchors.top: spliterGroupoutput.bottom
                anchors.topMargin: spacing

                Text {
                    id: cameraCalibText
                    x: 15
                    y: 12
                    color: textColor
                    text: qsTr("Camera Calib")
                    font.pixelSize: lblFont
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: leftMargin
                }

                FlatText {
                    id: cameraCalib
                    anchors.right: parent.right
                    anchors.rightMargin: rightMargin + 30
                    width: parent.width / 2 - 50
                    height: 30
                    //textFormat: Text.AutoText
                    anchors.verticalCenter: parent.verticalCenter
                    //color: "#4e8d15"
                    //wrapMode: Text.WordWrap
                }
                Rectangle{
                    width: 30
                    height: 30
                    z: 1
                    anchors.left: cameraCalib.right
                    anchors.verticalCenter: parent.verticalCenter
                    color: "#00000000"
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
                    color: "#4e8d15"
                    anchors.left: cameraCalib.right
                    anchors.verticalCenter: parent.verticalCenter
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
                    onClicked: calibFileDialoge.visible = true
                }
            }
            FileDialog{
                id:calibFileDialoge
                title: "Select configuration file"
                nameFilters: [ "Calib File (*.pac *.pts)", "All files (*)" ]
                selectMultiple: false

                onSelectionAccepted: {
                    var fileName = fileUrl.toString().substring(8); // Remove "file:///" prefix
                }
            }


            Item {
                id: outResolutionItem
                x: 0
                width: 350
                height: 30
                anchors.topMargin: spacing
                anchors.top: cameraCalibItem.bottom
                Text {
                    id: x_resolutionText
                    x: 15
                    y: 6
                    width: 75
                    height: 16
                    color: textColor
                    text: qsTr("X-Resolution")
                    font.pixelSize: 13
                    anchors.left: parent.left
                    anchors.leftMargin: leftMargin
                }

                FlatText {
                    id: panoXres
                    x: 106
                    y: 0
                    width: 50
                    height: 30
                }

                Text {
                    id: y_resolutionText
                    x: 188
                    y: 0
                    width: 75
                    height: 30
                    color: textColor
                    text: qsTr("Y-Resolution")
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: lblFont
                }

                FlatText {
                    id: panoYres
                    x: 279
                    y: 0
                    width: 50
                    height: 30
                }
            }

            Item {
                id: subgroupAudio
                x: 0
                width: 350
                height: 30
                anchors.topMargin: spacing
                anchors.top: outResolutionItem.bottom

                Text {
                    id: groupAudioText
                    x: 20
                    y: 6
                    color: textColor
                    text: qsTr("Audio")
                    //font.bold: true
                    font.pixelSize: groupFont
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: leftMargin
                }
            }

            Item {
                id: sampleFmtItem
                x: 0
                y: 206
                width: 350
                height: 30
                anchors.topMargin: 10
                anchors.top: subgroupAudio.bottom
                Text {
                    id: sampleFmtText
                    x: leftMargin
                    y: 12
                    color: textColor
                    text: qsTr("Sample Format")
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 13
                    anchors.left: parent.left
                    anchors.leftMargin: leftMargin
                }

                ComboBox {
                    id: sampleFmtCombo
                    width: parent.width * 0.5 - 20
                    height: 30
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: rightMargin
                    anchors.right: parent.right
                    model: ["U8 (unsigned 8 bits)",
                        "S16 (signed 16bits)",
                        "S32 (signed 32bits)",
                        "FLT (float)",
                        "DBL (double)",
                        "U8P (unsigned 8 bits, planar)",
                        "S16P (signed 16 bits)",
                        "S32P (signed 32 bits)",
                        "FLTP (float,planar)",
                        "DBLP (double,planar)"]
                }
            }

            Item {
                id: sampleRateItem
                x: 0
                width: 350
                height: 30
                anchors.top: sampleFmtItem.bottom
                anchors.topMargin: spacing
                Text {
                    id: sampleRateText
                    x: leftMargin
                    y: 12
                    color: textColor
                    text: qsTr("Sample Rate")
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 13

                }

                FlatText {
                    id: sampleRateFeild
                    width: parent.width * 0.5 - 20
                    height: 30
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: rightMargin
                    anchors.right: parent.right
                }
            }

            Item {
                id: lagItem
                x: 0
                width: 350
                height: 30
                anchors.top: sampleRateItem.bottom
                anchors.topMargin: spacing
                Text {
                    id: lagText
                    x: leftMargin
                    y: 12
                    color: textColor
                    text: qsTr("Lag")
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 13
                    anchors.left: parent.left
                    anchors.leftMargin: leftMargin
                }

                FlatText {
                    id: lagFeild
                    x: 174
                    width: parent.width * 0.5 - 20
                    height: 30
                    opacity: 0.8
                    anchors.verticalCenterOffset: 0
                    anchors.rightMargin: 21
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                }
            }


            Item {
                id: groupThreeItem
                x: 0
                width: 350
                height: 30
                anchors.topMargin: spacing
                anchors.top: lagItem.bottom

                Text {
                    y: 6
                    color: "#ffffff"
                    text: qsTr("Streaming")
                    font.pixelSize: groupFont
                    anchors.left: parent.left
                    anchors.leftMargin: leftMargin

                }
            }

            Item {
                id: modeStreamItem
                x: 2
                y: 358
                width: 350
                height: 30
                anchors.topMargin: 10
                anchors.top: groupThreeItem.bottom
                Text {
                    id: modeStreamText
                    x: leftMargin
                    y: 8
                    color: textColor
                    text: qsTr("Mode")
                    font.pixelSize: lblFont
                    anchors.left: parent.left
                    anchors.leftMargin: leftMargin

                }
                ComboBox {
                    id: modeStreamCombo
                    width: parent.width * 0.5 - 20
                    height: 30
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: rightMargin
                    anchors.right: parent.right
                    model: ["RMTP","Offline"]
                }
            }

            Item {
                id: urlItem
                x: 0
                y: 403
                width: 350
                height: 30
                anchors.top: modeStreamItem.bottom
                anchors.topMargin: spacing
                Text {
                    id: urlText
                    x: leftMargin
                    y: 8
                    color: textColor
                    text: qsTr("URL")
                    font.pixelSize: lblFont
                    anchors.left: parent.left
                    anchors.leftMargin: leftMargin
                }

                FlatText {
                    id: urlFeild
                    x: 176
                    y: 0
                    width: parent.width * 0.5 - 20
                    height: 30
                }
            }
        }
    }
    function refreshValues()
    {
        framePerSecondFeild.text = qmlMainWindow.getFps();
        panoXres.text = qmlMainWindow.getPanoXres();
        panoYres.text = qmlMainWindow.getPanoYres();
        xRes.text = qmlMainWindow.getXres();
        yRes.text = qmlMainWindow.getYres();
        sampleRateFeild.text = qmlMainWindow.getSampleRate();
        lagFeild.text = qmlMainWindow.getLag();
        captureTypeCombo.currentIndex = qmlMainWindow.getCaptureType();
        modeStreamCombo.currentIndex = qmlMainWindow.getStreamMode();
        urlFeild.text = qmlMainWindow.getStreamURL();
        deviceCountFeild.text = qmlMainWindow.getCameraCount();
        cameraCalib.text = qmlMainWindow.getCameraCalib()
        sampleFmtCombo.currentIndex = qmlMainWindow.getSampleFmt()
    }

    function setGlobalValues()
    {
       qmlMainWindow.setFps(framePerSecondFeild.text);
       qmlMainWindow.setPanoXres(panoXres.text);
       qmlMainWindow.setPanoYres(panoYres.text);
       qmlMainWindow.setXres(xRes.text);
       qmlMainWindow.setYres(yRes.text);
       qmlMainWindow.setSampleRate(sampleRateFeild.text);
       qmlMainWindow.setLag(lagFeild.text);
       qmlMainWindow.setCaptureType(captureTypeCombo.currentIndex);
       qmlMainWindow.setStreamMode(modeStreamCombo.currentIndex);
       qmlMainWindow.setStreamURL(urlFeild.text);
       qmlMainWindow.setCameraCount(deviceCountFeild.text);
       qmlMainWindow.setCaptureMode(captureModeCombo.currentIndex);
       qmlMainWindow.setCameraCalib(cameraCalib.text)
       qmlMainWindow.setSampleFmt(sampleFmtCombo.currentIndex)
    }
}
