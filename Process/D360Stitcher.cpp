#include <iostream>
#include "D360Stitcher.h"
#include "define.h"
#include <QQmlApplicationEngine>
#include "QmlMainWindow.h"
#include <QThread>
#include <QImage>
#include <QtGlobal>
#include <QOpenGL.h>

#include <stdio.h>  

extern QmlMainWindow* g_mainWindow;

#define ENABLE_LOG 1

using namespace std;

#define CUR_TIME							QTime::currentTime().toString("mm:ss.zzz")
#define ARGN(num)							arg(QString::number(num))


#include <iostream>
#include <fstream>
#include <string>
#include "Buffer.h"
#include "define.h"


#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG

extern QThread* g_mainThread;


//////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////// 

D360Sticher::D360Sticher(SharedImageBuffer *pSharedImageBuffer, QObject* main) : sharedImageBuffer(pSharedImageBuffer)
, m_stitcherThreadInstance(0)
, m_Main(main)
, m_finalPanorama(NULL)
#if 0 /*C*/
, m_liveViewer(0)
#endif
, m_nViewCount(0)
, m_fboBuffer(0)
, m_surface(0)
, m_context(0)
, readyFrameNr(0)
, m_Name("Stitcher")
, isAdvModeRaw(false)
, isAdvModeUse(isAdvModeRaw)
, advBufferRaw(ImageBufferData::NONE)
, advBufferUse(ImageBufferData::NONE)
{
	qRegisterMetaType<StitchFramePtr>("StitchFramePtr");
	initialize();
}


D360Sticher::~D360Sticher(void)
{
	qquit();

	if (m_stitcherThreadInstance) delete m_stitcherThreadInstance;

	reset();
}

void D360Sticher::initialize()
{
	readyFrameNr = 0;
	doStop = false;
	sampleNumber = 0;
	fpsSum = 0;

	fps.clear();
	statsData.averageFPS = 0;
	statsData.nFramesProcessed = 0;
	statsData.elapesedTime = 0;

	doPause = false;
	doResetGain = false;
	doCalcGain = false;
	doReStitch = false;
	doUpdateCameraParams = false;
}

