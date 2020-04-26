#include "GLSLProgram.h"

GLSLProgram::GLSLProgram(QObject *parent) : QObject(parent)
, m_program(0)
, m_initialized(false)
{
}

GLSLProgram::~GLSLProgram()
{
	if (m_initialized)
	{
		delete m_program;
		m_program = NULL;
		m_gl->glDeleteFramebuffers(1, &m_fboId);
		m_gl->glDeleteTextures(1, &m_fboTextureId);
		
		m_initialized = false;
	}
}

void GLSLProgram::setGL(QOpenGLFunctions* gl, QOpenGLFunctions_2_0* functions_2_0)
{
	m_gl = gl;
	m_functions_2_0 = functions_2_0;
}

void GLSLProgram::initialize()
{
	m_initialized = true;
}

void GLSLProgram::saveTexture(GLuint fboID, QString strSaveName)
{
	int myDataLength = getWidth() * getHeight() * 4;
	GLuint *buffer = (GLuint *)malloc(myDataLength);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	m_gl->glReadPixels(0, 0, getWidth(), getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	QImage img((uchar*)buffer, getWidth(), getHeight(), QImage::Format_RGBA8888);
	img.save(strSaveName);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float GLSLProgram::getLevelScale(int level)
{
	int scale = 1;
	for (int i = 0; i < level; i++)
		scale *= 2;
	return scale;
}