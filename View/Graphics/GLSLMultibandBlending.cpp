#include "GLSLMultibandBlending.h"


GLSLMultibandBlending::GLSLMultibandBlending(QObject *parent) : QObject(parent)
, m_program(0)
, m_initialized(false)
, m_laplacianBlend(NULL)
, m_gaussianBlend(NULL)
, m_pyramidSum(NULL)
{
}

GLSLMultibandBlending::~GLSLMultibandBlending()
{
	if (m_initialized)
	{
		for (int i = 0; i < m_boundaries.size(); i++)
			delete m_boundaries[i];
		m_boundaries.clear();

		for (int i = 0; i < m_colorGaussians.size(); i++)
			delete m_colorGaussians[i];
		m_colorGaussians.clear();

		for (int i = 0; i < m_blendMapGaussians.size(); i++)
			delete m_blendMapGaussians[i];
		m_blendMapGaussians.clear();

		for (int i = 0; i < m_laplacians.size(); i++)
			delete m_laplacians[i];
		m_laplacians.clear();

		for (int i = 0; i < m_pyrScaleLevelGaussian.size(); i++)
			delete m_pyrScaleLevelGaussian[i];
		m_pyrScaleLevelGaussian.clear();

		for (int i = 0; i < m_pyrScaleLevelBlendMap.size(); i++)
			delete m_pyrScaleLevelBlendMap[i];
		m_pyrScaleLevelBlendMap.clear();

		if (m_laplacianBlend)
		{
			delete m_laplacianBlend;
			m_laplacianBlend = NULL;
		}
		if (m_gaussianBlend)
		{
			delete m_gaussianBlend;
			m_gaussianBlend = NULL;
		}
		if (m_pyramidSum)
		{
			delete m_pyramidSum;
			m_pyramidSum = NULL;
		}
	}
}

void GLSLMultibandBlending::setGL(QOpenGLFunctions* gl, QOpenGLFunctions_2_0* functions_2_0)
{
	m_gl = gl;
	m_functions_2_0 = functions_2_0;
}

void GLSLMultibandBlending::initialize(int panoWidth, int panoHeight, int viewCount)
{
	this->panoramaWidth = panoWidth;
	this->panoramaHeight = panoHeight;
	this->m_viewCount = viewCount;
	
	for (int i = 0; i < m_viewCount; i++)
	{
		GLSLBoundary *pBoundary = new GLSLBoundary();
		pBoundary->setGL(m_gl, m_functions_2_0);
		pBoundary->initialize(panoWidth, panoHeight, viewCount, i);
		m_boundaries.push_back(pBoundary);
	}

	for (int i = 0; i < m_viewCount; i++)
	{
		GLSLGaussianBlur *pPartialGaussian = new GLSLGaussianBlur();
		pPartialGaussian->setGL(m_gl, m_functions_2_0);
		pPartialGaussian->initialize(panoWidth, panoHeight, true);
		m_colorGaussians.push_back(pPartialGaussian);
	}

	for (int i = 0; i < m_viewCount; i++)
	{
		GLSLGaussianBlur *pPartialGaussian = new GLSLGaussianBlur();
		pPartialGaussian->setGL(m_gl, m_functions_2_0);
		pPartialGaussian->initialize(panoWidth, panoHeight, false);
		m_blendMapGaussians.push_back(pPartialGaussian);
	}

	for (int i = 0; i < m_viewCount; i++)
	{
		GLSLLaplacian *pLaplacian = new GLSLLaplacian();
		pLaplacian->setGL(m_gl, m_functions_2_0);
		pLaplacian->initialize(panoWidth, panoHeight);
		m_laplacians.push_back(pLaplacian);
	}

	for (int i = 0; i < m_viewCount; i++)
	{
		GLSLResize *pResize = new GLSLResize();
		pResize->setGL(m_gl, m_functions_2_0);
		pResize->initialize(panoWidth, panoHeight);
		m_pyrScaleLevelGaussian.push_back(pResize);
	}
	for (int i = 0; i < m_viewCount; i++)
	{
		GLSLResize *pResize = new GLSLResize();
		pResize->setGL(m_gl, m_functions_2_0);
		pResize->initialize(panoWidth, panoHeight);
		m_pyrScaleLevelBlendMap.push_back(pResize);
	}

	m_laplacianBlend = new GLSLBlending();
	m_laplacianBlend->setGL(m_gl, m_functions_2_0);
	m_laplacianBlend->initialize(panoWidth, panoHeight, viewCount);
	m_gaussianBlend = new GLSLBlending();
	m_gaussianBlend->setGL(m_gl, m_functions_2_0);
	m_gaussianBlend->initialize(panoWidth, panoHeight, viewCount);
	m_pyramidSum = new GLSLPyramidSum();
	m_pyramidSum->setGL(m_gl, m_functions_2_0);
	m_pyramidSum->initialize(panoWidth, panoHeight);

	m_initialized = true;
}


void GLSLMultibandBlending::render(GLuint fboTextures[], int multibandLevel)
{
	GLuint boundaryTextures[8];
	for (int i = 0; i < m_viewCount; i++)
	{
		m_boundaries[i]->render(fboTextures);
		boundaryTextures[i] = m_boundaries[i]->getTargetTexture();
	}

	m_pyramidSum->clear();
	GLuint gaussianTextures[8];
	GLuint laplacianTextures[8];
	GLuint nextGaussianTextures[8];
	GLuint weightTextures[8];
	GLuint nextWeightTextures[8];
	for (int i = 0; i < m_viewCount; i++)
	{
		gaussianTextures[i] = m_colorGaussians[i]->getTargetTexture();
		laplacianTextures[i] = m_laplacians[i]->getTargetTexture();
		weightTextures[i] = m_blendMapGaussians[i]->getTargetTexture();
		nextGaussianTextures[i] = m_pyrScaleLevelGaussian[i]->getTargetTexture();
		nextWeightTextures[i] = m_pyrScaleLevelBlendMap[i]->getTargetTexture();
	}
	for (int lev = 0; lev <= multibandLevel; lev++)
	{
		GLuint * levelSources;
		GLuint * alphaSources;
		if (lev == 0)
		{
			levelSources = fboTextures;
			alphaSources = boundaryTextures;
		}
		else
		{
			levelSources = nextGaussianTextures;
			alphaSources = nextWeightTextures;
		}

		for (int i = 0; i < m_viewCount; i++)
		{
			m_colorGaussians[i]->render(levelSources[i]);
			m_laplacians[i]->render(levelSources[i], gaussianTextures[i]);
		}
		m_laplacianBlend->render(laplacianTextures, alphaSources, lev, lev);
		//if (lev == 0)
		m_pyramidSum->render(m_laplacianBlend->getTargetTexture());
		if (lev < multibandLevel)
		{
			for (int i = 0; i < m_viewCount; i++)
			{
				m_pyrScaleLevelGaussian[i]->render(gaussianTextures[i]);
				m_blendMapGaussians[i]->render(alphaSources[i]);
				m_pyrScaleLevelBlendMap[i]->render(weightTextures[i]);
			}
		}
	}
	m_gaussianBlend->render(gaussianTextures, fboTextures, multibandLevel, 0); // fboTextures as alphaTextures
	m_pyramidSum->render(m_gaussianBlend->getTargetTexture());
}

int GLSLMultibandBlending::getTargetTexture()
{
	return m_pyramidSum->getTargetTexture();
}

GLuint GLSLMultibandBlending::getBoundaryTexture(int idx)
{
	return m_boundaries[idx]->getTargetTexture();
}