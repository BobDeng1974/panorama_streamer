import QtQuick 2.0

Item {
    width: 255
    height: 48
    property bool isHoveredCheck: false
    property bool isHoveredUncheck: false
    property bool isHoveredMore: false
    property bool isHover: false

    Rectangle {
        id: backgroundRectangle
        width: parent.width
        height: parent.height
        color: "#1f1f1f"
    }

    Item{
        id: snapshotCtrl
        anchors.right: spliter.left
        anchors.rightMargin: 15
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

        Image {
            z: 1
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            fillMode: Image.PreserveAspectFit
            source: "../../resources/snapshot.png"
        }

    }

    Item {
        id: spliter
        y: (parent.height - height) / 2
        width: 2
        height: parent.height - 20
        anchors.right: checkItem.left
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
            onClicked:
            {
                onPressedCheck()
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
//                isHoveredCheck = true;
                isHoveredUncheck = !isHoveredUncheck
                if(isHoveredUncheck)
                {
                    unCheckHoveredRectangle.visible = true;
                }
                else
                    unCheckHoveredRectangle.visible = false
            }
             onClicked: onPressedUnCheck();
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
            hoverEnabled: isHover
            onHoveredChanged: {
//                isHoveredCheck = true;
                isHoveredMore = !isHoveredMore
                if(isHoveredMore)
                {
                    moreHoveredRectangle.visible = true
                }
                else
                    moreHoveredRectangle.visible = false
            }
             onClicked: onPressedMore();
        }


    }

    function clearHover()
    {
        isHoveredCheck = false;
        isHoveredUncheck = false;
        isHoveredMore = false;
        checkHoveredRectangle.visible = false;
        unCheckHoveredRectangle.visible = false;
        moreHoveredRectangle.visible = false;
    }


}

