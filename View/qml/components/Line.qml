import QtQuick 2.0

Canvas {
    id: canvas
    width: parent.width
    height: parent.height
    z: 2
    property var prePosX
    property var prePosY
    property var curPosX
    property var curPosY
    onPaint: {
        var ctx = getContext("2d")

        // setup the stroke
        ctx.strokeStyle = "#4e8d15"

        // create a path
        ctx.beginPath();
        ctx.moveTo(prePosX,prePosY);
        ctx.lineTo(curPosX,curPosY);
        //ctx.reset();
        // stroke path
        ctx.stroke()
     }

}
