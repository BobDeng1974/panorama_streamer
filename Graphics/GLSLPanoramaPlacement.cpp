#include "GLSLPanoramaPlacement.h"
#include "3DMath.h"
#include "define.h"

GLSLPanoramaPlacement::GLSLPanoramaPlacement(QObject *parent) : GLSLProgram(parent)
{
}

GLSLPanoramaPlacement::~GLSLPanoramaPlacement()
{
}

void GLSLPanoramaPlacement::initialize(int panoWidth, int panoHeight)
{
	this->panoramaWidth = panoWidth;
	this->panoramaHeight = panoHeight;
	
	// frame buffer
	m_gl->glGenTextures(1, &m_fboTextureId);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_fboTextureId);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, panoramaWidth, panoramaHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);

	// load textures and create framebuffers
	m_gl->glGenFramebuffers(1, &m_fboId);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
	m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextureId, 0);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// create fbo shader
	m_program = new QOpenGLShaderProgram();
#ifdef USE_SHADER_CODE
	ADD_SHADER_FROM_CODE(m_program, "vert", "stitcher");
	ADD_SHADER_FROM_CODE(m_program, "frag", "placement");
#else
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "./Shaders/stitcher.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "./Shaders/placement.frag");
#endif
	m_program->link();
	m_vertexAttr = m_program->attributeLocation("vertex");
	m_texCoordAttr = m_program->attributeLocation("texCoord");
	m_program->bind();

	m_program->setUniformValue("texture", 0);

	QMatrix4x4 matrix;
	matrix.ortho(-panoramaWidth / 2, panoramaWidth / 2, -panoramaHeight / 2, panoramaHeight / 2, -10.0f, 10.0f);
	matrix.translate(0, 0, -2);
	m_program->setUniformValue("matrix", matrix);
	m_program->release();

	placeMatUnif = m_program->uniformLocation("placeMat");

	m_initialized = true;
}

void GLSLPanoramaPlacement::render(GLuint originalTextures, float yaw, float pitch, float roll)
{
	mat3 m = mat3_id, invM = mat3_id;
	vec3 u(roll * sd_to_rad, pitch * sd_to_rad, yaw * sd_to_rad);
	m.set_rot_zxy(u);
	invert(invM, m);
	m_program->bind();
	
	QMatrix3x3 matrix;
	float* data = matrix.data();
	memcpy(data, (float*)invM.mat_array, sizeof(float) * 9);
	m_program->setUniformValue(placeMatUnif, matrix);

	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

	float width = panoramaWidth;
	float height = panoramaHeight;

	m_gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	m_gl->glClear(GL_COLOR_BUFFER_BIT);

	m_gl->glViewport(0, 0, width, height);

	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindTexture(GL_TEXTURE_2D, originalTextures);

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

const int GLSLPanoramaPlacement::getWidth()
{
	return panoramaWidth;
}

const int GLSLPanoramaPlacement::getHeight()
{
	return panoramaHeight;
}