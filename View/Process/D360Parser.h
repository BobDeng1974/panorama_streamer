#pragma once

#include <QObject>
#include <QDebug>
#include <vector>
#include <QMap>
#include <QSettings>
#include "libavcodec/avcodec.h"
#include "Structures.h"
#include "CaptureXimea.h"

class GlobalAnimSettings : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int fps READ getFps WRITE setFps NOTIFY stateChanged)
	Q_PROPERTY(int xres READ getXres WRITE setXres NOTIFY stateChanged)
	Q_PROPERTY(int yres READ getYres WRITE setYres NOTIFY stateChanged)
	Q_PROPERTY(QString cameraCalib READ getCameraCalib WRITE setCameraCalib NOTIFY stateChanged)
	Q_PROPERTY(int sampleFmt READ getSampleFmt WRITE setSampleFmt NOTIFY stateChanged)
	Q_PROPERTY(int sampleRate READ getSampleRate WRITE setSampleRate NOTIFY stateChanged)
	Q_PROPERTY(int lag READ getLag WRITE setLag NOTIFY stateChanged)
	Q_PROPERTY(int panoXres READ getPanoXres WRITE setPanoXres NOTIFY stateChanged)
	Q_PROPERTY(int panoYres READ getPanoYres WRITE setPanoYres NOTIFY stateChanged)
	Q_PROPERTY(int streamMode READ getStreamMode WRITE setStreamMode NOTIFY stateChanged)
	Q_PROPERTY(QString streamURL READ getStreamURL WRITE setStreamURL NOTIFY stateChanged)
	Q_PROPERTY(int deviceCount READ getDeviceCount WRITE setDeviceCount NOTIFY stateChanged)

signals:
	void stateChanged();
	public slots:
	void set(QObject* settings) {
		operator = (*(qobject_cast<GlobalAnimSettings*>(settings)));
	}
	QObject* get() { return (QObject*)this; }
	//captureType
	/*int getCaptureType() {
		switch (m_captureType)
		{
		case D360::Capture::CAPTURE_DSHOW:
			return 0;
			break;
		case D360::Capture::CAPTURE_XIMEA:
			return 1;
			break;
		case D360::Capture::CAPTURE_VIDEO:
			return 2;
			break;
		case D360::Capture::CAPTURE_FILE:
			return 3;
			break;
		default:
			break;
		}
		return m_captureType;
	}
	void setCaptureType(int captureType){
		switch (captureType)
		{
		case 0: m_captureType = D360::Capture::CAPTURE_DSHOW; break;
		case 1: m_captureType = D360::Capture::CAPTURE_XIMEA; break;
		case 2: m_captureType = D360::Capture::CAPTURE_VIDEO; break;
		case 3: m_captureType = D360::Capture::CAPTURE_FILE; break;
		default:
			break;
		}
	}*/
	//cameraCount
	int getCameraCount() { return m_cameraCount; }
	void setCameraCount(int cameraCount) { m_cameraCount = cameraCount; }
	//fps
	int getFps() { return m_fps; }
	void setFps(int fps) { m_fps = fps; }
	//input resolution
	int getXres() { return m_xres; }
	void setXres(int xres) { m_xres = xres; }
	int getYres() { return m_yres; }
	void setYres(int yres) { m_yres = yres; }
	//cameraClib
	QString getCameraCalib() { return m_cameraCalibFile; }
	void setCameraCalib(QString cameraCalib) { m_cameraCalibFile = cameraCalib; }
	//sample Fmt
	int getSampleFmt() { return m_sampleFmt; }
	void setSampleFmt(int sampleFmt) { m_sampleFmt = sampleFmt; }
	//sample Rate
	int getSampleRate() { return m_sampleRate; }
	void setSampleRate(int sampleRate) { m_sampleRate = sampleRate; }
	//lag
	int getLag() { return m_audioLag; }
	void setLag(int lag) { m_audioLag = lag; }
	//output resolution
	int getPanoXres() { return m_panoXRes; }
	void setPanoXres(int panoXres) { m_panoXRes = panoXres; }
	int getPanoYres() { return m_panoYRes; }
	void setPanoYres(int panoYres) { m_panoYRes = panoYres; }
	// stream
	int getStreamMode() {
		if (m_wowzaServer.trimmed().isEmpty())	return 1;
		return 0;
	}
	void setStreamMode(int mode) {
		// Not implemented.
	}
	QString getStreamURL() {
		if (m_wowzaServer.trimmed().isEmpty())	return m_offlineVideo;
		return m_wowzaServer;
	}
	void setStreamURL(QString streamURL) {
		m_offlineVideo = streamURL;
	}
	//device Count
	int getDeviceCount() { return m_cameraCount; }
	void setDeviceCount(int deviceCount) { }

	// Start/End
	int getStartFrame() { return m_startFrame; }
	void setStartFrame(int nFrame) { m_startFrame = nFrame; }
	int getEndFrame() { return m_endFrame; }
	void setEndFrame(int nFrame) { m_endFrame = nFrame; }
    // File Ext
	QString getFileExt() { return m_fileExt; }
	void setFileExt(QString fileExt) { m_fileExt = fileExt; }