void D360Sticher::init(QOpenGLContext* context)
{
	initialize();

	reset();

	GlobalAnimSettings* gasettings = sharedImageBuffer->getGlobalAnimSettings();
	m_gaSettings = gasettings;

	m_surface = new QOffscreenSurface();
	m_surface->create();

	m_context = new QOpenGLContext();
	QSurfaceFormat format = m_surface->requestedFormat();
	format.setSwapInterval(0);
	format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	m_context->setFormat(format);
	m_context->setShareContext(context);
	m_context->create();

	m_context->makeCurrent(m_surface);

	QOpenGLVersionProfile profile;
	profile.setProfile(m_context->surface()->format().profile());
	functions_2_0 = ((QOpenGLFunctions_2_0*)m_context->versionFunctions(profile));

	int imgWidth = gasettings->m_xres;
	int imgHeight = gasettings->m_yres;

	m_nPanoramaReadyCount = 0;

	int panoWidth = gasettings->m_panoXRes;
	int panoHeight = gasettings->m_panoYRes;

	m_nViewCount = gasettings->cameraSettingsList().size();

	for (int i = 0; i < m_nViewCount; i++)
	{
		GLSLColorCvt_2RGB* p2RGB;
		if (gasettings->m_captureType == D360::Capture::CAPTURE_VIDEO)
		{
			GLSLColorCvt_YUV2RGB* pYUV2RGB = new GLSLColorCvt_YUV2RGB();
			p2RGB = pYUV2RGB;
		}
		else if (gasettings->m_captureType == D360::Capture::CAPTURE_DSHOW)
		{
			GLSLColorCvt_YUV4222RGB* pYUV4222RGB = new GLSLColorCvt_YUV4222RGB();
			p2RGB = pYUV4222RGB;
		}
		else
		{
			GLSLColorCvt_RGB2RGB* pRGB2RGB = new GLSLColorCvt_RGB2RGB();
			p2RGB = pRGB2RGB;
		}

		p2RGB->setGL((QOpenGLFunctions*)m_context->functions());
		m_2rgbColorCvt.push_back(p2RGB);

		GLSLGainCompensation* pGainCompensation = new GLSLGainCompensation();
		pGainCompensation->setGL((QOpenGLFunctions*)m_context->functions(), functions_2_0);
		m_gainCompensation.push_back(pGainCompensation);

		GLSLUnwarp* pUnwarp = new GLSLUnwarp();
		pUnwarp->setCameraInput(m_gaSettings->cameraSettingsList()[i]);
		pUnwarp->setGL((QOpenGLFunctions*)m_context->functions());
		m_unwarp.push_back(pUnwarp);
	}
	int panoCount = gasettings->isStereo() ? 2 : 1;
	for (int i = 0; i < panoCount; i++)
	{
		GLSLComposite* pComposite = new GLSLComposite();
		pComposite->setGL((QOpenGLFunctions*)m_context->functions(), functions_2_0);
		m_composite.push_back(pComposite);

		GLSLPanoramaPlacement* pPlacement = new GLSLPanoramaPlacement();
		pPlacement->setGL((QOpenGLFunctions*)m_context->functions(), functions_2_0);
		m_placement.push_back(pPlacement);
	}

	m_finalPanorama = new GLSLFinalPanorama();
	m_finalPanorama->setGL((QOpenGLFunctions*)m_context->functions(), functions_2_0);
#if 0 /*C*/
	m_liveViewer = new GLSLLiveViewer();
	m_liveViewer->setGL((QOpenGLFunctions*)m_context->functions());
#endif
	for (int i = 0; i < m_nViewCount; i++)
	{
		m_2rgbColorCvt[i]->initialize(imgWidth, imgHeight);
		m_gainCompensation[i]->initialize(imgWidth, imgHeight);
		m_unwarp[i]->initialize(i, imgWidth, imgHeight, panoWidth, panoHeight);
	}


	// create texture id list for left and right unwarped images in panorama space
	int viewCnt[2] = { m_nViewCount, 0 };
	GLuint fboTextures[16];
	for (int i = 0; i < m_nViewCount; i++)
	{
		fboTextures[i] = m_unwarp[i]->getTargetTexture();
	}

	if (m_gaSettings->isStereo())
	{
		viewCnt[0] = gasettings->getLeftIndices().size();
		viewCnt[1] = gasettings->getRightIndices().size();

		for (int i = 0; i < m_gaSettings->getLeftIndices().size(); i++)
		{
			m_panoSegsLeft[i] = fboTextures[m_gaSettings->getLeftIndices()[i]];
		}
		for (int i = 0; i < m_gaSettings->getRightIndices().size(); i++)
		{
			m_panoSegsRight[i] = fboTextures[m_gaSettings->getRightIndices()[i]];
		}

	}
	else
	{
		// for mono mode, select left or right cameras only when subset is selected.
		// or select all cameras for all cameras are none.
		if (m_gaSettings->getLeftIndices().size() > 0)
		{
			for (int i = 0; i < m_gaSettings->getLeftIndices().size(); i++)
			{
				m_panoSegsLeft[i] = fboTextures[m_gaSettings->getLeftIndices()[i]];
			}
			viewCnt[0] = gasettings->getLeftIndices().size();
		}
		else if (m_gaSettings->getRightIndices().size() > 0)
		{
			for (int i = 0; i < m_gaSettings->getRightIndices().size(); i++)
			{
				m_panoSegsLeft[i] = fboTextures[m_gaSettings->getRightIndices()[i]];
			}
			viewCnt[0] = gasettings->getRightIndices().size();
		}
		else
		{
			for (int i = 0; i < m_nViewCount; i++)
			{
				m_panoSegsLeft[i] = fboTextures[i];
			}
		}
	}

	for (int i = 0; i < m_composite.size(); i++)
	{
		m_composite[i]->initialize(viewCnt[i], panoWidth, panoHeight);
		m_placement[i]->initialize(panoWidth, panoHeight);
	}

	m_interractPanorama = new GLSLFinalPanorama();
	m_interractPanorama->setGL((QOpenGLFunctions*)m_context->functions(), functions_2_0);
	m_interractPanorama->initialize(panoWidth, panoHeight, false);

	m_finalPanorama->initialize(panoWidth, panoHeight, gasettings->isStereo());
	// Panorama resolution will be 4k * 2k,
	// for stereo mode, the top-down video resolution will be 4k * 4k, (which is w by 2h)
	// for mono mode, the resolution will be 4k * 2k (which is w by h)
	int panoramaFrameBytes = panoWidth * panoHeight * 3;
	if (gasettings->isStereo())
	{
		m_fboBuffer = new unsigned char[panoramaFrameBytes * 2];
	}
	else
	{
		m_fboBuffer = new unsigned char[panoramaFrameBytes];
	}

	// 
	m_context->doneCurrent();

	isFirstFrame = true;
}

