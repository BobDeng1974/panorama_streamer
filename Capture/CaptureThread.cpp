

#include <iomanip>
#include <sstream>

#include <QFile>
#include <QDir>
#include "SharedImageBuffer.h"
#include "CaptureThread.h"
#include "CaptureProp.h"
#include "D360Stitcher.h"
#include "D360Parser.h"
#include "QmlMainWindow.h"
#include "define.h"
#include <QDateTime>

extern QmlMainWindow* g_mainWindow;

CaptureThread::CaptureThread(SharedImageBuffer *sharedImageBuffer,
	int  deviceNumber,
	CaptureType captureType,
	int  width, int height) : sharedImageBuffer(sharedImageBuffer),
	m_Name("CaptureThread"),
	m_grabbedFrame(ImageBufferData::NONE)
{
	//
	// Save passed parameters
	//
	this->m_deviceNumber = deviceNumber;
	this->width = width;
	this->height = height;
	this->m_captureType = captureType;

	//
	// Initialize variables(s)
	//
	doStop = false;
	doPause = false;
	doSnapshot = false;
	doCalibrate = false;
	sampleNumber = 0;
	fpsSum = 0;

	fps.clear();

	cap = NULL;

	statsData.averageFPS = 0;
	statsData.nFramesProcessed = 0;
	statsData.nAudioFrames = 0;

	m_streamer = sharedImageBuffer->getStreamer();

	m_isCanGrab = false;
}

CaptureThread::~CaptureThread()
{
	disconnect();
}