public:

	typedef std::vector<CameraInput> CameraSettingsList;

	GlobalAnimSettings()
	{
		setDefault();
	}
	GlobalAnimSettings& operator = (const GlobalAnimSettings& other);

	void setDefault()
	{
		m_cameraCount = 0;
		m_startFrame = 0;
		m_endFrame = -1;
		m_fileVersion = 2.0;
		m_fps = 60.0f;
		m_fYaw = 0;
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fInteractYaw = m_fInteractPitch = m_fInteractRoll = 0;

		m_playbackfps = 30.0f;

		m_xres = 1280;
		m_yres = 960;
		m_panoXRes = 4096;
		m_panoYRes = 2048;

		m_captureType = D360::Capture::CAPTURE_DSHOW;
		
		m_stitch = true;
		m_oculus = false;
		
		m_ui = true;
		
		m_snapshotDir = "D:/";

		m_audioLag = 0;
		m_sampleFmt = 8;
		m_sampleRate = 44100;

		m_audioDeviceName = "";

		m_offlineVideo = "";
		m_videoCodec = (int)AV_CODEC_ID_H264;
		m_audioCodec = (int)AV_CODEC_ID_AAC;
		m_isGPU = false;
		m_wowzaServer = "";

		m_cameraCalibFile = "";
		m_blendingMode = MultiBandBlending;
		m_multiBandLevel = 0;
		m_fileExt = "dpx";

		m_lensType = 1;	// Default fisheye lens
		m_fov = 240;
		
		m_leftIndices.clear();
		m_rightIndices.clear();
		m_cameraSettings.clear();

		m_crf = 23;

		m_isAdv = false;
		m_advFile = "";
		m_isVideoAdv = false;

		return;
	}

	enum Camera3DInputConfig
	{
		Stereo,
		Global
	};

	enum Format3DOutput
	{
		TopDown,
		SideBySide
	};

	enum BlendingMode
	{
		Feathering,
		MultiBandBlending
	};

	bool isStereo() { return m_leftIndices.size() > 0 && m_rightIndices.size() > 0; }

	int     m_cameraCount;

	bool	m_ui;

	int		m_startFrame;
	int		m_endFrame;
	float	m_fileVersion;

	float	m_fps;
	float	m_fYaw;
	float	m_fPitch;
	float	m_fRoll;
	float	m_fInteractYaw;
	float	m_fInteractPitch;
	float	m_fInteractRoll;

	float   m_playbackfps;

	int		m_xres;
	int		m_yres;
	
	
	D360::Capture::CaptureDomain m_captureType;
	
	bool	m_stitch;
	bool	m_oculus;
	
	QString m_fileExt;
	QString m_snapshotDir;

	// [StitchSettings]
	BlendingMode m_blendingMode;
	int m_multiBandLevel;
	int		m_panoXRes;
	int		m_panoYRes;

	// [Audio]
	QString m_audioDeviceName;		//'deviceName'
	int		m_audioLag;
	int		m_sampleFmt;
	int		m_sampleRate;

	// [Streaming]
	QString m_offlineVideo;			//'offlineVideo'	: video file name for offline stream saving. if '', skip saving video
	int m_videoCodec;
	int m_audioCodec;
	bool m_isGPU;
	QString m_wowzaServer;			//'server'			: wowza server url 

	typedef std::vector< int > CameraIndices;

	QString m_cameraCalibFile;

	int m_lensType;
	int m_fov;

	int m_crf;

	bool m_isAdv;
	QString m_advFile;
	bool m_isVideoAdv;

	CameraIndices& getLeftIndices()
	{
		return m_leftIndices;
	}
	CameraIndices& getRightIndices()
	{
		return m_rightIndices;
	}

	int getAudioChannelCount()
	{
		int audioChannelCount = 0;
		for (int i = 0; i < m_cameraSettings.size(); i++)
		{
			if (m_cameraSettings[i].audioType != CameraInput::NoAudio)
			{
				audioChannelCount++;
			}
		}

		return audioChannelCount;
	}

	CameraSettingsList& cameraSettingsList()
	{
		return m_cameraSettings;
	}

	void refreshTempCameraSettingsList() {
		m_tempCameraSettings = m_cameraSettings;
	}

	void rollbackCameraSettingsList() {
		m_cameraSettings = m_tempCameraSettings;
	}
	
	void setCameraSettingsList(CameraSettingsList cameraList)
	{
		m_cameraSettings = cameraList;
	}

