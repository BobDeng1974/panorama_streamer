import QtQuick 2.5
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import "components"
Item {
    id: toolBox
    width : 250
    height: 684
    opacity: 1

    property bool isHovered : false
    property bool isSelected: false
    property int hoverType:0
    property int heightVal: 50
    property int clickCnt: 0
    property int menuHight: root.height - headerItem.height - headerBottom2.height
    Component.onCompleted: {
        startImage.visible = true;
    }

    onWidthChanged: {
        if(width == 250){
            titleGroup.y = 1
        }else if(width == 50){
            titleGroup.y = -1000
        }
    }

    Text {
        id: titleText
        x: 70
        y: 13
        color: "#ffffff"
        text: qsTr("")
        z: 3
        font.bold: false
        font.pixelSize: 20
    }

    Rectangle {
        id: backgroundRectangle
        width: parent.width
        height: parent.height
        color: "#171717"
        opacity: 0.9

//        Text {
//            id: saveText
//            x: 69
//            //y: saveImage.y
//            anchors.bottom: saveAsText.top
//            anchors.bottomMargin: 33
//            color: "#8a8a8a"
//            text: qsTr("Save")
//            font.pixelSize: 15
//        }

//        Text {
//            id: saveAsText
//            x: 69
//            //y: saveAsImage.y
//            anchors.bottom: settingText.top
//            anchors.bottomMargin: 33
//            color: "#8a8a8a"
//            text: qsTr("SaveAs")
//            font.pixelSize: 15
//        }


//        Text {
//            id: settingText
//            x: 69
//            anchors.bottom: backgroundRectangle.bottom
//            anchors.bottomMargin: 17
//            color: "#8a8a8a"
//            text: qsTr("Settings")
//            font.pixelSize: 15
//        }
//        Item {
//            id: saveTextItem
//            x: 69
//            width: 50
//            height: 50
//            anchors.bottom: saveAsTextItem.top
//            Text {
//                id: saveText
//                color: "#8a8a8a"
//                text: qsTr("Save")
//                font.pixelSize: 15
//            }
//        }

//        Item {
//            id: saveAsTextItem
//            x: 69
//            width: 50
//            height: 50
//            anchors.bottom: settingTextItem.top
//            Text {
//                id: saveAsText
//                color: "#8a8a8a"
//                text: qsTr("SaveAs")
//                font.pixelSize: 15
//            }
//        }

//        Item {
//            id: settingTextItem
//            x: 69
//            width: 50
//            height: 50
//            anchors.bottom: backgroundRectangle.bottom
//            Text {
//                id: settingText
//                color: "#8a8a8a"
//                text: qsTr("Settings")
//                font.pixelSize: 15
//            }
//        }
    }
    Spliter{
        y: heightVal -1
        z: 3
        width: parent.width
    }
    Spliter{
        y: heightVal * 4 - 4
        z: 3
        width: parent.width

    }
//    Spliter{
//        y: heightVal * 6 - 4
//        z: 3
//        width: parent.width
//    }
//    Spliter{
//        y: heightVal * 8 - 4
//        z: 3
//        width: parent.width
//    }

    Image {
        id: groupToolBoxImage
        x: 0
        y: 0
        width: 50
        height: 684
        z: 1
        fillMode: Image.PreserveAspectFit
        source: "../resources/btn_group_toolbox.png"

        MouseArea {
            id: listMouseArea
            width: parent.width
            height: 50
            hoverEnabled: true
            onHoveredChanged: {
                hoverType = 1
                onHovered()
            }
            onClicked:toolbox.state == "expanded" ? toolbox.state = "collapsed" : toolbox.state = "expanded"
        }

        MouseArea {
            id: liveMouseArea
            x: 0
            y: 50
            height: 50
            width: toolbox.width
            hoverEnabled: true
            onHoveredChanged: {
                hoverType = 2
                onHovered()
            }
            onClicked:{
                root.onPressedUnCheck();
                onLive()
            }
        }

        MouseArea {
            id: sphericalMouseArea
            x: 0
            y: 100
            width: toolbox.width
            height: 50
            hoverEnabled: true
            onHoveredChanged: {
                hoverType = 3
                onHovered()
            }
            onClicked: {
                root.onPressedUnCheck()
                onSpherical()
            }
        }

        MouseArea {
            id: interativeMouseArea
            x: 1
            y: 150
            width: toolbox.width
            height: 50
            hoverEnabled: true
            onHoveredChanged: {
                hoverType = 4
                onHovered()
            }
            onClicked: {
                root.onPressedUnCheck()
                onInterative()
            }
        }

        MouseArea {
            id: pauseContinueMouseArea
            x: 0
            y: 200
            width: toolbox.width
            height: 50
            hoverEnabled: true
            onHoveredChanged: {
                hoverType = 5
                onHovered()
            }
            onClicked: {
                root.onPressedUnCheck()
                clickCnt=clickCnt + 1;
                if(clickCnt % 2){
                    pauseText.text = qsTr("Pause")
                    titleText.text = pauseText.text;
                    onPause();
                }else{
                    pauseText.text = qsTr("Start");
                    titleText.text = pauseText.text;
                    onStart();
                }


                onPauseContinue()
            }
        }

//        MouseArea {
//            id: snapshotMouseArea
//            x: 0
//            y: 250
//            width: toolbox.width
//            height: 50
//            hoverEnabled: true
//            onHoveredChanged: {
//                hoverType = 6
//                onHovered()
//            }
//            onClicked: {
//                root.onPressedUnCheck()
//                onSnapshot()
//            }
//        }

        /*MouseArea {
            id: exposureMouseArea
            x: 0
            y: 300
            width: toolbox.width
            height: 50
            hoverEnabled: true
            onHoveredChanged: {
                hoverType = 7
                onHovered()
            }
            onClicked: {
                root.onPressedUnCheck()
                onExposure()
            }
        }*/

        /*MouseArea {
            id: topBottomMouseArea
            x: 0
            y: 350
            width: toolbox.width
            height: 50
            hoverEnabled: true
            onHoveredChanged: {
                hoverType = 8
                onHovered()
            }
            onClicked: {
                root.onPressedUnCheck()
                onTopBottom()
            }
        }

        MouseArea {
            id: anaglyphMouseArea
            x: 0
            y: 400
            width: toolbox.width
            height: 50
            hoverEnabled: true
            onHoveredChanged: {
                hoverType = 9
                onHovered()
            }
            onClicked: {
                root.onPressedUnCheck()
                onAnaglyph()
            }
        }*/

//        MouseArea {
//            id: saveMouseArea
//            x: 0
//            y: 400
//            width: toolbox.width
//            height: 50
//            hoverEnabled: true
//            onHoveredChanged: {
//                hoverType = 9
//                onHovered()
//            }
//            onClicked: {
//                root.onPressedUnCheck()
//                onSave()
//            }
//        }

    }
    Image {
        id:  startImage
        x: 0
        y: 200
        width: 50
        height: 50
        visible: true
        z: 1
        source: "../resources/start.png"
        fillMode: Image.PreserveAspectFit
    }
    Image {
        id: pauseImage
        x: 0
        y: 200
        width: 50
        height: 50
        z: 1
        source: "../resources/pause.png"
        visible: false
        fillMode: Image.PreserveAspectFit
    }




    /*Image{
        id: bottomSpliterImage
        anchors.bottom: bottomGroupToolBoxImage.top
        width: parent.width
        height: 2
        source: "../resources/bg_spliter.PNG"
        fillMode: Image.PreserveAspectCrop
    }*/

    Spliter{
        id: bottomSpliter
        anchors.bottom: notifyHistoryItem.top
        width: parent.width
        z: 3
    }



    Image {
        id: saveImage
        source: "../resources/save.png"
        anchors.bottom: saveAsImage.top
        width: 50
        height: 50
        fillMode: Image.PreserveAspectFit
        z: 1

        MouseArea {
            id: saveMouseArea
            width: toolbox.width
            height: 50
            hoverEnabled: true
            onHoveredChanged: {
                hoverType = 13;
                onHovered();
            }
            onClicked: {
                onSave();
            }
        }

    }

    Image {
        id: saveAsImage
        source: "../resources/save_as.png"
        anchors.bottom: bottomGroupToolBoxImage.top
        width: 50
        height: 50
        fillMode: Image.PreserveAspectFit
        z: 1

        MouseArea {
            id: saveAsMouseArea
            width: toolbox.width
            height: 50
            hoverEnabled: true
            onHoveredChanged: {
                hoverType = 12;
                onHovered();
            }
            onClicked: {
                onSaveAs();

            }
        }

    }

    Image {
        id: bottomGroupToolBoxImage
        x: -1
        anchors.bottom: notifyHistoryItem.top
//        anchors.bottomMargin: -102
        width: 50
        height: 50
        fillMode: Image.PreserveAspectFit
        MouseArea {
            id: settingMouseArea
            width: toolbox.width
            height: 50
            hoverEnabled: true
            onHoveredChanged:  {
                hoverType = 11
                onHovered()
            }
            onClicked: {
                root.onPressedUnCheck()
                onSettings()
            }
        }

        z: 1
        source: "../resources/setting.png"
    }

    Item {
        id: notifyHistoryItem
        width: 50
        height: 50
        anchors.bottom: parent.bottom
        z: 1

        Image {
            id: notifyHistoryImage
            source: "../resources/notifyHistory.png"
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            width: 25
            height: 25
            fillMode: Image.PreserveAspectFit
            z: 1
        }

        MouseArea {
            id: historyMouseArea
            width: toolbox.width
            height: 50
            hoverEnabled: true
            onHoveredChanged: {
                hoverType = 10;
                onHovered();
            }
            onClicked: {
                onNotifyHistory();

            }
        }
    }

    Rectangle {
        id: hoveredRectangle
        x: 0
        y: -201
        width: parent.width
        height: 50
        color: "#1f1f1f"
        z: 0
        opacity: 0.9
    }

    Rectangle {
        id: selectedRectangle
        x: 0
        y: -145
        width: parent.width
        height: 50
        color: "#0e3e64"
        z: 0
        opacity: 0.9
    }

    Rectangle {
        id: disableRectangle
        x: 0
        y: -255
        width: parent.width
        height: 50
        color: "#1f1f1f"
        z: 1
        opacity: 1.0
    }

    ScrollView {
        id: scrollView
        width: parent.width
        height: parent.height
        opacity: 0.8
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
            height: scrollView.height

            Item {
                id: titleGroup
                x: 68
                y: 1
                width: parent.width * 3 / 4
                height: parent.height
                z: 1
                rotation: 0
                Text {
                    id: liveText
                    x: 1
                    y: 67
                    color: "#8a8a8a"
                    text: qsTr("Live")
                    font.pixelSize: 15
                }

                Text {
                    id: sphericalText
                    x: 1
                    y: 114
                    color: "#8a8a8a"
                    text: qsTr("Spherical")
                    font.pixelSize: 15
                }

                Text {
                    id: interactiveText
                    x: 1
                    y: 164
                    color: "#8a8a8a"
                    text: qsTr("Interactive")
                    font.pixelSize: 15
                }

                Text {
                    id: pauseText
                    x: 1
                    y: 214
                    color: "#8a8a8a"
                    text: qsTr("Start")
                    font.pixelSize: 15
                }

                Text {
                    id: saveText
                    x: 1
                    anchors.bottom: saveAsText.top
                    anchors.bottomMargin: 33
                    color: "#8a8a8a"
                    text: qsTr("Save")
                    font.pixelSize: 15
                }

                Text {
                    id: saveAsText
                    x: 1
                    anchors.bottom: settingText.top
                    anchors.bottomMargin: 33
                    color: "#8a8a8a"
                    text: qsTr("SaveAs")
                    font.pixelSize: 15
                }

                Text {
                    id: settingText
                    x: 0
                    anchors.bottom: notifyHistoryText.top
                    anchors.bottomMargin: 33
                    color: "#8a8a8a"
                    text: qsTr("Settings")
                    font.pixelSize: 15
                }

                Text {
                    id: notifyHistoryText
                    x: 1
//                    anchors.bottom: saveText.top
//                    anchors.bottomMargin: 33
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 17
                    color: "#8a8a8a"
                    text: qsTr("Notifications")
                    font.pixelSize: 15
                }


                Text {
                    id: currentTitleText
                    x: 0
                    y: 12
                    color: "#ffffff"
                    text: qsTr("")
                    font.bold: true
                    font.pixelSize: 19
                }
            }
        }
    }

    Timer {
        id: delaytimer
        interval: 100
        running: false
        repeat: false
        onTriggered: {
            if(toolbox.state == "collapsed" && !isSelected)
                toolbox.state = "expanded"
        }
    }
    function onHovered() {
        isHovered = !isHovered
        var text

        if(isHovered) {
            isSelected = false

            if(hoverType > 9) {
                hoveredRectangle.y = backgroundRectangle.height  - 50 * (hoverType - 9)
                text = titleGroup.childAt(3,backgroundRectangle.height  - 50 * (hoverType - 9) + 25)
            }
            else {
                text = titleGroup.childAt(3,17 + (hoverType - 1) * 50)
                hoveredRectangle.y = 50 * (hoverType - 1)
            }

            if(text)
                text.color = "#ffffff"

            delaytimer.restart()
        }else{
           // delaytimer.stop()

            clearHovered()

            if(hoverType > 9)
                text = titleGroup.childAt(3,backgroundRectangle.height  - 50 * (hoverType - 9) + 25)
            else
                text = titleGroup.childAt(3,25 + (hoverType - 1) * 50)
            if(text)
                text.color = "#8a8a8a"
           //  toolbox.state = "collapsed";


        }
    }


    function onViewList() {

    }

    function onLive() {
        if (menuLock)   return

        curMode = 1
        qmlMainWindow.setCurrentMode(curMode);
        clearSelected()
        selectedRectangle.y = liveMouseArea.y
        titleText.text = liveText.text
        //groupCtrl.visible = true;
        groupCtrl.state = "collapsed";
        clearTopCtrl();
        moreCtrl.visible = true;
        snapshotCtrl.visible = true;
        exposureGroup.state = "collapsed";
    }

    function onSpherical() {
        if (menuLock)   return

        curMode = 2
        qmlMainWindow.setCurrentMode(curMode);
        clearSelected()
        if(menuLock) return
        selectedRectangle.y = sphericalMouseArea.y - 1
        titleText.text = sphericalText.text
        clearTopCtrl();
        exposureCtrl.visible = true;
        moreCtrl.visible = true
        adsItem.visible = true;
        exposureGroup.state = "collapsed";
    }

    function onInterative() {
        if (menuLock)   return

        curMode = 3
        qmlMainWindow.setCurrentMode(curMode);
        clearSelected()
        selectedRectangle.y = interativeMouseArea.y - 3
        titleText.text = interactiveText.text
        clearTopCtrl();
        moreCtrl.visible = true
        exposureGroup.state = "collapsed";
    }
    function onStart(){
        playMode = 1;
        qmlMainWindow.setPlayMode(playMode);
        startImage.visible = true;
        pauseImage.visible = false;
    }


    function onPause(){
         playMode = 2;
         qmlMainWindow.setPlayMode(playMode);
         pauseImage.visible = true;
         startImage.visible = false;
    }

    function onPauseContinue() {
        if (menuLock)   return
        clearSelected()
        //selectedRectangle.y = pauseContinueMouseArea.y - 1
        //moreCtrl.visible = false;
        //snapshotCtrl.visible = false;
//        topBottomCtrl.visible = false;
//        anaglyphCtrl.visible = false;
        //exposureCtrl.visible = false;
        //exposureGroup.state = "collapsed";

    }

    function onSnapshot() {

        if (menuLock)   return

        menuLock = true

        clearSelected()
        selectedRectangle.y = snapshotMouseArea.y - 3
        titleText.text = snapshotText.text
        onStart();
        snapshotBox.getSnapshotDir();
        groupCtrl.visible = true
    }

    function onExposure() {
        if (menuLock)   return

        menuLock = true

        clearSelected()
        selectedRectangle.y = exposureMouseArea.y - 3
        titleText.text = exposureText.text;
        moreCtrl.visible = true;

    }

    function onTopBottom() {
        if (menuLock)   return

        menuLock = true

        clearSelected()
        selectedRectangle.y = topBottomMouseArea.y - 1
        titleText.text = topBottomText.text

        groupCtrl.visible = true
    }

    function onAnaglyph() {
        if (menuLock)   return
        menuLock = true
        clearSelected()
        selectedRectangle.y = anaglyphMouseArea.y - 3
        titleText.text = anaglyphText.text

        groupCtrl.visible = true
    }

    function onNotifyHistory()
    {
        clearSelected()
        selectedRectangle.y = notifyHistoryItem.y + 1;
        titleText.text = notifyHistoryText.text
        clearTopCtrl();
        exposureGroup.state = "collapsed";
        if(notifyHistorybox.isOpen){
             notifyHistorybox.state = "collapsed";
            notifyHistorybox.isOpen = false;
        }else{
            notifyHistorybox.state = "expanded";
            notifyHistorybox.isOpen = true;
        }

        notifyHistorybox.getNotifyHistory();
    }

    function onSave() {
        if (menuLock)   return
        onStart();
        clearSelected()
        selectedRectangle.y = saveImage.y + saveMouseArea.y + 1;
        titleText.text = saveText.text
        clearTopCtrl();
        saveCtrl.visible = true;
        exposureGroup.state = "collapsed";
    }


    function onSaveAs() {
        if (menuLock)   return
        onStart();
        clearSelected()
        selectedRectangle.y = saveAsImage.y + saveAsMouseArea.y + 1;
        titleText.text = saveAsText.text
        clearTopCtrl();
        saveINIFileDialog.open();
        exposureGroup.state = "collapsed";

    }

    FileDialog {
        id: saveINIFileDialog
        selectExisting: false
        selectFolder: false
        selectMultiple: false
        nameFilters: [ "INI file (*.ini)"]
        selectedNameFilter: "All files (*)"
        onAccepted: {
            fileName = fileUrl.toString().substring(8);
            root.deleteIndex = qmlMainWindow.saveIniPath(fileName)
            recent.deleteRecentListItem()
            recent.setCurrentFile();
            toolBox.clearSelected();
            toolBox.clearState();
        }
        onRejected: {
            toolBox.clearSelected();
            toolBox.clearState();
        }

    }

    function saveIniFile(){
        saveINIFileDialog.open()

    }

    function onSettings() {
        if (menuLock)   return
        qmlMainWindow.resetConfigList();
        isTemplate = false;
        globalStereoState = true;
        onStart();
        clearSelected()
        selectedRectangle.y = bottomGroupToolBoxImage.y + settingMouseArea.y
        titleText.text = settingText.text
        var captureType = qmlMainWindow.getCaptureType();
        switch(captureType)
        {
        case 0:
            clearBackForward();
            liveSettingbox.state = "expanded";
            liveSettingbox.setChangeTitle();
            liveSettingbox.setLiveSlot();
            //liveSettingbox.getStereoState();

            break;
        case 2:
            clearBackForward();
            videoSettingbox.state = "expanded";
            videoSettingbox.setChangeTitle();
            videoSettingbox.setVideoSlot();
            break;
        case 3:
            imageSettingbox.state = "expanded";
            clearBackForward();
            imageSettingbox.setChangeTitle();
            imageSettingbox.setImageSlot();
            break;
        }
    }

    function onHelp() {
        if (menuLock)   return

        clearSelected()
        selectedRectangle.y = bottomGroupToolBoxImage.y + helpMouseArea.y
        titleText.text = helpText.text

        if(helpBox.state == "collapsed")
            helpBox.state = "expanded"
    }

    function onAbout() {
        if (menuLock)   return

        clearSelected()
        selectedRectangle.y = bottomGroupToolBoxImage.y + aboutMouseArea.y
        titleText.text = aboutText.text

        if(aboutBox.state == "collapsed")
            aboutBox.state = "expanded"
    }

    function clearHovered() {
        hoveredRectangle.y = -200
    }

    function clearSelected() {
        selectedRectangle.y = liveMouseArea.y
        if(toolbox.state === "expanded")
            toolbox.state = "collapsed"

        if(toolbox.state === "collapsed")
            isSelected = true
        else
            isSelected = false

        titleText.text = ""

        liveBox.state = "collapsed"
        sphericalBox.state = "collapsed"
        interactiveBox.state = "collapsed"
        snapshotBox.state = "collapsed"
        exposureBox.state = "collapsed"
        topBottomBox.state = "collapsed"
        anaglyphBox.state = "collapsed"
        saveBox.state = "collapsed"
        helpBox.state = "collapsed"
        settingsBox.state = "collapsed"
        notifyHistorybox.state = "collapsed"
        adsCtrl.state = "collapsed"
        saveCtrl.visible = false;
        switch(curMode){
        case 1:     // Live mode
            selectedRectangle.y = liveMouseArea.y
            liveView.visible = true;
            sphericalView.visible = false;
            interactiveView.visible = false;
            moreCtrl.visible = true;
            exposureCtrl.visible = false;
//            topBottomCtrl.visible = false;
//            anaglyphCtrl.visible = false;
            break;
        case 2:     // Spherical mode
            selectedRectangle.y = sphericalMouseArea.y
            sphericalView.visible = true;
            liveView.visible = true;
            interactiveView.visible = false;
            exposureCtrl.visible = true;
//            topBottomCtrl.visible = true;
//            anaglyphCtrl.visible = true;
            moreCtrl.visible = true;
            break;
        case 3:     // Interactive mode
            selectedRectangle.y = interativeMouseArea.y - 3
            interactiveView.visible = true;
            liveView.visible = true;
            sphericalView.visible = false;
            exposureCtrl.visible = false;
//            topBottomCtrl.visible = false;
//            anaglyphCtrl.visible = false;
            moreCtrl.visible = true;
        }
    }

    function clearDisabled() {
        disableRectangle.y = -200
    }

    function showSpecifyBox() {
        switch(curMode) {
        case 1:
            if(snapshotBox.state == "collapsed")
                snapshotBox.state = "expanded"
            break;
        case 7:
            if(exposureBox.state == "collapsed")
                exposureBox.state = "expanded"
            break;

        case 8:
            if(topBottomBox.state == "collapsed")
                topBottomBox.state = "expanded"
            break;
        case 9:
            if(anaglyphBox.state == "collapsed")
                anaglyphBox.state = "expanded"
            break;
        case 10:
            if(saveBox.state == "collapsed")
                saveBox.state = "expanded"
            break;
        default:
            break;
        }
    }
    function showMainBox() {
        switch(curMode) {
        case 1:
            if(liveBox.state == "collapsed")
                liveBox.state = "expanded"
            liveBox.getLensSettings();
            break;
        case 2:

            if(sphericalBox.state === "collapsed")
                onStart();
            qmlMainWindow.setTempCameraSettings();
            sphericalBox.refreshCameraValues();
            sphericalBox.isFirst = true;
            sphericalBox.isExposure = false;
            sphericalBox.showBlendSettings();
            sphericalBox.getCameraList();
            sphericalBox.state = "expanded"
            break;

        case 3:
            if(interactiveBox.state === "collapsed")
                onStart();
            interactiveBox.getOculus();
            interactiveBox.state = "expanded"
            break;
        default:
            break;
        }
    }
    function showStartIamge()
    {
        startImage.visible = true;
        pauseImage.visible = false;
    }

    function clearState()
    {
        if(pauseText.text === qsTr("Start")){
            onStart();
        }
        else if( pauseText.text === qsTr("Pause"))
        {
            onPause();
        }
    }

    function clearSettingBox()
    {
        snapshotCtrl.visible = true;
        moreCtrl.visible = true;
        hoverType = 2;
        curMode = 1;

    }

   function clearTopCtrl(){
       moreCtrl.visible = false;
       snapshotCtrl.visible = false;
//        topBottomCtrl.visible = false;
//        anaglyphCtrl.visible = false;
       exposureCtrl.visible = false;
       saveCtrl.visible = false;
       groupCtrl.visible = false;
       adsItem.visible = false;
   }
}