void CaptureThread::run()
{
	m_grabbedFrame.clear();
	cap->start();
	cap->reset(m_grabbedFrame);
	GlobalAnimSettings* gasettings = sharedImageBuffer->getGlobalAnimSettings();
	CameraInput& camSettings = gasettings->cameraSettingsList()[m_deviceNumber];

	int intervalms = 1000 / gasettings->m_fps;


	QDir dir(camSettings.fileDir);
	bool dirExists = false;

	if (camSettings.fileDir != "" && dir.exists()) {
		dirExists = true;
	}
	PANO_DEVICE_DLOG("Cam Dir : " + camSettings.fileDir + " " + (dirExists?"true":"false"));

	// wonder if this has effect: we don't need sync now for this mode,
	// and this won't make things work with subset cameras working
	//sharedImageBuffer->sync( m_deviceNumber );

	// Start timer (used to calculate capture rate)
	t.start();

	// 50fps test
	float fps = gasettings->m_fps; // full speed
	if (gasettings->m_captureType == D360::Capture::CAPTURE_DSHOW)
		fps = fps * 2;

	QDateTime *curTime = new QDateTime;
	qint64 nFirstMS = 0, nSecondMS = 0, nDiffMS = 0;

	bool singleFrameMode = false;
	if (gasettings->m_startFrame == -1 && gasettings->m_endFrame == -1)
		singleFrameMode = true;
	
	bool bIsFirstFrame = true;
	while (1)
	{
		nFirstMS = curTime->currentMSecsSinceEpoch();

		if (QThread::currentThread()->isInterruptionRequested())
		{
			std::cout << "Got signal to terminate" << std::endl;
			doStop = true;
		}
		//
		// Stop thread if doStop = TRUE 
		//
		doStopMutex.lock();
		if (doStop)
		{
			std::cout << "Stop" << std::endl;
			doStop = false;
			doStopMutex.unlock();
			break;
		}
		if (!singleFrameMode && 
			(gasettings->m_endFrame != -1 && statsData.nFramesProcessed > gasettings->m_endFrame)
		)
		{
			PANO_DEVICE_LOG("Reading Frame Sequence is done!");
			m_isCanGrab = false;
			doStopMutex.unlock();
			break;
		}
		doStopMutex.unlock();
		
		//
		// Pause thread if doPause = TRUE 
		//
		doPauseMutex.lock();
		if (doPause)
		{
			if ((doSnapshot || doCalibrate) && statsData.nFramesProcessed > 0) {
				SharedImageBuffer::ImageDataPtr outImage = cap->convertToRGB888(m_grabbedFrame);
				QImage snapshotImg(outImage.mImageY.buffer, outImage.mImageY.width, outImage.mImageY.height, QImage::Format::Format_RGB888);
				QString imgName = QString(sharedImageBuffer->getGlobalAnimSettings()->m_snapshotDir + "/cam%1_%2_%3.bmp").ARGN(m_deviceNumber).arg(statsData.nFramesProcessed).arg(CUR_TIME_H);
				if (doCalibrate)
					imgName = QString("cam%1_%2_%3.bmp").ARGN(m_deviceNumber).arg(statsData.nFramesProcessed).arg(CUR_TIME_H);
				snapshotImg.save(imgName, NULL, 100);
				if (doSnapshot )	doSnapshot = false;
				else {
					doCalibrate = false;
					emit snapshoted(m_deviceNumber);
				}
			}
			doPauseMutex.unlock();
			QThread::msleep(intervalms);
			continue;
		}
		doPauseMutex.unlock();
#if 1
		if (bIsFirstFrame && m_grabbedFrame.mImageY.buffer) {	// For calibrating when loading...
			SharedImageBuffer::ImageDataPtr outImage = cap->convertToRGB888(m_grabbedFrame);
			QImage snapshotImg(outImage.mImageY.buffer, outImage.mImageY.width, outImage.mImageY.height, QImage::Format::Format_RGB888);
			QString imgName = QString("cam%1_%2_%3.bmp").ARGN(m_deviceNumber).arg(statsData.nFramesProcessed).arg(CUR_TIME_H);
			snapshotImg.save(imgName, NULL, 100);
			emit snapshoted(m_deviceNumber);
		}
#endif
#if 1
		sharedImageBuffer->syncForVideoProcessing(statsData.nFramesProcessed);
#endif

		// Capture frame (if available)
		while (cap->getIncomingType() != D360::Capture::IncomingFrameType::Video)
		{
			if (!cap->grabFrame(m_grabbedFrame))
			{
				m_isCanGrab = false;
				break;
			}

			//if (m_deviceNumber == nFirstViewId)
			{
				if (cap->getIncomingType() == D360::Capture::IncomingFrameType::Audio)
				{
					if (gasettings->m_offlineVideo != "" || gasettings->m_wowzaServer != "")
						sharedImageBuffer->syncForAudioProcessing(statsData.nAudioFrames);
					void* frame = cap->retrieveAudioFrame();
					if (frame) {
						sharedImageBuffer->setAudioFrame(frame);
						//emit newAudioFrameReady((void*)frame);
						if (sharedImageBuffer->getStreamer())
							sharedImageBuffer->getStreamer()->streamAudio(m_deviceNumber, frame);
						statsData.nAudioFrames++;
					}
				}
			}
		}

		if (m_isCanGrab == false) {
			break;
		}

		if (cap->getIncomingType() == D360::Capture::IncomingFrameType::Video)
		{
			sharedImageBuffer->sync(m_deviceNumber);
			cap->retrieveFrame(0, m_grabbedFrame);
			//gasettings->m_xres = m_grabbedFrame.mImageData->width();
			//gasettings->m_yres = m_grabbedFrame.mImageData->height();
		}
		else {
			QThread::msleep(intervalms);
			continue;
		}

		// It should delay if second frame is captured faster than 1000/fps				
		nDiffMS = nFirstMS - nSecondMS; // nFirstMS is the next Milliseconds of nSecondMS
		if (nFirstMS != 0 && nSecondMS != 0 && nDiffMS < 1000 / fps) 
		{
			if (nDiffMS >= 0) 
			{
				// If the second frame is captured faster than normal (1000/fps)
				//Sleep(1000 / fps - nDiffMS);
				QThread::msleep(1);
			} 					
		}		

		GlobalState& state = sharedImageBuffer->getState(m_deviceNumber);
		//std::cout << "Current Frame " << state.m_curFrame << std::endl;
		cap->setCurFrame(state.m_curFrame);
		if (!singleFrameMode)
			state.m_curFrame++;

		//SharedImageBuffer::ImageDataPtr qgrabbedframe;
		//qgrabbedframe = m_grabbedFrame.mImageData->clone();

		SharedImageBuffer::ImageDataPtr d = m_grabbedFrame;
		d.mFrame = statsData.nFramesProcessed;
		d.msToWait = cap->msToWait();

		statsData.nFramesProcessed++;

		if (statsData.nFramesProcessed >= 0)
		{
			//if( handler->write( statsData.nFramesProcessed, *m_grabbedFrame.mImageData, m_deviceNumber ) == false )
			{
			}
		}

		//
		// Add frame to buffer
		//
		//sharedImageBuffer->getByDeviceNumber( deviceNumber )->add( m_grabbedFrame, dropFrameIfBufferFull );
		//sharedImageBuffer->getByDeviceNumber( deviceNumber )->add( qgrabbedframe, dropFrameIfBufferFull );
		//sharedImageBuffer->getByDeviceNumber( deviceNumber )->add( d, dropFrameIfBufferFull );
		//sharedImageBuffer->getByDeviceNumber( m_deviceNumber )->add( d, dropFrameIfBufferFull );
		sharedImageBuffer->setRawImage(m_deviceNumber, d);

#ifdef REMOVE_PROCESSING_THREAD_IN_CAMERAVIEW	/* Porting from ProcessingThread by B*/

		int camIndex = 0;
		//
		// If its a file device (input) - the camera index should be set to the appropriate value since for
		// file devices device number is offset by D360_FILEDEVICESTART
		// 
		if (m_deviceNumber >= D360_FILEDEVICESTART)
		{
			camIndex = m_deviceNumber - D360_FILEDEVICESTART;
		}
		else
		{
			//
			// Camera input
			// 
			camIndex = m_deviceNumber;
		}

		if (gasettings->m_ui == true)
		{
			{
				//
				ImageBufferData newMat = d;

				//int imageLength = currentFrame.mImageData->cols * currentFrame.mImageData->rows * 3;
				//memcpy(buffer, currentFrame.mImageData->data, imageLength);
				//currentFrame.mImageData->copyTo(*newMat);
				if (gasettings->m_stitch == true)
				{
					std::shared_ptr< D360Sticher> stitcher = sharedImageBuffer->getStitcher();
					if (stitcher)
					{
						stitcher->updateStitchFrameMat(newMat, camIndex, statsData.nFramesProcessed);
					}
					//emit newFrameMat( MatBufferPtr( newMat ), deviceNumber, statsData.nFramesProcessed );
				}
				else
				{
					//emit newFrameMat( MatBufferPtr( newMat ), deviceNumber, statsData.nFramesProcessed );
				}
				//std::this_thread::sleep_for( std::chrono::milliseconds( currentFrame.msToWait ) );	
				//emit newFrameMat( MatBufferPtr( currentFrame.mImageData ), deviceNumber, statsData.nFramesProcessed );
			}
		}

#endif
		// Pause if first frame is captured
		if (bIsFirstFrame && statsData.nFramesProcessed >= 2) {
			playAndPause(true);
			bIsFirstFrame = false;
		}

		//std::cout << "Grabbed Frame " << m_grabbedFrame << " - data " << std::endl;
		if (gasettings->m_oculus) continue;
		//
		// Save capture time
		//
		captureTime = t.elapsed();
		t.start();

		//
		// Update statistics
		//
		updateFPS(captureTime);

		if (state.m_curFrame > gasettings->m_endFrame && gasettings->m_endFrame != -1)
		{
			m_isCanGrab = true;
			// for loop,
			//state.m_curFrame = gasettings->m_startFrame;
		}
		
		//
		// Inform GUI of updated statistics
		//
		if (statsData.nFramesProcessed % 10 == 0 && gasettings->m_ui == true)
		{
			emit updateStatisticsInGUI(statsData);
		}

		// nSecondMS is the final MS of ONE frame.
		nSecondMS = curTime->currentMSecsSinceEpoch();
	}
	//disconnect();

	//delete m_grabbedFrame.mImageData;
	
	PANO_DEVICE_LOG("Capture thread running is finished...");
}