protected:
	CameraIndices m_leftIndices;
	CameraIndices m_rightIndices;
	CameraSettingsList m_cameraSettings;
	CameraSettingsList m_tempCameraSettings;
};

class D360Parser : public QObject
{
	Q_OBJECT
public:
	D360Parser(QObject* parent = 0);
	~D360Parser();

	void clear();
	
	//void saveINI(std::string iniFileName);
	//int parseJSON(std::string fileName);
	//int saveJSON(std::string fileName);
	//int parseXML(QString fileName);
	int parsePAC(QString fileName, GlobalAnimSettings::CameraSettingsList& cameraSettings);
	int parsePTS(QString fileName, GlobalAnimSettings::CameraSettingsList& cameraSettings);
	int parseCameraCalibrationFile(QString fileName, GlobalAnimSettings::CameraSettingsList& cameraSettings);


	GlobalAnimSettings& getGlobalAnimSettings()
	{
		return m_globalAnimSettings;
	}

	GlobalAnimSettings& getTempGlobalAnimSettings() { return m_tempGlobalAnimSettings; }

	void resetTempGlobalAnimSettings() { 
		m_tempGlobalAnimSettings = m_globalAnimSettings;
	}

	void clearTempGlobalAnimSettings() {
		m_tempGlobalAnimSettings = GlobalAnimSettings();
	}

	//startFrame, endFrame
	float startFrame()
	{
		return m_globalAnimSettings.m_startFrame;
	}
	void setStartFrame(float fStartFrame){
		m_globalAnimSettings.m_startFrame = fStartFrame;
	}
	float endFrame()
	{
		return m_globalAnimSettings.m_endFrame;
	}

	void setEndFrame(float fEndFrame) { 
		m_globalAnimSettings.m_endFrame = fEndFrame;
	}

	//version
	float fileVersion()
	{
		return m_globalAnimSettings.m_fileVersion;
	}
	//captureType
	int getCaptureType() {
		int ret = 0;
		switch (m_globalAnimSettings.m_captureType)
		{
		case D360::Capture::CAPTURE_DSHOW:
			ret = 0;
			break;
		case D360::Capture::CAPTURE_XIMEA:
			ret = 1;
			break;
		case D360::Capture::CAPTURE_VIDEO:
			ret = 2;
			break;
		case D360::Capture::CAPTURE_FILE:
			ret = 3;
			break;
		default:
			break;
		}
		return ret;
	}
	void setCaptureType(int captureType){
		switch (captureType)
		{
		case 0: m_globalAnimSettings.m_captureType = D360::Capture::CAPTURE_DSHOW; break;
		case 1: m_globalAnimSettings.m_captureType = D360::Capture::CAPTURE_XIMEA; break;
		case 2: m_globalAnimSettings.m_captureType = D360::Capture::CAPTURE_VIDEO; break;
		case 3: m_globalAnimSettings.m_captureType = D360::Capture::CAPTURE_FILE; break;
		default:
			break;
		}
	}
	
	//cameraCount
	int getCameraCount() { return m_globalAnimSettings.m_cameraCount; }
	void setCameraCount(int cameraCount) { m_globalAnimSettings.m_cameraCount = cameraCount; }

	//fps
	float getFps() { return m_globalAnimSettings.m_fps; }
	void setFps(float fps) { m_globalAnimSettings.m_fps = fps; }

	//input resolution
	int getXres() { return m_globalAnimSettings.m_xres; }
	void setXres(int xres) { m_globalAnimSettings.m_xres = xres; }
	int getYres() { return m_globalAnimSettings.m_yres; }
	void setYres(int yres) { m_globalAnimSettings.m_yres = yres; }

