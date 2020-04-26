#include "QmlMainWindow.h"
#include <QApplication>
#include "SharedImageBuffer.h"
#include "CaptureDevices.h"
#include "QmlRecentDialog.h"
#include "D360Parser.h"
#include "define.h"
#include "SlotInfo.h"
#include <QDir>
#include <QtXml/QDomDocument>
#include <QQuickWindow>
#include <iostream>
#include <sstream>
#include <QFile>

QmlMainWindow* g_mainWindow = NULL;
int snapshotCount = 0;
PanoLog* g_logger = NULL;

QmlMainWindow::QmlMainWindow()
: m_Name("PanoOne")
, m_isExit(false)
, m_isStarted(false)
, m_calibResult(false)
, m_oculusDevice(NULL)
, m_isEndCaptureThreads(true)
{
	g_mainWindow = this;
	m_viewMode = LIVE_VIEW;		
	
	//m_offlineVideoSaveProcess = 0;
	m_streamProcess = 0;
	m_process = 0;
	sharedImageBuffer = 0;
	
	m_stitcherView = 0;

	m_recentFullPath = "";
	m_recentPath = "";
	m_recentTitle = "";
	m_recentTitleList.clear();
	m_recentSourcePathList.clear();
	m_recentImageIDList.clear();
	m_bstitch = false;
	m_binteract = false;
	m_bpreview = false;
	m_filecount = 0;
	m_recentFullPath = "";
	m_eTimeValue = "";
	m_fpsValue = "";
	m_connectedCameras = false;
	m_reconnectCameras = false;

	if (m_process == NULL) {		// Only will be call once.
		m_process = new D360Process;
		sharedImageBuffer = m_process->getSharedImageBuffer();
	}

// 	m_blMousePressed = false;
	initDevices();	

	m_calib = new CalibProcess("", this);

	m_logger.setParent(this);
	m_logger.initialize(PANO_LOG_LEVEL::WARNING, "PanoOneHistory.log");
	m_logger.enableStdout();
	g_logger = &m_logger;

	m_outType = 0;	// To file

	m_advThread = NULL;
}


QmlMainWindow::~QmlMainWindow()
{
	//closeMainWindow();

	{
		g_mainWindow = NULL;
		QGuiApplication::quit();
	}
}

void QmlMainWindow::releaseThreads()
{
	/*if (m_offlineVideoSaveProcess)
	{
		delete m_offlineVideoSaveProcess;
		m_offlineVideoSaveProcess = NULL;
	}*/
	if (m_streamProcess)
	{
		delete m_streamProcess;
		m_streamProcess = NULL;
	}
	if (m_process)
	{
		delete m_process;
		m_process = NULL;
	}
	if (m_oculusDevice)
	{
		delete m_oculusDevice;
		m_oculusDevice = NULL;
	}
	//std::shared_ptr< D360Sticher > stitcher = sharedImageBuffer->getStitcher();


#if 0 /*[C]*/
	if (liveView)
	{
		delete liveView;
		liveView = 0;
	}
	if (stitcherView)
	{
		delete stitcherView;
		stitcherView = 0;
	}
#endif

	//deviceNumberMap.clear();
}

void QmlMainWindow::receiveKeyIndex(int level)
{
}

int QmlMainWindow::level()
{ 
	if (getGlobalAnimSetting().m_blendingMode == GlobalAnimSettings::Feathering)
		return -1;
	else if (getGlobalAnimSetting().m_blendingMode == GlobalAnimSettings::MultiBandBlending)
	{
		return getGlobalAnimSetting().m_multiBandLevel;
	}
	return 0;
}

void QmlMainWindow::closeMainWindow()
{
	m_isExit = true;
	saveRecentMgrToINI();
	
	if (m_connectedCameras) {
		disconnectCameras();
	}
	else
		finishedThread(THREAD_TYPE_MAIN);
}

extern QString getStringFromWString(wstring wstr);

void QmlMainWindow::reportError(int type, QString msg, int id) {
	QString typeStr = "";
	static int cameraThreadsEndNum = 0;
	switch (type)
	{
	case THREAD_TYPE_MAIN:
		typeStr = "PanoOne";
		break;
	case THREAD_TYPE_STITCHER:
		typeStr = "Stitcher";
		break;
	case THREAD_TYPE_CAPTURE:
		typeStr = "Capture";
		if (msg == "EOF")
			cameraThreadsEndNum++;
		break;
	case THREAD_TYPE_AUDIO:
		typeStr = "Capture";
		break;
	case THREAD_TYPE_STREAM:
		typeStr = "Stream";
		break;
	case THREAD_TYPE_OCULUS:
		typeStr = "Oculus";
		break;
	default: typeStr = "PanoOne"; break;
	}

	QString errorStr = QString("[%1] [%2] [ERROR] %3").arg(typeStr).ARGN(id).arg(msg);
	if (cameraThreadsEndNum == m_d360Data.getGlobalAnimSettings().cameraSettingsList().size()) {
		PANO_ERROR(errorStr);
		m_isEndCaptureThreads = true;
	}
	else {
		PANO_N_ERROR(errorStr);
		//emit setErrorMsg(errorStr);
	}
}

void QmlMainWindow::startedThread(int type, QString msg, int id) {
	QString typeStr = "";
	static int cameraThreadsStartedNum = 0;
	static int audioThreadsStartedNum = 0;
	switch (type)
	{
	case THREAD_TYPE_MAIN:
		typeStr = "PanoOne";
		break;
	case THREAD_TYPE_CAPTURE:
		typeStr = "Capture";
		cameraThreadsStartedNum++;
		break;
	case THREAD_TYPE_AUDIO:
		typeStr = "Audio";
		audioThreadsStartedNum++;
		break;
	case THREAD_TYPE_STITCHER:
		typeStr = "Stitcher";
		if (m_d360Data.getGlobalAnimSettings().m_wowzaServer != "" || 
			m_d360Data.getGlobalAnimSettings().m_offlineVideo != "")
			break;
	case THREAD_TYPE_STREAM:
		if (type == THREAD_TYPE_STREAM)
			typeStr = "Stream";
		// Camera threads start...
		for (unsigned i = 0; i < cameraModuleMap.size(); ++i)
		{
			if (cameraModuleMap[i]->isConnected())
				cameraModuleMap[i]->startThreads();		// Must start after stream thread started!
		}

		if (m_d360Data.getGlobalAnimSettings().m_oculus)
			break;

		// Audio threads start...
		for (unsigned i = 0; i < cameraModuleMap.size(); ++i)
		{
			if (m_d360Data.getGlobalAnimSettings().cameraSettingsList()[i].isExistAudio() == false)
				continue;

			AudioThread* audioThread = audioModuleMap[i];
			AudioInput* mic = audioThread->getMic();
			if (mic)	audioThread->startThread();
		}
		break;
	case THREAD_TYPE_OCULUS:
		typeStr = "Oculus";
		break;
	default: typeStr = "PanoOne"; break;
	}

	PANO_LOG(QString("[%1] [%2] [Started] %3").arg(typeStr).ARGN(id).arg(msg));

	if (cameraThreadsStartedNum == m_d360Data.getGlobalAnimSettings().cameraSettingsList().size() &&
		audioThreadsStartedNum == getAudioSelCnt()) {
		cameraThreadsStartedNum = 0;
		audioThreadsStartedNum = 0;
		if (!m_d360Data.getGlobalAnimSettings().m_cameraCalibFile.isEmpty()) {
			//m_isStarted = true;
			//emit started(true);			// Sending started signal to QML.
		}
		else {
			//calibrate(0); // Will be call by startCalibrating()
		}
	}
}