void CaptureThread::process()
{
	m_finished = false;
	emit started(THREAD_TYPE_CAPTURE, "", m_deviceNumber);
	run();
	if (m_isCanGrab == false) {
		PANO_DEVICE_N_WARN("Reading frame has been finished.");
		emit report(THREAD_TYPE_CAPTURE, "EOF", m_deviceNumber);
	}
	else
	{
		PANO_DLOG("Emit finished signal");
		emit finished(THREAD_TYPE_CAPTURE, "", m_deviceNumber);
	}
	finishWC.wakeAll();
	m_finished = true;
}

bool CaptureThread::connect(int dupIndex)
{
	if (m_captureType == CAPTUREXIMEA)
	{
		CaptureXimea* capture = new CaptureXimea;

		if (!capture->open(m_deviceNumber))
		{
			PANO_LOG("Can't Open Camera!");
			emit report(THREAD_TYPE_CAPTURE, "Can't Open XMIEA Camera.", m_deviceNumber);
			delete capture;
			return false;
		}
		GlobalAnimSettings* gasettings = sharedImageBuffer->getGlobalAnimSettings();

		//
		// Set resolution
		//
		/*if (width != -1)
			capture->setProperty(CV_CAP_PROP_FRAME_WIDTH, width);
		else*/
		{
			width = capture->getProperty(CV_CAP_PROP_FRAME_WIDTH);
		}
		/*if (height != -1)
		{
			capture->setProperty(CV_CAP_PROP_FRAME_HEIGHT, height);
		}
		else*/
		{
			height = capture->getProperty(CV_CAP_PROP_FRAME_HEIGHT);
		}

		cap = capture;


		int trigsource = (int)capture->getProperty(CV_CAP_PROP_XI_TRG_SOURCE);
		int selector = (int)capture->getProperty(CV_CAP_PROP_XI_GPI_SELECTOR);
		int mode = (int)capture->getProperty(CV_CAP_PROP_XI_GPI_MODE);

		std::cout << "Mode " << mode << " Selector " << selector << " TrgSource " << trigsource << std::endl;
	}

	if (m_captureType == CAPTUREDSHOW || m_captureType == CAPTUREVIDEO)
	{
		GlobalAnimSettings* gasettings = sharedImageBuffer->getGlobalAnimSettings();
		QString cameraName = gasettings->cameraSettingsList()[m_deviceNumber].name;

		CaptureDShow* capture = new CaptureDShow;

		//if (!capture->open(m_deviceNumber, cameraName, gasettings->m_fps))
		if (!capture->open(m_deviceNumber, cameraName, gasettings->m_fps, m_captureType == CAPTUREDSHOW, dupIndex))
		{
			std::cout << "Can't Open Camera" << std::endl;
			emit report(THREAD_TYPE_CAPTURE, "Can't Open DirectShow device.", m_deviceNumber);
			delete capture;
			return false;
		}

		capture->setAudio(gasettings->cameraSettingsList()[m_deviceNumber].audioType);
			
		//
		// Set resolution
		//
		if (width != -1)
			capture->setProperty(CV_CAP_PROP_FRAME_WIDTH, width);
		else
		{
			width = capture->getProperty(CV_CAP_PROP_FRAME_WIDTH);
		}
		if (height != -1)
		{
			capture->setProperty(CV_CAP_PROP_FRAME_HEIGHT, height);
		}
		else
		{
			height = capture->getProperty(CV_CAP_PROP_FRAME_HEIGHT);
		}

		PANO_DEVICE_LOG(QString("Device resolution (Width: %1, Height: %2)").ARGN(width).ARGN(height));

		cap = capture;
	}

	if (m_captureType == CAPTUREFILE)
	{
		CaptureImageFile *capture = new CaptureImageFile;

		GlobalState &state = sharedImageBuffer->getState(m_deviceNumber);
		GlobalAnimSettings* gasettings = sharedImageBuffer->getGlobalAnimSettings();
		state.m_curFrame = gasettings->m_startFrame;

		//Buffer< cv::Mat >* imageBuffer = sharedImageBuffer->getByDeviceNumber( deviceNumber );
		CameraInput & camSettings = gasettings->cameraSettingsList()[m_deviceNumber];

		capture->setImageFileDir(camSettings.fileDir);
		capture->setImageFilePrefix(camSettings.filePrefix);
		capture->setImageFileExt(camSettings.fileExt);
		capture->setCurFrame(state.m_curFrame);

		std::cout << "Capturing From Image " << state.m_curFrame << std::endl;

		if (!capture->open(m_deviceNumber))
		{
			std::cout << "Can't Open File " << std::endl;
			//emit report(THREAD_TYPE_CAPTURE, "Can't Open image file.", m_deviceNumber);
			g_mainWindow->reportError(THREAD_TYPE_CAPTURE, "Can't Open image file.", m_deviceNumber);
			delete capture;
			return false;
		}

		capture->setProperty(CV_CAP_PROP_FPS, gasettings->m_fps);

		//
		// Set resolution
		//

		width = capture->getProperty(CV_CAP_PROP_FRAME_WIDTH);
		height = capture->getProperty(CV_CAP_PROP_FRAME_HEIGHT);
		//float fps = capture->getProperty(CV_CAP_PROP_FPS);

		gasettings->m_xres = width;
		gasettings->m_yres = height;
		//gasettings->m_fps = fps;

		cap = capture;
	}

	cap->setSnapshotPath(sharedImageBuffer->getGlobalAnimSettings()->m_snapshotDir);
	m_isCanGrab = true;

	//
	// 
	//
	return true;
}


