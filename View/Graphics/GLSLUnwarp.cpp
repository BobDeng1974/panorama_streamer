#include "GLSLUnwarp.h"
#include "QmlMainWindow.h"

extern QmlMainWindow* g_mainWindow;

GLSLUnwarp::GLSLUnwarp(QObject *parent, bool isYUV) : QObject(parent)
, m_program(0)
, m_initialized(false)
//, m_srcTexture(0)
{
}

GLSLUnwarp::~GLSLUnwarp()
{
	if (m_initialized)
	{
		//delete m_srcTexture;
		delete m_program;
		m_program = NULL;
		m_gl->glDeleteFramebuffers(1, &m_fboId);
		m_gl->glDeleteTextures(1, &m_fboTextureId);
	}
}

void GLSLUnwarp::setGL(QOpenGLFunctions* gl)
{
	m_gl = gl;
}

void GLSLUnwarp::initialize(int id,  int xres, int yres, int panoWidth, int panoHeight)
{
	camID = id;
	panoramaWidth = panoWidth;
	panoramaHeight = panoHeight;

	inputWidth = xres;
	inputHeight = yres;

	// frame buffer
	m_gl->glGenTextures(1, &m_fboTextureId);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_fboTextureId);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, panoramaWidth, panoramaHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	// load textures and create framebuffers
	m_gl->glGenFramebuffers(1, &m_fboId);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
	m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextureId, 0);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// create fbo shader
	m_program = new QOpenGLShaderProgram();
#ifdef USE_SHADER_CODE
	ADD_SHADER_FROM_CODE(m_program, "vert", "stitcher");
	ADD_SHADER_FROM_CODE(m_program, "frag", "stitcher");
#else
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "./Shaders/stitcher.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "./Shaders/stitcher.frag");
#endif
	m_program->link();

	m_program->bind();

	m_vertexAttr = m_program->attributeLocation("vertex");
	m_texCoordAttr = m_program->attributeLocation("texCoord");

	QMatrix4x4 matrix;
	matrix.ortho(-panoramaWidth / 2, panoramaWidth / 2, -panoramaHeight / 2, panoramaHeight / 2, -10.0f, 10.0f);
	matrix.translate(0, 0, -2);
	m_program->setUniformValue("matrix", matrix);
	m_program->setUniformValue("lens", camInput.m_cameraParams.m_lensType);
	m_program->setUniformValue("texture", 0);

	if (camInput.m_cameraParams.m_lensType == 0)
		m_program->setUniformValue("blendingFalloff", 0.5f);
	else if (camInput.m_cameraParams.m_lensType == 1)
		m_program->setUniformValue("blendingFalloff", 0.2f);

	// camera parameters
	float nwidth = xres;
	float nheight = yres;
	m_program->setUniformValue("imageWidth", nwidth);
	m_program->setUniformValue("imageHeight", nheight);
	m_program->setUniformValue("FoV", camInput.m_cameraParams.m_fov);
	m_program->setUniformValue("cx", nwidth / 2);
	m_program->setUniformValue("cy", nheight / 2);
	m_program->setUniformValue("offset_x", camInput.m_cameraParams.m_offset_x);
	m_program->setUniformValue("offset_y", camInput.m_cameraParams.m_offset_y);

	m_program->setUniformValue("k1", camInput.m_cameraParams.m_k1);
	m_program->setUniformValue("k2", camInput.m_cameraParams.m_k2);
	m_program->setUniformValue("k3", camInput.m_cameraParams.m_k3);

	m_program->setUniformValue("yaw", camInput.m_cameraParams.m_yaw);
	m_program->setUniformValue("pitch", camInput.m_cameraParams.m_pitch);
	m_program->setUniformValue("roll", camInput.m_cameraParams.m_roll);

	cpUnif = m_program->uniformLocation("cP");
	xrad1Unif = m_program->uniformLocation("xrad1");
	xrad2Unif = m_program->uniformLocation("xrad2");
	yrad1Unif = m_program->uniformLocation("yrad1");
	yrad2Unif = m_program->uniformLocation("yrad2");

	m_program->release();

	m_initialized = true;
}

