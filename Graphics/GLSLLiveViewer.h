#ifndef GLSLLIVEVIEWER_H
#define GLSLLIVEVIEWER_H

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLTexture>

class GLSLLiveViewer : public QObject
{
	Q_OBJECT
public:
	explicit GLSLLiveViewer(QObject *parent = 0);
	~GLSLLiveViewer();

	void setGL(QOpenGLFunctions* gl);
	void initialize(int viewCount, int viewerWidth, int viewerHeight, bool isStereo);
	void render(GLuint fbos[]);

	int getTargetTexture() { return m_fboTextureId;  }

private:
	int viewerWidth;
	int viewerHeight;

	// undistort GLSL program
	QOpenGLShaderProgram *m_program;

	GLuint m_vertexAttr;
	GLuint m_texCoordAttr;

	// framebuffer index and color texture
	GLuint m_fboId;
	GLuint m_fboTextureId;

	// gl functions
	QOpenGLFunctions* m_gl;

	bool m_initialized;
	bool m_stereo;
	int  m_nViewCount;
};

#endif // GLSLLIVEVIEWER_H