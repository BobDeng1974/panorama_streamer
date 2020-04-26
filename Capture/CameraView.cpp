#include "CameraView.h"

// Qt
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QMessageBox>



/////////////////////// CameraModule
CameraModule::CameraModule(int deviceNumber, SharedImageBuffer *sharedImageBuffer, QObject* main) :
sharedImageBuffer(sharedImageBuffer)
, captureThread(0)
, audioThread(0)
, captureThreadInstance(0)
, processingThreadInstance(0)
, m_Main(main)
{
	// Save Device Number
	this->m_deviceNumber = deviceNumber;
	m_Name = "CameraModule";

	// Initialize internal flag
	isCameraConnected = false;

	// Register type
	qRegisterMetaType< struct ThreadStatisticsData >("ThreadStatisticsData");
	qRegisterMetaType<MatBuffer>("MatBufferPtr");
}

CameraModule::~CameraModule()
{
	qquit();

	if (captureThread)
	{		
		captureThread->stop();		
		delete captureThread;
	}

	if (audioThread)
	{
		audioThread->stop();		
		delete audioThread;
	}
	if (captureThreadInstance)
	{
		delete captureThreadInstance;
		captureThreadInstance = NULL;
	}
	if (processingThreadInstance)
	{
		delete processingThreadInstance;
	}
}


bool CameraModule::connectToCamera(int width, int height,
	float exp, float fps, int audioDevIndex, int dupIndex)
{
	//
	// Create capture thread
	//
	if (sharedImageBuffer->getGlobalAnimSettings()->m_captureType == D360::Capture::CAPTURE_FILE)
	//if (m_deviceNumber >= D360_FILEDEVICESTART)
	{
		captureThread = new CaptureThread(sharedImageBuffer, m_deviceNumber, CaptureThread::CAPTUREFILE, width, height);
	}
	else if (sharedImageBuffer->getGlobalAnimSettings()->m_captureType == D360::Capture::CAPTURE_DSHOW)
	{
		captureThread = new CaptureThread(sharedImageBuffer, m_deviceNumber, CaptureThread::CAPTUREDSHOW, width, height);
	}
	else if (sharedImageBuffer->getGlobalAnimSettings()->m_captureType == D360::Capture::CAPTURE_VIDEO)
	{
		captureThread = new CaptureThread(sharedImageBuffer, m_deviceNumber, CaptureThread::CAPTUREVIDEO, width, height);
	}
	else
	{
		captureThread = new CaptureThread(sharedImageBuffer, m_deviceNumber, CaptureThread::CAPTUREXIMEA, width, height);
	}

	captureThread->setExposure(exp);
	captureThread->setFrameRate(fps);

	//
	// Attempt to connect to camera
	// 
	if (captureThread->connect(dupIndex))
	{
		//std::cout << "Starting Camera " << m_deviceNumber << std::endl;

		//
		// Create processing thread
		//
#ifndef REMOVE_PROCESSING_THREAD_IN_CAMERAVIEW	/* [Updated by B] */		
		processingThread = new ProcessingThread(sharedImageBuffer, m_deviceNumber);
#endif
		captureThreadInstance = new QThread;
		captureThread->moveToThread(captureThreadInstance);
		//connect(captureThread, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
		connect(captureThreadInstance, SIGNAL(started()), captureThread, SLOT(process()));
		//connect( captureThread, SIGNAL( finished() ), captureThreadInstance, SLOT( quit()));
		//connect(captureThreadInstance, SIGNAL(finished()), captureThread, SLOT(deleteLater()));
		connect(captureThread, SIGNAL(finished(int, QString, int)), captureThreadInstance, SLOT(deleteLater()));
		if (m_Main) {
			connect(captureThread, SIGNAL(finished(int, QString, int)), m_Main, SLOT(finishedThread(int, QString, int)));
			connect(captureThread, SIGNAL(started(int, QString, int)), m_Main, SLOT(startedThread(int, QString, int)));
			connect(captureThread, SIGNAL(report(int, QString, int)), m_Main, SLOT(reportError(int, QString, int)));
			connect(captureThread, SIGNAL(snapshoted(int)), m_Main, SLOT(finishedSnapshot(int)));
		}

#ifndef REMOVE_PROCESSING_THREAD_IN_CAMERAVIEW	/* [Updated by B] */
		processingThreadInstance = new QThread;
		processingThread->moveToThread(processingThreadInstance);
		connect(processingThreadInstance, SIGNAL(started()), processingThread, SLOT(process()));
		//connect( processingThread, SIGNAL(finished()), processingThreadInstance, SLOT(quit()));
		connect(processingThreadInstance, SIGNAL(finished()), processingThread, SLOT(deleteLater()));
		connect(processingThread, SIGNAL(finished()), processingThreadInstance, SLOT(deleteLater()));
#endif
		//
		// Set internal flag and return
		//
		isCameraConnected = true;
#if 0
		if (sharedImageBuffer->getGlobalAnimSettings().cameraSettingsList()[m_deviceNumber].isExistAudio() &&
			audioDevIndex != -1) {
			audioThread = new AudioThread(sharedImageBuffer);
			//audioThread->initialize(sharedImageBuffer, m_deviceNumber);
			//mic = audioThread->getMic();
		}
#endif
		return true;
	}
	// Failed to connect to camera
	else
		return false;
}