bool CaptureThread::disconnect()
{
	//
	// Camera is connected
	//
	if (cap)
	{
		PANO_DEVICE_LOG("Disconnecting Camera...");
		//
		// Disconnect camera
		//
		delete cap;
		cap = 0;
		return true;
	}
	//
	// Camera is NOT connected
	//
	else
		return false;
}

void CaptureThread::updateFPS(int timeElapsed)
{
	//
	// Add instantaneous FPS value to queue
	//
	if (timeElapsed > 0)
	{
		fps.enqueue((int)1000 / timeElapsed);
		//
		// Increment sample number
		//
		sampleNumber++;
	}
	statsData.instantFPS = (1000.0 / timeElapsed);
	//
	// Maximum size of queue is DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH
	//
	if (fps.size() > CAPTURE_FPS_STAT_QUEUE_LENGTH)
		fps.dequeue();

	//
	// Update FPS value every DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH samples
	//
	if ((fps.size() == CAPTURE_FPS_STAT_QUEUE_LENGTH) && (sampleNumber == CAPTURE_FPS_STAT_QUEUE_LENGTH))
	{
		//
		// Empty queue and store sum
		//
		while (!fps.empty())
			fpsSum += fps.dequeue();
		//
		// Calculate average FPS
		//
		statsData.averageFPS = fpsSum / CAPTURE_FPS_STAT_QUEUE_LENGTH;
		//statsData.averageFPS = cap->getProperty( CV_CAP_PROP_FPS );
		//std::cout << statsData.averageFPS << std::endl;
		//
		// Reset sum
		//
		fpsSum = 0;
		//
		// Reset sample number
		//
		sampleNumber = 0;
	}
	float fps = 0;
	if (timeElapsed > 0)
		fps = 1000 / timeElapsed;

	GlobalAnimSettings* gasettings = sharedImageBuffer->getGlobalAnimSettings();

	//
	// Adjust frame playback speed if its loading from file
	// 
	/*
	if( m_captureType == CAPTUREFILE )
	{
	if( gasettings->m_playbackfps < fps )
	{
	float sleepms = (1.0/gasettings->m_playbackfps)*1000.0f - timeElapsed ;
	//std::cout << "Ms " << sleepms << std::endl;
	#ifdef _WIN32
	Sleep( sleepms );
	#else
	usleep( sleepms );
	#endif
	}
	}
	*/
}



