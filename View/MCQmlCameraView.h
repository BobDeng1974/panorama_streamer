#ifndef MCQMLCAMERAVIEW_H
#define MCQMLCAMERAVIEW_H

#include <QQuickItem>
#include <QTimer>
#include <QSGTexture>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QtGui/QOpenGLShaderProgram>
#include <QPointer>
#include "D360Parser.h"

#include "CaptureThread.h"
//#include "ProcessingThread.h"
#include "Structures.h"
#include "D360Stitcher.h"
#include "SharedImageBuffer.h"


//#include <cuda_runtime.h>

QT_FORWARD_DECLARE_CLASS(QGLShaderProgram);

#define PBO_COUNT 2

class OpenCVTexture : public QSGTexture, protected QOpenGLFunctions
{

public:
	OpenCVTexture(bool ownTextureID);
	~OpenCVTexture();

	inline void bind() {
		glBindTexture(GL_TEXTURE_2D, m_textureId);

	}

	int textureId() const {
		int tex = (int)m_textureId;
		return tex;
	}

	inline QSize textureSize() const {
		return QSize(imageWidth, imageHeight);
	}

	inline bool hasAlphaChannel() const {
		return false;
	}

	inline bool hasMipmaps() const {
		return true;
	}

	inline QSGTexture* texture() const {
		return (QSGTexture*)this;
	}

	void updateTexture(int textureId, int width, int height, QImage::Format format);
	//void updateTexture(byte* data, int width, int height, QImage::Format format);
private:
	bool m_ownTextureID;
	GLuint m_textureId;
	QSize m_viewportSize;
	bool m_blFirst;

	unsigned int imageWidth;
	unsigned int imageHeight;
	int m_ImageFormatValue;
	bool m_bTextureMapped;
};


class MCQmlCameraView : public QQuickItem
{
	Q_OBJECT
	Q_PROPERTY(QString cameraViewName READ cameraViewName NOTIFY setCameraViewName)	
	Q_PROPERTY(int cameraNumber READ cameraNumber NOTIFY cameraNumberChanged)

public:
	MCQmlCameraView(QQuickItem* parent = 0);
	~MCQmlCameraView();

	QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*);

	virtual void resizeEvent(QResizeEvent * event);

	void setSharedImageBuffer(SharedImageBuffer* sharedImageBuffer) { m_sharedImageBuffer = sharedImageBuffer; }
	void init(bool blLiveView, QOpenGLContext* context);

	void	onMouseMove(QPoint pos);
	void	onMousePress(QPoint pos);
	void	onMouseRelease(QPoint pos);

protected:
	void	mouseMoveEvent(QMouseEvent * event);
	void	mousePressEvent(QMouseEvent * event);
	void	mouseReleaseEvent(QMouseEvent * event);

signals:	
	void setCameraViewName(QString cameraName);
	void cameraNumberChanged(int camreaNumber);
	void sendClose();

public:	
	QString cameraViewName() const;
	void setCameraName(QString cameraName);
	void setCameraNumber(int deviceNum);
	int cameraNumber() const;
	void closeCameraView(){ emit sendClose(); }
	

protected:
	int m_camDeviceNumber;
	SharedImageBuffer* m_sharedImageBuffer;
	
	GlobalAnimSettings	m_gaSettings;
	QOffscreenSurface* m_surface;
	QOpenGLContext* m_context;
	QOpenGLFunctions_2_0* functions_2_0;

protected slots:
	void handleWindowChanged(QQuickWindow *win);
	void sync();

private:
	QRectF m_oldGeometry;

	bool m_updateConstantly;
	bool m_blLiveView;
	OpenCVTexture * m_pVideoTexture;

	bool m_keepAspectRatio;
	double m_aspect;
	bool aspectChanged;

	QString m_strCameraName;

	float m_orgYaw;
	float m_orgPitch;
	float m_orgRoll;
	bool m_blMousePressed;
	QPoint	m_pressPt;
};

#endif // TEXTUREIMAGEITEM_H