void CameraModule::snapshot(bool isCalibrate)
{
	captureThread->snapshot(isCalibrate);
}

IAudioThread * CameraModule::getAudioThread()
{
	return (IAudioThread*)captureThread;
}

CaptureThread* CameraModule::getCaptureThread()
{
	return captureThread;
}

void CameraModule::startThreads()
{
	if (captureThreadInstance)
		captureThreadInstance->start();
	//if (audioThread) {
	//	audioThread->initialize(sharedImageBuffer, m_deviceNumber);
	//	audioThread->startThread();
	//}
	if (processingThreadInstance)
		processingThreadInstance->start();

	/*
	D360Sticher* stitcher = sharedImageBuffer->getStitcher().get();
	GlobalAnimSettings& gasettings = sharedImageBuffer->getGlobalAnimSettings();

	stitcher->setGlobalAnimSettings( gasettings );
	connect( processingThread, SIGNAL( newStitchFrameMat( MatBufferPtr, int, int ) ),
	stitcher, SLOT( updateStitchFrameMat( MatBufferPtr, int, int ) ) );
	*/
}

void CameraModule::qquit()
{
	//std::cout << "Camera Module Finished " << std::endl;

	if (isCameraConnected)
	{
		//
		// Stop capture thread
		//
		//if (captureThreadInstance && captureThreadInstance->isRunning()) // commented by B
		if (captureThreadInstance)
		{
			stopCaptureThread();
			// After this, on release function, will be delete.
			//delete captureThreadInstance;
			//captureThreadInstance = NULL;
		}

		//
		// Stop processing thread
		//
		if (processingThreadInstance && processingThreadInstance->isRunning())
			stopProcessingThread();


		// Automatically start frame processing (for other streams)
		/*
		if( sharedImageBuffer->isSyncEnabledForDeviceNumber( deviceNumber ) )
		sharedImageBuffer->setSyncEnabled( true );
		*/

		// Remove from shared buffer
		sharedImageBuffer->removeByDeviceNumber(m_deviceNumber);
		//
		// Disconnect camera
		//
		/*
		if( captureThread->disconnect() )
		qDebug() << "[" << deviceNumber << "] Camera successfully disconnected.";
		else
		qDebug() << "[" << deviceNumber << "] WARNING: Camera already disconnected.";
		*/
	}
	// Delete UI
	//delete ui;
}

void CameraModule::stopCaptureThread()
{
	PANO_DEVICE_LOG("About to stop capture thread...");

	captureThread->stop();
	captureThread->thread()->terminate();

	sharedImageBuffer->wakeAll(); // This allows the thread to be stopped if it is in a wait-state
	if (!captureThread->isFinished())
		captureThread->waitForFinish();

	// Take one frame off a FULL queue to allow the capture thread to finish

	PANO_DEVICE_LOG("Capture thread successfully stopped.");
}

void CameraModule::stopProcessingThread()
{
	PANO_DEVICE_LOG("About to stop processing thread...");


#if 0 /*[C]*/
	processingThread->stop();
	processingThread->thread()->wait(200);
#endif

	//sharedImageBuffer->wakeAll(); // This allows the thread to be stopped if it is in a wait-state

	PANO_DEVICE_LOG("Processing thread successfully stopped.");
}