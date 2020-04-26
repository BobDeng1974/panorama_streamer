import QtQuick 2.4
import MCQmlCameraView 1.0
MCVideoWindow {
    id: cameraView
    width: 400
    height: 330
    destroyOnClose: true
    property alias camView : display
    property alias drawView: background
    property int deviceNum: 0
    property var cameraIndex: display.cameraNumber
    property int name
    caption: qsTr(display.cameraViewName)
    Component.onCompleted: {
        qmlMainWindow.createCameraView(camView,deviceNum);
    }

    MCQmlCameraView {
        id: display
        anchors.fill: parent
        visible: true
		
        function closeView()
        {
            close();
        }
    }

    Rectangle {
        id: background
        anchors.fill: parent
        opacity: 0.0
    }

    LiveCameraSetting {
        id: liveCameraSetting
        anchors.right: parent.right
        anchors.rightMargin: 30
        width: 250
        height: 0
        z: 1
        state: "collapsed"

           states: [
               State {
                   name: "collapsed"
                   PropertyChanges { target: liveCameraSetting; height: 0}
                   PropertyChanges { target:  liveCameraSetting;width: 0

                   }
               },
               State {
                   name: "expanded"
                   PropertyChanges { target: liveCameraSetting; height: 300}
                   PropertyChanges {target: liveCameraSetting;width: 200}
               }
           ]

           transitions: [
               Transition {
                   NumberAnimation { target: liveCameraSetting; property: "height"; duration: 300 }
                   NumberAnimation { target: liveCameraSetting;property: "width";duration: 300}
               }
           ]

    }

    function setCameraSetting()
    {
        if(liveCameraSetting.state == "expanded"){
            liveCameraSetting.state = "collapsed";

        }else if(liveCameraSetting.state == "collapsed"){
            liveCameraSetting.state = "expanded";
            liveCameraSetting.getCameraValues()
        }
    }
}