void QmlMainWindow::finishedThread(int type, QString msg, int id) {
	QString typeStr = "";
	static int cameraThreadsFinishedNum = 0;
	static int audioThreadsFinishedNum = 0;
	static int cameraThreadsEndNum = 0;
	switch (type)
	{
	case THREAD_TYPE_MAIN:
		typeStr = "PanoOne";
		if (m_isExit == true) {
			setExit(true);
		}
		break;
	case THREAD_TYPE_STITCHER:
		typeStr = "Stitcher";
		/*if (m_offlineVideoSaveProcess && m_offlineVideoSaveProcess->isOpened())
		{
			if (!m_offlineVideoSaveProcess->isFinished())
				m_offlineVideoSaveProcess->stopStreamThread();
		}
		else */if (m_streamProcess && m_streamProcess->isOpened())
		{
			if (!m_streamProcess->isFinished())
				m_streamProcess->stopStreamThread();
		}
		else if (m_oculusDevice && m_oculusDevice->isConnected())
		{
			m_oculusDevice->stop();
		}
		else {	// If streaming process is not initialized.
			if (m_isExit == true) {
				releaseThreads();
				setExit(true);		// On this function, send onExitChagned signal
			}

			if (!m_isExit && m_connectedCameras) {
				m_connectedCameras = false;
				m_process->initialize();
				connectToCameras();
				//QMetaObject::invokeMethod(this, "connectToCameras", Qt::QueuedConnection);
			}
		}
		break;
	case THREAD_TYPE_CAPTURE:
		typeStr = "Capture";
		if (msg == "EOF")
			cameraThreadsEndNum++;
		else
			cameraThreadsFinishedNum++;
		break;
	case THREAD_TYPE_AUDIO:
		typeStr = "Audio";
		audioThreadsFinishedNum++;
		break;
	case THREAD_TYPE_OCULUS:
		typeStr = "Oculus";
		if (m_d360Data.getGlobalAnimSettings().m_wowzaServer != "" ||
			m_d360Data.getGlobalAnimSettings().m_offlineVideo != "")
			break;
	case THREAD_TYPE_STREAM:
		if (type == THREAD_TYPE_STREAM)
			typeStr = "Stream";
		if (m_isExit == true) {
			releaseThreads();
			setExit(true);		// On this function, send onExitChagned signal
		}

		if (!m_isExit && m_connectedCameras && type == THREAD_TYPE_STREAM) {
			m_connectedCameras = false;
			m_process->initialize();
			//QMetaObject::invokeMethod(this, "connectToCameras", Qt::QueuedConnection);
			connectToCameras();
		}
		break;
	default: typeStr = "PanoOne"; break;
	}

	PANO_LOG(QString("[%1] [%2] [Finished] %3 : %4").arg(typeStr).ARGN(id).arg(m_isExit ? " [Exit]" : "").arg(msg));

	if (cameraThreadsFinishedNum > 0 && cameraThreadsFinishedNum == m_deletedCameraCnt &&
		audioThreadsFinishedNum == m_deletedAudioCnt && m_isStarted) {
		cameraThreadsFinishedNum = 0;
		audioThreadsFinishedNum = 0;
		m_isStarted = false;
		//Only must be stop stitcher thread after stop all thread associated with cameras.
		if (!m_process->getStitcherThread()->isFinished())
			m_process->getStitcherThread()->stopStitcherThread();
	}
}

void QmlMainWindow::initDevices()
{
	std::vector<std::string> videoDevices, audioDevices;

	CaptureDevices *capDev = new CaptureDevices();
	std::vector<std::wstring> videoDevicesW, audioDevicesW;
	capDev->GetVideoDevices(&videoDevicesW);
	capDev->GetAudioDevices(&audioDevicesW);
	delete capDev;
	m_cameraCnt = videoDevicesW.size();
	m_audioCnt = audioDevicesW.size();
	for (int i = 0; i < videoDevicesW.size(); i++)
		m_videoDevices[i] = getStringFromWString(videoDevicesW[i]);
	for (int i = 0; i < audioDevicesW.size(); i++)
		m_audioDevices[i] = getStringFromWString(audioDevicesW[i]);
}

bool QmlMainWindow::isSelectedCamera(QString name) {
	for (unsigned i = 0; i < m_d360Data.getGlobalAnimSettings().cameraSettingsList().size(); i++)
	{
		if (m_d360Data.getGlobalAnimSettings().cameraSettingsList()[i].name == name)
			return true;
	}
	return false;
}

bool QmlMainWindow::isSelectedAudio(QString name) {
	for (unsigned i = 0; i < m_d360Data.getGlobalAnimSettings().cameraSettingsList().size(); i++)
	{
		if (m_d360Data.getGlobalAnimSettings().cameraSettingsList()[i].audioName == name)
			return true;
	}
	return false;
}

int QmlMainWindow::getAudioSelCnt()
{
	int count = 0;
	for (unsigned i = 0; i < m_d360Data.getGlobalAnimSettings().cameraSettingsList().size(); i++)
	{
		if (m_d360Data.getGlobalAnimSettings().cameraSettingsList()[i].isExistAudio() == false)
			continue;

		count++;
	}

	return count;
}

void QmlMainWindow::resetConfigList()
{
	QMap<int, QString> list;
	m_d360Data.clearStereoList();
	for (unsigned i = 0; i < m_d360Data.getGlobalAnimSettings().cameraSettingsList().size(); i++)
	{
		if (m_d360Data.getGlobalAnimSettings().m_captureType == D360::Capture::CaptureDomain::CAPTURE_FILE)
			list[i] = m_d360Data.getGlobalAnimSettings().cameraSettingsList()[i].fileDir;
		else
			list[i] = m_d360Data.getGlobalAnimSettings().cameraSettingsList()[i].name;
		m_d360Data.setTempStereoType(i, m_d360Data.getGlobalAnimSettings().cameraSettingsList()[i].stereoType);
		if (m_d360Data.getGlobalAnimSettings().m_captureType != D360::Capture::CAPTURE_DSHOW)
			m_d360Data.setTempAudioSettings(i, m_d360Data.getGlobalAnimSettings().cameraSettingsList()[i].audioType);
	}
	switch (m_d360Data.getGlobalAnimSettings().m_captureType)
	{
	case D360::Capture::CaptureDomain::CAPTURE_VIDEO:
		m_videoPathList.clear(); m_videoPathList = list; break;
	case D360::Capture::CaptureDomain::CAPTURE_FILE:
		m_imagePathList.clear(); m_imagePathList = list; break;
	default:	// CAPTURE_DSHOW, CAPTURE_XIMIA, CAPTURE_OPENCV
		break;
	}

	m_d360Data.resetTempGlobalAnimSettings();
}

