#include "GLSLPanoramaInteract.h"
#include "3DMath.h"
#include "define.h"
#include "OculusViewer.h"

#define HORZ_SEGS 32
#define VERT_SEGS 16

#define TEST	0

GLSLPanoramaInteract::GLSLPanoramaInteract(QObject *parent) : GLSLProgram(parent)
{
	m_projMat = mat4_id;
	m_viewMat = mat4_id;
}

GLSLPanoramaInteract::~GLSLPanoramaInteract()
{
	if (m_initialized)
	{
		if (vertices)
			delete[] vertices;
		if (texCoords)
			delete[] texCoords;
		m_gl->glDeleteRenderbuffers(1, &m_renderbuffer);
	}
}

void GLSLPanoramaInteract::initialize(int panoWidth, int panoHeight)
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

	// Create a depth buffer for our framebuffer
	
	m_gl->glGenRenderbuffers(1, &m_renderbuffer);
	m_gl->glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer);
	m_gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
		panoramaWidth, panoramaHeight);

	// load textures and create framebuffers
	m_gl->glGenFramebuffers(1, &m_fboId);
	m_gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboId);
	m_gl->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextureId, 0);
	m_gl->glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderbuffer);
	m_gl->glEnable(GL_DEPTH_TEST);
	m_gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);



	// create fbo shader
	m_program = new QOpenGLShaderProgram();
#ifdef USE_SHADER_CODE
	ADD_SHADER_FROM_CODE(m_program, "vert", "Oculus");
	ADD_SHADER_FROM_CODE(m_program, "frag", "Oculus");
#else
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "./Shaders/Oculus.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "./Shaders/Oculus.frag");
#endif
	m_program->link();
	m_vertexAttr = m_program->attributeLocation("vertex");
	m_texCoordAttr = m_program->attributeLocation("texCoord");
	m_program->bind();

	m_program->setUniformValue("texture", 0);

	m_program->release();

	m_modelViewMatrix = m_program->uniformLocation("matrix");

	// Create a unit sphere
	float PI = 3.1415927;
	vertices = new GLfloat[VERT_SEGS*HORZ_SEGS * 6 * 3];
	texCoords = new GLfloat[VERT_SEGS*HORZ_SEGS * 6 * 2];
	//texCoordsRight = new GLfloat[VERT_SEGS*HORZ_SEGS * 6 * 2];
	for (int isS = 0; isS < VERT_SEGS; isS++)
	{
		float y1 = 1.0f * isS / (float) VERT_SEGS;
		float y2 = 1.0f * (isS + 1) / (float) VERT_SEGS;
		float phi1 = (-1.0f + 2.0f * y1) * PI / 2.0f;
		float phi2 = (-1.0f + 2.0f * y2) * PI / 2.0f;	
		for (int j = 0; j < HORZ_SEGS; j++)
		{
			float x1 = 1.0f * j / HORZ_SEGS;
			float x2 = 1.0f * (j + 1) / HORZ_SEGS;

			float theta1 = (2.0f * x1 - 1.0f) * PI;
			float theta2 = (2.0f * x2 - 1.0f) * PI;

			float x11 = cosf(phi1) * sinf(theta1);
			float y11 = sinf(phi1);
			float z11 = -cosf(phi1) * cosf(theta1);

			float x12 = cosf(phi1) * sinf(theta2);
			float y12 = sinf(phi1);
			float z12 = -cosf(phi1) * cosf(theta2);

			float x21 = cosf(phi2) * sinf(theta1);
			float y21 = sinf(phi2);
			float z21 = -cosf(phi2) * cosf(theta1);

			float x22 = cosf(phi2) * sinf(theta2);
			float y22 = sinf(phi2);
			float z22 = -cosf(phi2) * cosf(theta2);

			// 11 - 12
			//    /
			// 21
			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 0 * 3 + 0] = x11;
			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 0 * 3 + 1] = y11;
			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 0 * 3 + 2] = z11;

			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 1 * 3 + 0] = x12;
			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 1 * 3 + 1] = y12;
			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 1 * 3 + 2] = z12;

			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 2 * 3 + 0] = x21;
			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 2 * 3 + 1] = y21;
			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 2 * 3 + 2] = z21;

			texCoords[(isS * HORZ_SEGS + j) * 6 * 2 + 0 * 2 + 0] = x1;
			texCoords[(isS * HORZ_SEGS + j) * 6 * 2 + 0 * 2 + 1] = y1;

			texCoords[(isS * HORZ_SEGS + j) * 6 * 2 + 1 * 2 + 0] = x2;
			texCoords[(isS * HORZ_SEGS + j) * 6 * 2 + 1 * 2 + 1] = y1;

			texCoords[(isS * HORZ_SEGS + j) * 6 * 2 + 2 * 2 + 0] = x1;
			texCoords[(isS * HORZ_SEGS + j) * 6 * 2 + 2 * 2 + 1] = y2;

			// 21 - 12
			//    /
			// 22
			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 3 * 3 + 0] = x21;
			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 3 * 3 + 1] = y21;
			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 3 * 3 + 2] = z21;

			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 4 * 3 + 0] = x12;
			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 4 * 3 + 1] = y12;
			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 4 * 3 + 2] = z12;

			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 5 * 3 + 0] = x22;
			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 5 * 3 + 1] = y22;
			vertices[(isS * HORZ_SEGS + j) * 6 * 3 + 5 * 3 + 2] = z22;

			texCoords[(isS * HORZ_SEGS + j) * 6 * 2 + 3 * 2 + 0] = x1;
			texCoords[(isS * HORZ_SEGS + j) * 6 * 2 + 3 * 2 + 1] = y2;

			texCoords[(isS * HORZ_SEGS + j) * 6 * 2 + 4 * 2 + 0] = x2;
			texCoords[(isS * HORZ_SEGS + j) * 6 * 2 + 4 * 2 + 1] = y1;

			texCoords[(isS * HORZ_SEGS + j) * 6 * 2 + 5 * 2 + 0] = x2;
			texCoords[(isS * HORZ_SEGS + j) * 6 * 2 + 5 * 2 + 1] = y2;
		}
	}
	m_initialized = true;
}

