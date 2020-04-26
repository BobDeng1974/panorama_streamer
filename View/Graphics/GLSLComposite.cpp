#include "GLSLComposite.h"
#include <Dense>
#include "common.h"
#include "QmlMainWindow.h"

using namespace Eigen;

#define DEBUG_SHADER	0

GLSLComposite::GLSLComposite(QObject *parent) : QObject(parent)
, m_initialized(false)
, m_feathering(NULL)
, m_multibandBlending(NULL)
{
	m_blendingMode = GlobalAnimSettings::MultiBandBlending;
	m_multiBandLevel = 0;
	m_showSeamIdx = -1;
}

GLSLComposite::~GLSLComposite()
{
	if (m_initialized)
	{
		if (m_feathering)
		{
			delete m_feathering;
			m_feathering = NULL;
		}

		if (m_multibandBlending)
		{
			delete m_multibandBlending;
			m_multibandBlending = NULL;
		}

		if (m_maskProgram)
		{
			delete m_maskProgram;
			m_maskProgram = NULL;
			m_gl->glDeleteFramebuffers(1, &m_fboMaskId);
			m_gl->glDeleteTextures(1, &m_fboTextureMaskId);
		}
		if (m_seamProgram)
		{
			delete m_seamProgram;
			m_seamProgram = NULL;
			m_gl->glDeleteFramebuffers(1, &m_fboSeamId);
			m_gl->glDeleteTextures(1, &m_fboTextureSeamId);
		}
		if (m_regionProgram)
		{
			delete m_regionProgram;
			m_regionProgram = NULL;
			m_gl->glDeleteFramebuffers(1, &m_fboRegionId);
			m_gl->glDeleteTextures(1, &m_fboTextureRegionId);
		}
		if (m_billProgram)
		{
			delete m_billProgram;
			m_billProgram = NULL;
			m_gl->glDeleteFramebuffers(1, &m_fboBillId);
			m_gl->glDeleteTextures(1, &m_fboTextureBillId);
		}
	}
}

void GLSLComposite::setGL(QOpenGLFunctions* gl, QOpenGLFunctions_2_0* functions_2_0)
{
	m_gl = gl;
	m_functions_2_0 = functions_2_0;
}

void GLSLComposite::initialize(int nViewCount, int panoWidth, int panoHeight)
{
	m_viewCount = nViewCount;
	m_panoramaWidth = panoWidth;
	m_panoramaHeight = panoHeight;
	g_panoramaWidth = panoWidth;
	g_panoramaHeight = panoHeight;
	
	m_feathering = new GLSLFeathering();
	m_feathering->setGL(m_gl, m_functions_2_0);
	m_feathering->initialize(m_panoramaWidth, m_panoramaHeight, m_viewCount);

	m_multibandBlending = new GLSLMultibandBlending();
	m_multibandBlending->setGL(m_gl, m_functions_2_0);
	m_multibandBlending->initialize(m_panoramaWidth, m_panoramaHeight, m_viewCount);

	////////////////////////////////////////////////////////
	//initialize camera region shader
	// frame buffer
	m_gl->glGenTextures(1, &m_fboTextureRegionId);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_fboTextureRegionId);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_panoramaWidth, m_panoramaHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);

	// load textures and create framebuffers
	m_gl->glGenFramebuffers(1, &m_fboRegionId);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboRegionId);
	m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextureRegionId, 0);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// create fbo shader
	m_regionProgram = new QOpenGLShaderProgram();
#ifdef USE_SHADER_CODE
	ADD_SHADER_FROM_CODE(m_regionProgram, "vert", "stitcher");
	ADD_SHADER_FROM_CODE(m_regionProgram, "frag", "cameraRegion");