void QmlMainWindow::connectToCameras()
{
	resetConfigList();	// For after reconfiguration.
	snapshotCount = 0;

	GlobalAnimSettings::CameraSettingsList& cameraSettingsList = m_d360Data.getGlobalAnimSettings().cameraSettingsList();

	QString videoName = m_d360Data.getGlobalAnimSettings().m_offlineVideo;
	QString serverName = m_d360Data.getGlobalAnimSettings().m_wowzaServer;
	int videoCodec = m_d360Data.getGlobalAnimSettings().m_videoCodec;
	int audioCodec = m_d360Data.getGlobalAnimSettings().m_audioCodec;
	int audioLag = m_d360Data.getGlobalAnimSettings().m_audioLag;
	int sampleFmt = m_d360Data.getGlobalAnimSettings().m_sampleFmt;
	int sampleRate = m_d360Data.getGlobalAnimSettings().m_sampleRate;
	int panoWidth = m_d360Data.getGlobalAnimSettings().m_panoXRes;
	int panoHeight = m_d360Data.getGlobalAnimSettings().m_panoYRes;

	int startframe = m_d360Data.getGlobalAnimSettings().m_startFrame;
	int endframe = m_d360Data.getGlobalAnimSettings().m_endFrame;
	float fps = m_d360Data.getGlobalAnimSettings().m_fps;
	int crf = m_d360Data.getGlobalAnimSettings().m_crf;
	int nCameras = cameraSettingsList.size();

	std::shared_ptr< D360Sticher > stitcher = m_process->getStitcherThread();

	sharedImageBuffer->setStitcher(stitcher);
	sharedImageBuffer->setGlobalAnimSettings(&m_d360Data.getGlobalAnimSettings());
	
	if (m_d360Data.getGlobalAnimSettings().m_oculus)
		m_outType = 2;	// To oculus
	else if (!(videoName.isNull() || videoName.isEmpty()))
		m_outType = 0;	// To file
	else if (!(serverName.isNull() || serverName.isEmpty()))
		m_outType = 1;	// To RTMP stream

	if (nCameras <= 0) {
		PANO_ERROR(QString("Number of camera is invalid! (%1)").ARGN(nCameras));
		return;
	}

	if (!m_d360Data.getGlobalAnimSettings().m_oculus)
	{
		for (unsigned i = 0; i < nCameras; ++i)
		{
			if (m_d360Data.getGlobalAnimSettings().cameraSettingsList()[i].isExistAudio() == false)
				continue;

			AudioThread* audioThread = new AudioThread(this);
			audioThread->initialize(sharedImageBuffer, i);
#if 0	// Capture threads will be started after stream/stitching threads started!
			AudioInput* mic = audioThread->getMic();
			if (mic)	audioThread->startThread();
#endif
			audioModuleMap[i] = audioThread;
		}
	}

	m_calib->initialize();

	QMap <QString, int> camNameMap;
	for (unsigned i = 0; i < cameraSettingsList.size(); ++i)
	{
		PANO_LOG(QString("Attaching camera(%1)...").ARGN(i));

		CameraInput& camInput = cameraSettingsList[i];
		if (camNameMap.contains(camInput.name))
			camNameMap[camInput.name] = camNameMap[camInput.name]++;
		else
			camNameMap[camInput.name] = 0;
		if (!attach(camInput, i, startframe, endframe, fps, camNameMap[camInput.name])) {
			PANO_N_ERROR(QString("Camera(%1) connecting failed!").ARGN(i));
			continue;
		}

		PANO_LOG(QString("Attached camera(%1).").ARGN(i));
	}

	// Process initialize
	{
		D360Sticher* stitcher = sharedImageBuffer->getStitcher().get();

		disconnect(stitcher, SIGNAL(newPanoramaFrameReady(unsigned char*)),
			this, SLOT(streamPanorama(unsigned char*)));

		disconnect(stitcher, SIGNAL(updateStatisticsInGUI(struct ThreadStatisticsData)),
			this, SLOT(updateStitchingThreadStats(struct ThreadStatisticsData)));

		disconnect(stitcher, SIGNAL(finished(int, QString, int)), this, SLOT(finishedThread(int, QString, int)));
		disconnect(stitcher, SIGNAL(started(int, QString, int)), this, SLOT(startedThread(int, QString, int)));


		connect(stitcher, SIGNAL(newPanoramaFrameReady(unsigned char*)),
			this, SLOT(streamPanorama(unsigned char*)));

		connect(stitcher, SIGNAL(updateStatisticsInGUI(struct ThreadStatisticsData)),
			this, SLOT(updateStitchingThreadStats(struct ThreadStatisticsData)));

		connect(stitcher, SIGNAL(finished(int, QString, int)), this, SLOT(finishedThread(int, QString, int)));
		connect(stitcher, SIGNAL(started(int, QString, int)), this, SLOT(startedThread(int, QString, int)));
	}

	stitcher->init(QOpenGLContext::globalShareContext());
	stitcher->startThread();

	//m_offlineVideoSaveProcess = new StreamProcess(sharedImageBuffer, true, this);
	m_streamProcess = new StreamProcess(sharedImageBuffer, m_outType == 0, this);

	if (m_d360Data.getGlobalAnimSettings().m_oculus)
	{
		m_oculusDevice = new OculusRender(stitcher->getContext());
		if (!m_oculusDevice->isCreated()) {
			PANO_N_ERROR("OculusRift device is not created!");
			return;
		}
		connect(m_oculusDevice, SIGNAL(finished(int, QString, int)), this, SLOT(finishedThread(int, QString, int)));
		connect(m_oculusDevice, SIGNAL(started(int, QString, int)), this, SLOT(startedThread(int, QString, int)));
		m_interactView->setOculusObject(m_oculusDevice);
		m_oculusDevice->initialize(sharedImageBuffer, panoWidth, panoHeight);
		m_oculusDevice->setPanoramaTexture(stitcher->getPanoramaTextureId());
		m_oculusDevice->installEventFilter(this);
	}
	else
	{
		int channels = m_d360Data.getGlobalAnimSettings().getAudioChannelCount();
		int resultHeight = panoHeight;
		if (sharedImageBuffer->getGlobalAnimSettings()->isStereo())
			resultHeight *= 2;

		bool retval = false;
		/*retval = m_offlineVideoSaveProcess->initialize(true, videoName.toUtf8().constData(), panoWidth, resultHeight,
			sharedImageBuffer->getGlobalAnimSettings().m_fps, channels,
			(AVSampleFormat)sampleFmt, sampleRate, audioLag, videoCodec, audioCodec, crf);

		if (!videoName.isEmpty() && retval == false)
			PANO_N_ERROR("Cannot initialize HDD streaming process!");
		else if (retval == true)
			sharedImageBuffer->setStreamer(m_offlineVideoSaveProcess);*/

		retval = m_streamProcess->initialize(false, m_outType==0 ? videoName:serverName, panoWidth, resultHeight,
			sharedImageBuffer->getGlobalAnimSettings()->m_fps, channels,
			(AVSampleFormat)sampleFmt, sampleRate, audioLag, videoCodec, audioCodec);

		if (!serverName.isEmpty() && retval == false)
			PANO_N_ERROR("Cannot initialize RMTP streaming process!");
		else if (retval == true)
			sharedImageBuffer->setStreamer(m_streamProcess);
	}
#if 0		// Capture threads will be started after stream/stitching threads started!
	for (unsigned i = 0; i < cameraModuleMap.size(); ++i)
	{
		if (cameraModuleMap[i]->isConnected())
			cameraModuleMap[i]->startThreads();
	}
#endif
	//m_connectedCameras = true;

	if (m_d360Data.getGlobalAnimSettings().m_oculus)
	{
		m_oculusDevice->startThread();
	}
}

void QmlMainWindow::initProject() 
{
	disconnectCameras();
}

void QmlMainWindow::setCurrentMode(int viewMode)
{
	m_viewMode = (ViewMode) viewMode;
}

void QmlMainWindow::setBlendMode(int blendMode)
{
	if (blendMode == FEATHER_MODE)
		getGlobalAnimSetting().m_blendingMode = GlobalAnimSettings::Feathering;
	else if (blendMode == MULTIBAND_MODE)
		getGlobalAnimSetting().m_blendingMode = GlobalAnimSettings::MultiBandBlending;

	sharedImageBuffer->getStitcher()->setBlendingMode(getGlobalAnimSetting().m_blendingMode);
}

void QmlMainWindow::setPlayMode(int playMode)
{
	m_playMode = (PlayMode) playMode;

	for (unsigned i = 0; i < cameraModuleMap.size(); ++i)
	{
		cameraModuleMap[i]->getCaptureThread()->playAndPause(m_playMode == 1);
	}

	if (m_d360Data.getGlobalAnimSettings().m_isAdv && m_advThread)
		m_advThread->playAndPause(m_playMode == 1);
}

void QmlMainWindow::openProject()
{
	if (m_reconnectCameras == false) {
		m_reconnectCameras = true;
		connectToCameras();
		emit cameraResolutionChanged();
	}
}
bool QmlMainWindow::attach(CameraInput& camSettings, int deviceNumber, float startframe, float endframe, float fps, int dupIndex)
{
	//
	// Add created ImageBuffer to SharedImageBuffer object
	//
	sharedImageBuffer->add(deviceNumber, true);

	GlobalState s;
	s.m_curFrame = startframe;

	sharedImageBuffer->addState(deviceNumber, s, true);
	//
	// Create View 
	//
	PANO_LOG("New Camera module");
	cameraModuleMap[deviceNumber] = new CameraModule(deviceNumber, sharedImageBuffer, this);

	PANO_LOG(QString("Connecting Cam (FPS: %1)").ARGN(camSettings.fps));
	if (cameraModuleMap[deviceNumber]->connectToCamera(camSettings.xres,
		camSettings.yres,
		camSettings.exposure,
		camSettings.fps,
		getDeviceIndex(camSettings.audioName, 1), dupIndex))
	{
		PANO_LOG("Connected Camera");
	}
	else
	{
		PANO_ERROR(QString("[%1] Failed to connect camera!").ARGN(deviceNumber));
		sharedImageBuffer->setViewSync(deviceNumber, false);
		return false;
	}

	return true;
}


void QmlMainWindow::disconnectCameras()
{
	/*if (m_oculusDevice)	
	m_oculusDevice->stop();*/
	/*
	if (m_offlineVideoSaveProcess)
	{
		if (!m_offlineVideoSaveProcess->isFinished())
			m_offlineVideoSaveProcess->stopStreamThread();
	}

	if (m_streamProcess)
	{
		if (!m_streamProcess->isFinished())
			m_streamProcess->stopStreamThread();
	}
	
	if (m_process)
	{
		if (!m_process->getStitcherThread()->isFinished())		
			m_process->getStitcherThread()->stopStitcherThread();		
	}
	*/

	if (m_advThread) {
		m_advThread->stop();
		delete m_advThread;
		m_advThread = NULL;
	}

	m_deletedCameraCnt = cameraModuleMap.size();
	m_deletedAudioCnt = audioModuleMap.size();

	PANO_LOG("Terminating audio and camera capture threads...");
	QMapIterator<int, AudioThread*> iter(audioModuleMap);
	while (iter.hasNext()) {
		iter.next();
		PANO_LOG(QString("[%1] Stop audio thread.").ARGN(iter.key()));
		AudioThread* audio = iter.value();
		audio->stopAudioThread();
		delete audio;
	}
	audioModuleMap.clear();

	for (QMap<int, CameraModule *>::const_iterator itr = cameraModuleMap.begin(); itr != cameraModuleMap.end(); ++itr)
	{
		PANO_LOG(QString("[%1] Stop camera thread.").ARGN(itr.key()));
		CameraModule * camera = itr.value();
		delete camera;
	}
	cameraModuleMap.clear();
	deviceNumberMap.clear();

	if (m_isEndCaptureThreads && m_deletedAudioCnt == 0 && m_isStarted) {
		m_isStarted = false;
		//Only must be stop stitcher thread after stop all thread associated with cameras.
		if (!m_process->getStitcherThread()->isFinished())
			m_process->getStitcherThread()->stopStitcherThread();
	}
}


void QmlMainWindow::disconnectCamera(int index)
{
	// Local variable(s)
	bool doDisconnect = true;

	//
	// Disconnect camera
	//
	if (doDisconnect)
	{
		if (audioModuleMap.contains(index)) {
			audioModuleMap[index]->stopAudioThread();
			audioModuleMap.remove(index);
		}
	}
}


