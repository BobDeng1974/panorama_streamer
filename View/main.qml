import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Extras 1.4
import QtQuick.Controls.Styles.Flat 1.0 as Flat
import "qml"
import "qml/components"
import "qml/controls"
import QtQuick.Controls 1.3
import MCQmlCameraView 1.0
import QmlInteractiveView 1.0
import QmlMainWindow 1.0
import QmlRecentDialog 1.0

ApplicationWindow {
    id: root
    width: 1280
    height: 720
    color: "#000000"
    visible: true
    flags: Qt.Window | Qt.FramelessWindowHint
	visibility: Window.Maximized
    property int recentsize: 0
    property bool m_bRecent: false
    property string title
    Component.onCompleted: {
        createRecentList();
    }

    property bool       isFullScreen: true
    property bool       isHoveredCheck: false
    property bool       isHoveredUnCheck: false
    property bool       isHoveredMore: false
    property bool       isHoveredMainMore: false
    property bool       isBackHovered: false
    property int        type: 0
    property int        curMode: 1
    property bool       menuLock: false
    property int        topMargin: 20
    property int        marginVal: 70
    property string     fileName:""
    //Others
    property int        recentIndex: 0
    property string     headerTitle: "PanoOne"
    property bool       isTemplateCheck: false
    property int        deleteIndex: 0
    property string     titleName
    property bool       isWait: true
    property bool       isLoaded: true
    property int        playMode: 1
    property bool       isTemplate: true
    property bool       globalStereoState: true
    property bool       isFirst: true
    property var        msg
    property var        camDrawViews
    property string     contentLabel
    QmlRecentDialog {
        id: recentDialog
    }

    QmlMainWindow {
        id:qmlMainWindow
        recDialog : recentDialog
		onElapsedTimeChanged:
		{
            statusItem.elapseText = qmlMainWindow.elapsedTime
		}
		onFpsChanged:
		{
            statusItem.fpsText = qmlMainWindow.fps
        }

		onExitChanged:
		{
			//root.close();
			Qt.quit();
		}

		onStarted:
		{
            maskID.visible = false;
            isWait = true;
		}

		onError:
		{
			maskID.visible = false;
            isWait = true;
            recent.visible = true;
        }

        onCalibratingFinished:
        {
			if (calibrated == true)
				busyLabel.text = "Calibrating succeed!";
			else
				busyLabel.text = "Calibrating failed!";
            calibTimer.restart()
        }

        onNotify:
        {
            msg = notifyMsg.split(":")
            if (!isWait && (msg[0] === "Error"))
            {

                isWait = true;
                maskID.visible = false;
                //recent.state = "expanded"
                //backItem.visible = false;
                //backItem.visible = true;
            }

			notification.state = "expanded"
            showNotify();
        }
    }

    Timer {
        id: resetTimer
        interval: 2000
        running: false
        repeat: false
        onTriggered:
        {
            if (notification.state === "expanded")
                notification.state = "collapsed";

        }
    }

    Timer {
        id: notifyTimer
        interval: 3000
        running: false
        repeat: false
        onTriggered:
        {
            if(notification.state === "expanded")
                notification.state = "collapsed";
        }
    }


    Timer {
        id: busyTimer
        interval: 100
        running: false
        repeat: false
        onTriggered: openIniProject()
    }

    Timer {
        id: calibTimer
        interval: 1000
        running: false
        repeat: false
        onTriggered: {
            maskID.visible = false;
            isLoaded = true;
        }
    }

    // File dialog for selecting image file from local file system
    FileDialog {
       id: fileOpenDialog
       title: "Select configuration file"
       nameFilters: [ "INI file (*.ini)", "All files (*)" ]
       selectMultiple: false

       onSelectionAccepted: {
           isTemplateCheck = false
           recent.state = "collapsed";
           liveView.visible = true;
           openCameraViews();
           busyTimer.restart();

       }
    }

    Header {
        id: headerItem
        width: parent.width
        height: 32
        z: 5

    }
    Text{
        id: currentName
        y: (32 - height) / 2
        z: 5
        anchors.left: parent.left
        anchors.leftMargin: 65
        text: qsTr(headerTitle);
        color: "#ffffff"
        font.pointSize: 14
    }

    Item {
        id: backItem
        z: 5
        width: 50
        height: 31
        visible: false

        Rectangle {
            id: backHoveredRectangle
            y: 1
            z: 1
            anchors.fill: parent
            color:"#1f1f1f"
            visible: false
        }

        Image {
            id: backImage
            width: 50
            height: 31
            z: 1
            fillMode: Image.PreserveAspectFit
            source: "resources/btn_back.PNG"
        }

        MouseArea {
            id: backMouseArea
            anchors.fill: parent
            z: 2
            hoverEnabled: true
            onHoveredChanged: {
                isBackHovered = !isBackHovered
                if(isBackHovered)
                {
                    backHoveredRectangle.visible = true
                    backHoveredRectangle.y = 1;
                }
                else
                    backHoveredRectangle.visible = false
            }
            onClicked: onBack()
        }
    }
    Item {
           id: forwardItem
           z: 6
           width: 50
           height: 31
           visible: false

           Rectangle {
               id: forwardHoveredRectangle
               y: 1
               z: 1
               //anchors.fill: parent
               width: parent.width
               height: parent.height
               color:"#1f1f1f"
               visible: false
           }

           Image {
               id: forwardImage
               width: 50
               height: 31
               z: 1
               fillMode: Image.PreserveAspectFit
               source: "resources/btn_forward.png"
           }

           MouseArea {
               id: forwardMouseArea
               anchors.fill: parent
               z: 2
               hoverEnabled: true
               onHoveredChanged: {
                   isBackHovered = !isBackHovered
                   if(isBackHovered)
                   {
                       forwardHoveredRectangle.visible = true;
                       forwardHoveredRectangle.y = 1;
                   }
                   else
                       forwardHoveredRectangle.visible = false;


               }
               onClicked: onForward()
           }
       }

    Rectangle {
        id: headerBottom2
        y: 32
        z: 2
        width: parent.width
        height: 48
        color: "#1f1f1f"

        Item{
            id: snapshotCtrl
            anchors.right: moreCtrl.left
            width: 68
            height: 48
            visible: true
            Rectangle{
                id: snapshotHoverRectangle
                width: parent.width
                height: parent.height
                color: "#353535"
                visible: false
            }
            MouseArea {
                x: snapshotHoverRectangle.x
                z: 2
                width: snapshotHoverRectangle.width
                height: snapshotHoverRectangle.height
                hoverEnabled: true
                onHoveredChanged: {
                    isHoveredMainMore = !isHoveredMainMore
                    if(isHoveredMainMore)
                        snapshotHoverRectangle.visible = true
                    else
                        snapshotHoverRectangle.visible = false
                }
                onClicked: {
                    toolbox.onStart();
                    snapshotBox.getSnapshotDir();
                    groupCtrl.state = "expanded";
                    snapshotCtrl.visible = false;
                }
            }
            Image {
                z: 1
                x: (parent.width - width) / 2
                y: (parent.height - height) / 2
                fillMode: Image.PreserveAspectFit
                source: "../resources/snapshot.png"
            }

        }

//        Item{
//            id: topBottomCtrl
//            anchors.right: anaglyphCtrl.left
//            width: 68
//            height: 48
//            visible: false
//            Rectangle{
//                id: topBottomHoverRectangle
//                width: parent.width
//                height: parent.height
//                color: "#353535"
//                visible: false
//            }
//            MouseArea {
//                x: topBottomHoverRectangle.x
//                z: 2
//                width: topBottomHoverRectangle.width
//                height: topBottomHoverRectangle.height
//                hoverEnabled: true
//                onHoveredChanged: {
//                }
//                onClicked: {
//                }
//            }
//            Image {
//                z: 1
//                x: (parent.width - width) / 2
//                y: (parent.height - height) / 2
//                fillMode: Image.PreserveAspectFit
//                source: "../resources/top_bottom.png"
//            }

//        }


//        Item{
//            id: anaglyphCtrl
//            anchors.right: moreCtrl.left
//            width: 68
//            height: 48
//            visible: false
//            Rectangle{
//                id: anaglyphHoverRectangle
//                width: parent.width
//                height: parent.height
//                color: "#353535"
//                visible: false
//            }
//            MouseArea {
//                x: anaglyphHoverRectangle.x
//                z: 2
//                width: anaglyphHoverRectangle.width
//                height: anaglyphHoverRectangle.height
//                hoverEnabled: true
//                onHoveredChanged: {
//                }
//                onClicked: {

//                }
//            }
//            Image {
//                z: 1
//                x: (parent.width - width) / 2
//                y: (parent.height - height) / 2
//                fillMode: Image.PreserveAspectFit
//                source: "../resources/anaglyph.png"
//            }

//        }

        Item{
            id: adsItem
            anchors.right: exposureCtrl.left
            width: 68
            height: 48
            visible: false
            Rectangle{
                id: adsHoverRectangle
                width: parent.width
                height: parent.height
                color: "#353535"
                visible: false
            }
            MouseArea {
                x: adsHoverRectangle.x
                z: 2
                width: adsHoverRectangle.width
                height: adsHoverRectangle.height
                hoverEnabled: true
                onEntered: adsHoverRectangle.visible = true;
                onExited: adsHoverRectangle.visible = false;
                onClicked: {
					var isAdv = qmlMainWindow.isAdv();
					if(isAdv){
					} else {
					}
                    toolbox.onStart();
                    adsCtrl.state = "expanded";
                    adsItem.visible = false;
                    exposureCtrl.visible = false;
                    moreCtrl.visible = false;
                    sphericalView.isAds = true;
					qmlMainWindow.resetCamSettings();
                }
            }
            Image {
				id: insertImage
                z: 1
                x: (parent.width - width) / 2
                y: (parent.height - height) / 2
                width: 35
                height: 35
                fillMode: Image.PreserveAspectFit
                source: "../resources/ico_ads.png"
            }
        }

        Item{
            id: exposureCtrl
            anchors.right: moreCtrl.left
            width: 68
            height: 48
            visible: false
            Rectangle{
                id: exposureHoverRectangle
                width: parent.width
                height: parent.height
                color: "#353535"
                visible: false
            }
            MouseArea {
                x: exposureHoverRectangle.x
                z: 2
                width: exposureHoverRectangle.width
                height: exposureHoverRectangle.height
                hoverEnabled: true
                onEntered: exposureHoverRectangle.visible = true;
                onExited: exposureHoverRectangle.visible = false;
                onClicked: {
                    toolbox.onStart();
                    qmlMainWindow.onCalculatorGain();
                    exposureGroup.state = "expanded";
                    exposureCtrl.visible = false;
                    adsItem.visible = false;

                }
            }
            Image {
                z: 1
                x: (parent.width - width) / 2
                y: (parent.height - height) / 2
                fillMode: Image.PreserveAspectFit
                source: "../resources/ico_exposure.png"
            }

        }


        Item{
            id: moreCtrl
            anchors.right: parent.right
            width: 48
            height: 48
            visible: true
            Rectangle{
                id: moreHoverRectangle
                width: moreCtrl.width
                height: moreCtrl.height
                color: "#353535"
                visible: false
            }
            MouseArea {
                x: moreHoverRectangle.x
                z: 2
                width: moreHoverRectangle.width
                height: moreHoverRectangle.height
                hoverEnabled: true
                onHoveredChanged: {
                    isHoveredMainMore = !isHoveredMainMore
                    if(isHoveredMainMore)
                        moreHoverRectangle.visible = true
                    else
                        moreHoverRectangle.visible = false
                }
                onClicked: onPressedMainMore()
            }
            Image {
                z: 1
                fillMode: Image.PreserveAspectFit
                source: "../resources/more_control.png"
            }

        }

        Item {
            id: saveCtrl
            anchors.right: parent.right
            width: 230
            height: 48
            visible: false

            Item {
                id: applyItem
                anchors.right: cancelItem.left
                width: 68
                height: 48
                z: 2
                Rectangle {
                    id: applyHoveredRectangle
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
                    source: "../../resources/check.png"
                }
                MouseArea {
                    z: 2
                    anchors.fill: parent
                    hoverEnabled: true
                    onHoveredChanged: {
                        isHoveredCheck = !isHoveredCheck
                        if(isHoveredCheck)
                            applyHoveredRectangle.visible = true
                        else
                            applyHoveredRectangle.visible = false

                    }
                    onClicked:  {

                        if(root.isTemplateCheck)
                        {
                            toolbox.saveIniFile()
                        }else{
                            qmlMainWindow.saveIniPath(qsTr(""))
                        }
                        saveCtrl.visible = false;
                        toolbox.clearSelected();
                        toolbox.clearState();
                    }
                }

            }

            Item {
                id:　cancelItem
                anchors.right: parent.right
                width:　68
                height: 48
                z: 2
                Rectangle {
                    id: cancelHoveredRectangle
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
                    source: "../../resources/uncheck.png"
                }
                MouseArea {
                    z: 2
                    anchors.fill: parent
                    hoverEnabled: true
                    onHoveredChanged: {
                        isHoveredUnCheck = !isHoveredUnCheck
                        if(isHoveredUnCheck)
                            cancelHoveredRectangle.visible = true
                        else
                            cancelHoveredRectangle.visible = false
                    }
                     onClicked: {
                         saveCtrl.visible = false;
                         toolbox.clearSelected();
                         toolbox.clearState();
                     }
                }
            }
        }

        GroupCtrl {
            id: groupCtrl
            anchors.right: parent.right
            anchors.rightMargin: - width
            x: moreCtrl.x
            width: 138
            height: 48
            state: "collapsed"

            states: [
                State {
                    name: "expanded"
                    PropertyChanges {
                        target: groupCtrl
                        width: 0
                        visible: true
                        isHover: true
                    }
                },
                State {
                    name: "collapsed"
                    PropertyChanges {
                        target: groupCtrl
                        width: 138
                        visible: false
                        isHover: false
                    }
                }
            ]

            transitions: [
                Transition {
                    NumberAnimation { target: groupCtrl; property: "width"; duration: 300 }
                    NumberAnimation { target: groupCtrl; property: "opacity"; duration: 300 }
                    NumberAnimation { target: groupCtrl; property: "visible"; duration: 300 }
                    NumberAnimation { target: groupCtrl; property: "isHover"; duration: 300 }
                }
            ]

        }

        ExposureCtrl {
            id: exposureGroup
            anchors.right: parent.right
            anchors.rightMargin: - width
            x: moreCtrl.x
            width: 138
            height: 48
            state: "collapsed"

            states: [
                State {
                    name: "expanded"
                    PropertyChanges {
                        target: exposureGroup
                        width: 0
                        visible: true
                        isHover: true
                    }
                },
                State {
                    name: "collapsed"
                    PropertyChanges {
                        target: exposureGroup
                        width: 138
                        visible: false
                        isHover: false
                    }
                }
            ]

            transitions: [
                Transition {
                    NumberAnimation { target: exposureGroup; property: "width"; duration: 300 }
                    NumberAnimation { target: exposureGroup; property: "opacity"; duration: 300 }
                    NumberAnimation { target: exposureGroup; property: "visible"; duration: 300 }
                    NumberAnimation { target: exposureGroup; property: "isHover"; duration: 300 }
                }
            ]

        }

        AdvertisementCtrl {
            id: adsCtrl
            anchors.right: parent.right
            anchors.rightMargin: - width
            x: moreCtrl.x
            width: 138
            height: 48
            state: "collapsed"

            states: [
                State {
                    name: "expanded"
                    PropertyChanges {
                        target: adsCtrl
                        width: 0
                        visible: true
                        isHover: true
                    }
                },
                State {
                    name: "collapsed"
                    PropertyChanges {
                        target: adsCtrl
                        width: 138
                        visible: false
                        isHover: false
                    }
                }
            ]

            transitions: [
                Transition {
                    NumberAnimation { target: adsCtrl; property: "width"; duration: 300 }
                    NumberAnimation { target: adsCtrl; property: "opacity"; duration: 300 }
                    NumberAnimation { target: adsCtrl; property: "visible"; duration: 300 }
                    NumberAnimation { target: adsCtrl; property: "isHover"; duration: 300 }
                }
            ]

        }




    }

    Notification {
        id: notification
        anchors.top: headerItem.bottom
        width: 360
        height: 80
        z: 10
        state: "collapsed"
        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: notification
                    x: root.width - 360

                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: notification
                    x: root.width
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: notification; property: "x"; duration: 300 }
            }
        ]
    }

    /*************************************central layout**********************************/
    Item {
        id: centralItem
        width: parent.width - 50
        height: parent.height - headerItem.height - headerBottom2.height - statusItem.height
        x: 50
        y: headerItem.height + headerBottom2.height
        z: 1

        focus: true
        Keys.onPressed: {
            if(event.key ===Qt.Key_Up)
                qmlMainWindow.receiveKeyIndex(1)
            else if(event.key === Qt.Key_Down)
                qmlMainWindow.receiveKeyIndex(-1)
            statusItem.levelText = qmlMainWindow.level()        }

        LiveView{
            id: liveView
            visible: true
            state: "show"

            states: [
                State {
                    name: "show"
                    PropertyChanges {
                        target: liveView
                        visible: true
                    }
                },
                State {
                    name: "hidden"
                    PropertyChanges {
                        target: liveView
                        visible: false
                    }
                }
            ]

        }




        SphericalView {
            id: sphericalView
            visible: false
            state: "hidden"
            states: [
                State {
                    name: "show"
                    PropertyChanges {
                        target: sphericalView
                        visible: true
                    }
                },
                State {
                    name: "hidden"
                    PropertyChanges {
                        target: sphericalView
                        visible: false
                    }
                }
            ]
        }

        InteractiveView {
            id: interactiveView
            visible: false
            state: "hidden"
            states: [
                State {
                    name: "show"
                    PropertyChanges {
                        target: interactiveView
                        visible: true
                    }
                },
                State {
                    name: "hidden"
                    PropertyChanges {
                        target: interactiveView
                        visible: false
                    }
                }
            ]
        }
    }


    /*************************************status layout**********************************/

    Status {
        id:statusItem
        y:parent.height - 65
        width: parent.width
        height: 65
        z: 2
    }
    //ExposureDialog
    ExposureDialog{
        id: exposureDialog
        y:  (root.height - height) / 2
        x:  (root.width - width) / 2
        z:  10
    }
    /************************************************************************************/

    Recent {
        id: recent
        x: 0
        y: 32
        z: 3
        width: parent.width
        height: parent.height - 32

        state: "expanded"

        states: [
            State {
                name: "collapsed"
                PropertyChanges {
                    target: recent
                    visible: false
                }
            },
            State {
                name: "expanded"
                PropertyChanges {
                    target: recent
                    visible: true
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: recent; property: "width"; duration: 100 }
                NumberAnimation { target: recent; property: "opacity"; duration: 100 }
            }
        ]
    }


    Toolbox {
        id: toolbox
        height: root.height - 32
        x: 0
        y: 32
        z: 2

        state: "collapsed"

        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: toolbox
                    width: 250
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: toolbox
                    width: 51
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: toolbox; property: "width"; duration: 300}
                NumberAnimation { target: toolbox; property: "opacity"; duration: 300 }
            }
        ]
    }

    Livebox {
        id: liveBox
        x: parent.width - width
        y: 32
        z: 2

        height: root.height - y

        state: "collapsed"

        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: liveBox
                    width: 350
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: liveBox
                    width: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: liveBox; property: "width"; duration: 100 }
                NumberAnimation { target: liveBox; property: "opacity"; duration: 100 }
            }
        ]
    }

    Sphericalbox {
        id: sphericalBox
        x: parent.width - width
        y: 32
        z: 3

        height: root.height - y

        state: "collapsed"

        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: sphericalBox
                    width: 350
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: sphericalBox
                    width: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: sphericalBox; property: "width"; duration: 100 }
                NumberAnimation { target: sphericalBox; property: "opacity"; duration: 100 }
            }
        ]
    }

    Interactivebox {
        id: interactiveBox
        x: parent.width - width
        y: 32
        z: 4

        height: root.height - y

        state: "collapsed"

        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: interactiveBox
                    width: 350
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: interactiveBox
                    width: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: interactiveBox; property: "width"; duration: 100 }
                NumberAnimation { target: interactiveBox; property: "opacity"; duration: 100 }
            }
        ]
    }

    Snapshotbox {
        id: snapshotBox
        x: parent.width - width
        y: 32
        z: 5

        height: root.height - y

        state: "collapsed"

        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: snapshotBox
                    width: 350
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: snapshotBox
                    width: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: snapshotBox; property: "width"; duration: 100 }
                NumberAnimation { target: snapshotBox; property: "opacity"; duration: 100 }
            }
        ]
    }

    Exposurebox {
        id: exposureBox
        x: parent.width - width
        y: 32
        z: 6

        height: root.height - y

        state: "collapsed"

        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: exposureBox
                    width: 350
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: exposureBox
                    width: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: exposureBox; property: "width"; duration: 100 }
                NumberAnimation { target: exposureBox; property: "opacity"; duration: 100 }
            }
        ]
    }

    Topbottombox {
        id: topBottomBox
        x: parent.width - width
        y: 32
        z: 7

        height: root.height - y

        state: "collapsed"

        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: topBottomBox
                    width: 250
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: topBottomBox
                    width: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: topBottomBox; property: "width"; duration: 100 }
                NumberAnimation { target: topBottomBox; property: "opacity"; duration: 100 }
            }
        ]
    }

    Anaglyphbox {
        id: anaglyphBox
        x: parent.width - width
        y: 32
        z: 8

        height: root.height - y

        state: "collapsed"

        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: anaglyphBox
                    width: 250
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: anaglyphBox
                    width: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: anaglyphBox; property: "width"; duration: 100 }
                NumberAnimation { target: anaglyphBox; property: "opacity"; duration: 100 }
            }
        ]
    }

    NotificationHistory {
        id: notifyHistorybox
        x: parent.width - width
        y: 32
        z: 2

        height: root.height - y

        state: "collapsed"

        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: notifyHistorybox
                    //width: 350
                    x: root.width - 350
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: notifyHistorybox
                    //width: 0
                    x: root.width
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: notifyHistorybox; property: "x"; duration: 100 }
                NumberAnimation { target: notifyHistorybox; property: "opacity"; duration: 100 }
            }
        ]
    }

    Savebox {
        id: saveBox
        x: parent.width - width
        y: 32
        z: 9

        height: root.height - y

        state: "collapsed"

        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: saveBox
                    width: 350
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: saveBox
                    width: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: saveBox; property: "width"; duration: 100 }
                NumberAnimation { target: saveBox; property: "opacity"; duration: 100 }
            }
        ]
    }

    VideoSettingbox {
        id: videoSettingbox
        x: parent.width - width
        y: headerItem.height
        z: 3
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
        y: headerItem.height
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
        y: headerItem.height
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

    Settingsbox {
        id: settingsBox
        x: parent.width - width
        y: 32
        z: 10

        height: root.height - y

        state: "collapsed"

        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: settingsBox
                    width: 350
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: settingsBox
                    width: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: settingsBox; property: "width"; duration: 100 }
                NumberAnimation { target: settingsBox; property: "opacity"; duration: 100 }
            }
        ]
    }

    Helpbox {
        id: helpBox
        x: parent.width - width
        y: 32
        z: 10

        height: root.height - y

        state: "collapsed"

        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: helpBox
                    width: 250
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: helpBox
                    width: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: helpBox; property: "width"; duration: 100 }
                NumberAnimation { target: helpBox; property: "opacity"; duration: 100 }
            }
        ]
    }

    Aboutbox {
        id: aboutBox
        x: parent.width - width
        y: 32
        z: 11

        height: root.height - y

        state: "collapsed"

        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: aboutBox
                    width: 250
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: aboutBox
                    width: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: aboutBox; property: "width"; duration: 100 }
                NumberAnimation { target: aboutBox; property: "opacity"; duration: 100 }
            }
        ]
    }

    Rectangle{
        id: maskID
        anchors.fill: parent
        y:headerItem.height
        height:parent.height - headerItem.height
        width: parent.width
        visible: false
        color: "#171717"
        opacity: 0.7
        z:12
        MouseArea {
           id: mouseArea1
           anchors.fill: parent
           hoverEnabled: true
        }

        Label {
           id: busyLabel
           z: 13
           y: parent.height / 2 + 30
           text: "Starting..."
           width: Math.min(maskID.width, maskID.height) * 0.8
           height: font.pixelSize
           anchors.horizontalCenter: parent.horizontalCenter
           renderType: Text.QtRendering
           font.pixelSize: 30
           horizontalAlignment: Text.AlignHCenter
           fontSizeMode: Text.Fit
           font.family: Flat.FlatStyle.fontFamily
           font.weight: Font.Light
           color: "#ffffff"
       }
    }

    BusyIndicator {
        running: !isWait
        anchors.centerIn: parent
        z:13
    }

    BusyIndicator {
        running: !isLoaded
        anchors.centerIn: parent
        z: 13
    }

    function showNotify()
    {
        if (msg[0] === "Error")
        {
            notification.imagePath = "../../resources/ico_error.png"
            notification.typeText = "Error"
        }
        else if(msg[0] === "Warning")
        {
            notification.imagePath = "../../resources/ico_warning.png"
            notification.typeText = "Warning"
        }
        else if(msg[0] === "Information")
        {
            notification.imagePath = "../../resources/ico_notify.png"
            notification.typeText = "Information"
        }

        notification.contentText = msg[1];
        notifyTimer.restart();
    }

    function openCameraViews() {
        maskID.visible = true
        busyLabel.text = "Starting...";
        isWait = false
    }

    function onCalibrate()
    {
        maskID.visible = true;
        busyLabel.text = "Calibrating...";
        isLoaded = false;
    }

    function onPressedCheck() {
        menuLock = false;
        toolbox.clearSelected();
        switch (curMode)
        {
        case 1:
            groupCtrl.state = "collapsed"
            snapshotCtrl.visible = true;
            qmlMainWindow.snapshotFrame();
            break;
        case 2:
//            topBottomCtrl.visible = true;
//            groupCtrl.visible = false;
            anaglyphCtrl.visible  = true;
            break;

        }
        toolbox.clearState();
    }

    function onPressedUnCheck() {
        menuLock = false;
        toolbox.clearSelected();
        toolbox.clearState();
        switch (curMode)
        {
        case 1:
            groupCtrl.state = "collapsed"
            groupCtrl.isHover = false;
            snapshotCtrl.visible = true;
            break;
        case 2:
//            topBottomCtrl.visible = true;
            groupCtrl.visible = false;
            //anaglyphCtrl.visible  = true;
            break;

        }
    }

    function onPressedMore() {
        toolbox.showSpecifyBox()
    }
    function onPressedMainMore(){
        toolbox.showMainBox()

    }

    function onBack() {
        toolbox.clearSelected();
        toolbox.onStart();
        backItem.visible = false;
        forwardItem.visible = true;
        recent.state = "expanded";
    }
    function onForward() {
        toolbox.clearSelected()
        toolbox.clearState();
        recent.state = "collapsed"
        backItem.visible = true
        forwardItem.visible = false
    }

    function showBack()
    {
        backItem.visible = true;
        forwardItem.visible = false;
    }

    function clearBackForward()
    {
        backItem.visible = false;
        forwardItem.visible = false;
    }

    function onFileOpen()
    {
        fileOpenDialog.open()
    }
    function createRecentList()
    {
        m_bRecent = qmlMainWindow.openRecentMgrToINI()
        if(m_bRecent)
        {
            recentsize = qmlMainWindow.getRecentCnt()
            if(recentsize > 0)
            {
                for( var i = 0; i < recentsize; i++)
                {
                    qmlMainWindow.sendRecentIndex(i)
                    recent.setCurrentFile()
                }
            }
        }
    }
    function setCurrentTitle(fileName){
        currentName.text = fileName + " - PanoOne";
    }

    function openIniProject()
    {
        recent.state = "collapsed";
        var fileName = fileOpenDialog.fileUrl.toString().substring(8); // Remove "file:///" prefix
        clearMainSetting();
        deleteIndex = qmlMainWindow.openIniPath(fileName)
        clearCameras();
        qmlMainWindow.initProject();
        if (deleteIndex == -2) return;
        if(deleteIndex >= 0)
        {
            recent.deleteRecentListItem()
            recent.setCurrentFile()
        }
        clearCameraSettings();
        clearMainView();
        qmlMainWindow.openProject();
        titleName = qmlMainWindow.getRecentTitle() + ".ini"
        setCurrentTitle(titleName)
    }

    function clearMainSetting()
    {
        backItem.visible = true;
        forwardItem.visible = false;
        toolbox.clearSettingBox();
        toolbox.clearSelected();
    }

    function clearCameras()
    {
        qmlMainWindow.deleteCameraView();
        qmlMainWindow.deleteStitchView();
        qmlMainWindow.deleteInteractView();
    }

    function clearCameraSettings()
    {
        sphericalBox.clearCombo();
        sphericalBox.getBlendValues();
        sphericalBox.createBlendViewWindow();
        settingsBox.refreshValues();
    }

    function clearMainView()
    {
        liveView.createCameraWindow();
        sphericalView.createSphericalCameraWindow();
        sphericalView.visible = false;
        interactiveView.createInteractiveCameraWindow();
        interactiveView.visible = false;
    }

}