	//cameraClib
	QString getCameraCalib() { return m_globalAnimSettings.m_cameraCalibFile; }
	void setCameraCalib(QString cameraCalib) {
		m_globalAnimSettings.m_cameraCalibFile = cameraCalib; 
	}

	//sample Fmt
	int getSampleFmt() { return m_globalAnimSettings.m_sampleFmt; }
	void setSampleFmt(int sampleFmt) { m_globalAnimSettings.m_sampleFmt = sampleFmt; }

	//sample Rate
	int getSampleRate() { return m_globalAnimSettings.m_sampleRate; }
	void setSampleRate(int sampleRate) { m_globalAnimSettings.m_sampleRate = sampleRate; }

	//lag
	int getLag() { return m_globalAnimSettings.m_audioLag; }
	void setLag(int lag) { m_globalAnimSettings.m_audioLag = lag; }

	//output resolution
	int getPanoXres() { return m_globalAnimSettings.m_panoXRes; }
	void setPanoXres(int panoXres) { m_globalAnimSettings.m_panoXRes = panoXres; }
	int getPanoYres() { return m_globalAnimSettings.m_panoYRes; }
	void setPanoYres(int panoYres) { m_globalAnimSettings.m_panoXRes = panoYres; }

	// stream
	int getStreamMode() {
		if (m_globalAnimSettings.m_wowzaServer.trimmed().isEmpty())	return 1;
		return 0;
	}
	void setStreamMode(int mode) {
		// Not implemented.
	}
	QString getStreamURL() {
		if (m_globalAnimSettings.m_wowzaServer.trimmed().isEmpty())	return m_globalAnimSettings.m_offlineVideo;
		return m_globalAnimSettings.m_wowzaServer;
	}
	void setStreamURL(QString streamURL) {
		m_globalAnimSettings.m_offlineVideo = streamURL;
	}

	//device Count
	int getDeviceCount() { return m_globalAnimSettings.m_cameraCount; }
	void setDeviceCount(int deviceCount) { }
	//yaw
	float  getYaw(int deviceNum);
	void setYaw(float yaw, int deviceNum);
	//pitch
	float getPitch(int deviceNum);
	void setPitch(float pitch, int deviceNum);
	
	float getRoll(int deviceNum);
	void setRoll(float roll, int deviceNum);

	// Blending parameters
	float  getLeft(int iDeviceNum);
	void setLeft(float fLeft, int iDeviceNum);
	float getRight(int iDeviceNum);
	void setRight(float fRight, int iDeviceNum);
	float getTop(int iDeviceNum);
	void setTop(float fTop, int iDeviceNum);
	float getBottom(int iDevicNum);
	void setBottom(float fBottom, int iDeviceNum);
	
	// gain
	int getCameraGain(int iDeviceNum);
	void setCameraGain(int iGain, int iDeviceNum);

	//stereo
	int getStereoType(int iDeviceNum);
	void setStereoType(int iStereoType, int iDeviceNum);
	void setTempCameraSettings();
	void resetCameraSettings();
	//audioType
	void setAudioType(int iAudioType, int iDeviceNum);

	void setTempStereoType(int, int);
	int getTempStereoType(int iDeviceNum);
	void setTempImagePath(int, QString);
	QString getTempImagePath(int iDeviceNum);
	void setTempImagePrefix(int, QString);
	QString getTempImagePrefix(int iDeviceNum);
	void setTempImageExt(int, QString);
	QString getTempImageExt(int iDeviceNum);
	void setTempAudioSettings(int, int);
	int getTempAudioSettings(int iDeviceNum);
	void clearStereoList() { m_tempStereoList.clear(); m_tempAudioList.clear(); }
public slots:	
	bool parseINI(QString iniFileName);
	void saveINI(QString iniFileName);
	void initTemplateVideo(QMap<int, QString> videoPathList);
	void initTemplateCamera(QMap<int, QString> cameraNameList, QMap<int, QString> audioNameList);
	void initTemplateImage(QMap<int, QString> imagePathList);
signals:

protected:
	GlobalAnimSettings m_globalAnimSettings;
	GlobalAnimSettings m_tempGlobalAnimSettings;
	GlobalAnimSettings::CameraSettingsList m_tempCameraList;
	QMap<int, int> m_tempStereoList;
	QMap<int, int> m_tempAudioList;
	QMap<int, QString> m_tempPathList;
	QMap<int, QString> m_tempPrefixList;
	QMap<int, QString> m_tempExtList;
private:

};
#pragma once