#else
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "./Shaders/stitcher.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "./Shaders/seam.frag");
#endif
	m_regionProgram->link();
	m_vertexAttr = m_regionProgram->attributeLocation("vertex");
	m_texCoordAttr = m_regionProgram->attributeLocation("texCoord");
	m_regionProgram->bind();

	int textureIds[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	m_gl->glUniform1iv(m_regionProgram->uniformLocation(QString("textures")), 8, textureIds);

	QMatrix4x4 matrix;
	matrix.ortho(-m_panoramaWidth / 2, m_panoramaWidth / 2, -m_panoramaHeight / 2, m_panoramaHeight / 2, -10.0f, 10.0f);
	matrix.translate(0, 0, -2);
	m_regionProgram->setUniformValue("matrix", matrix);
	m_regionProgram->setUniformValue("viewCnt", nViewCount);	
	m_regionProgram->release();

	////////////////////////////////////////////////////////
	//initialize mask shader
	// frame buffer
	m_gl->glGenTextures(1, &m_fboTextureMaskId);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_fboTextureMaskId);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_panoramaWidth, m_panoramaHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);

	// load textures and create framebuffers
	m_gl->glGenFramebuffers(1, &m_fboMaskId);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboMaskId);
	m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextureMaskId, 0);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// create fbo shader
	m_maskProgram = new QOpenGLShaderProgram();
#ifdef USE_SHADER_CODE
	ADD_SHADER_FROM_CODE(m_maskProgram, "vert", "stitcher");
	ADD_SHADER_FROM_CODE(m_maskProgram, "frag", "mask");
#else
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "./Shaders/stitcher.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "./Shaders/seam.frag");
#endif
	m_maskProgram->link();
	m_vertexAttr = m_maskProgram->attributeLocation("vertex");
	m_texCoordAttr = m_maskProgram->attributeLocation("texCoord");
	m_maskProgram->bind();
	
	m_maskProgram->setUniformValue("texture", 0);
	m_maskProgram->setUniformValue("width", m_panoramaWidth);
	m_maskProgram->setUniformValue("height", m_panoramaHeight);
	m_maskProgram->setUniformValue("matrix", matrix);
	m_maskProgram->release();

	////////////////////////////////////////////////////////
	// initialize seam shader
	// frame buffer
	m_gl->glGenTextures(1, &m_fboTextureSeamId);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_fboTextureSeamId);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_panoramaWidth, m_panoramaHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);

	// load textures and create framebuffers
	m_gl->glGenFramebuffers(1, &m_fboSeamId);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboSeamId);
	m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextureSeamId, 0);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// create fbo shader
	m_seamProgram = new QOpenGLShaderProgram();
#ifdef USE_SHADER_CODE
	ADD_SHADER_FROM_CODE(m_seamProgram, "vert", "stitcher");
	ADD_SHADER_FROM_CODE(m_seamProgram, "frag", "seam");
#else
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "./Shaders/stitcher.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "./Shaders/seam.frag");
#endif
	m_seamProgram->link();
	m_vertexAttr = m_seamProgram->attributeLocation("vertex");
	m_texCoordAttr = m_seamProgram->attributeLocation("texCoord");
	m_seamProgram->bind();

	m_seamProgram->setUniformValue("texture", 0);
	m_seamProgram->setUniformValue("mask", 1);
	m_seamProgram->setUniformValue("matrix", matrix);
	m_seamProgram->release();

	////////////////////////////////////////////////////////
	//initialize mask shader
	// frame buffer
	m_gl->glGenTextures(1, &m_fboTextureBillId);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_fboTextureBillId);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_panoramaWidth, m_panoramaHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);

	// load textures and create framebuffers
	m_gl->glGenFramebuffers(1, &m_fboBillId);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboBillId);
	m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextureBillId, 0);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// create fbo shader
	m_billProgram = new QOpenGLShaderProgram();
#ifdef USE_SHADER_CODE
	ADD_SHADER_FROM_CODE(m_billProgram, "vert", "stitcher");
	ADD_SHADER_FROM_CODE(m_billProgram, "frag", "texture");
