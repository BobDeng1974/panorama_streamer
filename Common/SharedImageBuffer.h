
#pragma once

// Qt
#include <QHash>
#include <QSet>
#include <QWaitCondition>
#include <QMutex>
#include "ImageBuffer.h"
#include <QImage>


// Local
#include "D360Parser.h"
#include "Buffer.h"



#define D360_FILEDEVICESTART 100


class GlobalState
{
public:
	float m_curFrame;
};

class D360Sticher;
class StreamProcess;

class SharedImageBuffer
{
public:
	typedef ImageBufferData ImageDataPtr;

	SharedImageBuffer();
	~SharedImageBuffer();
	void initialize();

	void add(int deviceNumber, bool sync = false);
	void setRawImage(int deviceNumber, ImageDataPtr image);
	ImageDataPtr getRawImage(int deviceNumber);

	void setAudioFrame(void* frame);
	void* getAudioFrame();

	void addState(int deviceNumber, GlobalState& state, bool sync = false);
	GlobalState& getState(int deviceNumber)
	{
		return m_globalStates[deviceNumber];
	}

	void removeByDeviceNumber(int deviceNumber);
	void sync(int deviceNumber);
	void wakeAll();
	void setSyncEnabled(bool enable);
	void setViewSync(int deviceNumber, bool enable);
	bool isSyncEnabledForDeviceNumber(int deviceNumber);
	bool getSyncEnabled();
	int	 getSyncedCameraCount();
	int getFirstAvailableViewId();

	void setGlobalAnimSettings(GlobalAnimSettings* globalAnimSettings)
	{
		m_globalAnimSettings = globalAnimSettings;
	}

	GlobalAnimSettings* getGlobalAnimSettings()
	{
		return m_globalAnimSettings;
	}

	std::shared_ptr< D360Sticher > getStitcher()
	{
		return m_stitcher;
	}

	void setStitcher(std::shared_ptr< D360Sticher > stitcher);

	void syncForVideoProcessing(int videoFrameId);
	void syncForAudioProcessing(int audioFrameId = 0);
	void wakeForVideoProcessing(int videoFrameId);
	void wakeForAudioProcessing(int audioFrameId);

	void setStreamer(StreamProcess* streamer) { m_streamer = streamer; }
	StreamProcess* getStreamer() { return m_streamer; }

private:
	StreamProcess* m_streamer;
	std::map<int, ImageDataPtr> rawImages;
	void* audioFrame;

	QSet< int > syncSet;
	QWaitCondition wc;
	QMutex mutex;
	int nArrived;
	bool doSync;

	// Offline processing
	QMutex videoMutex;
	QWaitCondition videoWCondition;
	int videoProcessedId;

	QMutex audioMutex;
	QWaitCondition audioWCondition;
	int audioCapturedCount;
	int audioProcessedId;

	GlobalAnimSettings* m_globalAnimSettings;
	QHash< int, GlobalState> m_globalStates;

	std::shared_ptr< D360Sticher > m_stitcher;
};