void QmlMainWindow::createCameraView(QObject* camView, int deviceNum)
{
	MCQmlCameraView *cameraView = qobject_cast<MCQmlCameraView*>(camView);
	cameraView->setCameraNumber(deviceNum);
	cameraView->setSharedImageBuffer(sharedImageBuffer);
	cameraView->init(true, NULL);
	m_cameraViewMap[deviceNum] = cameraView;
	GlobalAnimSettings::CameraSettingsList& cameraSettings = m_d360Data.getGlobalAnimSettings().cameraSettingsList();
	QString camName = cameraSettings[deviceNum].name;
	QString title = "";
	QString path = "";
	for (int i = camName.length() - 1; i > 0; i--)
	{
		if (camName.at(i) == '/')
		{
			path = camName.mid(0, i);
			title = camName.mid(i + 1, camName.length() - path.length() - 5);
			break;
		}
	}
	if (title == "")
	{
		cameraView->setCameraName(camName);
	}
	else{
		cameraView->setCameraName(title);
	}
}

void QmlMainWindow::deleteCameraView()
{
	if (m_cameraViewMap.size() > 0)
	{
		for (int i = 0; i < m_cameraViewMap.size(); i++)
		{
			MCQmlCameraView* cameraView = m_cameraViewMap[i];	
			cameraView->connect(cameraView, SIGNAL(sendClose()), cameraView, SLOT(closeView()));
			cameraView->closeCameraView();
			delete cameraView;
		}
	}
	m_cameraViewMap.clear();
}

void QmlMainWindow::createStitchView(QObject* stitchView){
	MCQmlCameraView *stitcherView = qobject_cast<MCQmlCameraView*>(stitchView);
	m_stitcherView = stitcherView;
	m_stitcherView->setSharedImageBuffer(sharedImageBuffer);
	m_stitcherView->init(false, NULL);
	m_bstitch = true;
}

void QmlMainWindow::deleteStitchView()
{
	if (!m_bstitch)
		return;	
	m_stitcherView->connect(m_stitcherView, SIGNAL(sendClose()), m_stitcherView, SLOT(closeView()));
	m_stitcherView->closeCameraView();
	delete m_stitcherView;	
}

void QmlMainWindow::createInteractView(QObject* interactView){
	QmlInteractiveView *interactiveView = qobject_cast<QmlInteractiveView*>(interactView);
	m_interactView = interactiveView;
	m_interactView->setSharedImageBuffer(sharedImageBuffer);
	m_binteract = true;
}

void QmlMainWindow::deleteInteractView()
{
	if (!m_binteract)
		return;
	m_interactView->connect(m_interactView, SIGNAL(sendClose()), m_interactView, SLOT(closeView()));
	m_interactView->closeCameraView();
	delete m_interactView;
}

void QmlMainWindow::createPreView(QObject* preView){
	MCQmlCameraView *preViewer = qobject_cast<MCQmlCameraView*>(preView);
	m_preView = preViewer;
	m_preView->setSharedImageBuffer(sharedImageBuffer);
	m_preView->init(false, NULL);
	m_bpreview = true;
}

void QmlMainWindow::deletePreView()
{
	if (!m_bpreview)
		return;
	m_preView->connect(m_preView, SIGNAL(sendClose()), m_preView, SLOT(closeView()));
	m_preView->closeCameraView();
	delete m_preView;
}

int QmlMainWindow::openIniPath(QString iniPath)
{	
	QString path;
	int repeatNum = -2;
	PANO_LOG("Open Ini File " + iniPath);
	if (!m_d360Data.parseINI(iniPath))
	{
		PANO_N_ERROR("This version of configuration is not supported.");
		return repeatNum;
	}
	repeatNum = -1;
	m_recentFullPath = iniPath;
	for (int i = iniPath.length() - 1; i > 0; i--)
	{
		if (iniPath.at(i) == '/')
		{
			path = iniPath.mid(0, i);
			m_recentTitle = iniPath.mid(i + 1, iniPath.length() - path.length() - 5);
			break;
		}
	}
	if (m_recentTitleList.size() == 0)
	{
		m_recentTitleList.push_back(m_recentTitle);
		m_recentSourcePathList.push_back(m_recentFullPath);
		m_recentImageIDList.push_back(conv.toString(m_d360Data.getCaptureType()));
	}
	else
	{
		if (m_recentSourcePathList.size() > 0)
		{
			for (int i = 0; i < m_recentSourcePathList.size(); i++)
			{
				if (m_recentSourcePathList[i] == m_recentFullPath)
				{
					m_recentTitleList.removeAt(i);
					m_recentSourcePathList.removeAt(i);
					m_recentImageIDList.removeAt(i);
					repeatNum = i;
					break;
				}
			}
			m_recentTitleList.push_back(m_recentTitle);
			m_recentSourcePathList.push_back(m_recentFullPath);
			m_recentImageIDList.push_back(conv.toString(m_d360Data.getCaptureType()));
		}
	}
	emit cameraResolutionChanged();
	return repeatNum;
}

int QmlMainWindow::saveIniPath(QString iniPath)
{	
	QString path;
	int repeatNum = -1;
	if (iniPath == "")
	{
		PANO_LOG("Save Ini File " + m_recentFullPath);
		m_d360Data.saveINI(m_recentFullPath);
	}
	else{
		PANO_LOG("Save Ini File " + iniPath);
		m_recentFullPath = iniPath;
		m_d360Data.saveINI(iniPath);
		for (int i = iniPath.length() - 1; i > 0; i--)
		{
			if (iniPath.at(i) == '/')
			{
				path = iniPath.mid(0, i);
				m_recentTitle = iniPath.mid(i + 1, iniPath.length() - path.length() - 5);
				break;
			}
		}
		if (m_recentTitleList.size() == 0)
		{
			m_recentTitleList.push_back(m_recentTitle);
			m_recentSourcePathList.push_back(m_recentFullPath);
			m_recentImageIDList.push_back(conv.toString(m_d360Data.getCaptureType()));
		}
		else
		{
			if (m_recentSourcePathList.size() > 0)
			{
				for (int i = 0; i < m_recentSourcePathList.size(); i++)
				{
					if (m_recentSourcePathList[i] == m_recentFullPath)
					{
						m_recentTitleList.removeAt(i);
						m_recentSourcePathList.removeAt(i);
						m_recentImageIDList.removeAt(i);
						repeatNum = i;
						break;
					}
				}
				m_recentTitleList.push_back(m_recentTitle);
				m_recentSourcePathList.push_back(m_recentFullPath);
				m_recentImageIDList.push_back(conv.toString(m_d360Data.getCaptureType()));
			}
		}
	}
	return repeatNum;
}

QString QmlMainWindow::getRecentPath()
{
	QString path;
	if (m_recentFullPath.length() > 34)
	{
		path = m_recentFullPath.mid(0, 31) + "...";
	}
	else{
		path = m_recentFullPath;
	}
	return path;
}

// type:0(video), type:1(audio)
int QmlMainWindow::getDeviceIndex(QString name, int type)
{
	int index = -1;
	if (type == 0)
	{
		QMapIterator<int, QString> iter(m_videoDevices);
		while (iter.hasNext()) {
			iter.next();
			if (iter.value().trimmed() == name.trimmed()) {
				index = iter.key();
				break;
			}
		}
	}
	else if (type == 1)
	{
		QMapIterator<int, QString> iter(m_audioDevices);
		while (iter.hasNext()) {
			iter.next();
			if (iter.value().trimmed() == name.trimmed()) {
				index = iter.key();
				break;
			}
		}
	}

	return index;
}

QString QmlMainWindow::getDeviceName(int index, int type)
{
	QString name = "";
	if (type == 0)
	{
		if (m_videoDevices.contains(index))
			name = m_videoDevices[index];
	}
	else if (type == 1)
	{
		if (m_audioDevices.contains(index))
			name = m_audioDevices[index];
	}

	return name;
}

QObject* QmlMainWindow::recDialog() const
{
	return m_qmlRecentDialog;
}

void QmlMainWindow::setRecDialog(QObject *recDialog)
{
	if (m_qmlRecentDialog == recDialog)
		return;

	QmlRecentDialog *s = qobject_cast<QmlRecentDialog*>(recDialog);
	if (!s)
	{
		PANO_LOG("Source must be a QmlRecentDialog type");
		return;
	}
	m_qmlRecentDialog = s;

	emit recDialogChanged(m_qmlRecentDialog);
}