#else
	m_billProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "./Shaders/stitcher.vert");
	m_billProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "./Shaders/texture.frag");
#endif
	m_billProgram->link();
	m_billProgram->bind();

	m_billProgram->setUniformValue("texture", 0);
	m_billProgram->setUniformValue("matrix", matrix);
	m_billProgram->release();

	///
	m_initialized = true;
}

// render composited image from frame buffer
void GLSLComposite::render(GLuint textures[])
{
	if (m_blendingMode == GlobalAnimSettings::Feathering) // feathering blending
	{
		m_feathering->render(textures);
	}
	else if (m_blendingMode = GlobalAnimSettings::MultiBandBlending)
	{
		m_multibandBlending->render(textures, m_multiBandLevel);
	}
}

void GLSLComposite::saveTexture(GLuint fboID, QString strSaveName)
{
	int myDataLength = getPanoramaWidth() * getPanoramaHeight() * 4;
	GLuint *buffer = (GLuint *)malloc(myDataLength);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	m_gl->glReadPixels(0, 0, getPanoramaWidth(), getPanoramaHeight(), GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	QImage img((uchar*)buffer, getPanoramaWidth(), getPanoramaHeight(), QImage::Format_RGBA8888);
	img.save(strSaveName);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int GLSLComposite::getBlendTexture()
{
	GLuint textureID;
	if (m_blendingMode == GlobalAnimSettings::Feathering)
	{
		textureID = m_feathering->getTargetTexture();
	}
	else
	{
		textureID = m_multibandBlending->getTargetTexture();
	}
	return textureID;
}

int GLSLComposite::getTargetTexture(bool useBill)
{
	GLuint textureID = useBill ? m_fboTextureBillId : getBlendTexture();

	if (m_showSeamIdx < 0)
	{
		return textureID;
	}
	else
	{
		getCameraRegionImage();
		getSeamMaskImage();
		getConfirmImage(textureID);
		return m_fboTextureSeamId;
	}
}

float getMean(VectorXf gains)
{
	float means = 0.0f;
	for (int i = 0; i < gains.rows(); i++)
	{
		means += gains(i, 0);
	}
	if (gains.rows() != 0)
		means /= gains.rows();
	return means;
}

std::vector<float> GLSLComposite::getExposureData(GLuint fbos[], int viewCnt)
{
	INTERSECT_DATA ** intersect_datas = new INTERSECT_DATA*[viewCnt];
	for (int i = 0; i < viewCnt; i++)
	{
		intersect_datas[i] = new INTERSECT_DATA[viewCnt];
	}
	
	// since datas[i,j].meanSrc1Intensity == datas[j,i].meanSrc2Intensity, and intersectPixelCnt is symmetric, we can speed up this calculation
	for (int i = 0; i < viewCnt; i++) {
		std::vector<INTERSECT_DATA> datas;
		intersect_datas[i][i].intersectPixelCnt = 0;
		intersect_datas[i][i].meanSrc1Intensity = 0;
		intersect_datas[i][i].meanSrc2Intensity = 0;
		for (int j = i + 1; j < viewCnt; j++) {
			intersect_datas[i][j] = getInterSectData(fbos[i], fbos[j]);
			intersect_datas[j][i].intersectPixelCnt = intersect_datas[i][j].intersectPixelCnt;
			intersect_datas[j][i].meanSrc1Intensity = intersect_datas[i][j].meanSrc2Intensity;
			intersect_datas[j][i].meanSrc2Intensity = intersect_datas[i][j].meanSrc1Intensity;
		}
	}

	std::vector<std::vector<float>> gain_a;
	std::vector<float> gain_b;
	float sigma_n = 10.0;
	float sigma_g = 0.1;
	for (int k = 0; k < viewCnt; k++) {
		std::vector<float> a_datas;
		for (int j = 0; j < viewCnt; j++) {
			float a = 0.0f;
			if (k == j) {
				for (int i = 0; i < viewCnt; i++) {
					if (i != k) {
						a += 2.0f / (sigma_n*sigma_n)
							* intersect_datas[i][k].intersectPixelCnt
							* (intersect_datas[k][i].meanSrc1Intensity*intersect_datas[k][i].meanSrc1Intensity);
					}
					a += intersect_datas[k][i].intersectPixelCnt / (sigma_g * sigma_g);
				}
			}
			else {
				a = -2 * intersect_datas[j][k].intersectPixelCnt / (sigma_n*sigma_n)
					* (intersect_datas[j][k].meanSrc1Intensity * intersect_datas[k][j].meanSrc1Intensity);
			}
			a_datas.push_back(a);
		}

		float b = 0;
		for (int i = 0; i < viewCnt; i++)
		{
			b += intersect_datas[k][i].intersectPixelCnt / (sigma_g*sigma_g);
		}
		gain_b.push_back(b);
		gain_a.push_back(a_datas);
	}

	for (int i = 0; i < viewCnt; i++)
		delete[] intersect_datas[i];
	delete[] intersect_datas;

	std::vector<float> gains;
	if (viewCnt > 0) {
		MatrixXf A(viewCnt, viewCnt);
		VectorXf b = VectorXf(viewCnt);
		for (int i = 0; i < viewCnt; i++) {
			for (int j = 0; j < viewCnt; j++) {
				A(i, j) = gain_a[i][j];
			}	
			b(i, 0) = gain_b[i];
		}
		VectorXf vGains = A.jacobiSvd(ComputeThinU | ComputeThinV).solve(b);
		float meanGain = getMean(vGains);
		for (int i = 0; i < m_viewCount; i++)
		{
			gains.push_back(vGains(i, 0) / meanGain);
		}
	}
	return gains;
}

INTERSECT_DATA GLSLComposite::getInterSectData(GLuint src1ID, GLuint src2ID)
{
	INTERSECT_DATA intersectData;

	int myDataLength = m_panoramaWidth * m_panoramaHeight * 4;
	uchar *buffer1 = (uchar *)malloc(myDataLength);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, src1ID);
	m_gl->glReadPixels(0, 0, m_panoramaWidth, m_panoramaHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer1);

	uchar *buffer2 = (uchar *)malloc(myDataLength);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, src2ID);
	m_gl->glReadPixels(0, 0, m_panoramaWidth, m_panoramaHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer2);

	int intersectCnt = 0;
	float meanSrc1 = 0.0, meanSrc2 = 0.0;
	for (int i = 0; i < m_panoramaWidth; i++) {
		for (int j = 0; j < m_panoramaHeight; j++) {
			int idx = j * m_panoramaWidth * 4 + i * 4;
			if (buffer1[idx + 3] != 0 && buffer2[idx + 3] != 0) {
				meanSrc1 += 0.299 * (float)buffer1[idx] + 0.587 * (float)buffer1[idx + 1] + 0.114 * (float)buffer1[idx + 2];
				meanSrc2 += 0.299 * (float)buffer2[idx] + 0.587 * (float)buffer2[idx + 1] + 0.114 * (float)buffer2[idx + 2];
				intersectCnt++;
			}
		}
	}

	if (intersectCnt > 0) {
		intersectData.intersectPixelCnt = intersectCnt;
		intersectData.meanSrc1Intensity = meanSrc1 / intersectCnt;
		intersectData.meanSrc2Intensity = meanSrc2 / intersectCnt;
	}
	else {
		intersectData.intersectPixelCnt = 0;
		intersectData.meanSrc1Intensity = 0;
		intersectData.meanSrc2Intensity = 0;
	}
	free(buffer1);
	free(buffer2);
	return intersectData;
}

