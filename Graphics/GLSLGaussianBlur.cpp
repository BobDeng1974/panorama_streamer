#include "GLSLGaussianBlur.h"

bool GLSLGaussianBlur::isGaussianRadiusInitialized = false;
int GLSLGaussianBlur::gaussianRadius[3];

GLSLGaussianBlur::GLSLGaussianBlur(QObject *parent) : GLSLProgram(parent)
, m_horizontalBoxBlur(NULL)
, m_verticalBoxBlur(NULL)
{
}

GLSLGaussianBlur::~GLSLGaussianBlur()
{
	if (m_horizontalBoxBlur)
	{
		delete m_horizontalBoxBlur;
		m_horizontalBoxBlur = NULL;
	}
	if (m_verticalBoxBlur)
	{
		delete m_verticalBoxBlur;
		m_verticalBoxBlur = NULL;
	}
}

void GLSLGaussianBlur::initialize(int panoWidth, int panoHeight, bool isPartial)
{
	this->panoramaWidth = panoWidth;
	this->panoramaHeight = panoHeight;
	
	m_horizontalBoxBlur = new GLSLBoxBlur();
	m_horizontalBoxBlur->setGL(m_gl, m_functions_2_0);
	m_horizontalBoxBlur->initialize(panoramaWidth, panoramaHeight, false, isPartial);
	m_verticalBoxBlur = new GLSLBoxBlur();
	m_verticalBoxBlur->setGL(m_gl, m_functions_2_0);
	m_verticalBoxBlur->initialize(panoramaWidth, panoramaHeight, true, isPartial);

	GLSLGaussianBlur::getGaussianRadius();

	GLSLProgram::initialize();
}


void GLSLGaussianBlur::render(GLuint textureId)
{
	GLuint src = textureId;
	GLuint dst;
	dst = boxBlur(src, (gaussianRadius[0]-1) / 2);
	dst = boxBlur(dst, (gaussianRadius[1]-1) / 2);
	dst = boxBlur(dst, (gaussianRadius[2]-1) / 2);
}

GLuint GLSLGaussianBlur::boxBlur(GLuint src, float radius)
{
	m_horizontalBoxBlur->render(src, radius);
	m_verticalBoxBlur->render(m_horizontalBoxBlur->getTargetTexture(), radius);
	return m_verticalBoxBlur->getTargetTexture();
}

int GLSLGaussianBlur::getTargetTexture()
{
	if (m_verticalBoxBlur == NULL)
	{
		return -1;
	}
	return m_verticalBoxBlur->getTargetTexture();
}

const int GLSLGaussianBlur::getWidth()
{
	return panoramaWidth;
}

const int GLSLGaussianBlur::getHeight()
{
	return panoramaHeight;
}

void GLSLGaussianBlur::getGaussianRadius()
{
	if (isGaussianRadiusInitialized)
		return;
	isGaussianRadiusInitialized = true;

	int n = 3;
	float maxSize = 20;
	float sigma = (maxSize / 2.0 - 1.0) * 0.3 + 0.8;
	float wIdeal = sqrt((12 * sigma*sigma / n) + 1);  // Ideal averaging filter width 
	int wl = floor(wIdeal);
	if (wl % 2 == 0)
		wl--;
	float wu = wl + 2;

	float mIdeal = (12 * sigma*sigma - n*wl*wl - 4 * n*wl - 3 * n) / (-4 * wl - 4);
	//float m = round(mIdeal);

	for (int i = 0; i < n; i++)
		gaussianRadius[i] = (i < mIdeal ? wl : wu);
}