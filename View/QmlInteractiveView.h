#ifndef QMLINTERACTIVEVIEW_H
#define QMLINTERACTIVEVIEW_H

#include <QQuickItem>
#include <QTimer>
#include <QSGTexture>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QtGui/QOpenGLShaderProgram>
#include <QPointer>
#include "D360Parser.h"
#include "CaptureThread.h"
#include "Structures.h"
#include "D360Stitcher.h"
#include "SharedImageBuffer.h"
#include "OculusViewer.h"
#include "GLSLPanoramaInteract.h"

QT_FORWARD_DECLARE_CLASS(QGLShaderProgram);

#include <math.h>

class OculusTexture : public QSGTexture, protected QOpenGLFunctions
{

public:
	OculusTexture(bool ownTextureID);
	~OculusTexture();

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
private:
	bool m_ownTextureID;
	GLuint m_textureId;
	QSize m_viewportSize;

	unsigned int imageWidth;
	unsigned int imageHeight;
};


class QmlInteractiveView : public QQuickItem
{
	Q_OBJECT

public:
	QmlInteractiveView(QQuickItem* parent = 0);
	~QmlInteractiveView();

	QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*);
	void	 setSharedImageBuffer(SharedImageBuffer* sharedImageBuffer) { m_sharedImageBuffer = sharedImageBuffer; }
	void	 setOculusObject(OculusRender* m_oculusdevice) { m_oculus = m_oculusdevice; }


	void	onMouseMove(QPoint pos);
	void	onMousePress(QPoint pos);
	void	onMouseRelease(QPoint pos);

	virtual void	mouseMoveEvent(QMouseEvent * event);
	virtual void	mousePressEvent(QMouseEvent * event);
	virtual void	mouseReleaseEvent(QMouseEvent * event);
signals:	
	void sendClose();

public:	
	void closeCameraView(){ emit sendClose(); }
private:
	vec3 getCrossPt(QPoint pos);
protected:
	SharedImageBuffer*	m_sharedImageBuffer;
	QOffscreenSurface*	m_surface;
	OculusRender*		m_oculus;
	GLSLPanoramaInteract*		m_interact;
	QOpenGLFunctions_2_0* functions_2_0;

private:
	QRectF	m_oldGeometry;
	bool	m_updateConstantly;
	bool	m_keepAspectRatio;
	double	m_aspect;
	bool	aspectChanged;
	OculusTexture * m_pOculusTexture;

	bool m_blMousePressed;
	QPoint	m_pressPt;
	vec3	m_crossPt;
	//sd_scalar	m_orgYaw;
	//sd_scalar	m_orgPitch;
	//sd_scalar	m_orgRoll;
};

#endif // TEXTUREIMAGEITEM_H
