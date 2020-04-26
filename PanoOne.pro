TEMPLATE = app

QT += qml quick
SOURCES += main.cpp

RESOURCES += qml.qrc

OTHER_FILES = main.qml \
                  qml/Recent.qml  \
                  qml/RecentListitem.qml \
                  qml/RecentGriditem.qml \
                  qml/Header.qml  \
                  qml/Toolbox.qml \
                  qml/Helpbox.qml\
                  qml/Livebox.qml \
                  qml/Savebox.qml\
                  qml/Status.qml\
                  qml/FlatText.qml\
                  qml/FlatCombo.qml\
                  qml/Settingsbox.qml\
                  qml/controls/FlatCombo.qml \
                  qml/controls/FlatText.qml \
                  qml/components/Spliter.qml\
                  qml/components/ImageSettingbox.qml \
                  qml/components/VideoSettingbox.qml \
                  qml/components/LiveSettingbox.qml \
                  qml/components/Snapshotbox.qml  \
                  qml/components/Sphericalbox.qml \
                  qml/components/Topbottombox.qml \
                  qml/components/Snapshotbox.qml  \
                  qml/components/LiveCamlistitem.qml\
                  qml/components/Sphericalbox.qml \
                  qml/components/Topbottombox.qml \
                  qml/components/Interactivebox.qml  \
                  qml/components/Aboutbox.qml \
                  qml/components/Settingsbox.qml\
                  qml/components/Anaglyphbox.qml \
                  qml/components/Exposurebox.qml \
                  qml/components/LiveSettingbox.qml \
                  qml/components/ImageSettingbox.qml \
                  qml/components/VideoListitem.qml\
                  qml/components/LiveAudioitem.qml\
                  qml/components/LiveCamListitem.qml\
                  qml/components/ExposureDialog.qml\
                  qml/components/ToolWindow.qml

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = qml
# Default rules for deployment.
include(deployment.pri)

HEADERS +=

DISTFILES += \
    qml/components/FloatingWindow.qml \
    qml/components/FloatingStitchWindow.qml \
    qml/components/MCCameraView.qml \
    qml/components/MCVideoWindow.qml \
    qml/components/LiveView.qml \
    qml/components/SphericalView.qml \
    qml/components/InteractiveView.qml \
    ToolWindowPositions.js \
    qml/components/MCStitchCameraView.qml \
    qml/components/MCStitchVideoWindow.qml \
    qml/components/ImageListitem.qml \
    qml/components/StitchView.qml \
    qml/components/FloatingInteractiveWindow.qml \
    qml/components/MCInteractCameraView.qml \
    qml/components/MCInteractVideoWindow.qml \
    qml/components/CameraPreview.qml \
    qml/components/CameraSettingbox.qml \
    qml/components/VideoGlobalSettings.qml \
    qml/components/LiveCameraSetting.qml \
    qml/components/LiveTempCamSetting.qml \
    qml/components/LiveTempGlobalSetting.qml \
    qml/components/ImageTempCamSetting.qml \
    qml/components/GroupCtrl.qml \
    qml/components/ControlPoint.qml \
    qml/components/SeamLabel.qml \
    qml/components/Notification.qml \
    qml/components/ExposureCtrl.qml \
    qml/components/NotificationHistory.qml \
    qml/components/NotificationItem.qml \
    qml/components/Line.qml \
    qml/components/AdvertisementCtrl.qml