void D360Sticher::qquit()
{
	PANO_LOG("Stitcher Finished ");

	if (m_stitcherThreadInstance && m_stitcherThreadInstance->isRunning())
		stopStitcherThread();
}

void D360Sticher::stopStitcherThread()
{
	PANO_LOG("About to stop stitcher thread...");

	this->stop();
	this->thread()->terminate();
	
	sharedImageBuffer->wakeAll(); // This allows the thread to be stopped if it is in a wait-state
	if (!this->isFinished())
		this->waitForFinish();
	

	PANO_LOG("Stitcher thread successfully stopped.");

}

void D360Sticher::stop()
{
	QMutexLocker locker(&doStopMutex);
	doStop = true;
}


void D360Sticher::setup()
{
	// no need delete thread instance due to connect(this, SIGNAL(finished(int, QString, int)), m_stitcherThreadInstance, SLOT(deleteLater()));
	//if (m_stitcherThreadInstance)		// If reopen main window
	//	delete m_stitcherThreadInstance;
	
	m_stitcherThreadInstance = new QThread;
	this->moveToThread(m_stitcherThreadInstance);
	connect(m_stitcherThreadInstance, SIGNAL(started()), this, SLOT(process()));
	//connect( processingThread, SIGNAL(finished()), processingThreadInstance, SLOT(quit()));
	connect(m_stitcherThreadInstance, SIGNAL(finished()), this, SLOT(deleteLater()));
	connect(this, SIGNAL(finished(int, QString, int)), m_stitcherThreadInstance, SLOT(deleteLater()));
}

void D360Sticher::startThread()
{
	setup();
	m_stitcherThreadInstance->start();
	m_context->doneCurrent();
	m_context->moveToThread(m_stitcherThreadInstance);
}

QOpenGLContext* D360Sticher::getContext()
{
	return m_context;
}

int D360Sticher::getRawTextureId(int cameraIndex)
{
	if (m_gainCompensation.size() == 0) return -1;
	return m_gainCompensation[cameraIndex]->getTargetTexture();
}

int D360Sticher::getPanoramaTextureId()
{
	if (m_finalPanorama == NULL) return -1;
	return m_finalPanorama->getTargetTexture();
}

int D360Sticher::getPanoramaTextureIdForInteract()
{
	if (m_composite.size()== 0) return -1;
	return m_interractPanorama->getTargetTexture();
}


#if 0 /*C*/
int D360Sticher::getLiveViewerTextureId()
{
	if (m_liveViewer == 0) return -1;
	return m_liveViewer->getTargetTexture();
}
#endif

void D360Sticher::doCaptureIncomingFrames()
{
	m_stitchMutex.lock();
	map<int, ImageBufferData> matImages = readyBuffer;
	m_stitchMutex.unlock();

	if (m_context != QOpenGLContext::currentContext())
		m_context->makeCurrent(m_surface);
	for (map<int, ImageBufferData>::iterator iter = matImages.begin(); iter != matImages.end(); iter++)
	{
		int i = iter->first;
		m_2rgbColorCvt[i]->render(iter->second);
	}
}