void QmlMainWindow::saveRecentMgrToINI()
{
	QString recentfilepath = QDir::currentPath() + QLatin1String("/RecentManagement.ini");
	
	if (QFile(recentfilepath).exists())
		QFile(recentfilepath).remove();
	QSettings settings(recentfilepath, QSettings::IniFormat);
	{
		settings.beginGroup("Information");
			settings.setValue(QLatin1String("count"), (m_recentTitleList.size()));
		settings.endGroup();
		for (int i = 0; i < m_recentTitleList.size(); i++)
		{
			QString recentName = "RecentManagement_";
			recentName = recentName + conv.toString(i + 1);
			settings.beginGroup(recentName);
				settings.setValue(QLatin1String("Title"), (m_recentTitleList[i]));
				settings.setValue(QLatin1String("Path"), (m_recentSourcePathList[i]));
				settings.setValue(QLatin1String("CaptureType"), (m_recentImageIDList[i]));
			settings.endGroup();
		}
	}
}
bool QmlMainWindow::openRecentMgrToINI()
{
	m_recentImageIDList.clear();
	m_recentSourcePathList.clear();
	m_recentTitleList.clear();
	m_filecount = 0;
	QString recentfilepath = QDir::currentPath() + QLatin1String("/RecentManagement.ini");
	if (!QFile(recentfilepath).exists())
	{
		PANO_N_ERROR("RecentManagement.ini doesnot exist!");
		return false;
	}
	QSettings settings(recentfilepath, QSettings::IniFormat);
	{
		settings.beginGroup("Information");
		const QStringList childKeys = settings.childKeys();
		foreach(const QString &childKey, childKeys)
		{
			if (childKey == "count")
				m_filecount = settings.value(childKey).toInt();
		}
		settings.endGroup();
		for (int i = 0; i < m_filecount; i++)
		{
			QString recentName = "RecentManagement_";
			recentName = recentName + conv.toString(i + 1);

			settings.beginGroup(recentName);
			const QStringList childKeys = settings.childKeys();
			foreach(const QString &childKey, childKeys)
			{
				if (childKey == "Title")
				{
					//std::string camName = settings.value( childKey ).toString().toStdString();
					QString title = settings.value(childKey).toString();
					m_recentTitleList.push_back(title);
				}
				if (childKey == "Path")
				{
					QString sourcepath = settings.value(childKey).toString();
					m_recentSourcePathList.push_back(sourcepath);
				}
				if (childKey == "CaptureType")
				{
					QString imageID = settings.value(childKey).toString();
					m_recentImageIDList.push_back(imageID);
				}
			}
			settings.endGroup();
		}
	}
	return true;
}

void QmlMainWindow::sendRecentIndex(int index)
{
	m_d360Data.getGlobalAnimSettings().m_captureType = (D360::Capture::CaptureDomain)m_recentImageIDList[index].toInt();
	m_recentFullPath = m_recentSourcePathList[index];
	m_recentTitle = m_recentTitleList[index];
}

void QmlMainWindow::sendCurrentIndex(int index)
{
	if (index >= m_recentSourcePathList.size())
		return;
	int currentIndex = m_recentSourcePathList.size() - index - 1;
	m_d360Data.getGlobalAnimSettings().m_captureType = (D360::Capture::CaptureDomain)m_recentImageIDList[currentIndex].toInt();
	m_recentFullPath = m_recentSourcePathList[currentIndex];
	m_recentTitle = m_recentTitleList[currentIndex];
}

int QmlMainWindow::recentOpenIniPath(QString sourcepath)
{
	QString path;
	int repeatNum = -2;
	if (!QFile(sourcepath).exists()){
		PANO_N_ERROR("File does not exist");
		return repeatNum;
	}
	if (!m_d360Data.parseINI(sourcepath))
	{
		PANO_N_ERROR("This version of configuration is not supported.");
		return repeatNum;
	}
	repeatNum = -1;
	m_recentFullPath = sourcepath;
	
	for (int i = sourcepath.length() - 1; i > 0; i--)
	{
		if (sourcepath.at(i) == '/')
		{
			path = sourcepath.mid(0, i);
			m_recentTitle = sourcepath.mid(i + 1, sourcepath.length() - path.length() - 5);
			break;
		}
	}
	if (m_recentSourcePathList.size() > 0)	
	{
		for (int i = 0; i < m_recentSourcePathList.size(); i++)
		{
			if (m_recentSourcePathList[i] == m_recentFullPath)
			{
				m_recentTitleList.removeAt(i);
				m_recentSourcePathList.removeAt(i);
				m_recentImageIDList.removeAt(i);
				repeatNum = i;
				break;
			}
		}
		m_recentTitleList.push_back(m_recentTitle);
		m_recentSourcePathList.push_back(m_recentFullPath);
		m_recentImageIDList.push_back(conv.toString(m_d360Data.getCaptureType()));
	}
	return repeatNum;
}

int QmlMainWindow::checkAudioExist(QString videoFilePath){
	BaseFfmpeg* baseFfmpeg = new BaseFfmpeg();
	return baseFfmpeg->checkAudioChannelInVideoFile(videoFilePath);
}

void QmlMainWindow::streamPanorama(unsigned char* panorama)
{
	// if (!m_oculusDevice) // commented by B
	if (/*m_offlineVideoSaveProcess && */m_streamProcess)
	{
		/*m_offlineVideoSaveProcess->streamPanorama(panorama);*/
		m_streamProcess->streamPanorama(panorama);
	}
}

void QmlMainWindow::streamAudio(int devNum, void* audioFrame)
{
	// if (!m_oculusDevice) // commented by B
	if (/*m_offlineVideoSaveProcess && */m_streamProcess)
	{
		/*m_offlineVideoSaveProcess->streamAudio(devNum, audioFrame);*/
		m_streamProcess->streamAudio(devNum, audioFrame);
	}
}

void QmlMainWindow::openTemplateCameraIniFile()
{
	m_d360Data.getGlobalAnimSettings().m_cameraCount = m_cameraNameList.size();
	m_d360Data.initTemplateCamera(m_cameraNameList, m_audioNameList);
}


void QmlMainWindow::openTemplateVideoIniFile()
{
	m_d360Data.getGlobalAnimSettings().m_cameraCount = m_videoPathList.size();
	m_d360Data.initTemplateVideo(m_videoPathList);
}

void QmlMainWindow::openTemplateImageIniFile()
{
	m_d360Data.getGlobalAnimSettings().m_cameraCount = m_imagePathList.size();
	m_d360Data.initTemplateImage(m_imagePathList);
}

void QmlMainWindow::sendCameraName(int slotIndex, QString cameraName)
{
	m_cameraNameList[slotIndex] = cameraName;
}

void QmlMainWindow::sendAudioName(int slotIndex, QString audioName){
	m_audioNameList[slotIndex] = audioName;
}

void QmlMainWindow::sendVideoPath(int slotIndex, QString videoPath){
	m_videoPathList[slotIndex] = videoPath;
}

void QmlMainWindow::sendImagePath(int slotIndex, QString imagePath){
	m_imagePathList[slotIndex] = imagePath;
}

void QmlMainWindow::Stop()
{

}

void QmlMainWindow::Pause()
{
	PANO_LOG("Terminating threads...");
	QMapIterator<int, AudioThread*> iter(audioModuleMap);
	while (iter.hasNext()) {
		iter.next();
		PANO_LOG(QString("[%1] Stop audio thread.").ARGN(iter.key()));
		AudioThread* audio = iter.value();
		audio->pause();
	}
	audioModuleMap.clear();
}

void QmlMainWindow::Start()
{

}

void QmlMainWindow::deleteRecentList(QString title)
{
	int index = 0;
	for (int i = 0; i < m_recentTitleList.size(); i++)
	{
		if (title.compare(m_recentTitleList[i]) == 0)
		{
			index = i;
			break;
		}
	}
	m_recentImageIDList.removeAt(index);
	m_recentSourcePathList.removeAt(index);
	m_recentTitleList.removeAt(index);
}

void QmlMainWindow::updateStitchingThreadStats(struct ThreadStatisticsData statData)
{
	QString strFps = QString::number(statData.averageFPS, 'f', 1) + " fps";
	setETimeValue(statData.toString(m_d360Data.getFps()));
	setFpsValue(strFps);
}

void QmlMainWindow::setETimeValue(QString elapsedTime)
{
	m_eTimeValue = elapsedTime;
	emit elapsedTimeChanged(m_eTimeValue);
}

void QmlMainWindow::setFpsValue(QString fps)
{
	m_fpsValue = fps;
	emit fpsChanged(m_fpsValue);
}

void QmlMainWindow::enableOculus(bool oculusRift)
{
	int panoWidth = m_d360Data.getGlobalAnimSettings().m_panoXRes;
	int panoHeight = m_d360Data.getGlobalAnimSettings().m_panoYRes;
	std::shared_ptr< D360Sticher > stitcher = m_process->getStitcherThread();

	if (oculusRift) {
		if (m_oculusDevice == NULL) {
			m_oculusDevice = new OculusRender(stitcher->getContext());
			if (!m_oculusDevice->isCreated()) {
				PANO_N_ERROR("OculusRift device is not created!");
				return;
			}
			connect(m_oculusDevice, SIGNAL(finished(int, QString, int)), this, SLOT(finishedThread(int, QString, int)));
			connect(m_oculusDevice, SIGNAL(started(int, QString, int)), this, SLOT(startedThread(int, QString, int)));
			m_interactView->setOculusObject(m_oculusDevice);
			m_oculusDevice->initialize(sharedImageBuffer, panoWidth, panoHeight);
			m_oculusDevice->setPanoramaTexture(stitcher->getPanoramaTextureId());
			m_oculusDevice->installEventFilter(this);
		}
		
		m_oculusDevice->startThread();
	}
	else {
		m_oculusDevice->stop();
	}

	m_d360Data.getGlobalAnimSettings().m_oculus = oculusRift;
}