void GLSLUnwarp::updateCameraParams()
{
	// camera parameters
	m_program->bind();

	m_program->setUniformValue("lens", camInput.m_cameraParams.m_lensType);

	int nwidth = camInput.xres;
	int nheight = camInput.yres;
	m_program->setUniformValue("imageWidth", nwidth);
	m_program->setUniformValue("imageHeight", nheight);
	m_program->setUniformValue("cx", nwidth / 2);
	m_program->setUniformValue("cy", nheight / 2);


	m_program->setUniformValue("FoV", camInput.m_cameraParams.m_fov);
	m_program->setUniformValue("offset_x", camInput.m_cameraParams.m_offset_x);
	m_program->setUniformValue("offset_y", camInput.m_cameraParams.m_offset_y);

	m_program->setUniformValue("k1", camInput.m_cameraParams.m_k1);
	m_program->setUniformValue("k2", camInput.m_cameraParams.m_k2);
	m_program->setUniformValue("k3", camInput.m_cameraParams.m_k3);

	m_program->setUniformValue("yaw", camInput.m_cameraParams.m_yaw);
	m_program->setUniformValue("pitch", camInput.m_cameraParams.m_pitch);
	m_program->setUniformValue("roll", camInput.m_cameraParams.m_roll);

	m_program->release();
}

void GLSLUnwarp::render(unsigned int rgbTextureId, int camID)
{
	m_program->bind();
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

	m_gl->glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
	m_gl->glClear(GL_COLOR_BUFFER_BIT);

	m_gl->glViewport(0, 0, panoramaWidth, panoramaHeight);
	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindTexture(GL_TEXTURE_2D, rgbTextureId);

#if 0
	QMatrix3x3 mYaw = getViewMatrix(setting.m_fYaw, 0, 0);
	QMatrix3x3 mPitch = getViewMatrix(0, setting.m_fPitch, 0);
	QMatrix3x3 mRoll = getViewMatrix(0, 0, setting.m_fRoll);
	QMatrix3x3 m2 = getViewMatrix(cam.m_yaw, cam.m_pitch, cam.m_roll);
	QMatrix3x3 m = m2 * mYaw * mPitch * mRoll;
	m_program->setUniformValue(cpUnif, m);
#else
	QMatrix3x3 m = getViewMatrix(camInput.m_cameraParams.m_yaw, camInput.m_cameraParams.m_pitch, camInput.m_cameraParams.m_roll);
	m_program->setUniformValue(cpUnif, m);
#endif

	// blending parameters
	// 8 camera set
	if (camInput.m_cameraParams.m_lensType == 1)
	{
		m_program->setUniformValue(xrad1Unif, camInput.m_cameraParams.m_xrad1);
		m_program->setUniformValue(xrad2Unif, camInput.m_cameraParams.m_xrad2);
		m_program->setUniformValue(yrad1Unif, camInput.m_cameraParams.m_yrad1);
		m_program->setUniformValue(yrad2Unif, camInput.m_cameraParams.m_yrad2);
	}
	else if (camInput.m_cameraParams.m_lensType == 0)
	{
		// 6 camera set
		m_program->setUniformValue("fisheyeLensRadiusRatio1", 1.0f);
		m_program->setUniformValue("fisheyeLensRadiusRatio2", 1.0f);
		m_program->setUniformValue("blendCurveStart", 0.4f);
	}

	GLfloat vertices[] = {
		-panoramaWidth / 2, -panoramaHeight / 2,
		-panoramaWidth / 2, panoramaHeight / 2,
		panoramaWidth / 2, panoramaHeight / 2,
		panoramaWidth / 2, -panoramaHeight / 2
	};

	GLfloat texCoords[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
	};

	int vertexAttr = m_vertexAttr;  
	int texCoordAttr = m_texCoordAttr;

	m_gl->glVertexAttribPointer(vertexAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	m_gl->glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

	m_gl->glEnableVertexAttribArray(vertexAttr);
	m_gl->glEnableVertexAttribArray(texCoordAttr);

	m_gl->glDrawArrays(GL_QUADS, 0, 4);

	m_gl->glDisableVertexAttribArray(texCoordAttr);
	m_gl->glDisableVertexAttribArray(vertexAttr);

	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_program->release();
}
