#ifndef GLSLGAUSSIANBLUR_H
#define GLSLGAUSSIANBLUR_H

#include "GLSLProgram.h"
#include "GLSLBoxBlur.h"

/// <summary>
/// The opengl shader that calculates the gaussian blur for each camera view
/// </summary>
class GLSLGaussianBlur : public GLSLProgram
{
	Q_OBJECT
public:
	explicit GLSLGaussianBlur(QObject *parent = 0);
	~GLSLGaussianBlur();

	void initialize(int panoWidth, int panoHeight, bool isPartial);
	void render(GLuint textureId);

	virtual int getTargetTexture();

	virtual const int getWidth();
	virtual const int getHeight();

	GLuint boxBlur(GLuint src, float radius);
	static void getGaussianRadius();

private:
	int panoramaWidth;
	int panoramaHeight;

	GLSLBoxBlur * m_horizontalBoxBlur;
	GLSLBoxBlur * m_verticalBoxBlur;

	static bool isGaussianRadiusInitialized;
	static int gaussianRadius[3];
};

#endif // GLSLGAUSSIANBLUR_H