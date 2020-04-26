#ifndef GLSLPROGRAM_H
#define GLSLPROGRAM_H

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLFunctions_2_0>
#include <QOpenGLTexture>

class GLSLProgram : public QObject
{
	Q_OBJECT
public:
	explicit GLSLProgram(QObject *parent = 0);
	~GLSLProgram();

	void setGL(QOpenGLFunctions* gl, QOpenGLFunctions_2_0* functions_2_0);
	void initialize();

	virtual int getTargetTexture() { return m_fboTextureId; }
	virtual int getTargetBuffer() { return m_fboId; }

	virtual const int getWidth() = 0;
	virtual const int getHeight() = 0;

protected:
	void saveTexture(GLuint fboID, QString strSaveName);
	float getLevelScale(int level);

	// GLSL program
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
};

#endif // GLSLPROGRAM_H