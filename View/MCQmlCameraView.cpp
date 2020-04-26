#include "MCQmlCameraView.h"

#include <QSGOpaqueTextureMaterial>
#include <QSGNode>
#include <QQuickWindow>
#include <QQmlApplicationEngine>
#include <QTime>
#include "QThread.h"
#include "3DMath.h"
#include "QmlMainWindow.h"

extern QmlMainWindow* g_mainWindow;


#define VIDEO_WIDTH		1220.00
#define VIDEO_HEIGHT	1000.00


OpenCVTexture::OpenCVTexture(bool ownTextureID)
{
	m_ownTextureID = ownTextureID;
	m_textureId = 0;
	m_blFirst = true;

	initializeOpenGLFunctions();

	glGenTextures(1, &m_textureId);
	glBindTexture(GL_TEXTURE_2D, m_textureId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	m_bTextureMapped = false;
	imageWidth = 0;
	imageHeight = 0;

}


OpenCVTexture::~OpenCVTexture() {
	if (m_ownTextureID)
		glDeleteTextures(1, &m_textureId);
}

void OpenCVTexture::updateTexture(int textureId, int width, int height, QImage::Format format)
{
	imageWidth = width;
	imageHeight = height;
	if (m_ownTextureID)
	{
		glDeleteTextures(1, &m_textureId);
		m_ownTextureID = false;
	}
	m_textureId = textureId;
}

//void OpenCVTexture::updateTexture(byte* data, int width, int height, QImage::Format format)
//{
//	glBindTexture(GL_TEXTURE_2D, m_textureId);
//	if (imageWidth != width || imageHeight != height)
//	{
//		imageWidth = width;
//		imageHeight = height;
//		if (format == QImage::Format_Indexed8) {
//			glTexImage2D(GL_TEXTURE_2D,     // Type of texture
//				0,                 // Pyramid level (for mip-mapping) - 0 is the top level
//				GL_DEPTH_COMPONENT,            // Internal colour format to convert to
//				width,          // Image width
//				height,          // Image height
//				0,                 // Border width in pixels (can either be 1 or 0)
//				GL_DEPTH_COMPONENT, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
//				GL_UNSIGNED_BYTE,  // Image data type
//				NULL);        // The actual image data itself
//		}
//		else if (format == QImage::Format_RGB888){
//			glTexImage2D(GL_TEXTURE_2D,     // Type of texture
//				0,                 // Pyramid level (for mip-mapping) - 0 is the top level
//				GL_RGB,            // Internal colour format to convert to
//				width,          // Image width
//				height,          // Image height
//				0,                 // Border width in pixels (can either be 1 or 0)
//				GL_RGB, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
//				GL_UNSIGNED_BYTE,  // Image data type
//				NULL);        // The actual image data itself
//		}
//		else if (format == QImage::Format_RGB32){
//			glTexImage2D(GL_TEXTURE_2D,     // Type of texture
//				0,                 // Pyramid level (for mip-mapping) - 0 is the top level
//				GL_RGBA,            // Internal colour format to convert to
//				width,          // Image width
//				height,          // Image height
//				0,                 // Border width in pixels (can either be 1 or 0)
//				GL_RGBA, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
//				GL_UNSIGNED_BYTE,  // Image data type
//				NULL);        // The actual image data itself
//		}
//	}
//	if (data != nullptr)
//	{
//		if (format == QImage::Format_Indexed8)
//		{
//			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, data);
//		}
//		else if (format == QImage::Format_RGB888)
//		{
//			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
//		}
//		else if (format == QImage::Format_RGB32)
//		{
//			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
//		}
//	}
//}




MCQmlCameraView::MCQmlCameraView(QQuickItem* parent) : QQuickItem(parent),
m_pVideoTexture(0),
m_keepAspectRatio(true),
aspectChanged(false)
{
	setFlag(ItemHasContents, true);
	m_aspect = VIDEO_WIDTH / VIDEO_HEIGHT;
	m_pVideoTexture = NULL;
	m_camDeviceNumber = 0;
	m_blLiveView = true;
	m_strCameraName = "";
	connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(handleWindowChanged(QQuickWindow*)));
	//connect(this, SIGNAL(sendClose()), this, SLOT(closeView()));

	setAcceptedMouseButtons(Qt::MouseButtonMask);

	m_blMousePressed = false;
	m_orgYaw = m_orgPitch = m_orgRoll = 0;

}

