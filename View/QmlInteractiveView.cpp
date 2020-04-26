#include "QmlInteractiveView.h"

#include <QSGOpaqueTextureMaterial>
#include <QSGNode>
#include <QQuickWindow>
#include <QQmlApplicationEngine>
#include <QTime>
#include "QThread.h"

#include <QApplication>
#include "Config.h"
#include <QOpenGLFunctions_3_0>
#include "QmlMainWindow.h"

#define VIDEO_WIDTH		1220.00
#define VIDEO_HEIGHT	1000.00

extern QmlMainWindow* g_mainWindow;

OculusTexture::OculusTexture(bool ownTextureID)
{
	m_ownTextureID = ownTextureID;
	m_textureId = 0;

	initializeOpenGLFunctions();

	glGenTextures(1, &m_textureId);
	glBindTexture(GL_TEXTURE_2D, m_textureId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	imageWidth = 0;
	imageHeight = 0;

}

OculusTexture::~OculusTexture() {
	if (m_ownTextureID)
		glDeleteTextures(1, &m_textureId);
}

void OculusTexture::updateTexture(int textureId, int width, int height, QImage::Format format)
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


QmlInteractiveView::QmlInteractiveView(QQuickItem* parent) : QQuickItem(parent),
m_pOculusTexture(0),
m_keepAspectRatio(true),
aspectChanged(false)
{
	setFlag(ItemHasContents, true);
	m_aspect = VIDEO_WIDTH / VIDEO_HEIGHT;
	m_oculus = NULL;
	m_blMousePressed = false;
}

QmlInteractiveView::~QmlInteractiveView()
{
	if (m_pOculusTexture){
		m_pOculusTexture->deleteLater();
	}

	if (m_interact)
	{
		m_interact->deleteLater();
		m_interact = NULL;
	}
}

QSGNode* QmlInteractiveView::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
	QSGGeometryNode* node = 0;
	QSGGeometry* geometry = 0;

	QSGNode::DirtyState dirtyState;
	
	if (m_pOculusTexture && m_sharedImageBuffer && m_oculus && m_oculus->isCreated() && m_sharedImageBuffer->getGlobalAnimSettings()->m_oculus)
	{
		GlobalAnimSettings* gasettings = m_sharedImageBuffer->getGlobalAnimSettings();
		m_aspect = ((double) gasettings->getPanoXres()) / gasettings->getPanoYres();
		m_pOculusTexture->updateTexture(m_oculus->getTextureID(), gasettings->getPanoXres(), gasettings->getPanoYres(), QImage::Format_RGB888);
	}
	else if (m_pOculusTexture && m_sharedImageBuffer && m_interact && !m_sharedImageBuffer->getGlobalAnimSettings()->m_oculus)
 	{
		GlobalAnimSettings& gasettings = g_mainWindow->getGlobalAnimSetting();
		m_aspect = ((double)gasettings.getPanoXres()) / gasettings.getPanoYres();
		m_interact->render(m_sharedImageBuffer->getStitcher().get()->getPanoramaTextureIdForInteract(), gasettings.m_fInteractYaw, gasettings.m_fInteractPitch, gasettings.m_fInteractRoll);
		m_pOculusTexture->updateTexture(m_interact->getTargetTexture(), gasettings.getPanoXres(), gasettings.getPanoYres(), QImage::Format_RGB888);
 	}
	GlobalAnimSettings* gasettings = m_sharedImageBuffer->getGlobalAnimSettings();
	if (!oldNode) {
		// Create the node.
		node = new QSGGeometryNode;
		geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
		geometry->setDrawingMode(GL_TRIANGLE_STRIP);
		node->setGeometry(geometry);
		node->setFlag(QSGNode::OwnsGeometry);
		
		m_pOculusTexture = new OculusTexture(true);
		m_pOculusTexture->setFiltering(QSGTexture::Linear);
		m_pOculusTexture->setMipmapFiltering(QSGTexture::None);
		QSGOpaqueTextureMaterial* material = new QSGOpaqueTextureMaterial; //QSGTextureMaterial  can be used for opacity support
		material->setTexture(m_pOculusTexture);
		node->setMaterial(material);
		node->setFlag(QSGNode::OwnsMaterial);

		GlobalAnimSettings* gasettings = m_sharedImageBuffer->getGlobalAnimSettings();
#if 1 /*[updated by B]*/
		m_interact = new GLSLPanoramaInteract();
		m_interact->setGL((QOpenGLFunctions*)QOpenGLContext::currentContext()->functions(), NULL);
		m_interact->initialize(gasettings->m_panoXRes, gasettings->m_panoYRes);
#endif
	}
	else {
		node = static_cast<QSGGeometryNode *>(oldNode);
		geometry = node->geometry();
	}

// 	if (m_oldGeometry != boundingRect() || aspectChanged)
	{
		aspectChanged = false;

		// Create the vertices and map to texture.
		QRectF bounds = boundingRect();
		QSGGeometry::TexturedPoint2D* vertices = geometry->vertexDataAsTexturedPoint2D();
		if (m_keepAspectRatio)
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

void QmlInteractiveView::mouseMoveEvent(QMouseEvent * event)
{
}

void QmlInteractiveView::mousePressEvent(QMouseEvent * event)
{

}

void QmlInteractiveView::mouseReleaseEvent(QMouseEvent * event)
{
}

void QmlInteractiveView::onMouseMove(QPoint pos)
{
	if (!m_sharedImageBuffer)
		return;
	//if (abs(m_pressPt.x() - pos.x()) < 4 || abs(m_pressPt.y() - pos.y()) < 4)
	//	return;
		GlobalAnimSettings& setting = g_mainWindow->getGlobalAnimSetting();
	if (m_blMousePressed && m_crossPt != vec3_null)
	{
		vec3 crossPt = getCrossPt(pos);
		if (crossPt != vec3_null)
		{
			//m_orgYaw = setting.m_fInteractYaw;
			//m_orgPitch = setting.m_fInteractPitch;
			//m_orgRoll = setting.m_fInteractRoll;

			mat3 m = mat3_id, mOrg = mat3_id, mOffset = mat3_id, invM = mat3_id;
			vec3 u(setting.m_fInteractRoll * sd_to_rad, setting.m_fInteractPitch * sd_to_rad, setting.m_fInteractYaw * sd_to_rad);
			mOrg.set_rot_zxy(u);

			sd_scalar rotAngle = 0;
			vec3 rotAxis;
			get_rot(vec3_z, m_crossPt, crossPt, rotAngle, rotAxis);

			mOffset.set_rot(rotAngle, rotAxis);

			m = mult(mOffset, mOrg);
			m.get_rot_zxy(u);
			setting.m_fInteractYaw = u[1] * sd_to_deg;
			setting.m_fInteractPitch = u[2] * sd_to_deg;
			setting.m_fInteractRoll = 0; // u[0] * sd_to_deg;

			m_crossPt = crossPt;
		}
	}

}

void QmlInteractiveView::onMousePress(QPoint pos)
{
	if (!m_sharedImageBuffer)
		return;
	GlobalAnimSettings& setting = g_mainWindow->getGlobalAnimSetting();
	m_blMousePressed = true;
	m_pressPt = pos;

	//m_orgYaw = setting.m_fInteractYaw;
	//m_orgPitch = setting.m_fInteractPitch;
	//m_orgRoll = setting.m_fInteractRoll;


	m_crossPt = getCrossPt(pos);
}

vec3 QmlInteractiveView::getCrossPt(QPoint pos)
{
	QRectF bounds = boundingRect();
	int iWidth = bounds.width();
	int iHeight = bounds.height();
	int iPosX = pos.x();
	int iPosY = pos.y();

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
	int left, top;
	left = (iWidth - viewWidth) / 2;
	top = (iHeight - viewHeight) / 2;
	mat4 mProj = m_interact->getProjMat();
	mat4 mView = m_interact->getViewMat();
	vec3 pickRayDir = vec3_neg_z, pickOrg = vec3_null;
	getPickRay(iPosX, iPosY, mat4_id, mProj, left, top, viewWidth, viewHeight, pickRayDir, pickOrg);
	pickRayDir.normalize();
	sd_scalar t = intersects(pickOrg, pickRayDir, vec3_null, 1, false);
	if (t == FLT_MAX)
	{
		return vec3_null;
	}
	vec3 crossPt = pickOrg + pickRayDir * t;
	return crossPt;
}

void QmlInteractiveView::onMouseRelease(QPoint pos)
{
	if (!m_sharedImageBuffer)
		return;
	m_blMousePressed = false;
	m_crossPt = vec3_null;
}