void QmlMainWindow::setBlendLevel(int iBlendLevel)
{
	if (getGlobalAnimSetting().m_blendingMode == GlobalAnimSettings::MultiBandBlending)
	{
		getGlobalAnimSetting().m_multiBandLevel = iBlendLevel;
		sharedImageBuffer->getStitcher()->setBlendingMode(getGlobalAnimSetting().m_blendingMode, iBlendLevel);
	}
}

float QmlMainWindow::getLeft(int iDeviceNum)
{
	return m_d360Data.getLeft(iDeviceNum);
}

void QmlMainWindow::setLeft(float fLeft, int iDeviceNum)
{
	m_d360Data.setLeft(fLeft, iDeviceNum);
}

float QmlMainWindow::getRight(int iDeviceNum)
{
	return m_d360Data.getRight(iDeviceNum);
}
void QmlMainWindow::setRight(float fRight, int iDeviceNum)
{
	m_d360Data.setRight(fRight, iDeviceNum);
}

float QmlMainWindow::getTop(int iDeviceNum)
{
	return m_d360Data.getTop(iDeviceNum);
}
void QmlMainWindow::setTop(float fTop, int iDeviceNum)
{
	m_d360Data.setTop(fTop, iDeviceNum);
}

float QmlMainWindow::getBottom(int iDeviceNum)
{
	return m_d360Data.getBottom(iDeviceNum);
}
void QmlMainWindow::setBottom(float fBottom, int iDeviceNum)
{
	m_d360Data.setBottom(fBottom, iDeviceNum);
}

int QmlMainWindow::getStereoType(int iIndex)
{
	return m_d360Data.getStereoType(iIndex);
}
void QmlMainWindow::setStereoType(int iStereoType, int iDeviceNum)
{
	m_d360Data.setStereoType(iStereoType, iDeviceNum);
}

void QmlMainWindow::setAudioType(int iAudioType, int iDeviceNum)
{
	m_d360Data.setAudioType(iAudioType, iDeviceNum);
}

void QmlMainWindow::onCalculatorGain()
{
	m_d360Data.getGlobalAnimSettings().refreshTempCameraSettingsList();
	sharedImageBuffer->getStitcher()->calcGain();
}

void QmlMainWindow::onRollbackGain()
{
	m_d360Data.getGlobalAnimSettings().rollbackCameraSettingsList();
	sharedImageBuffer->getStitcher()->reStitch();
}

void QmlMainWindow::onResetGain()
{
	sharedImageBuffer->getStitcher()->resetGain();
}

void QmlMainWindow::setTempStereoType(int iIndex, int iStereoType)
{
	m_d360Data.setTempStereoType(iIndex, iStereoType);
}

int QmlMainWindow::getTempStereoType(int iIndex)
{
	return m_d360Data.getTempStereoType(iIndex);
}

void QmlMainWindow::setTempImagePath(int iIndex, QString fileDir)
{
	m_d360Data.setTempImagePath(iIndex, fileDir);
}

QString QmlMainWindow::getTempImagePath(int iIndex)
{
	return m_d360Data.getTempImagePath(iIndex);
}

void QmlMainWindow::setTempImagePrefix(int iIndex, QString filePrefix)
{
	m_d360Data.setTempImagePrefix(iIndex, filePrefix);
}

QString QmlMainWindow::getTempImagePrefix(int iIndex)
{
	return m_d360Data.getTempImagePrefix(iIndex);
}

void QmlMainWindow::setTempImageExt(int iIndex, QString fileExt)
{
	m_d360Data.setTempImageExt(iIndex, fileExt);
}

QString QmlMainWindow::getTempImageExt(int iIndex)
{
	return m_d360Data.getTempImageExt(iIndex);
}



void QmlMainWindow::setTempAudioSettings(int iIndex, int iAudioType)
{
	m_d360Data.setTempAudioSettings(iIndex, iAudioType);
}

int QmlMainWindow::getTempAudioSettings(int iIndex)
{
	return m_d360Data.getTempAudioSettings(iIndex);

}

int QmlMainWindow::getTempAudioSettingsEx(QString devName)
{
	for (unsigned i = 0; i < m_d360Data.getGlobalAnimSettings().cameraSettingsList().size(); i++)
	{
		if (m_d360Data.getGlobalAnimSettings().cameraSettingsList()[i].audioName == devName)
			return m_d360Data.getGlobalAnimSettings().cameraSettingsList()[i].audioType;
	}
	return 1;	// Default is Left mode.
}

void QmlMainWindow::onPressed(int iPosX, int iPosY)
{
	QPoint pos(iPosX, iPosY);
	m_stitcherView->onMousePress(pos);
}

void QmlMainWindow::onMoved(int iPosX, int iPosY)
{
	QPoint pos(iPosX, iPosY);
	m_stitcherView->onMouseMove(pos);
}

void QmlMainWindow::onReleased(int iPosX, int iPosY)
{
	QPoint pos(iPosX, iPosY);
	m_stitcherView->onMouseRelease(pos);
}

void QmlMainWindow::onPressedInteractive(int iPosX, int iPosY)
{
	QPoint pos(iPosX, iPosY);
	m_interactView->onMousePress(pos);
}

void QmlMainWindow::onMovedInteractive(int iPosX, int iPosY)
{
	QPoint pos(iPosX, iPosY);
	m_interactView->onMouseMove(pos);
}

void QmlMainWindow::onReleasedInteractive(int iPosX, int iPosY)
{
	QPoint pos(iPosX, iPosY);
	m_interactView->onMouseRelease(pos);
}


void QmlMainWindow::snapshotFrame()
{
	GlobalAnimSettings::CameraSettingsList& cameraSettingsList = m_d360Data.getGlobalAnimSettings().cameraSettingsList();

	PANO_LOG(QString("Snapshot current frame. (%1)").arg(CUR_TIME_H));
	QDir snapshotDir(sharedImageBuffer->getGlobalAnimSettings()->m_snapshotDir);
	if (!snapshotDir.exists()) {
		PANO_LOG(QString("Snapshot directory not exist! (%1)").arg(sharedImageBuffer->getGlobalAnimSettings()->m_snapshotDir));
		return;
	}
	for (int i = 0; i < cameraModuleMap.size(); i++)
	{
		cameraModuleMap.values()[i]->snapshot();
	}
}

QString QmlMainWindow::onImageFileDlg(QString filePath)
{
	if (!QDir(filePath).exists() || 
		QDir(filePath).count() == 0) 
		return "";
	
	m_iPathList = QDir(filePath).entryList(QDir::NoDotAndDotDot | QDir::Dirs);

	if (m_iPathList.size() == 0)
		return "";

	return m_iPathList.join(",") + ",";
}