void MCQmlCameraView::resizeEvent(QResizeEvent *event) 
{
}

MCQmlCameraView::~MCQmlCameraView()
{
	//if (m_pVideoTexture){
	//	m_pVideoTexture->deleteLater();
	//}
	//if (m_2rgbColorCvt)
	//	delete m_2rgbColorCvt;
}

void MCQmlCameraView::handleWindowChanged(QQuickWindow *win)
{
	if (win) {
		connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(sync()), Qt::DirectConnection);
		//connect(win, SIGNAL(sceneGraphInvalidated()), this, SLOT(cleanup()), Qt::DirectConnection);
		//! [1]
		// If we allow QML to do the clearing, they would clear what we paint
		// and nothing would show.
		//! [3]
		win->setClearBeforeRendering(false);
	}
}

void MCQmlCameraView::sync()
{
}

QString MCQmlCameraView::cameraViewName() const
{
	return m_strCameraName;
}

void MCQmlCameraView::setCameraName(QString cameraName)
{
	m_strCameraName = cameraName;
	emit setCameraViewName(m_strCameraName);
}

void MCQmlCameraView::setCameraNumber(int deviceNum)
{
	m_camDeviceNumber = deviceNum;
	emit cameraNumberChanged(m_camDeviceNumber);
}

int MCQmlCameraView::cameraNumber() const
{
	return m_camDeviceNumber;
}

