#ifndef GLSLLAPLACIAN_H
#define GLSLLAPLACIAN_H

#include "GLSLProgram.h"

/// <summary>
/// The opengl shader that calculates the laplacian for each camera view
/// </summary>
class GLSLLaplacian : public GLSLProgram
{
	Q_OBJECT
public:
	explicit GLSLLaplacian(QObject *parent = 0);
	~GLSLLaplacian();

	void initialize(int panoWidth, int panoHeight);
	void render(GLuint srcTextureId, GLuint gaussianTextureId);

	virtual const int getWidth();
	virtual const int getHeight();

private:
	int panoramaWidth;
	int panoramaHeight;
};

#endif // GLSLLAPLACIAN_H