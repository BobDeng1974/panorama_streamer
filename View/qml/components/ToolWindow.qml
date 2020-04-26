import QtQuick 2.4
import QtQuick.Controls 1.4

FloatingWindow {
    //windowMenuGroup: 'tool'
    property int        uiSpacing: 4
    property string     toolWindowTextID
    property bool       destroyOnClose: true
    property bool       isHovered: false
    detachable:         true
    signal closing()

    handle: Rectangle {
        color: "#000000"
        anchors.margins: 4
        anchors.centerIn: parent.Center
        height: 30

        Text {
            y: (30 - height) / 2
            color: "#ffffff"
            text: qsTr("Details")
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.left: parent.left
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 20
        }


        Row {
            id: r1
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            spacing: uiSpacing
//            Image {
//                id: img1
//                source: "/resources/icon-minimize.png"
//                width:  handle.height * 0.7
//                height: handle.height * 0.7
//                anchors.verticalCenter: parent.verticalCenter
//                MouseArea {
//                    anchors.fill: parent
//                    onClicked: toggleMinimized()
//                }
//            }
//            Image {
//                id: img2
//                source: "/resources/icon-expand-black.png"
//                width: handle.height * 0.7
//                height: handle.height * 0.7
//                anchors.verticalCenter: parent.verticalCenter
//                MouseArea {
//                    anchors.fill: parent
//                    onClicked: toggleMaximized()
//                }
//            }
            Image {
                id: img3
                z: 1
                source: "/resources/icon-close-black.png"
                width: 45
                height: 30
                fillMode: Image.PreserveAspectFit
                anchors.verticalCenter: parent.verticalCenter

                Rectangle {
                    id: closeHoverRetangle
                    z: -1
                    width: 45
                    height: 30
                    color: "#c75050"
                    visible: false
                }
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
//                    onHoveredChanged: {
//                        isHovered = !isHovered
//                        if(isHovered)
//                            closeHoverRetangle.visible = true;
//                        else
//                            closeHoverRetangle.visible = false;


//                    }

                    onEntered: closeHoverRetangle.visible = true;
                    onExited: closeHoverRetangle.visible = false;

                    onClicked: {
                        details.visible = false;
                        clearTab();
                    }
                }
            }
        }
    }


}

