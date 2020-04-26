
#include <iostream>
#include "SharedImageBuffer.h"
#include "D360Stitcher.h"
#include "D360Parser.h"


SharedImageBuffer::SharedImageBuffer()
{
	initialize();
}

SharedImageBuffer::~SharedImageBuffer()
{
	m_stitcher = NULL;
}

void SharedImageBuffer::initialize()
{
	nArrived = 0;
	doSync = false;

	videoProcessedId = -1;
	audioCapturedCount = 0;
	audioProcessedId = -1;
	m_globalStates.clear();
	rawImages.clear();
	m_streamer = NULL;
}


void SharedImageBuffer::add(int deviceNumber, bool sync)
{
	//
	// Device stream is to be synchronized
	//
	if (sync)
	{
		mutex.lock();
		doSync = sync;
		//std::cout << "Inserting " << deviceNumber << std::endl;
		syncSet.insert(deviceNumber);
		mutex.unlock();
	}
}

void SharedImageBuffer::addState(int deviceNumber, GlobalState& state, bool sync)
{
	//
	// If File Streams should be synced
	//
	if (sync)
	{
		mutex.lock();
		syncSet.insert(deviceNumber);
		mutex.unlock();
	}
	m_globalStates[deviceNumber] = state;
}

void SharedImageBuffer::setRawImage(int deviceNumber, ImageDataPtr image)
{
	rawImages[deviceNumber] = image;
}

SharedImageBuffer::ImageDataPtr SharedImageBuffer::getRawImage(int deviceNumber)
{
	return rawImages[deviceNumber];
}

void SharedImageBuffer::setAudioFrame(void* frame)
{
	audioFrame = frame;
	audioCapturedCount++;
}

void* SharedImageBuffer::getAudioFrame()
{
	return audioFrame;
}

//
// Add image buffer to map
//


void SharedImageBuffer::removeByDeviceNumber(int deviceNumber)
{//
	// Also remove from syncSet (if present)
	//
	mutex.lock();
	if (syncSet.contains(deviceNumber))
	{
		syncSet.remove(deviceNumber);
		wc.wakeAll();
	}
	mutex.unlock();

	videoMutex.lock();
	videoWCondition.wakeAll();
	videoMutex.unlock();

	audioMutex.lock();
	audioWCondition.wakeAll();
	audioMutex.unlock();
}

void SharedImageBuffer::sync(int deviceNumber)
{
	//
	// Only perform sync if enabled for specified device/stream
	//
	//std::cout << "Waiting count " << nArrived << std::endl;
	mutex.lock();

	if (syncSet.contains(deviceNumber))
	{
		//
		// Increment arrived count
		//
		nArrived++;

		//
		// We are the last to arrive: wake all waiting threads
		//

		if (doSync && (nArrived == syncSet.size()))
			wc.wakeAll();
		//
		// Still waiting for other streams to arrive: wait
		//
		else
		{
			//std::cout << deviceNumber << " Still Waiting count unlock " << nArrived << " Size " << syncSet.size() << " " << doSync << std::endl;
			wc.wait(&mutex);
		}

		//
		// Decrement arrived count
		//
		nArrived--;
	}
	mutex.unlock();
}

void SharedImageBuffer::wakeAll()
{
	QMutexLocker locker(&mutex);
	wc.wakeAll();
}

void SharedImageBuffer::setSyncEnabled(bool enable)
{
	doSync = enable;
}

void SharedImageBuffer::setViewSync(int deviceNumber, bool enable)
{
	if (enable)
	{
		if (!syncSet.contains(deviceNumber))
			syncSet.insert(deviceNumber);
	}
	else
	{
		if (syncSet.contains(deviceNumber))
			syncSet.remove(deviceNumber);
	}
}

bool SharedImageBuffer::isSyncEnabledForDeviceNumber(int deviceNumber)
{
	return syncSet.contains(deviceNumber);
}

bool SharedImageBuffer::getSyncEnabled()
{
	return doSync;
}

int	SharedImageBuffer::getSyncedCameraCount()
{
	return syncSet.size();
}

int SharedImageBuffer::getFirstAvailableViewId()
{
	for (int i = 0; i < m_globalAnimSettings->cameraSettingsList().size(); i++)
	{
		if (isSyncEnabledForDeviceNumber(i))
		{
			return i;
		}
	}
	return -1;
}

void SharedImageBuffer::setStitcher(std::shared_ptr< D360Sticher > stitcher)
{
	m_stitcher = stitcher;
	//m_stitcher->setGlobalAnimSettings(m_globalAnimSettings);
}

void SharedImageBuffer::syncForVideoProcessing(int videoFrameId)
{
	videoMutex.lock();
	if (videoFrameId > videoProcessedId + 1)
		videoWCondition.wait(&videoMutex, 2000);
	videoMutex.unlock();
}

void SharedImageBuffer::syncForAudioProcessing(int audioFrameId)
{
	audioMutex.lock();
#if 0
	if (audioCapturedCount > audioProcessedId + 1)
#else
	if (audioFrameId > audioProcessedId + 1)
#endif
		audioWCondition.wait(&audioMutex, 2000);
	audioMutex.unlock();
}

void SharedImageBuffer::wakeForVideoProcessing(int videoFrameId)
{
	videoMutex.lock();
	videoProcessedId = videoFrameId;
	videoWCondition.wakeAll();
	videoMutex.unlock();
}

void SharedImageBuffer::wakeForAudioProcessing(int audioFrameId)
{
	audioMutex.lock();
	audioProcessedId = audioFrameId;
	audioWCondition.wakeAll();
	audioMutex.unlock();
}