void D360Sticher::doStitch(bool isPlacementOnly)
{
	if (m_context != QOpenGLContext::currentContext())
		m_context->makeCurrent(m_surface);
	GlobalAnimSettings& setting = g_mainWindow->getGlobalAnimSetting();
	GlobalAnimSettings::CameraSettingsList& camsettings = setting.cameraSettingsList();

	m_advMutex.lock();
	if (isAdvModeRaw)
		advBufferUse = advBufferRaw;
	else
		advBufferUse.clear();
	isAdvModeUse = isAdvModeRaw && advBufferUse.isValid();

	if (isAdvModeUse)
		m_advColorCvt.DynRender(advBufferUse);
	m_advMutex.unlock();

	if (!isPlacementOnly)
	{
		for (int i = 0; i < m_gainCompensation.size(); i++)
		{
			m_gainCompensation[i]->render(m_2rgbColorCvt[i]->getTargetTexture(), camsettings[i].exposure);
		}

		for (int i = 0; i < m_unwarp.size(); i++)
		{
			m_unwarp[i]->render(m_gainCompensation[i]->getTargetTexture(), i);
		}

		for (int i = 0; i < m_composite.size(); i++)
		{
			GLSLComposite* composite = m_composite[i];
			if (i == 0)
				composite->render(m_panoSegsLeft);
			else
				composite->render(m_panoSegsRight);
			
			if (isAdvModeUse)
			{
				m_advMutex2.lock();
				composite->mixBill(m_advColorCvt.getTargetTexture(), advQuad);
				m_advMutex2.unlock();
			}
		}
	}

	for (int i = 0; i < m_placement.size(); i++)
	{
		m_placement[i]->render(m_composite[i]->getTargetTexture(isAdvModeUse), setting.m_fYaw, setting.m_fPitch, setting.m_fRoll);
	}

	if (m_placement.size() > 0)
	{
		GLuint individualPanoramaTextures[1];
		individualPanoramaTextures[0] = m_composite[0]->getTargetTexture(isAdvModeUse);
		m_interractPanorama->render(individualPanoramaTextures);
	}

	GLuint individualPanoramaTextures[2];
	for (int i = 0; i < m_placement.size(); i++)
	{
		individualPanoramaTextures[i] = m_placement[i]->getTargetTexture();
	}
	m_finalPanorama->render(individualPanoramaTextures);

	bool isOculus = sharedImageBuffer->getGlobalAnimSettings()->m_oculus;
	bool isFfmpegOutputOn = (sharedImageBuffer->getGlobalAnimSettings()->m_offlineVideo != "" ||
		sharedImageBuffer->getGlobalAnimSettings()->m_wowzaServer != "");
	bool isFfmpegOutputNeeded = !isOculus && isFfmpegOutputOn;
	if (isFfmpegOutputNeeded)
	{
		//m_2yuvColorCvt->getYUVBuffer(m_fboBuffer);
		m_finalPanorama->getRGBBuffer(m_fboBuffer);
	}

	m_context->doneCurrent();

	if (isFfmpegOutputNeeded)
	{
		// stream buffer
		emit newPanoramaFrameReady(m_fboBuffer);
	}
}

