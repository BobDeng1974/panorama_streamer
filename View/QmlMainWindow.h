#pragma once
#include <QObject>
#include "D360Parser.h"
#include "D360Process.h"
#include "MCQmlCameraView.h"
#include "QmlInteractiveView.h"
#include "SharedImageBuffer.h"
#include "CameraView.h"
#include "D360Process.h"
#include "QmlRecentDialog.h"
#include "OculusViewer.h"
#include <QtXml/QDomDocument>
#include <qvector.h>
#include "BaseFfmpeg.hpp"
#include "common.h"
#include "OculusViewer.h"
#include "qfiledialog.h"
#include "CalibProcess.h"
#include "common.h"
#include "PanoLog.h"
#include "AdvThread.h"

extern QString getVideoCodecStringFromType(int videoCodec);
extern int getVideoCodecTypeFromString(QString videoCodecString);
extern QString getAudioCodecStringFromType(int audioCodec);
extern int getAudioCodecTypeFromString(QString audioCodecString);

enum ViewMode {
	LIVE_VIEW= 1,
	SPHERICAL_VIEW = 2,
	STITCH_VIEW = 3
};

enum PlayMode {
	STRAT_MODE = 1,
	PAUSE_MODE = 2
};

enum BlendMode
{
	FEATHER_MODE = 1,
	MULTIBAND_MODE = 2
};

struct MousePos
{
	int x;
	int y;
	int width;
	int height;
};

class QmlMainWindow : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QObject *recDialog READ recDialog WRITE setRecDialog NOTIFY recDialogChanged)
	Q_PROPERTY(QString elapsedTime READ elapsedTime NOTIFY elapsedTimeChanged)
	Q_PROPERTY(QString fps READ fps NOTIFY fpsChanged)
	Q_PROPERTY(bool exit READ exit NOTIFY exitChanged)
	Q_PROPERTY(bool start READ start NOTIFY started)
	Q_PROPERTY(QString errorMsg READ errorMsg NOTIFY error)
	Q_PROPERTY(int xRes READ getXres NOTIFY cameraResolutionChanged)
	Q_PROPERTY(int yRes READ getYres NOTIFY cameraResolutionChanged)
	Q_PROPERTY(bool calibrated READ calibrated NOTIFY calibratingFinished)
	Q_PROPERTY(QString notifyMsg READ notifyMsg NOTIFY notify)

public:
	QmlMainWindow();
	~QmlMainWindow();

	QObject *recDialog() const;
	QString elapsedTime() const { return m_eTimeValue; }
	void setETimeValue(QString elapsedTime);
	QString fps() const { return m_fpsValue; }
	void setFpsValue(QString fps);
	bool exit() { return m_isExit; }
	void setExit(bool isExit) { m_isExit = isExit; emit exitChanged(true); }
	bool start() { return m_isStarted; }
	void setStart(bool isStarted) { m_isStarted = isStarted; emit started(true); }
	QString errorMsg() { return m_errorMsg; }
	void setErrorMsg(QString errorMsg) { m_errorMsg = errorMsg; emit error(); }
	bool calibrated() { return m_calibResult; }
	QString notifyMsg() { return m_notifyMsg; }

protected:
	D360Parser  m_d360Data;
	D360Process*	m_process;

	SharedImageBuffer* sharedImageBuffer;
	
	QMap< int, int > deviceNumberMap;

	QMap<int, AudioThread*> audioModuleMap;
	QMap< int, CameraModule* > cameraModuleMap;
	QMap<int, QString> m_videoDevices;
	QMap<int, QString> m_audioDevices;
		
	//StreamProcess* m_offlineVideoSaveProcess;
	StreamProcess* m_streamProcess;

	AdvThread* m_advThread;

public:
	QMap<int, MCQmlCameraView*> m_cameraViewMap;
	MCQmlCameraView* m_stitcherView;
	QmlInteractiveView* m_interactView;
	MCQmlCameraView* m_preView;
	ViewMode		m_viewMode;
    PlayMode m_playMode;
signals:
	void recDialogChanged(const QObject* recDialog);
	void elapsedTimeChanged(QString elapsedTime);
	void fpsChanged(QString fps);
	void levelChanged(QString level);
	void exitChanged(bool isExit);
	void started(bool isStarted);
	void error();
	void cameraResolutionChanged();
	void calibratingFinished();
	void notify();

