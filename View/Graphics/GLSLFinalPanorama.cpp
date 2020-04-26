#include "GLSLFinalPanorama.h"
#include "define.h"

GLSLFinalPanorama::GLSLFinalPanorama(QObject *parent) : GLSLProgram(parent)
{
}

GLSLFinalPanorama::~GLSLFinalPanorama()
{
	if (m_initialized)
	{
		for (int i = 0; i < 2; i++)
			delete m_pbos[i];
	}
}

void GLSLFinalPanorama::initialize(int panoWidth, int panoHeight, bool isStereo)
{
	this->m_stereo = isStereo;
	this->panoramaViewWidth = panoWidth;
	if (m_stereo)
	{
		this->panoramaViewHeight = panoHeight * 2;
	}
	else
	{
		this->panoramaViewHeight = panoHeight;
	}

	// frame buffer
	m_gl->glGenTextures(2, m_fboTextureIds);
	m_gl->glGenFramebuffers(2, m_fboIds);
	for (int i = 0; i < 2; i++)
	{
		m_gl->glBindTexture(GL_TEXTURE_2D, m_fboTextureIds[i]);
		m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, panoramaViewWidth, panoramaViewHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);

		// load textures and create framebuffers
		m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboIds[i]);
		m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextureIds[i], 0);
	}
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_workingTexture = 0;

	// create fbo shader
	m_program = new QOpenGLShaderProgram();
#ifdef USE_SHADER_CODE
	ADD_SHADER_FROM_CODE(m_program, "vert", "stitcher");
	ADD_SHADER_FROM_CODE(m_program, "frag", "finalPanoView");
#else
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "./Shaders/stitcher.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "./Shaders/finalPanoView.frag");
#endif
	m_program->link();
	m_vertexAttr = m_program->attributeLocation("vertex");
	m_texCoordAttr = m_program->attributeLocation("texCoord");
	m_program->bind();

	int textureIds[2] = { 0, 1 };
	m_gl->glUniform1iv(m_program->uniformLocation(QString("textures")), 2, textureIds);
	m_gl->glUniform1i(m_program->uniformLocation(QString("isStereo")), isStereo);

	for (int i = 0; i < 2; i++)
	{
		QOpenGLBuffer* pbo = new QOpenGLBuffer(QOpenGLBuffer::PixelPackBuffer);
		pbo->create();
		pbo->setUsagePattern(QOpenGLBuffer::UsagePattern::StreamRead);
		pbo->bind();
		pbo->allocate(panoramaViewWidth * panoramaViewHeight * 3);
		pbo->release();
		m_pbos[i] = pbo;
	}
	m_pboIndex = 0;

	QMatrix4x4 matrix;
	matrix.ortho(-panoramaViewWidth / 2, panoramaViewWidth / 2, -panoramaViewHeight / 2, panoramaViewHeight / 2, -10.0f, 10.0f);
	matrix.translate(0, 0, -2);
	m_program->setUniformValue("matrix", matrix);
	m_program->release();

	GLSLProgram::initialize();
}


void GLSLFinalPanorama::render(GLuint fbos[])
{
	m_program->bind();
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboIds[m_workingTexture]);

	float width = panoramaViewWidth;
	float height = panoramaViewHeight;

	m_gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	m_gl->glClear(GL_COLOR_BUFFER_BIT);

	m_gl->glViewport(0, 0, width, height);

	int panoCount = m_stereo ? 2 : 1;
	for (int i = 0; i < panoCount; i++)
	{
		m_gl->glActiveTexture(GL_TEXTURE0 + i);
		m_gl->glBindTexture(GL_TEXTURE_2D, fbos[i]);
	}

	GLfloat vertices[] = {
		-width / 2, -height / 2,
		-width / 2, height / 2,
		width / 2, height / 2,
		width / 2, -height / 2
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

	m_gl->glFinish();

	m_workingTexture = 1 - m_workingTexture;

	m_program->release();
}

void GLSLFinalPanorama::getRGBBuffer(unsigned char* rgbBuffer)
{
	if (m_initialized) {
		m_program->bind();
		m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboIds[1 - m_workingTexture]);

		m_pboIndex = (m_pboIndex + 1) % 2;
		QOpenGLBuffer * pboCurrent = m_pbos[m_pboIndex];
		QOpenGLBuffer * pboNext = m_pbos[(m_pboIndex + 1) % 2];

		pboCurrent->bind();
		m_functions_2_0->glReadPixels(0, 0, panoramaViewWidth, panoramaViewHeight, GL_BGR, GL_UNSIGNED_BYTE, 0);
		pboCurrent->release();

		pboNext->bind();
		GLubyte* ptr = (GLubyte*)pboNext->map(QOpenGLBuffer::ReadOnly);
		if (ptr)
		{
			memcpy(rgbBuffer, ptr, panoramaViewWidth * panoramaViewHeight * 3);
			pboNext->unmap();
		}
		pboNext->release();

		m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_program->release();
	}
}

const int GLSLFinalPanorama::getWidth()
{
	return panoramaViewWidth;
}

const int GLSLFinalPanorama::getHeight()
{
	return panoramaViewHeight;
}