QSGNode* MCQmlCameraView::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{

	QSGGeometryNode* node = 0;
	QSGGeometry* geometry = 0;

	QSGNode::DirtyState dirtyState;
	
	if (m_pVideoTexture && m_sharedImageBuffer)
	{
		if (m_blLiveView)
		{
#if 1
			//std::cout << "Processing Frame " << statsData.nFramesProcessed << std::endl;
			SharedImageBuffer::ImageDataPtr c;
			SharedImageBuffer::ImageDataPtr currentFrame = m_sharedImageBuffer->getRawImage(m_camDeviceNumber);
		
			bool isImageAvailable = false;
			isImageAvailable = currentFrame.mImageY.width != 0;

			if (currentFrame.mImageY.buffer != NULL && isImageAvailable)
			{
#if 0
				ImageBufferData newMat;
				newMat.mImageY = currentFrame.mImageY;
				newMat.mImageU = currentFrame.mImageU;
				newMat.mImageV = currentFrame.mImageV;
				m_2rgbColorCvt->render(newMat);
				m_pVideoTexture->updateTexture(m_2rgbColorCvt->getTargetTexture(), currentFrame.mImageY.width, currentFrame.mImageY.height, QImage::Format_RGB888);
#else
				m_aspect = ((double)currentFrame.mImageY.width) / currentFrame.mImageY.height;
				m_pVideoTexture->updateTexture(m_sharedImageBuffer->getStitcher()->getRawTextureId(m_camDeviceNumber), currentFrame.mImageY.width, currentFrame.mImageY.height, QImage::Format_RGB888);
#endif
			}
#endif
		}
		else
		{
			GlobalAnimSettings* gasettings = m_sharedImageBuffer->getGlobalAnimSettings();
			int panoXRes = gasettings->getPanoXres();
			int panoYRes = gasettings->getPanoYres();
			if (gasettings->isStereo())
				panoYRes *= 2;
			m_aspect = ((double)panoXRes) / panoYRes;
			
			if (m_sharedImageBuffer->getStitcher())
				m_pVideoTexture->updateTexture(m_sharedImageBuffer->getStitcher()->getPanoramaTextureId(), panoXRes, panoYRes, QImage::Format_RGB888);
		}
	}

	if (!oldNode) {
		// Create the node.
		node = new QSGGeometryNode;
		geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
		geometry->setDrawingMode(GL_TRIANGLE_STRIP);
		node->setGeometry(geometry);
		node->setFlag(QSGNode::OwnsGeometry);

		QSGOpaqueTextureMaterial* material = new QSGOpaqueTextureMaterial; //QSGTextureMaterial  can be used for opacity support


		m_pVideoTexture = new OpenCVTexture(true);
		m_pVideoTexture->setFiltering(QSGTexture::Linear);
		m_pVideoTexture->setMipmapFiltering(QSGTexture::None);
		m_pVideoTexture->setHorizontalWrapMode(QSGTexture::WrapMode::ClampToEdge);
		m_pVideoTexture->setVerticalWrapMode(QSGTexture::WrapMode::ClampToEdge);

		material->setTexture(m_pVideoTexture);

		node->setMaterial(material);
		node->setFlag(QSGNode::OwnsMaterial);
	}
	else {
		node = static_cast<QSGGeometryNode *>(oldNode);
		geometry = node->geometry();
	}

	//if (m_oldGeometry != boundingRect() || aspectChanged)
	{
		aspectChanged = false;
		// Create the vertices and map to texture.
		QRectF bounds = boundingRect();
		QSGGeometry::TexturedPoint2D* vertices = geometry->vertexDataAsTexturedPoint2D();
		if (!m_keepAspectRatio)
		{
			//vertices[0].set(bounds.x(), bounds.y() + bounds.height(), 0.0f, 1.0f);
			//vertices[1].set(bounds.x() + bounds.width(), bounds.y() + bounds.height(), 1.0f, 1.0f);
			//vertices[2].set(bounds.x(), bounds.y(), 0.0f, 0.0f);
			//vertices[3].set(bounds.x() + bounds.width(), bounds.y(), 1.0f, 0.0f);
		}
		else
		{
			double currentAspect = bounds.width() / bounds.height();
			if (currentAspect >= m_aspect)//width should be adjusted
			{
				vertices[0].set(bounds.x() + bounds.width() / 2.0 - bounds.height() * m_aspect / 2.0,
					bounds.y() + bounds.height(),
					0.0f, 1.0f);
				vertices[1].set(bounds.x() + bounds.width() / 2.0 + bounds.height() * m_aspect / 2.0,
					bounds.y() + bounds.height(),
					1.0f, 1.0f);
				vertices[2].set(bounds.x() + bounds.width() / 2.0 - bounds.height() * m_aspect / 2.0,
					bounds.y(),
					0.0f, 0.0f);
				vertices[3].set(bounds.x() + bounds.width() / 2.0 + bounds.height() * m_aspect / 2.0,
					bounds.y(),
					1.0f, 0.0f);
			}
			else //height should be adjusted
			{
				vertices[0].set(bounds.x(),
					bounds.y() + bounds.height() / 2.0 + bounds.width() / m_aspect / 2.0,
					0.0f, 1.0f);
				vertices[1].set(bounds.x() + bounds.width(),
					bounds.y() + bounds.height() / 2.0 + bounds.width() / m_aspect / 2.0,
					1.0f, 1.0f);
				vertices[2].set(bounds.x(),
					bounds.y() + bounds.height() / 2.0 - bounds.width() / m_aspect / 2.0,
					0.0f, 0.0f);
				vertices[3].set(bounds.x() + bounds.width(),
					bounds.y() + bounds.height() / 2.0 - bounds.width() / m_aspect / 2.0,
					1.0f, 0.0f);
			}
		}


		dirtyState |= QSGNode::DirtyGeometry;

	}
	if (m_updateConstantly)
	{
		dirtyState |= QSGNode::DirtyMaterial;
		update();
	}
	node->markDirty(dirtyState);

	m_oldGeometry = boundingRect();

	return node;
}

void MCQmlCameraView::init(bool blLiveView, QOpenGLContext* context)
{
#if 1
	m_blLiveView = blLiveView;
#if 0
	GlobalAnimSettings& gasettings = m_sharedImageBuffer->getGlobalAnimSettings();
	m_gaSettings = gasettings;

	m_surface = new QOffscreenSurface();
	m_surface->create();

	m_context = new QOpenGLContext();
	QSurfaceFormat format = m_surface->requestedFormat();
	format.setSwapInterval(0);
	format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	if (context)
		m_context->setShareContext(context);
	m_context->setFormat(format);
	m_context->create();


	QOpenGLVersionProfile profile;
	profile.setProfile(m_context->surface()->format().profile());
	functions_2_0 = ((QOpenGLFunctions_2_0*)m_context->versionFunctions(profile));

	int imgWidth = gasettings.m_xres;
	int imgHeight = gasettings.m_yres;

	m_2rgbColorCvt = new GLSLColorCvt_RGB2RGB();
	m_2rgbColorCvt->setGL((QOpenGLFunctions*)m_context->functions());

	m_2rgbColorCvt->initialize(imgWidth, imgHeight);
#endif
#endif
}