protected:
	void initDevices();
	int getDeviceIndex(QString name, int type = 0);		// type:0(video), type:1(audio)

	void Stop();
	void Pause();
	void Start();

	void releaseThreads();

public:
	
	bool attach(CameraInput& camSettings, int deviceNumber, float startframe, float endframe, float fps, int dupIndex = 0);
	void disconnectCamera(int index);
	void disconnectCameras();
	GlobalAnimSettings& getGlobalAnimSetting(){ return m_d360Data.getGlobalAnimSettings();}

private:
	QString m_Name;
	bool m_isExit;
	bool m_isStarted;
	bool m_isEndCaptureThreads;
	bool m_calibResult;
	QString m_errorMsg;
	QPointer<QmlRecentDialog> m_qmlRecentDialog;
	int m_cameraCnt;
	int m_audioCnt;
	bool m_bstitch;
	bool m_binteract;
	bool m_bpreview;
	bool m_bblendview;
	int m_filecount;
	bool m_connectedCameras;
	bool m_reconnectCameras;

	QMap<int, QString> m_videoPathList;
	QMap<int, QString> m_imagePathList;
	QMap<int, QString> m_cameraNameList;
	QMap<int, QString> m_audioNameList;
	QStringList m_recentTitleList;
	QStringList m_recentSourcePathList;
	QStringList	m_recentImageIDList;
	QString		m_recentTitle;
	QString		m_recentPath;
	QString		m_recentFullPath;
	QLocale		conv;
	QString		m_eTimeValue;
	QString		m_fpsValue;
	QString		m_yawValue;
	QString		m_pitchValue;
	QString		m_rollValue;
	OculusRender* m_oculusDevice;
	QStringList m_iPathList;
	CalibProcess* m_calib;

	int			m_deletedCameraCnt;
	int			m_deletedAudioCnt;
	QList<CPOINT> m_cpList;
	QList<QString>	m_notifyList;

	PanoLog		m_logger;
	QString		m_notifyMsg;

	int			m_outType;	// 0:toFile, 1:toStream, 2:toOculus

