#ifndef GLSLMULTIBANDBLENDING_H
#define GLSLMULTIBANDBLENDING_H

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLFunctions_2_0>
#include <QOpenGLTexture>

#include "GLSLBoundary.h"
#include "GLSLGaussianBlur.h"
#include "GLSLLaplacian.h"
#include "GLSLResize.h"
#include "GLSLBlending.h"
#include "GLSLPyramidSum.h"
#include "common.h"

/// <summary>
/// The opengl shader that feathers the overlapping area.
/// </summary>
class GLSLMultibandBlending : public QObject
{
	Q_OBJECT
public:
	explicit GLSLMultibandBlending(QObject *parent = 0);
	~GLSLMultibandBlending();

	void setGL(QOpenGLFunctions* gl, QOpenGLFunctions_2_0* functions_2_0);
	void initialize(int panoWidth, int panoHeight, int viewCount);
	void render(GLuint fboTextures[], int multibandLevel);

	virtual int getTargetTexture();
	GLuint getBoundaryTexture(int idx);

private:
	int panoramaWidth;
	int panoramaHeight;

	// Final panorama GLSL program
	QOpenGLShaderProgram *m_program;

	GLuint m_vertexAttr;
	GLuint m_texCoordAttr;

	// framebuffer index and color texture
	GLuint m_fboId;
	GLuint m_fboTextureId;

	// gl functions
	QOpenGLFunctions* m_gl;
	QOpenGLFunctions_2_0* m_functions_2_0;

	bool m_initialized;
	int m_viewCount;

	std::vector<GLSLBoundary*> m_boundaries;
	std::vector<GLSLGaussianBlur*> m_colorGaussians;
	std::vector<GLSLGaussianBlur*> m_blendMapGaussians;
	std::vector<GLSLLaplacian*> m_laplacians;
	std::vector<GLSLResize*> m_pyrScaleLevelGaussian;
	std::vector<GLSLResize*> m_pyrScaleLevelBlendMap;
	GLSLBlending* m_gaussianBlend;
	GLSLBlending* m_laplacianBlend;
	GLSLPyramidSum* m_pyramidSum;
};

#endif // GLSLMULTIBANDBLENDING_H