void D360Sticher::calcExposure()
{
	m_stitchMutex.lock();
	map<int, ImageBufferData> matImages = readyBuffer;
	m_stitchMutex.unlock();
	if (m_context != QOpenGLContext::currentContext())
		m_context->makeCurrent(m_surface);

	GlobalAnimSettings::CameraSettingsList& camsettings = g_mainWindow->getGlobalAnimSetting().cameraSettingsList();
	for (map<int, ImageBufferData>::iterator iter = matImages.begin(); iter != matImages.end(); iter++)
	{
		int i = iter->first;
		m_gainCompensation[i]->render(m_2rgbColorCvt[i]->getTargetTexture(), 0.0f);
	}
	for (map<int, ImageBufferData>::iterator iter = matImages.begin(); iter != matImages.end(); iter++)
	{
		int i = iter->first;
		m_unwarp[i]->render(m_gainCompensation[i]->getTargetTexture(), i);
	}

	GLuint fbos[16];
	for (int i = 0; i < m_nViewCount; i++) {
		fbos[i] = m_unwarp[i]->getTargetFrameBuffer();
	}

	if (m_gaSettings->isStereo())
	{
		GLuint fbosLeft[16];
		GLuint fbosRight[16];
		int nLeftViewCount;
		int nRightViewCount;
		std::map<int, int> leftIndex2GlobalIndex;
		std::map<int, int> rightIndex2GlobalIndex;

		nLeftViewCount = m_gaSettings->getLeftIndices().size();
		nRightViewCount = m_gaSettings->getRightIndices().size();
		for (int i = 0; i < nLeftViewCount; i++)
		{
			int globalIndex = m_gaSettings->getLeftIndices()[i];
			leftIndex2GlobalIndex[i] = globalIndex;
			fbosLeft[i] = fbos[globalIndex];
		}
		for (int i = 0; i < nRightViewCount; i++)
		{
			int globalIndex = m_gaSettings->getRightIndices()[i];
			rightIndex2GlobalIndex[i] = globalIndex;
			fbosRight[i] = fbos[globalIndex];
		}
		std::vector<float> leftGains = m_composite[0]->getExposureData(fbosLeft, nLeftViewCount);
		std::vector<float> rightGains = m_composite[1]->getExposureData(fbosRight, nRightViewCount);
		for (int i = 0; i < leftGains.size(); i++)
		{
			camsettings[leftIndex2GlobalIndex[i]].exposure = gain2ev(leftGains[i]);
		}
		for (int i = 0; i < rightGains.size(); i++)
		{
			camsettings[rightIndex2GlobalIndex[i]].exposure = gain2ev(rightGains[i]);
		}
	}
	else
	{
		std::vector<float> gains = m_composite[0]->getExposureData(fbos, m_nViewCount);
		for (int i = 0; i < m_nViewCount; i++)
		{
			camsettings[i].exposure = gain2ev(gains[i]);
		}
	}
	
	m_context->doneCurrent();
	doStitch();
}


void D360Sticher::reset()
{
	if (m_surface)
	{
		m_context->doneCurrent();
		m_context->moveToThread(g_mainThread);
		m_context->makeCurrent(m_surface);
		for (int i = 0; i < m_nViewCount; i++)
			delete m_2rgbColorCvt[i];
		m_2rgbColorCvt.clear();
		for (int i = 0; i < m_nViewCount; i++)
			delete m_gainCompensation[i];
		m_gainCompensation.clear();
		for (int i = 0; i < m_nViewCount; i++)
			delete m_unwarp[i];
		m_unwarp.clear();
		for (int i = 0; i < m_composite.size(); i++)
			delete m_composite[i];
		m_composite.clear();
		for (int i = 0; i < m_placement.size(); i++)
			delete m_placement[i];
		m_placement.clear();
		delete m_interractPanorama;
		if (m_finalPanorama)
		{
			delete m_finalPanorama;
			m_finalPanorama = NULL;
		}
		if (m_fboBuffer)
		{
			delete[] m_fboBuffer;
			m_fboBuffer = NULL;
		}
		m_advColorCvt.Free();

		m_surface->destroy();
		m_context->doneCurrent();
		delete m_context;
		m_context = NULL;
		m_surface->deleteLater();
		delete m_surface;
	}
}

void D360Sticher::updateAdvFrameMat(ImageBufferData& mat, int frameNum)
{
	QMutexLocker locker(&m_advMutex);
	advBufferRaw = mat;
}

void D360Sticher::updateAdvImage(QImage image)
{
	QMutexLocker locker(&m_advMutex);
	advBufferImage = image.convertToFormat(QImage::Format_RGB888);
	advBufferRaw.mFormat = ImageBufferData::RGB888;
	advBufferRaw.mImageY.setImage(advBufferImage);
}