void GLSLPanoramaInteract::render(GLuint originalTextures, float yaw, float pitch, float roll)
{
	if (!m_program)
		return;
	mat3 m = mat3_id, invM = mat3_id;
	vec3 u(-roll * sd_to_rad, -pitch * sd_to_rad, yaw * sd_to_rad);
	m.set_rot_zxy(u);
	invert(invM, m);

	m_program->bind();
	
	float aspectRatio = panoramaWidth / (float)panoramaHeight;

	mat4 view = mat4_id;
	mat4 matrix = mat4_id;
#if 0
	vec3 eyePos = vec3_null;
	vec3 up = vec3_y;
	vec3 target = vec3_z;
	eyePos.z = 0;
	target = eyePos + vec3_neg_z;
	look_at(view, eyePos, target, up);
#endif
	view.set_mat3(m);
	m_viewMat = view;
	perspective(m_projMat, 60, aspectRatio, 0.001f, 2.f);
	mult(matrix, m_projMat, m_viewMat);

	QMatrix4x4 finalMat;
	float* data = finalMat.data();
	memcpy(data, (float*)matrix.mat_array, sizeof(float)* 16);

	m_program->setUniformValue(m_modelViewMatrix, finalMat);

	m_gl->glEnable(GL_DEPTH_TEST);
	//m_gl->glEnable(GL_CLIP_DISTANCE0);

	m_gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboId);

	float width = panoramaWidth;
	float height = panoramaHeight;

	m_gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	m_gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_gl->glViewport(0, 0, width, height);

	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindTexture(GL_TEXTURE_2D, originalTextures);

	m_gl->glEnableVertexAttribArray(m_vertexAttr);
	m_gl->glEnableVertexAttribArray(m_texCoordAttr);

	m_gl->glVertexAttribPointer(m_vertexAttr, 3, GL_FLOAT, GL_FALSE, 0, vertices);
	m_gl->glVertexAttribPointer(m_texCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

	m_gl->glDrawArrays(GL_TRIANGLES, 0, HORZ_SEGS * VERT_SEGS * 6);

	m_gl->glDisableVertexAttribArray(m_texCoordAttr);
	m_gl->glDisableVertexAttribArray(m_vertexAttr);

	m_gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	m_gl->glDisable(GL_DEPTH_TEST);

	m_program->release();
}

const int GLSLPanoramaInteract::getWidth()
{
	return panoramaWidth;
}

const int GLSLPanoramaInteract::getHeight()
{
	return panoramaHeight;
}