bool QmlMainWindow::calibrate(int stepIndex)
{
	if (!QFile::exists(CALIB_WORK_PATH)) {
		if (m_d360Data.getGlobalAnimSettings().m_cameraCalibFile.isEmpty()) {
			m_isStarted = true;
			emit started(true);			// Sending started signal to QML.
		}
		emit calibratingFinished();
		PANO_N_ERROR("'PanoramaTools' directory not exist!");
		return false;
	}

	int ret = false;
	static int step = 0;

	if (stepIndex > -1)	step = stepIndex;

	int fov = m_d360Data.getGlobalAnimSettings().m_fov;
	int lensType = m_d360Data.getGlobalAnimSettings().m_lensType == 1 ? 3 : m_d360Data.getGlobalAnimSettings().m_lensType;

	switch (step++)
	{
	case 0:
		ret = m_calib->calibrate(QString("pto_gen -o project.pto -p %1 -f %2 *.bmp").ARGN(lensType).ARGN(fov));
		break;
	case 1:
		ret = m_calib->calibrate("cpfind -o cpoints.pto project.pto");
		break;
	case 2:
		ret = m_calib->calibrate("cpclean -o pre_calib.pto cpoints.pto");
		break;
	case 3:
		ret = m_calib->calibrate("pto_var --opt y,p,r -o pre_step1.pto pre_calib.pto");
		break;
	case 4:
		ret = m_calib->calibrate("autooptimiser -n -o post_step1.pto pre_step1.pto");
		break;
	case 5:
		ret = m_calib->calibrate("pto_var --opt y,p,r,v -o pre_step2.pto post_step1.pto");
		break;
	case 6:
		ret = m_calib->calibrate("autooptimiser -n -o post_step2.pto pre_step2.pto");
		break;
	case 7:
		ret = m_calib->calibrate("pto_var --opt y,p,r,v,b -o pre_step3.pto post_step2.pto");
		break;
	case 8:
		ret = m_calib->calibrate("autooptimiser -n -o post_step3.pto pre_step3.pto");
		break;
	case 9:
		ret = m_calib->calibrate("pto_var --opt y,p,r,v,a,b,c,d,e -o pre_step4.pto post_step3.pto");
		break;
	case 10:
		ret = m_calib->calibrate("autooptimiser -n -o calib_result.pto pre_step4.pto");
		break;
	case 11:
		ret = m_calib->calibrate("pto_var --opt Eev,!Eev0 -o pre_photometric.pto calib_result.pto");
		break;
	case 12:
		ret = m_calib->calibrate("vig_optimize -o complete.pto pre_photometric.pto", true);	// Final command
		break;
	default:
		ret = true;
		break;
	}

	if (ret && step == 100) {
		// Initialize PAC and PTS map
		QMap<QString, QString>  pacPTO;
		pacPTO["lensType"] = "f";
		pacPTO["yaw"] = "y";
		pacPTO["pitch"] = "p";
		pacPTO["roll"] = "r";
		pacPTO["fov"] = "v";
		pacPTO["k1"] = "a";
		pacPTO["k2"] = "b";
		pacPTO["k3"] = "c";
		pacPTO["offset_x"] = "d";
		pacPTO["offset_y"] = "e";
		pacPTO["expOffset"] = "Eev";

		QMap<QString, PTOVAL> paramMap = m_calib->getCalibParams();

		if (paramMap.size() == 0) {
			// Error Process
			if (m_d360Data.getGlobalAnimSettings().m_cameraCalibFile.isEmpty()) {
				m_isStarted = true;
				emit started(true);			// Sending started signal to QML.
			}
			m_calibResult = false;
			emit calibratingFinished();
			PANO_N_ERROR("Calibrating failed!");
			return false;
		}

		GlobalAnimSettings::CameraSettingsList& cameraSettings = m_d360Data.getGlobalAnimSettings().cameraSettingsList();

		int devIndex = 0;
		while (paramMap.contains(QString("%1%2").arg(IMAGE_VAL).arg(QString::number(devIndex)))) {
			PTOVAL entry = paramMap[QString("%1%2").arg(IMAGE_VAL).arg(QString::number(devIndex))];
			//qDebug() << endl << entry.value;
			QMap<QString, float> params = entry.camParams;
			QMapIterator<QString, QString> i(pacPTO);
			while (i.hasNext()) {
				i.next();
				QString key = i.value();
				float value = 0;
				if (params.contains(key)) {
					value = params[key];
				}
				if (i.key() == "lensType")
					cameraSettings[devIndex].m_cameraParams.m_lensType = (value == 3 ? 1 : value);
				else if (i.key() == "yaw")
					cameraSettings[devIndex].m_cameraParams.m_yaw = value;
				else if (i.key() == "pitch")
					cameraSettings[devIndex].m_cameraParams.m_pitch = value;
				else if (i.key() == "roll")
					cameraSettings[devIndex].m_cameraParams.m_roll = value;
				else if (i.key() == "fov")
					cameraSettings[devIndex].m_cameraParams.m_fov = (devIndex == 0 ? value : cameraSettings[0].m_cameraParams.m_fov);
				else if (i.key() == "k1")
					cameraSettings[devIndex].m_cameraParams.m_k1 = (devIndex == 0 ? value : cameraSettings[0].m_cameraParams.m_k1);
				else if (i.key() == "k2")
					cameraSettings[devIndex].m_cameraParams.m_k2 = (devIndex == 0 ? value : cameraSettings[0].m_cameraParams.m_k2);
				else if (i.key() == "k3")
					cameraSettings[devIndex].m_cameraParams.m_k3 = (devIndex == 0 ? value : cameraSettings[0].m_cameraParams.m_k3);
				else if (i.key() == "offset_x")
					cameraSettings[devIndex].m_cameraParams.m_offset_x = (devIndex == 0 ? value : cameraSettings[0].m_cameraParams.m_offset_x);
				else if (i.key() == "offset_y")
					cameraSettings[devIndex].m_cameraParams.m_offset_y = (devIndex == 0 ? value : cameraSettings[0].m_cameraParams.m_offset_y);
				else if (i.key() == "expOffset")
				{
					cameraSettings[devIndex].m_cameraParams.m_expOffset = value;
					cameraSettings[devIndex].exposure = value;
				}
				//qDebug() << QString("%1 (%2) = %3").arg(i.key()).arg(key).arg(value);
			}
			devIndex++;
			if (devIndex >= cameraSettings.size())
				break;
		}

		// Getting control points...
		m_cpList.clear();
		QMapIterator<QString, PTOVAL> iter(paramMap);
		while (iter.hasNext()) {
			iter.next();
			QString key = iter.key();
			PTOVAL value = iter.value();

			if (!key.startsWith(CP_VAL))	continue;

			QMap<QString, float> cpoint = value.camParams;
			CPOINT cp;
			cp.point1 = cpoint["n"];
			cp.point2 = cpoint["N"];
			cp.x1 = cpoint["x"];
			cp.y1 = cpoint["y"];
			cp.x2 = cpoint["X"];
			cp.y2 = cpoint["Y"];
			m_cpList.append(cp);
		}

		PANO_N_LOG("Calibrating final step finished.");
		sharedImageBuffer->getStitcher()->updateCameraParams();
		if (m_d360Data.getGlobalAnimSettings().m_cameraCalibFile.isEmpty() && m_isStarted == false) {
			m_isStarted = true;
			emit started(true);			// Sending started signal to QML.
		}
		else {
			m_calibResult = true;
			emit calibratingFinished();
		}
		sharedImageBuffer->getStitcher()->reStitch();
	}
	
	if (ret == false)
	{
		// Error Process
		if (m_d360Data.getGlobalAnimSettings().m_cameraCalibFile.isEmpty()) {
			m_isStarted = true;
			emit started(true);			// Sending started signal to QML.
		}
		m_calibResult = false;
		emit calibratingFinished();
		PANO_N_ERROR("Calibrating failed!");
	}

	return ret;
}

int QmlMainWindow::getCPointCount(int camIndex1, int camIndex2)
{
	if (camIndex1 == camIndex2)	return 0;

	if (camIndex1 < 0 || camIndex2 < 0)	return 0;

	QListIterator<CPOINT> iter(m_cpList);
	int count = 0;
	while (iter.hasNext()) {
		CPOINT cp = iter.next();

		if ((cp.point1 == camIndex1 && cp.point2 == camIndex2) ||
			(cp.point2 == camIndex1 && cp.point1 == camIndex2))
			count++;
	}

	return count;
}

QString QmlMainWindow::getCPoint(int index, int camIndex1, int camIndex2)
{
	if (camIndex1 == camIndex2)	return "";

	if (camIndex1 < 0 || camIndex2 < 0)	return "";

	if (index < 0)	return "";

	QListIterator<CPOINT> iter(m_cpList);
	QString cpStr = "";
	int selIndex = 0;
	while (iter.hasNext()) {
		CPOINT cp = iter.next();

		if (!((cp.point1 == camIndex1 && cp.point2 == camIndex2) ||
			(cp.point2 == camIndex1 && cp.point1 == camIndex2)))
			continue;

		if (selIndex == index) {
			cpStr = QString("%1:%2:%3:%4").ARGN(cp.x1).ARGN(cp.y1).ARGN(cp.x2).ARGN(cp.y2);
			break;
		}
		selIndex++;
	}

	return cpStr;
}

int QmlMainWindow::getCPointCountEx(int camIndex)
{
	if (camIndex < 0)	return 0;

	QListIterator<CPOINT> iter(m_cpList);
	int count = 0;
	while (iter.hasNext()) {
		CPOINT cp = iter.next();

		if (cp.point1 == camIndex || cp.point2 == camIndex)
			count++;
	}

	return count;
}

QString QmlMainWindow::getCPointEx(int index, int camIndex)
{
	if (camIndex < 0)	return "";

	if (index < 0)	return "";

	QMap<QString, int> cpGroupMap;
	int groupIndex = 0;
	QListIterator<CPOINT> iter(m_cpList);
	while (iter.hasNext()) {
		CPOINT cp = iter.next();
		QString cpGroup = QString("%1:%2").ARGN(cp.point1).ARGN(cp.point2);
		if (cpGroupMap.contains(cpGroup))	continue;
		else
			cpGroupMap[cpGroup] = groupIndex++;
	}

	QString cpStr = "";
	int selIndex = 0;
	iter.toFront();
	while (iter.hasNext()) {
		CPOINT cp = iter.next();

		if (!(cp.point1 == camIndex || cp.point2 == camIndex))
			continue;

		if (selIndex == index) {
			QString cpGroup = QString("%1:%2").ARGN(cp.point1).ARGN(cp.point2);
			if (cp.point1 == camIndex)
				cpStr = QString("%1:%2:%3").ARGN(cp.x1).ARGN(cp.y1).ARGN(cpGroupMap[cpGroup]);
			else
				cpStr = QString("%1:%2:%3").ARGN(cp.x2).ARGN(cp.y2).ARGN(cpGroupMap[cpGroup]);
			break;
		}
		selIndex++;
	}

	return cpStr;
}