void MCQmlCameraView::mouseMoveEvent(QMouseEvent * event)
{
}

void MCQmlCameraView::mousePressEvent(QMouseEvent * event)
{
}

void MCQmlCameraView::mouseReleaseEvent(QMouseEvent * event)
{
}

void MCQmlCameraView::onMouseMove(QPoint pos)
{
	if (!m_sharedImageBuffer)
		return;
	GlobalAnimSettings& setting = g_mainWindow->getGlobalAnimSetting();

	QRectF bounds = boundingRect();
	int iWidth = bounds.width();
	int iHeight = bounds.height();
	int iPosX = pos.x();
	int iPosY = pos.y();
	if (m_blMousePressed)
	{
		float fPanoAspect = m_aspect;
		float fViewAspect = iWidth / (float)iHeight;
		int viewWidth, viewHeight;
		if (fPanoAspect > fViewAspect)
		{
			viewWidth = iWidth;
			viewHeight = iWidth / fPanoAspect;
		}
		else
		{
			viewHeight = iHeight;
			viewWidth = iHeight * fPanoAspect;
		}
		int cPx = iWidth / 2;
		int cPy = iHeight / 2;
		float scaleYaw, scalePitch, scaleRoll;
		scaleYaw = sd_two_pi * sd_to_deg / viewWidth;
		scalePitch = sd_pi * sd_to_deg / viewHeight;
		scaleRoll = sd_two_pi * sd_to_deg / viewHeight;
		float wYaw, wPitch, wRoll;
		// view -x:	-			-			    cP				-				-
		// Sphere:  back		left			front			right			back
		// wRoll:	0			1				0				-1				0
		// wPitch:	-1			0				1				0				-1
		// wYaw:	1			1				1				1				1
		wYaw = 1;
		wPitch = 2 * (1 - sd_min(1, 2 * abs(cPx - m_pressPt.x()) / (float)viewWidth)) - 1;
		wRoll = 1 - sd_min(abs(cPx + iWidth / 4 - m_pressPt.x()), abs(cPx - iWidth / 4 - m_pressPt.x())) * 4 / (float)viewWidth;
		wRoll = m_pressPt.x() > cPx ? wRoll : -wRoll;

		float yaw = wYaw * scaleYaw * (iPosX - m_pressPt.x());
		float pitch = - wPitch * scalePitch * (iPosY - m_pressPt.y());
		float roll = wRoll * scaleRoll * (iPosY - m_pressPt.y());
#if 1
		mat3 m = mat3_id, mOrg = mat3_id, mOffset = mat3_id, invM = mat3_id;
		vec3 u(m_orgRoll * sd_to_rad, m_orgPitch * sd_to_rad, m_orgYaw * sd_to_rad);
		mOrg.set_rot_zxy(u);
		u[0] = roll * sd_to_rad; u[1] = pitch * sd_to_rad, u[2] = yaw * sd_to_rad;
		mOffset.set_rot_zxy(u);
		m = mult(mOffset, mOrg);
		m.get_rot_zxy(u);
		setting.m_fYaw = u[1] * sd_to_deg;
		setting.m_fPitch = u[2] * sd_to_deg;
		setting.m_fRoll = u[0] * sd_to_deg;
#else
		setting.m_fYaw = m_orgYaw + yaw;
		setting.m_fPitch = m_orgPitch + pitch;
		setting.m_fRoll = m_orgRoll + roll;
#endif
		m_sharedImageBuffer->getStitcher()->reStitch();
	}
}

void MCQmlCameraView::onMousePress(QPoint pos)
{
	if (!m_sharedImageBuffer)
		return;
	GlobalAnimSettings& setting = g_mainWindow->getGlobalAnimSetting();
	m_blMousePressed = true;
	m_pressPt = pos;
	m_orgYaw = setting.m_fYaw;
	m_orgPitch = setting.m_fPitch;
	m_orgRoll = setting.m_fRoll;
}

void MCQmlCameraView::onMouseRelease(QPoint pos)
{
	if (!m_sharedImageBuffer)
		return;
	m_blMousePressed = false;
}
