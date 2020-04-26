#pragma once

#define ENABLE_LOG 1

#include <QObject>
#include <QMutex>
#include <QRunnable>
#include <QDateTime>

#include <QOpenGLFunctions>
#include <QtGui/QOpenGLShaderProgram>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions_2_0>

#include <iostream>
#include <fstream>
#include <string>

#include "Structures.h"
#include "SharedImageBuffer.h"
#include "Config.h"

#include "GLSLUnwarp.h"
#include "GLSLComposite.h"
#include "GLSLColorCvt.h"
#include "GLSLGainCompensation.h"
#include "GLSLPanoramaPlacement.h"
#include "GLSLLiveViewer.h"
#include "GLSLFinalPanorama.h"

class D360Sticher: public QObject//, public QRunnable
{
	  Q_OBJECT

public:
	D360Sticher(SharedImageBuffer *sharedImageBuffer, QObject* main = NULL);
	~D360Sticher( void );

	void init(QOpenGLContext* context);

	void reset(); // this will be called to dispose any objects that were allocated in init()
	void setup();
	void startThread();
	void stopStitcherThread();
	void playAndPause(bool isPause);
	void calcGain();
	void resetGain();
	void reStitch();
	void updateCameraParams();

	QOpenGLContext* getContext();
	int getRawTextureId(int cameraIndex);
	int getPanoramaTextureId();
	int getPanoramaTextureIdForInteract();
#if 0 /*C*/
	int getLiveViewerTextureId();
#endif

	void waitForFinish();
	bool isFinished();

	void setSeamIndex(int index);
	void setBlendingMode(GlobalAnimSettings::BlendingMode mode, int level = 0);

	void setAdvMode(bool isAdvMode);
	void setAdvPoints(vec2 advPt[]);

protected:
	SharedImageBuffer* sharedImageBuffer;	

	std::map<int, ImageBufferData> rawImagesBuffer;
	std::map<int, ImageBufferData> readyBuffer;
	int readyFrameNr;

public:
	bool isAdvModeRaw;
	bool isAdvModeUse;
protected:
	ImageBufferData advBufferRaw;
	ImageBufferData advBufferUse;
	QImage advBufferImage;
	vec2 advQuad[4]; // advert coords in 1-1-1 gl coordinates.

	QMutex m_stitchMutex;
	QMutex m_advMutex;
	QMutex m_advMutex2;
	QThread* m_stitcherThreadInstance;

	GlobalAnimSettings* m_gaSettings;

private:
	QObject* m_Main;
	QString m_Name;
	QOffscreenSurface* m_surface;
	QOpenGLContext* m_context;
	QOpenGLFunctions_2_0* functions_2_0;

	int m_nViewCount;
	std::vector<GLSLColorCvt_2RGB*> m_2rgbColorCvt;
	std::vector<GLSLGainCompensation*> m_gainCompensation;
	std::vector<GLSLUnwarp*> m_unwarp;
	std::vector<GLSLComposite*> m_composite;
	std::vector<GLSLPanoramaPlacement*> m_placement;
	GLSLFinalPanorama*		m_interractPanorama; // to make source texture for interactive panorama
	GLSLFinalPanorama* m_finalPanorama;
	UniColorCvt m_advColorCvt;

	QMutex finishMutex;
	QWaitCondition finishWC;
	bool m_finished;

	QMutex doPauseMutex;
	bool doPause;
	bool doCalcGain;
	bool doResetGain;
	bool doReStitch;
	bool doUpdateCameraParams;

#if 0 /*C*/
	GLSLLiveViewer *m_liveViewer;
#endif

	unsigned char *m_fboBuffer;
	// FBO related texture ids for unwarped textures
	GLuint m_panoSegsLeft[16];
	GLuint m_panoSegsRight[16];
	int m_nPanoramaReadyCount;

public slots:
	void process();
	void qquit();

	void updateStitchFrameMat(ImageBufferData& mb, int camIndex, int frameNum);
	void updateAdvFrameMat(ImageBufferData& mb, int frameNum = -1);
	void updateAdvImage(QImage image);
	void doCaptureIncomingFrames();
	void doStitch(bool isPlacementOnly = false);

protected:
	void run();	
	void stop();

	void initialize();

private:
	void calcExposure();
	void updateFPS(int);

	bool doStop;
	bool isFirstFrame;
	int stitchingTime;

	QTime t;
	QQueue<int> fps;
	QMutex doStopMutex;
	struct ThreadStatisticsData statsData;
	int fpsSum;
	int sampleNumber;
signals:
	void newPanoramaFrameReady(unsigned char* buffer);

	void updateStatisticsInGUI(struct ThreadStatisticsData);
	void finished(int type, QString msg, int id);
	void started(int type, QString msg, int id);
};

