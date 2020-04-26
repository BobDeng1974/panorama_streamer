#include "GLSLLiveViewer.h"
#include "define.h"

GLSLLiveViewer::GLSLLiveViewer(QObject *parent) : QObject(parent)
, m_program(0)
, m_initialized(false)
{
}

GLSLLiveViewer::~GLSLLiveViewer()
{
	if (m_initialized)
	{
		delete m_program;
		m_program = NULL;
		m_gl->glDeleteFramebuffers(1, &m_fboId);
		m_gl->glDeleteTextures(1, &m_fboTextureId);
	}
}

void GLSLLiveViewer::setGL(QOpenGLFunctions* gl)
{
	m_gl = gl;
}

void GLSLLiveViewer::initialize(int viewCount, int viewerWidth, int viewerHeight, bool isStereo)
{
	this->viewerWidth = viewerWidth;
	this->viewerHeight = viewerHeight;
	this->m_stereo = isStereo;
	this->m_nViewCount = viewCount;

	// frame buffer
	m_gl->glGenTextures(1, &m_fboTextureId);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_fboTextureId);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, viewerWidth, viewerHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	// load textures and create framebuffers
	m_gl->glGenFramebuffers(1, &m_fboId);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
	m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextureId, 0);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// create fbo shader
	m_program = new QOpenGLShaderProgram();
#ifdef USE_SHADER_CODE
	ADD_SHADER_FROM_CODE(m_program, "vert", "stitcher");
	ADD_SHADER_FROM_CODE(m_program, "frag", "liveViewer");
#else
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "./Shaders/stitcher.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "./Shaders/liveViewer.frag");
#endif
	m_program->link();
	m_vertexAttr = m_program->attributeLocation("vertex");
	m_texCoordAttr = m_program->attributeLocation("texCoord");
	m_program->bind();

	int textureIds[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	m_gl->glUniform1iv(m_program->uniformLocation(QString("textures")), 8, textureIds);
	m_gl->glUniform1i(m_program->uniformLocation(QString("stereo")), isStereo);

	QMatrix4x4 matrix;
	matrix.ortho(-viewerWidth / 2, viewerWidth / 2, -viewerHeight / 2, viewerHeight / 2, -10.0f, 10.0f);
	matrix.translate(0, 0, -2);
	m_program->setUniformValue("matrix", matrix);
	m_program->setUniformValue("viewCnt", viewCount);

	m_program->release();

	m_initialized = true;
}


void GLSLLiveViewer::render(GLuint fbos[])
{
	m_program->bind();
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

	float width = viewerWidth;
	float height = viewerHeight;

	m_gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	m_gl->glClear(GL_COLOR_BUFFER_BIT);

	m_gl->glViewport(0, 0, width, height);

	for (int i = 0; i < m_nViewCount; i++)
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

	m_program->release();
}