void D360Sticher::updateStitchFrameMat(ImageBufferData& mat, int camIndex, int frameNum)
{
	m_stitchMutex.lock();

	rawImagesBuffer[camIndex] = mat;

	GlobalAnimSettings::CameraSettingsList& camsettings = sharedImageBuffer->getGlobalAnimSettings()->cameraSettingsList();
	int camCount = camsettings.size();

	//if (camIndex == 0 && rawImagesBuffer.size() == sharedImageBuffer->getSyncedCameraCount())
	if (rawImagesBuffer.size() == sharedImageBuffer->getSyncedCameraCount())
	{
		//setup();

		readyBuffer = rawImagesBuffer;
		rawImagesBuffer.clear();
		readyFrameNr++;
		//emit startStitch();
	}
	m_stitchMutex.unlock();
}

void D360Sticher::process()
{
	m_finished = false;
	emit started(THREAD_TYPE_STITCHER, "", -1);
	run();
	PANO_DLOG("Stitcher - Emit Finished");
	emit finished(THREAD_TYPE_STITCHER, "", -1);
	finishWC.wakeAll();
	m_finished = true;

	this->moveToThread(g_mainThread);
	if (m_context)
		m_context->moveToThread(g_mainThread);
}

void D360Sticher::run()
{
	//
	// Saving Images
	//
	GlobalAnimSettings* gasettings = sharedImageBuffer->getGlobalAnimSettings();
	//state.m_curFrame = gasettings->m_startFrame;



	//
	// Start timer ( used to calculate processing rate )
	//
	t.start();
	int delay = 1000 / gasettings->m_fps;
	int continueCount = 0;
	int continueDelay = 1;
	stitchingTime = 0;

	while (1)
	{
		if (QThread::currentThread()->isInterruptionRequested())
		{
			doStop = true;
		}

		//
		// Stop thread if doStop = TRUE 
		//
		doStopMutex.lock();
		if (doStop)
		{
			doStop = false;
			doStopMutex.unlock();
			break;
		}
		doStopMutex.unlock();		

		// for extra manipulation, we just restitch
		if (!isFirstFrame && doPause)
		{
			doPauseMutex.lock();
			if (doCalcGain || doReStitch)
			{
				if (doCalcGain)
				{
					qDebug() << "Reset Gain...";
					calcExposure();
					doCalcGain = false;
				}
				else if (doReStitch)
				{
					//qDebug() << "Stitching again...";
					doStitch(false);
					doReStitch = false;
				}
			}
			else if (doUpdateCameraParams)
			{
				if (m_context != QOpenGLContext::currentContext())
					m_context->makeCurrent(m_surface);
				for (int i = 0; i < m_nViewCount; i++)
				{
					m_unwarp[i]->updateCameraParams();
				}
				m_context->doneCurrent();
				doUpdateCameraParams = false;
			}
			else
			{
				//
				// Pause thread if doPause = TRUE 
				//

				// This "IF" code is needed for fixing the issue that some cameras do NOT show any video screen sometimes, 
				// when firstFrame is captured 
				// (APP should pause when first frame is captured)
				if (rawImagesBuffer.size() == sharedImageBuffer->getSyncedCameraCount())
				{
					doPauseMutex.unlock();
					Sleep(100);
					continue;
				}
			}
			doPauseMutex.unlock();
		}
		else
		{
			if (readyFrameNr <= statsData.nFramesProcessed) {
				continueCount++;
				QThread::msleep(continueDelay);
				continue;
			}

			if (stitchingTime + continueDelay * continueCount < delay)
				QThread::msleep(delay - stitchingTime - continueDelay * continueCount);

			continueCount = 0;

			doCaptureIncomingFrames();
			doStitch(false);

			sharedImageBuffer->wakeForVideoProcessing(statsData.nFramesProcessed);

			//
			// Save processing time
			//
			stitchingTime = t.elapsed();
			t.restart();
			//
			// Update statistics
			//
			updateFPS(stitchingTime);
			statsData.nFramesProcessed++;

			if (isFirstFrame && statsData.nFramesProcessed >= 2)
				isFirstFrame = false;

			//if (gasettings->m_oculus) continue;
			// Inform GUI of updated statistics
			//
			if (/*statsData.nFramesProcessed % 1 == 0 && */gasettings->m_ui == true)
			{
				emit updateStatisticsInGUI(statsData);
			}

			/*if( gasettings->m_endFrame > 0 && statsData.nFramesProcessed > gasettings->m_endFrame )
			{
			doStop = true;
			}*/
		}
	}

	PANO_LOG("Stopping stitcher thread...");
}

