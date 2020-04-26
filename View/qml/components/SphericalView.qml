import QtQuick 2.5
import	MCQmlCameraView 1.0

Item {
    id: spherical
    width: centralItem.width
    height:　centralItem.height
    visible: false
    property int    cellWidthVal: 425
    property int    cellHeightVal: 425
    property var componentList : null
    property bool   isSeam: false
    property bool   isAds: false
    property bool   adsEnded: false
    property var    clickPos
    property var    releasedPos
    property var    movedPos: "0,0"
    property var    curPosX
    property var    curPosY
    property var    prePosX
    property var    prePosY
    property int    clickCnt: 0
    property var    ctxList: []
    property bool   isClicked: false
    property int    adIndex: -1
    //property var    adX: []
    //property var    adY: []
    property var    adPoints: []
    property int    delta: 20
    property bool   isEndPos: false

    Component.onCompleted: {
    }

    Component.onDestruction: {
        componentList = null;
    }

    Rectangle {
        id: backgroundRectangle
        color: "#000000"
        width: parent.width
        height: parent.height
        GridView {
            id: sphericalGridView
            anchors.fill: parent

            model: ListModel {
                id: gridModel
            }
        }
    }

    Rectangle {
        id : seamView
        width: parent.width
        height: parent.height
        opacity: 0.0
        z: 1
        MouseArea {
            id: mouseArea
            enabled: !isSeam
            anchors.fill: parent
            hoverEnabled: true
            onPressed: {
                clickPos  = mapToItem(parent, mouse.x,mouse.y);
                if(!isAds){
                    qmlMainWindow.onPressed(clickPos.x, clickPos.y);
                } else {
                    prePosX = clickPos.x;
                    prePosY = clickPos.y;
                    curPosX = clickPos.x;
                    curPosY = clickPos.y;
                    if(adIndex > 3){
                        return;
                    }

                    adIndex++;
    //                adX[adIndex] = clickPos.x;
    //                adY[adIndex] = clickPos.y;
                    adPoints[adIndex] = clickPos;
                }
            }

            onReleased: {
                releasedPos = mapToItem(parent, mouse.x,mouse.y);
                if (!isAds){
                    qmlMainWindow.onReleased(releasedPos.x, releasedPos.y);
                }else{
                    if(adIndex > 4) return;
                    curPosX = releasedPos.x;
                    curPosY = releasedPos.y;
                    clearLine(adIndex);
                    if(adIndex === 4){
                        if (isEndPos) {
                            curPosX = adPoints[0].x;
                            curPosY = adPoints[0].y;
                            //drawLine(adX[3],adY[3],curPosX,curPosY);
                            drawLine(adPoints[3].x,adPoints[3].y,curPosX,curPosY);
                            clearLine(adIndex - 1);
                            adsEnded = true;
                            return;
                        } else {
                            adIndex--;
                            clearLine(adIndex);
                        }
                    }

                   // drawLine(adX[adIndex],adY[adIndex],curPosX,curPosY);
                    drawLine(adPoints[adIndex].x,adPoints[adIndex].y,curPosX,curPosY);
                }
            }

            onPositionChanged: {
                movedPos  = mapToItem(parent,mouse.x,mouse.y);
                if(!isAds){
                    qmlMainWindow.onMoved(movedPos.x, movedPos.y);
                } else{
                    if(adIndex > 3) return;
                    if(adIndex === 3 &&  (Math.abs(adPoints[0].x - movedPos.x) < delta) && (Math.abs(adPoints[0].y - movedPos.y) < delta)){
                        isEndPos = true;
                    }else{
                        isEndPos = false;
                    }

                    clickPos = movedPos;
                    curPosX = clickPos.x;
                    curPosY = clickPos.y;
                    clearLine(adIndex);
                   // drawLine(adX[adIndex],adY[adIndex],curPosX,curPosY);
                    if(adIndex !== -1)
                        drawLine(adPoints[adIndex].x,adPoints[adIndex].y,curPosX,curPosY);
                }
            }

            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onClicked: {
                if (mouse.button == Qt.RightButton){
                    clearAll();
//                    if(adIndex === -1) return;
//                    for(var i = 0; i < adIndex+ 1; i ++){
//                        clearLine(i);
//                    }
//                    adIndex = -1;
                }
            }

        }
    }

    function createSphericalCameraWindow()
    {
        var component = Qt.createComponent("MCStitchCameraView.qml");
        if (component.status === Component.Ready) {
            var stitchViewObject = component.createObject(sphericalGridView, {"x": 0, "y": 0});
            if(stitchViewObject === null) {
            }else{
                qmlMainWindow.createStitchView(stitchViewObject.camView)
            }
        }
        else
            console.log(component.errorString())
    }

    function createSeamLabel(pos, index, isRight)
    {
		var isStereo = qmlMainWindow.isStereo();

        var frameWidth = centralItem.width;
        var frameHeight = centralItem.height;

        var panoWidth = qmlMainWindow.getTempPanoWidth();
		var panoH = qmlMainWindow.getTempPanoHeight();
        var panoHeight = panoH;		
		if (isStereo)
			panoHeight *= 2;

        var curWidth = frameWidth;
        var curHeight = frameHeight;

        if(panoHeight / panoWidth > frameHeight / frameWidth)
        {
            curHeight = centralItem.height;
            curWidth = curHeight * (panoWidth / panoHeight);
        }
        else
        {
            curWidth = centralItem.width;
            curHeight = curWidth * panoHeight / panoWidth;
        }
		var xOrg = (frameWidth - curWidth) / 2;
		var yOrg = (frameHeight - curHeight) / 2;
        
		var posList = pos.split(":");
        var xPos = (curWidth / panoWidth) * posList[0] + xOrg;
        var yPos = (curHeight / panoHeight) * posList[1] + yOrg;
		if (isStereo && isRight)
			yPos += curHeight/2;
        var component = Qt.createComponent("SeamLabel.qml");
        if (component.status === Component.Ready)
        {
            var seamLabel = component.createObject(spherical, {"x": xPos, "y": yPos,"z": 10,"seamIndex": index + 1});
			if (componentList == null)
				componentList = new Array();
            componentList.push(seamLabel);
        }
    }

    function clearSeamLabels()
    {
        for (var i = 0; i < componentList.length; i++)
        {
            var component = componentList[i];
			if (typeof(component.destroy) == "function")
				component.destroy();
        }
        //componentList = null;
    }

    function drawLine(startX,startY,endX,endY)
    {
        var component = Qt.createComponent("Line.qml");
        if (component.status === Component.Ready)
        {
            var line = component.createObject(spherical, {"prePosX": startX, "prePosY": startY, "curPosX": endX,"curPosY": endY});
            ctxList[adIndex] = line;
        }
    }

    function clearLine(index){
        if(index === -1) return;
        var component;
        if (ctxList.length > 0 && typeof(ctxList[index]) != "undefined" ) {
            //console.log("clearIndex->" + index + " " + typeof(ctxList[index].destroy));
            component = ctxList[index];
            if (typeof(component.destroy) == "function")
                component.destroy();
        }
    }

    function clearAll(){
        if(adIndex === -1) return;
        for(var i = 0; i < adIndex+ 1; i ++){
            clearLine(i);
        }
        adIndex = -1;
    }
}