void CaptureThread::stop()
{
	QMutexLocker locker(&doStopMutex);
	doStop = true;
}

void CaptureThread::playAndPause(bool isPause)
{
	QMutexLocker locker(&doPauseMutex);
	doPause = isPause;

	sharedImageBuffer->getStitcher()->playAndPause(isPause);
	if (sharedImageBuffer->getStreamer())
		sharedImageBuffer->getStreamer()->playAndPause(isPause);
}

void CaptureThread::snapshot(bool isCalibrate)
{
	QMutexLocker locker(&doPauseMutex);
	if (!doPause)	return;
	if (isCalibrate)
		doCalibrate = true;
	else
		doSnapshot = true;
}

bool CaptureThread::isConnected()
{
	return !cap;
}

int CaptureThread::getInputSourceWidth()
{
	return cap->getProperty(CV_CAP_PROP_FRAME_WIDTH);
}

int CaptureThread::getInputSourceHeight()
{
	return cap->getProperty(CV_CAP_PROP_FRAME_HEIGHT);
}
#if 0	// Original snapshot function
void CaptureThread::snapshot()
{
	cap->snapshot();
}
#endif
AudioInput * CaptureThread::getMic()
{
	return (AudioInput*)((CaptureDShow*)cap);
}

void CaptureThread::waitForFinish()
{
	finishMutex.lock();
	finishWC.wait(&finishMutex);
	finishMutex.unlock();
}

bool CaptureThread::isFinished()
{
	return m_finished;
}