public slots:
	void connectToCameras();
	void finishedThread(int type, QString msg = "", int id = -1);
	void startedThread(int type, QString msg = "", int id = -1);
	void reportError(int type, QString msg = "", int id = -1);
	void setRecDialog(QObject *recDialog);
	int  openIniPath(QString iniPath);
	int  saveIniPath(QString iniPath);
	bool openRecentMgrToINI();
	void saveRecentMgrToINI();
	void closeMainWindow(); 
	void receiveKeyIndex(int level);
	int level();
	bool calibrate(int step = -1);
	void startCalibrating();
	void finishedSnapshot(int id);

	//Recent Title, Path
	QString getRecentTitle(){ return m_recentTitle; }
	QString getRecentPath();
	QString getRecentFullPath(){
		return m_recentFullPath; 
	}
	void sendRecentIndex(int index);
	void sendCurrentIndex(int index);
	// return value: -2 if ini file open failed, -1 if this file is not registered to recent list yet
	int  recentOpenIniPath(QString sourcepath);
	void deleteRecentList(QString title);

	void createCameraView(QObject* camView, int deviceNum);
	void deleteCameraView();
	void createStitchView(QObject* stitchView);
	void deleteStitchView();
	void createInteractView(QObject* interactView);
	void deleteInteractView();
	void createPreView(QObject* preView);
	void deletePreView();
	int  getRecentCnt() { return m_filecount; }
	QString getDeviceName(int index, int type);
	int getCameraCnt() { return m_cameraCnt; }
	int getAudioCnt() { return m_audioCnt;}
	int getAudioSelCnt();
	int checkAudioExist(QString videoFilePath);
	void openProject();
	void initProject();
	void setCurrentMode(int);
	void setPlayMode(int);
	void setBlendMode(int);
	int getBlendMode() { return getGlobalAnimSetting().m_blendingMode; }
	//Template
	void sendVideoPath(int slotIndex, QString videoPath);
	QString getVideoPath(int slotIndex) { 
		if (m_videoPathList.contains(slotIndex)) 
			return m_videoPathList[slotIndex];
		return "";
	}
	int getVideoCount() { 
		return m_videoPathList.size(); 
	}
	void sendCameraName(int slotIndex, QString cameraName);
	bool isSelectedCamera(QString name);
	void sendAudioName(int slotIndex, QString audioName);
	bool isSelectedAudio(QString name);
	void sendImagePath(int slotIndex, QString imagePath);
	QString getImagePath(int slotIndex) {
		if (m_imagePathList.contains(slotIndex))
			return m_imagePathList[slotIndex];
		return "";
	}
	int getImageCount() { return m_imagePathList.size(); }
	void initTemplateImageObject(){ if (m_imagePathList.size() > 0){ m_imagePathList.clear(); } }
	void initTemplateVideoObject(){ if (m_videoPathList.size() > 0){ m_videoPathList.clear(); } }
	void initTemplateCameraObject(){ if (m_cameraNameList.size() > 0){ m_cameraNameList.clear(); m_audioNameList.clear(); } }
	int  getTemplateImageCount(){ return m_imagePathList.size(); }
	int  getTemplateVideoCount(){ return m_videoPathList.size(); }
	int  getTemplateCameraCount(){ return m_cameraNameList.size(); }
	void openTemplateVideoIniFile();
	void openTemplateCameraIniFile();
	void openTemplateImageIniFile();

	void streamPanorama(unsigned char* panorama);
	void streamAudio(int devNum, void* audioFrame);

	void updateStitchingThreadStats(struct ThreadStatisticsData statData);

	//startFrame, endFrame
	float startFrame()
	{
		return m_d360Data.startFrame();
	}
	void setStartFrame(float fStartFrame) { m_d360Data.setStartFrame(fStartFrame);}

	float endFrame()
	{
		return m_d360Data.endFrame();
	}

	void setEndFrame(float fEndFrame) { m_d360Data.setEndFrame(fEndFrame);}

	//version
	float fileVersion()
	{
		return m_d360Data.fileVersion();
	}
	//captureType
	int getCaptureType() {
		return m_d360Data.getCaptureType();
	}
	void setCaptureType(int captureType){
		m_d360Data.setCaptureType(captureType);
	}

	//cameraCount
	int getCameraCount() { return m_d360Data.getCameraCount(); }
	void setCameraCount(int cameraCount) { m_d360Data.setCameraCount(cameraCount); }

	//fps
	float getFps() { return m_d360Data.getFps(); }
	void setFps(float fps) { m_d360Data.setFps(fps); }


	//input resolution
	int getXres() { return m_d360Data.getXres(); }
	void setXres(int xres) { m_d360Data.setXres(xres); }
	int getYres() { return m_d360Data.getYres(); }
	void setYres(int yres) { m_d360Data.setYres(yres); }

	//cameraClib
	QString getCameraCalib() {
		return m_d360Data.getCameraCalib(); 
	}
	void setCameraCalib(QString cameraCalib) { m_d360Data.setCameraCalib(cameraCalib); }

	//sample Fmt
	int getSampleFmt() { return m_d360Data.getSampleFmt(); }
	void setSampleFmt(int sampleFmt) { m_d360Data.setSampleFmt(sampleFmt); }

	//sample Rate
	int getSampleRate() { return m_d360Data.getSampleRate(); }
	void setSampleRate(int sampleRate) { m_d360Data.setSampleRate(sampleRate); }

	//lag
	int getLag() { return m_d360Data.getLag(); }
	void setLag(int lag) { m_d360Data.setLag(lag); }

	//output resolution
	int getPanoXres() { return m_d360Data.getPanoXres(); }
	void setPanoXres(int panoXres) { m_d360Data.setPanoXres(panoXres); }
	int getPanoYres() { return m_d360Data.getPanoYres(); }
	void setPanoYres(int panoYres) { m_d360Data.setPanoYres(panoYres); }

	// stream
	int getStreamMode() {
		if (m_d360Data.getGlobalAnimSettings().m_wowzaServer.trimmed().isEmpty())	return 1;
		return 0;
	}
	void setStreamMode(int mode) {
		// Not implemented.
	}
	QString getStreamURL() {
		if (m_d360Data.getGlobalAnimSettings().m_wowzaServer.trimmed().isEmpty())	return m_d360Data.getGlobalAnimSettings().m_offlineVideo;
		return m_d360Data.getGlobalAnimSettings().m_wowzaServer;
	}
	void setStreamURL(QString streamURL) {
		m_d360Data.getGlobalAnimSettings().m_offlineVideo = streamURL;
	}

	//device Count
	int getDeviceCount() { return m_d360Data.getGlobalAnimSettings().m_cameraCount; }
	void setDeviceCount(int deviceCount) { }	
	
	void setBlendLevel(int iBlendLevel);

	// Crop 
	float  getLeft(int iDeviceNum);
	void setLeft(float fLeft, int iDeviceNum);
	float getRight(int iDeviceNum);
	void setRight(float fRight, int iDeviceNum);
	float getTop(int iDeviceNum);
	void setTop(float fTop, int iDeviceNum);
	float getBottom(int iDevicNum);
	void setBottom(float fBottom, int iDeviceNum);

	//stereo
	int getStereoType(int);
	void setStereoType(int iStereoType, int iDeviceNum);
	bool isLeftEye(int iDeviceNum) {
		std::vector< int >& indices = m_d360Data.getGlobalAnimSettings().getLeftIndices();
		for (int i = 0; i < indices.size(); i++) {
			if (indices[i] == iDeviceNum)
				return true;
		}
		return false;
	}
	bool isRightEye(int iDeviceNum) {
		std::vector< int >& indices = m_d360Data.getGlobalAnimSettings().getRightIndices();
		for (int i = 0; i < indices.size(); i++) {
			if (indices[i] == iDeviceNum)
				return true;
		}
		return false;
	}
	//audioType
	void setAudioType(int iAudioType, int iDeviceNum);
	//exposure
	float getCameraExposure(int iDeviceNum) { 
		return m_d360Data.getGlobalAnimSettings().cameraSettingsList()[iDeviceNum].exposure; 
	}
	void setCameraExposure(int iDeviceNum, float fExposure) { 
		m_d360Data.getGlobalAnimSettings().cameraSettingsList()[iDeviceNum].exposure = fExposure; 
		reStitch();
	}
	void onCalculatorGain();
	void onRollbackGain();
	void onResetGain();
	void setTempCameraSettings() { m_d360Data.setTempCameraSettings(); }
	void onCancelCameraSettings() { 
		m_d360Data.resetCameraSettings(); 
		sharedImageBuffer->getStitcher()->updateCameraParams();
	}

	//Calibration
	void setCalibFile(QString calibFile) { m_d360Data.getGlobalAnimSettings().setCameraCalib(calibFile); }
	QString getCalibFile()  { return m_d360Data.getGlobalAnimSettings().m_cameraCalibFile; }
	int getLensType() { return m_d360Data.getGlobalAnimSettings().m_lensType; }
	void setLensType(int lensType) { m_d360Data.getGlobalAnimSettings().m_lensType = lensType; }
	int getFov() { return m_d360Data.getGlobalAnimSettings().m_fov; }
	void setFov(int fov) { m_d360Data.getGlobalAnimSettings().m_fov = fov; }
	
	//oculus
	void enableOculus(bool);
	bool getOculus() { return m_d360Data.getGlobalAnimSettings().m_oculus; }

	//Slots for template settings
	void setTempStereoType(int iIndex, int iStereoType);
	int getTempStereoType(int iIndex);
	void setTempImagePath(int iIndex, QString fileDir);
	QString getTempImagePath(int iIndex);
	void setTempImagePrefix(int iIndex, QString filePrefix);
	QString getTempImagePrefix(int iIndex);
	void setTempImageExt(int iIndex, QString fileExt);
	QString getTempImageExt(int iIndex);
	void setTempAudioSettings(int iIndex, int iAudioType);
	int getTempAudioSettings(int iIndex);
	int getTempAudioSettingsEx(QString devName);
	int getTempWidth() { return m_d360Data.getTempGlobalAnimSettings().m_xres;}
	void setTempWidth(int width) { m_d360Data.getTempGlobalAnimSettings().setXres(width); }
	int getTempHeight() { return m_d360Data.getTempGlobalAnimSettings().m_yres; }
	void setTempHeight(int height) { m_d360Data.getTempGlobalAnimSettings().setYres(height); }
	int getTempPanoWidth() { return m_d360Data.getTempGlobalAnimSettings().m_panoXRes; }
	void setTempPanoWidth(int width) { m_d360Data.getTempGlobalAnimSettings().setPanoXres(width); }
	int getTempPanoHeight()  { return m_d360Data.getTempGlobalAnimSettings().m_panoYRes; }
	void setTempPanoHeight(int height) { m_d360Data.getTempGlobalAnimSettings().setPanoYres(height); }
	int getTempFPS()  { return m_d360Data.getTempGlobalAnimSettings().m_fps; }
	void setTempFPS(int fps) { m_d360Data.getTempGlobalAnimSettings().setFps(fps); }
	QString getTempCalibFile()  { return m_d360Data.getTempGlobalAnimSettings().m_cameraCalibFile; }
	void setTempCalibFile(QString calibFile) { m_d360Data.getTempGlobalAnimSettings().setCameraCalib(calibFile); }
	int getTempStartFrame()  { return m_d360Data.getTempGlobalAnimSettings().m_startFrame; }
	void setTempStartFrame(int nFrame) { m_d360Data.getTempGlobalAnimSettings().setStartFrame(nFrame); }
	int getTempEndFrame()  { return m_d360Data.getTempGlobalAnimSettings().m_endFrame; }
	void setTempEndFrame(int nFrame) { m_d360Data.getTempGlobalAnimSettings().setEndFrame(nFrame); }
	QString getTempFileExt() { return m_d360Data.getTempGlobalAnimSettings().m_fileExt; }
	void setTempFileExt(QString fileExt) {	m_d360Data.getTempGlobalAnimSettings().setFileExt(fileExt); }
	bool getTempOculus() { return m_d360Data.getTempGlobalAnimSettings().m_oculus; }
	void setTempOculus(bool oculus) { m_d360Data.getTempGlobalAnimSettings().m_oculus = oculus; }
	QString getTempOfflineFile() { return m_d360Data.getTempGlobalAnimSettings().m_offlineVideo; }
	void setTempOfflineFile(QString file) { m_d360Data.getTempGlobalAnimSettings().m_offlineVideo = file; }
	QString getTempOfflineVideoCodec() { return getVideoCodecStringFromType(m_d360Data.getTempGlobalAnimSettings().m_videoCodec); }
	void setTempOfflineVideoCodec(QString codec) { m_d360Data.getTempGlobalAnimSettings().m_videoCodec = getVideoCodecTypeFromString(codec); }
	QString getTempOfflineAudioCodec() { return getAudioCodecStringFromType(m_d360Data.getTempGlobalAnimSettings().m_audioCodec); }
	void setTempOfflineAudioCodec(QString codec) { m_d360Data.getTempGlobalAnimSettings().m_audioCodec = getAudioCodecTypeFromString(codec); }
	QString getTempStreamURL() { return m_d360Data.getTempGlobalAnimSettings().m_wowzaServer; }
	void setTempStreamURL(QString url) { m_d360Data.getTempGlobalAnimSettings().m_wowzaServer = url; }
	QString getTempStreamVideoCodec() { return getVideoCodecStringFromType(m_d360Data.getTempGlobalAnimSettings().m_videoCodec); }
	void setTempStreamVideoCodec(QString codec) { m_d360Data.getTempGlobalAnimSettings().m_videoCodec = getVideoCodecTypeFromString(codec); }
	QString getTempStreamAudioCodec() { return getAudioCodecStringFromType(m_d360Data.getTempGlobalAnimSettings().m_audioCodec); }
	void setTempStreamAudioCodec(QString codec) { m_d360Data.getTempGlobalAnimSettings().m_audioCodec = getAudioCodecTypeFromString(codec); }
	// Auto Calibrating...
	int getTempLensType() { return m_d360Data.getTempGlobalAnimSettings().m_lensType; }
	void setTempLensType(int lensType) { m_d360Data.getTempGlobalAnimSettings().m_lensType = lensType; }
	int getTempFov() { return m_d360Data.getTempGlobalAnimSettings().m_fov; }
	void setTempFov(int fov) { m_d360Data.getTempGlobalAnimSettings().m_fov = fov; }

	void resetTempGlobalSettings() { m_d360Data.resetTempGlobalAnimSettings(); }
	void clearTempGlobalSettings() { m_d360Data.clearTempGlobalAnimSettings(); }

	//snapshot
	QString getSnapshotDir() { 
		return m_d360Data.getGlobalAnimSettings().m_snapshotDir; 
	}
	void setSnapshotDir(QString snapshotDir) { 
		m_d360Data.getGlobalAnimSettings().m_snapshotDir = snapshotDir;
		sharedImageBuffer->getGlobalAnimSettings()->m_snapshotDir = snapshotDir;
	}
	void snapshotFrame();
	void reStitch() { sharedImageBuffer->getStitcher()->reStitch(); }
	void onMoved(int, int);
	void onReleased(int, int);
	void onPressed(int, int);
	void onMovedInteractive(int, int);
	void onReleasedInteractive(int, int);
	void onPressedInteractive(int, int);
	QString onImageFileDlg(QString);
	int getCaptyreType() { return m_d360Data.getGlobalAnimSettings().m_captureType; }
	void resetConfigList();

	// Control points...
	int getCPointCount(int camIndex1, int camIndex2);
	QString getCPoint(int index, int camIndex1, int camIndex2);	// Return format -> "x1:y1:x2:y2"
	int getCPointCountEx(int camIndex);
	QString getCPointEx(int index, int camIndex);	// Return format -> "x:y:index"

	// Seam view...
	QString getSeamLabelPos(int camIndex);
	void enableSeam(int camIndex = -1);		// -1: all disbable, 0: view all camera seams

	// Notification
	void onNotify(int type, QString title, QString msg);
	int getNotificationCount();
	QString getNotification(int index);
	bool removeNotification(int index);		// -1: all remove

	// Get slot info
	// type parameter is following..
	// CAPTURE_DSHOW = 1
	// CAPTURE_VIDEO = 2
	// CAPTURE_FILE = 3
	QString getSlotInfo(QString name, QString ext, int type);	// name: video or image file name or DirectShow camera name

	// Save file quality
	int getQuality() { return m_d360Data.getTempGlobalAnimSettings().m_crf; }
	void setQuality(int quality) { m_d360Data.getTempGlobalAnimSettings().m_crf = quality; }

	// Use NVidia
	void enableNvidia(bool isGpu) { m_d360Data.getGlobalAnimSettings().m_isGPU = isGpu; }
	bool isNvidia() { return m_d360Data.getGlobalAnimSettings().m_isGPU; }

	// Advertisement
	void insertAdv(int frameWidth, int frameHeight, QPoint pt0, QPoint pt1, QPoint pt2, QPoint pt3, QString advFile, bool isVideo);
	void removeAdv();
	void enableAdv(bool);
	bool isAdv() { return m_d360Data.getGlobalAnimSettings().m_isAdv; }
	void resetCamSettings();

	bool isStereo() { return m_d360Data.getGlobalAnimSettings().isStereo(); }

	// Camera Parameters
	float  getYaw(int deviceNum) { return m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_yaw; }
	void setYaw(float yaw, int deviceNum) { m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_yaw = yaw; }
	float getPitch(int deviceNum) { return m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_pitch; }
	void setPitch(float pitch, int deviceNum) { m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_pitch = pitch; }
	float getRoll(int deviceNum) { return m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_roll; }
	void setRoll(float roll, int deviceNum) { m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_roll = roll; }
	float getFov(int deviceNum) { return m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_fov; }
	void setFov(float fov, int deviceNum) { m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_fov = fov; }
	float getK1(int deviceNum) { return m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_k1; }
	void setK1(float k1, int deviceNum) { m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_k1 = k1; }
	float getK2(int deviceNum) { return m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_k2; }
	void setK2(float k2, int deviceNum) { m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_k2 = k2; }
	float getK3(int deviceNum) { return m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_k3; }
	void setK3(float k3, int deviceNum) { m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_k3 = k3; }
	float getOffsetX(int deviceNum) { return m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_offset_x; }
	void setOffsetX(float offset_x, int deviceNum) { m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_offset_x = offset_x; }
	float getOffsetY(int deviceNum) { return m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_offset_y; }
	void setOffsetY(float offset_y, int deviceNum) { m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_offset_y = offset_y; }
	float getExpOffset(int deviceNum) { return m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_expOffset; }
	void setExpOffset(float expOffset, int deviceNum) { m_d360Data.getGlobalAnimSettings().cameraSettingsList()[deviceNum].m_cameraParams.m_expOffset = expOffset; }
};