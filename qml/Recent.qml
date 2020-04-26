import QtQuick 2.5
import QtQuick.Dialogs 1.2
import QtQuick.Extras 1.4
import "components"
import QmlRecentDialog 1.0

Item {
    id: recent
    width: 1030
    height: 688

    property bool       isHovered: false
    property int        minWidth: 1080
    property int        minHeight: 720
    property string     imagepath: ""
    

    Text {
        id: recentTitleText
        x: 16
        y: 12
        color: "#ffffff"
        text: qsTr("Recent")
        z: 2
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 22
    }

    Rectangle {
        id: titleRectangle
        width: parent.width
        height: 48
        color: "#1f1f1f"
        z: 1
    }

    Item {
        id: recentListItem
        width: 350
        height: parent.height
        z: 1
        Rectangle {
            id: recentListRectangle
            width: parent.width
            height: parent.height
            color: "#171717"
        }

        ListView {
            id: recentListView
            x: recentTitleText.x
            y: recentTitleText.y + recentTitleText.height + 20
            width: parent.width - recentTitleText.x
            height: 58 * 10
            z: 1
            spacing: 10
            model: ListModel {
                id: recentList

            }
            delegate: Item {
                x: 5
                width: parent.width
                height: 50
                Row {
                    RecentListitem {
                        title: titleText
                        iconPath: imagePath
                        path: sourcePath
//                        fullPath: qmlMainWindow.getRecentFullPath()
                    }
                }
            }

            remove: Transition {
                   ParallelAnimation {
                       NumberAnimation { properties: "x"; to: -500; duration: 300 }
                   }
            }
        }


        Rectangle {
            id: spliterRectangle
            x: 10
            z: 2
            width: parent.width - 20
            height: 3
            anchors {
                top:recentListView.bottom
                topMargin: 5
            }

            color: "#1f1f1f"
        }

        Item {
            id: openItem
            x: recentTitleText.x
            z: 2
            anchors {
                top: spliterRectangle.bottom
                topMargin: 5
            }

            width: parent.width - x
            height: 40

            Rectangle {
                id: openhoveredRectangle
                x: openText.x - 1
                y: openText.y + openText.height + 2
                z: 0
                width: openText.width + 2
                height: 1
                color: "#ffffff"
                visible: false
            }

            Image {
                id: openIconImage
                x: 19
                y: (parent.height - height) / 2
                z: 1
                width: 28
                height: 28
                fillMode: Image.PreserveAspectFit
                source: "../resources/icon_open.png"
            }

            Text {
                id: openText
                anchors {
                    left: openIconImage.right
                    leftMargin: 10
                }

                y: (parent.height - height) / 2
                z: 1
                color: "#ffffff"
                text: qsTr("Open Project")
                font.pixelSize: 14
            }

            MouseArea {
                id: mouseArea
                x: 0
                z: 2
                width: parent.width
                height: parent.height
                antialiasing: true
                hoverEnabled: true
                onHoveredChanged: {
                    isHovered = !isHovered

                    if(isHovered) {
                        openhoveredRectangle.visible = true
                    }
                    else {
                        openhoveredRectangle.visible = false
                    }
                }
                onClicked: {
                    root.onFileOpen()
                }
            }
        }
    }

    Item {
        id: templateItem
        width: parent.width - recentListItem.width
        height: parent.height
        x: recentListItem.width

        Rectangle {
            id: templateRectangle
            width: parent.width
            height: parent.height
            color: "#1f1f1f"

            GridView {
                id: templateGridView
                x: 43
                y: recentListView.y
                width: parent.width - x
                height: parent.height - y
                contentWidth: 0
                cellHeight: 220
                model: ListModel {
                    ListElement {
                        titleText: "Live Camera"
                        //imagePath:"../resources/icon_camera.png"
                        imagePath:"../resources/icon_tempCamera.png"
                        selectType: 1

                    }

                    ListElement {
                        titleText: "Video"
						imagePath: "../resources/icon_video.png"
                        //imagePath: "../resources/icon_temp_video.png"
                        selectType: 2
                    }

                    ListElement {
                        titleText: "Frame Sequence"
                        imagePath: "../resources/icon_image.png"
                        selectType: 3
                    }
                }
                delegate: Item {
                    x: 5
                    width: 60
                    height: 320
                    Column {
                        RecentGriditem {
                            title: titleText
                            iconPath: imagePath
                            type: selectType
                        }
                    }
                }
                cellWidth: 200
            }
        }
    }

    MouseArea {
        id: resizeMouseArea
        width: 5
        height: 5
        y: parent.height - height
        x: parent.width - width

        hoverEnabled: true
        onHoveredChanged: {
            if(!isFullScreen)
                cursorShape = Qt.SizeFDiagCursor
        }

        property bool isMoveMode: false
        property var clickPos: "1,1"
        property real movedX: 0
        property real movedY: 0

        onPressed: {
            if(mouse.button == Qt.LeftButton && !isFullScreen) {
                isMoveMode = true
                clickPos  = mapToItem(statusItem.parent, mouse.x,mouse.y)
            }
        }

        onPositionChanged: {
            if(isMoveMode) {
                var x = mouse.x
                var y = mouse.y
                var parentPos = mapToItem(statusItem.parent, x, y)
                var delta = Qt.point(parentPos.x-clickPos.x - movedX, parentPos.y-clickPos.y - movedY)
                if (root.width + delta.x >= minWidth) {
                    movedX += delta.x
                    root.width += delta.x
                }
                if (root.height + delta.y >= minHeight) {
                    movedY += delta.y
                    root.height += delta.y
                }
            }
        }

        onReleased: {
            if(mouse.button == Qt.LeftButton) {
                isMoveMode = false
                movedX = 0
                movedY = 0
            }
        }
    }

    Rectangle {
        id: resizeLine
        width: 30
        height: 2
        x: parent.width - width + 4
        y: parent.height - height - 3
        z: 5
        color: "#555555"
        rotation: -45
    }

    Rectangle {
        id: resizeLine2
        width: 20
        height: 2
        x: parent.width - width + 4
        y: parent.height - height - 3
        z: 5
        color: "#555555"
        rotation: -45
    }

    VideoSettingbox {
        id: videoSettingbox
        x: parent.width - width
        z: 2
        height: parent.height
        state: "collapsed"
        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: videoSettingbox
                    width: root.width
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: videoSettingbox
                    width: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: videoSettingbox; property: "width"; duration: 100 }
                NumberAnimation { target: videoSettingbox; property: "opacity"; duration: 100 }
            }
        ]
    }

    ImageSettingbox {
        id: imageSettingbox
        x: parent.width - width
        z: 2
        height: parent.height
        state: "collapsed"
        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: imageSettingbox
                    width: root.width
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: imageSettingbox
                    width: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: imageSettingbox; property: "width"; duration: 100 }
                NumberAnimation { target: imageSettingbox; property: "opacity"; duration: 100 }
            }
        ]


    }

    LiveSettingbox {
        id: liveSettingbox
        x: parent.width - width
        z: 2
        height: parent.height
        state: "collapsed"
        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: liveSettingbox
                    width: root.width

                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: liveSettingbox
                    width: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: liveSettingbox; property: "width"; duration: 100 }
                NumberAnimation { target: liveSettingbox; property: "opacity"; duration: 100 }
            }
        ]

    }
    function setCurrentFile(){
        switch(qmlMainWindow.getCaptureType())
        {
            case 0:
            case 1:
                imagepath = "../resources/icon_camera_small.png"
                break;
            case 2:
                imagepath = "../resources/icon_video_small.png"
                break;
            case 3:
                imagepath = "../resources/icon_image_small.png"
                break;
            default:
                break;
        }
        recentList.insert(0,{"titleText": qmlMainWindow.getRecentTitle(),"imagePath": imagepath,
                              "sourcePath": qmlMainWindow.getRecentPath()})

        var listCount = recentList.count;
//        if(listCount > 11)
//            recentList.remove(listCount - 1);
        qmlMainWindow.saveRecentMgrToINI()
    }

    function deleteRecentListItem()
    {
        var dindex = recentList.count - root.deleteIndex - 1
        recentList.remove(dindex)
    }
}
