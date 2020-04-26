#ifndef GLSLBLENDING_H
#define GLSLBLENDING_H

#include "GLSLProgram.h"

/// <summary>
/// The opengl shader that blends all color views with blend map
/// </summary>
class GLSLBlending : public GLSLProgram
{
	Q_OBJECT
public:
	explicit GLSLBlending(QObject *parent = 0);
	~GLSLBlending();

	void initialize(int panoWidth, int panoHeight, int viewCount);
	void render(GLuint colorTextures[], GLuint alphaTextures[], int colorLevel, int alphaLevel);

	virtual const int getWidth();
	virtual const int getHeight();

private:
	int panoramaWidth;
	int panoramaHeight;

	int m_viewCount;

	GLuint levelScaleUnif;
};

#endif // GLSLBLENDING_H