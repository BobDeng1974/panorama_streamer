import QtQuick 2.0
import QtQuick.Dialogs 1.2

Item {
    width: 255
    height: 48
    property bool isHoveredCheck: false
    property bool isHoveredUncheck: false
    property bool isHoveredMore: false
    property bool isHover: false
    property bool isAdv: false

    Rectangle {
        id: backgroundRectangle
        width: parent.width
        height: parent.height
        color: "#1f1f1f"
    }

    Item{
        id: adsCtrl
        anchors.right: spliter.left
        anchors.rightMargin: 15
        width: 68
        height: 48
        visible: true
        Rectangle{
            id: exposureHoverRectangle
            width: parent.width
            height: parent.height
            color: "#353535"
            visible: false
        }

        Image {
			id: insertImage
            z: 1
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            width: 35
            height: 35
            fillMode: Image.PreserveAspectFit
            source: "../../resources/ico_ads.png"
        }

    }


    Item {
        id: spliter
        y: (parent.height - height) / 2
        width: 2
        height: parent.height - 20
        anchors.right: deleteItem.left
        anchors.rightMargin: 15
        Rectangle{
            color: "#1f1f1f"
            x: 1
            height: parent.height
            width: 1

        }
        Rectangle{
            color: "#3f3f3f"
            x: 2
            height: parent.height
            width: 1
        }
    }

    Item{
        id: deleteItem
        anchors.right: checkItem.left
        anchors.rightMargin: 15
        width: 68
        height: 48
        visible: true
        Rectangle{
            id: deleteHoverRectangle
            width: parent.width
            height: parent.height
            color: "#353535"
            visible: false
        }

         Image {
            id: deleteImage
            z: 1
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            width: 30
            height: 30
            fillMode: Image.PreserveAspectFit
            source: "../../resources/icon_delete.png"
        }

         MouseArea {
             z: 2
             anchors.fill: parent
             hoverEnabled: isHover
             onEntered: deleteHoverRectangle.visible = true;
             onExited: deleteHoverRectangle.visible = false;
             onClicked:
             {
                 isAdv = qmlMainWindow.isAdv();
                 if(!isAdv) return;
                 qmlMainWindow.enableAdv(false);
             }
         }

    }

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
            hoverEnabled: isHover
            onHoveredChanged: {
                isHoveredCheck = !isHoveredCheck
                if(isHoveredCheck)
                    checkHoveredRectangle.visible = true
                else
                    checkHoveredRectangle.visible = false

            }
//            onEntered: checkHoveredRectangle.visible = true;
//            onExited: checkHoveredRectangle.visible = false;
            onClicked:
            {
				if(isAdv) {
					qmlMainWindow.removeAdv();
					clearAds();
				}
                if(!sphericalView.adsEnded) return;
                fileDlg.open();
                sphericalView.adsEnded = false;
            }
        }

    }

    Item {
        id:　uncheckItem
        anchors.right: parent.right
        width:　68
        height: 48
        z: 2
        Rectangle {
            id: unCheckHoveredRectangle
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
            hoverEnabled: isHover
            onHoveredChanged: {
                isHoveredUncheck = !isHoveredUncheck
                if(isHoveredUncheck)
                {
                    unCheckHoveredRectangle.visible = true;
                }
                else
                    unCheckHoveredRectangle.visible = false
            }
//            onEntered: unCheckHoveredRectangle.visible = true;
//            onExited: unCheckHoveredRectangle.visible = false;
             onClicked:{
                 //sphericalView.clearAll();
                 if(isAdv){
                     qmlMainWindow.enableAdv(true);

                 }

                 clearAds();
             }
        }
    }

    function clearAds(){
        toolbox.clearSelected();
        toolbox.clearState();
        //adsCtrl.isHover = false;
        adsCtrl.state = "collapsed";
        adsItem.visible =  true;
        moreCtrl.visible = true;
        sphericalView.isAds = false;
        sphericalView.clearAll();
    }

    FileDialog {
        id: fileDlg
        title: "Open Advertisement image for video file"
        nameFilters: [ "Advertisment file (*.mp4 *.avi *.jpg *.png *.bmp)", "All files (*)" ]
        selectMultiple: true
        onSelectionAccepted: {
            clearAds();
            var isVideo = true;
            var adsPath = fileUrl.toString().substring(8); // Remove "file:///" prefix
            var pos = adsPath.lastIndexOf(".") + 1;
            var fileExt = adsPath.substring(pos);
            if(fileExt === "mp4" || fileExt === "avi"){
                isVideo = true;
            }
            else if( fileExt === "jpg" || fileExt === "png" || fileExt === "bmp"){
                isVideo = false;
            }

            qmlMainWindow.insertAdv(centralItem.width, centralItem.height, sphericalView.adPoints[0],sphericalView.adPoints[1],
                                    sphericalView.adPoints[2],sphericalView.adPoints[3],
                                    adsPath,isVideo);
        }
    }
}

