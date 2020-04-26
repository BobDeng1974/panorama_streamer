#ifndef GLSLRESIZE_H
#define GLSLRESIZE_H

#include "GLSLProgram.h"

/// <summary>
/// The opengl shader that resize the texture in half
/// </summary>
class GLSLResize : public GLSLProgram
{
	Q_OBJECT
public:
	explicit GLSLResize(QObject *parent = 0);
	~GLSLResize();

	void initialize(int panoWidth, int panoHeight);
	void render(GLuint textureId);

	virtual const int getWidth();
	virtual const int getHeight();

private:
	int panoramaWidth;
	int panoramaHeight;
};

#endif // GLSLRESIZE_H