void QmlMainWindow::finishedSnapshot(int deviceIndex)
{
	PANO_DLOG(QString("Snapshot image saved. [%1]").ARGN(deviceIndex));
	snapshotCount++;
	
	if (snapshotCount != m_d360Data.getGlobalAnimSettings().cameraSettingsList().size())
		return;

	if (!m_d360Data.getGlobalAnimSettings().m_cameraCalibFile.isEmpty() && m_connectedCameras == false)
	{
		PANO_N_LOG("Project started!");
		m_isStarted = true;
		emit started(true);			// Sending started signal to QML.
	}
	else if (m_d360Data.getGlobalAnimSettings().m_cameraCalibFile.isEmpty() && m_connectedCameras == false)
		calibrate(0);
	else if (m_connectedCameras == true)
		calibrate(0);

	m_connectedCameras = true;
	m_isEndCaptureThreads = false;
}

void QmlMainWindow::startCalibrating()
{
	m_calib->initialize();
	snapshotCount = 0;
	for (int i = 0; i < cameraModuleMap.size(); i++)
	{
		cameraModuleMap.values()[i]->snapshot(true);
	}
}

QString QmlMainWindow::getSeamLabelPos(int camIndex)
{
	if (camIndex < 0 || camIndex >= m_d360Data.getGlobalAnimSettings().cameraSettingsList().size())
		return "";

	GlobalAnimSettings& setting = g_mainWindow->getGlobalAnimSetting();
	CameraParameters& cam = setting.cameraSettingsList()[camIndex].m_cameraParams;
	QMatrix3x3 m = getViewMatrix(cam.m_yaw, cam.m_pitch, cam.m_roll);
	float *m_Data = m.data();

	mat3 matrix = mat3(m_Data[0], m_Data[1], m_Data[2], m_Data[3], m_Data[4], m_Data[5], m_Data[6], m_Data[7], m_Data[8]);
	mat3 inverseMatrix;
	invert(inverseMatrix, matrix);

	mat3 mPlacement = mat3_id;
	vec3 u(-setting.m_fRoll * sd_to_rad, -setting.m_fPitch * sd_to_rad, setting.m_fYaw * sd_to_rad);
	mPlacement.set_rot_zxy(u);
	
	mat3 finalMat;
	mult(finalMat, mPlacement, inverseMatrix);

	vec3 cartesian = finalMat * vec3_z;
	int XYn[2];
	cartesianToXYn(cartesian, XYn);

	QString posStr = QString("%1:%2").ARGN(XYn[0]).ARGN(g_panoramaHeight - XYn[1]);
	return posStr;
}

void QmlMainWindow::enableSeam(int camIndex)
{
	if (!m_isStarted)	return;

	sharedImageBuffer->getStitcher()->setSeamIndex(camIndex);
	sharedImageBuffer->getStitcher()->reStitch();
}

void QmlMainWindow::onNotify(int type, QString title, QString msg)
{
	QMap<int, QString> titleMap;
	titleMap[PANO_LOG_LEVEL::CRITICAL] = "Error";
	titleMap[PANO_LOG_LEVEL::WARNING] = "Warning";
	titleMap[PANO_LOG_LEVEL::INFO] = "Information";

 	QString titleStr = "";

	if (!(title.isNull() || title.isEmpty()))
		titleStr = title;
	else if (type >= PANO_LOG_LEVEL::WARNING && type <= PANO_LOG_LEVEL::CRITICAL)
		titleStr = titleMap[type];

	QString msgStr = "";
	if (!(msg.isNull() || msg.isEmpty()))
		msgStr = msg;

	//m_notifyMsg = titleStr + ":" + msgStr;
	QString timeStr = QTime::currentTime().toString("hh:mm");
	m_notifyMsg = titleStr + ":" + msgStr + ":" + timeStr;
	
	m_notifyList.append(m_notifyMsg);
	emit notify();
}

int QmlMainWindow::getNotificationCount()
{
	QListIterator<QString> iter(m_notifyList);
	int count = 0;
	while (iter.hasNext()) {
		QString notifyMsg = iter.next();
		count++;
	}

	return count;
}

QString QmlMainWindow::getNotification(int index)
{
	QListIterator<QString> iter(m_notifyList);
	QString notifyStr = "";
	//if (m_notifyList.size() == 0) return "";
	notifyStr = m_notifyList.at(index);

	return notifyStr;
}

bool QmlMainWindow::removeNotification(int index)
{
	QListIterator<QString> iter(m_notifyList);
	while (iter.hasNext())
	{
		iter.next();
		if (index == -1)
			m_notifyList.clear();
		else{
			m_notifyList.removeAt(index);
		}
		return true;
	}
	       
	return false;
	
}

QString QmlMainWindow::getSlotInfo(QString name, QString ext, int type)
{
	SlotInfo slot(this, type);

	QString path = name;
	
	if ( type == D360::Capture::CAPTURE_FILE) {
		QDir curPath(name + "/");
		QStringList fileList = curPath.entryList(QDir::Files);
		foreach(const QString file, fileList) {
			if (file.endsWith("." + ext)) {
				path += "/" + file;
				break;
			}
		}
	}
	
	if (slot.open(path) < 0)
	{
		PANO_N_ERROR(QString("Can not open slot (%1)").arg(name));
		return "";
	}

	QString infoStr = QString("%1:%2:%3").ARGN(slot.getWidth()).ARGN(slot.getHeight()).ARGN(slot.getRate());

	slot.close();

	return infoStr;
}

void QmlMainWindow::insertAdv(int frameWidth, int frameHeight, QPoint pt0, QPoint pt1, QPoint pt2, QPoint pt3, QString advFile, bool isVideo)
{
	if (isVideo) {
		SlotInfo slot(this);
		if (slot.open(advFile) < 0)
		{
			PANO_N_ERROR(QString("Can not open adv video file (%1)").arg(advFile));
			return;
		}
		m_d360Data.getGlobalAnimSettings().m_advFile = advFile;
		m_d360Data.getGlobalAnimSettings().m_isVideoAdv = isVideo;
		m_advThread = new AdvThread(sharedImageBuffer, advFile, slot.getWidth(), slot.getHeight(), slot.getRate());
		PANO_CONN(m_advThread, finished(), m_advThread, finishedThread());
		PANO_CONN(m_advThread, finished(), m_advThread, deleteLater());
		PANO_CONN(m_advThread, started(), m_advThread, startedThread());
		m_advThread->connect();
		m_advThread->start();
	}
	else {
		QImage image;
		QString ext = advFile.right(advFile.length() - advFile.lastIndexOf(".") - 1);
		bool ret = image.load(advFile, ext.toLocal8Bit().data());
		if (ret == false)
		{
			PANO_N_ERROR(QString("Can not open adv image file (%1)").arg(advFile));
			return;
		}
		sharedImageBuffer->getStitcher()->updateAdvImage(image);
	}

	// conversion from window-points to gl-coords(1-1-1)
	QPoint pt[4] = { pt0, pt1, pt2, pt3 };
	vec2 advQuad[4];
	double px, py;
	double panoW = getPanoXres();
	double panoH = getPanoYres();
	for (int i = 0; i < 4; i++)
	{
		window2pano_1x1(panoW, panoH, frameWidth, frameHeight, px, py, pt[i].x(), pt[i].y(), isStereo(), false);
		advQuad[i].x = (2 * px - 1);
		advQuad[i].y = (2 * py - 1); // or minus sign ?
	}
	sharedImageBuffer->getStitcher()->setAdvPoints(advQuad);
	sharedImageBuffer->getStitcher()->setAdvMode(true);
	m_d360Data.getGlobalAnimSettings().m_isAdv = true;

	if (!isVideo)
	{
		QThread::msleep(10);
		sharedImageBuffer->getStitcher()->reStitch();
	}
}

void QmlMainWindow::enableAdv(bool flag)
{
	if (m_d360Data.getGlobalAnimSettings().m_isAdv == false)
		return;

	sharedImageBuffer->getStitcher()->setAdvMode(flag);
	sharedImageBuffer->getStitcher()->reStitch();
}

void QmlMainWindow::removeAdv()
{
	if (m_advThread) {
		m_advThread->stop();
		//delete m_advThread;	// deleteLater()
		m_advThread = NULL;
	}
	sharedImageBuffer->getStitcher()->setAdvMode(false);
	m_d360Data.getGlobalAnimSettings().m_isAdv = false;
}

void QmlMainWindow::resetCamSettings()
{
	GlobalAnimSettings& setting = g_mainWindow->getGlobalAnimSetting();
	setting.m_fYaw = 0;
	setting.m_fPitch = 0;
	setting.m_fRoll = 0;
	sharedImageBuffer->getStitcher()->reStitch();
}