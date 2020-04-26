#pragma once

#include <QMap>
#include <QtNetwork/QUdpSocket>
#include <QObject>


#include "SharedImageBuffer.h"

#include "CaptureThread.h"

#include "D360Parser.h"
#include "D360Stitcher.h"



class D360Process: public QObject
{
public:
	D360Process();
	~D360Process();

	void initialize();

	SharedImageBuffer* getSharedImageBuffer() 
	{
		return m_sharedImageBuffer;
	}

	
	void processTheDatagram( QByteArray& datagram );

	void startStitchThread();
	std::shared_ptr<D360Sticher> getStitcherThread()
	{
		return stitcherThread;
	}

protected:

	bool connectToCamera();
	void connectToCameras();
	void disconnectCamera( int index );
	void disconnectCameras();

	void loadSavedFrames();



	bool attach( CameraInput& camSettings, int deviceNumber, float startframe, float endframe, float fps, int imageBufferSize, int nextTabIndex = 0 );
	bool removeFromMapByTabIndex( QMap<int, int> &map, int tabIndex );

	D360Parser m_d360Data;

	QMap< int, int > m_deviceNumberMap;
#if 0 /*[C]*/
	QMap< int, CameraView* > m_cameraViewMap;
#endif
	
	SharedImageBuffer *m_sharedImageBuffer;

#if 0 /*[C]*/
	ProcessingThread *processingThread;
#endif
	CaptureThread    *captureThread;
	std::shared_ptr<D360Sticher> stitcherThread;


	QUdpSocket* udpSocket;

public slots:
		void close();
		void record();
		void pause();
		void captureNew();
		void readPendingDatagrams();
};