void D360Sticher::updateFPS(int timeElapsed)
{
	statsData.elapesedTime += timeElapsed; 

	// Add instantaneous FPS value to queue
	if (timeElapsed > 0)
	{
		fps.enqueue((int)1000 / timeElapsed);
		// Increment sample number
		sampleNumber++;
	}

	// Maximum size of queue is DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH
	if (fps.size() > STITCH_FPS_STAT_QUEUE_LENGTH)
		fps.dequeue();

	// Update FPS value every DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH samples
	if ((fps.size() == STITCH_FPS_STAT_QUEUE_LENGTH) && (sampleNumber == STITCH_FPS_STAT_QUEUE_LENGTH))
	{
		// Empty queue and store sum
		while (!fps.empty())
			fpsSum += fps.dequeue();
		// Calculate average FPS
		statsData.averageFPS = 1.0f * fpsSum / STITCH_FPS_STAT_QUEUE_LENGTH;
		// Reset sum
		fpsSum = 0;
		// Reset sample number
		sampleNumber = 0;
	}
}

void D360Sticher::waitForFinish()
{
	finishMutex.lock();
	finishWC.wait(&finishMutex);
	finishMutex.unlock();
}

bool D360Sticher::isFinished()
{
	return m_finished;
}

void D360Sticher::playAndPause(bool isPause)
{
	QMutexLocker locker(&doPauseMutex);
	doPause = isPause;

	if (!isPause)
		t.restart();
}

void D360Sticher::calcGain()
{
	QMutexLocker locker(&doPauseMutex);
	doCalcGain = true;
}

void D360Sticher::resetGain()
{
	GlobalAnimSettings::CameraSettingsList& camsettings = g_mainWindow->getGlobalAnimSetting().cameraSettingsList();
	for (int i = 0; i < m_nViewCount; i++)
	{
		camsettings[i].exposure = camsettings[i].m_cameraParams.m_expOffset;
	}
	reStitch();
}

void D360Sticher::reStitch()
{
	QMutexLocker locker(&doPauseMutex);
	if (doPause && !doReStitch)	doReStitch = true;
}

void D360Sticher::updateCameraParams()
{
	QMutexLocker locker(&doPauseMutex);
	for (int i = 0; i < m_nViewCount; i++)
	{
		m_unwarp[i]->setCameraInput(m_gaSettings->cameraSettingsList()[i]);
	}
	if (!doUpdateCameraParams)	doUpdateCameraParams = true;
}

void D360Sticher::setSeamIndex(int index)
{
	for (int i = 0; i < m_composite.size(); i++)
	{
		m_composite[i]->setSeamIndex(index);
	}
}

void D360Sticher::setBlendingMode(GlobalAnimSettings::BlendingMode mode, int level)
{
	for (int i = 0; i < m_composite.size(); i++)
	{
		m_composite[i]->setBlendingMode(mode, level);
	}
}

void D360Sticher::setAdvMode(bool isAdvMode)
{
	QMutexLocker locker(&m_advMutex);
	isAdvModeRaw = isAdvMode;
}

void D360Sticher::setAdvPoints(vec2 advPt[])
{
	QMutexLocker locker(&m_advMutex2);

	memcpy(advQuad, advPt, sizeof(advQuad));
}