void GLSLComposite::getCameraRegionImage()
{
	m_regionProgram->bind();
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboRegionId);

	float width = m_panoramaWidth;
	float height = m_panoramaHeight;

	m_gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	m_gl->glClear(GL_COLOR_BUFFER_BIT);

	m_gl->glViewport(0, 0, width, height);

	for (int i = 0; i < m_viewCount; i++)
	{
		m_gl->glActiveTexture(GL_TEXTURE0 + i);
		m_gl->glBindTexture(GL_TEXTURE_2D, m_multibandBlending->getBoundaryTexture(i));
	}

	//GlobalAnimSettings& setting = g_mainWindow->getGlobalAnimSetting();
	m_regionProgram->setUniformValue("viewIdx", m_showSeamIdx);

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
	m_regionProgram->release();
}

void GLSLComposite::getSeamMaskImage()
{
	m_maskProgram->bind();
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboMaskId);

	float width = m_panoramaWidth;
	float height = m_panoramaHeight;

	m_gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	m_gl->glClear(GL_COLOR_BUFFER_BIT);
	m_gl->glViewport(0, 0, width, height);

	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_fboTextureRegionId);

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
	m_maskProgram->release();
}

void GLSLComposite::getConfirmImage(GLuint fboTextureID)
{
	m_seamProgram->bind();
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboSeamId);

	float width = m_panoramaWidth;
	float height = m_panoramaHeight;

	m_gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	m_gl->glClear(GL_COLOR_BUFFER_BIT);
	m_gl->glViewport(0, 0, width, height);

	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindTexture(GL_TEXTURE_2D, fboTextureID);
	m_gl->glActiveTexture(GL_TEXTURE0 + 1);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_fboTextureMaskId);

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
	m_seamProgram->release();
}

