import QtQuick 2.5

Item {
    width: 150
    height: 220

    property string iconPath: "path"
    property string title: "title"
    property  int  type: 0
    property bool isHovered: false

    Rectangle {
        id: hoveredRectangle
        z: 0
        width: parent.width
        height: parent.height
        color: "#0e3e64"
        visible: false
    }

    Rectangle {
        id: templateRectangle
        x: (parent.width - width) / 2
        y: (parent.height - height) / 4
        z: 1
        width: parent.width - 20
        height: parent.height - 50
        color: "#171717"
        border.color: "#AAAAAA"
        border.width: 1

        Image {
            id: canvasImage
            width: 60
            height: 60
            opacity: 0.8
            scale: 1
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            x: 10
            y: 10
            z: 2
            fillMode: Image.PreserveAspectFit
            source: iconPath
        }
/*
        Rectangle {
            id: blackRectangle
            x: 0
            y: 0
            width: 130
            height: 170
            color: "#AAAAAA"

            Rectangle {
                id: whiteRectangle
                width: parent.width - 2
                height: parent.height - 2
                border.color: "#AAAAAA"
                border.width: 0
                gradient: Gradient {
                    GradientStop {
                        position: 0
                        color: "#171717"
                    }

                    GradientStop {
                        position: 1
                        color: "#000000"
                    }
                }
                x: 1
                y: 1
                z: 1
            }
        }*/
    }

    Text {
        id: templateText
        x: (parent.width - width) / 2
        y: templateRectangle.y + templateRectangle.height + 5
        z: 1
        color: "#ffffff"
        text: qsTr(title)
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 16
    }

    MouseArea {
        id: mouseArea
        z: 2
        width: parent.width
        height: parent.height
        hoverEnabled: true
        onHoveredChanged: {
            isHovered = !isHovered

            if(isHovered) {
                hoveredRectangle.visible = true
            }
            else {
                hoveredRectangle.visible = false
            }
        }
        onClicked: onSetting()
    }
    function onSetting(){
        globalStereoState = true;
        isTemplate = true;
        qmlMainWindow.clearTempGlobalSettings();
        root.isTemplateCheck = true
        clearSelected()
        switch(type){
        case 1:
            root.setCurrentTitle(qsTr("Template(Camera)"))
            if(liveSettingbox.state == "collapsed")
            {
                liveSettingbox.state = "expanded"
                liveSettingbox.setCameraList();
            }

            break;
        case 2:
            root.setCurrentTitle(qsTr("Template(Video)"))
            if(videoSettingbox.state === "collapsed")
                videoSettingbox.state = "expanded"
             break;
        case 3:
            root.setCurrentTitle(qsTr("Template(Image)"))
            if(imageSettingbox.state === "collapsed")
                imageSettingbox.state = "expanded"
             break;
           default:
                break;
        }
    }
    function clearSelected(){
        forwardItem.visible = false;
        liveSettingbox.state = "collapsed"
        videoSettingbox.state = "collapsed"
        imageSettingbox.state = "collapsed"

    }
}
