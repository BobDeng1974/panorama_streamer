#ifndef GLSLFEATHERING_H
#define GLSLFEATHERING_H

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLFunctions_2_0>
#include <QOpenGLTexture>

/// <summary>
/// The opengl shader that feathers the overlapping area.
/// </summary>
class GLSLFeathering : public QObject
{
	Q_OBJECT
public:
	explicit GLSLFeathering(QObject *parent = 0);
	~GLSLFeathering();

	void setGL(QOpenGLFunctions* gl, QOpenGLFunctions_2_0* functions_2_0);
	void initialize(int panoWidth, int panoHeight, int viewCount);
	void render(GLuint fbos[]);

	int getTargetTexture() { return m_fboTextureId;  }
	GLuint getBoundaryTexture(int idx) { return m_boundaryTextureIDs[idx]; }

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

	GLuint m_boundaryTextureIDs[8];
};

#endif // GLSLFEATHERING_H