void GLSLComposite::mixBill(GLuint bill, vec2 billPos[])
{
	m_billProgram->bind();
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboBillId);

	float width = m_panoramaWidth;
	float height = m_panoramaHeight;

	m_gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	m_gl->glClear(GL_COLOR_BUFFER_BIT);
	m_gl->glViewport(0, 0, width, height);

	{	// background
		m_gl->glActiveTexture(GL_TEXTURE0);
		GLuint blendTextureID = getBlendTexture();
		m_gl->glBindTexture(GL_TEXTURE_2D, blendTextureID);

		width = m_panoramaWidth;
		height = m_panoramaHeight;

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

		int vertexAttr = m_billProgram->attributeLocation("vertex");
		int texCoordAttr = m_billProgram->attributeLocation("texCoord");
		m_gl->glVertexAttribPointer(vertexAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		m_gl->glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

		m_gl->glEnableVertexAttribArray(vertexAttr);
		m_gl->glEnableVertexAttribArray(texCoordAttr);

		m_gl->glDrawArrays(GL_QUADS, 0, 4);

		m_gl->glDisableVertexAttribArray(texCoordAttr);
		m_gl->glDisableVertexAttribArray(vertexAttr);
	}
	{	// bill
		m_gl->glActiveTexture(GL_TEXTURE0);
		m_gl->glBindTexture(GL_TEXTURE_2D, bill);

		GLfloat vertices[] = {
			width / 2 * billPos[0].x, height / 2 * billPos[0].y,
			width / 2 * billPos[1].x, height / 2 * billPos[1].y,
			width / 2 * billPos[2].x, height / 2 * billPos[2].y,
			width / 2 * billPos[3].x, height / 2 * billPos[3].y,
		};
		GLfloat texCoords[] = {
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
		};

		int vertexAttr = m_billProgram->attributeLocation("vertex");
		int texCoordAttr = m_billProgram->attributeLocation("texCoord");
		m_gl->glVertexAttribPointer(vertexAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		m_gl->glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

		m_gl->glEnableVertexAttribArray(vertexAttr);
		m_gl->glEnableVertexAttribArray(texCoordAttr);

		m_gl->glDrawArrays(GL_QUADS, 0, 4);

		m_gl->glDisableVertexAttribArray(texCoordAttr);
		m_gl->glDisableVertexAttribArray(vertexAttr);
	}

	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_billProgram->release();
}