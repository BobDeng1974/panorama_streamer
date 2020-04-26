#ifndef GLSLGAINCOMPENSATION_H
#define GLSLGAINCOMPENSATION_H

#include "GLSLProgram.h"

/// <summary>
/// The opengl shader that compensates the gain.
/// </summary>
class GLSLGainCompensation : public GLSLProgram
{
	Q_OBJECT
public:
	explicit GLSLGainCompensation(QObject *parent = 0);
	~GLSLGainCompensation();

	void initialize(int cameraWidth, int cameraHeight);
	void render(GLuint textureId, float ev);

	virtual const int getWidth();
	virtual const int getHeight();

private:
	GLuint m_gainUnif;

	int cameraWidth;
	int cameraHeight;
};

#endif // GLSLGAINCOMPENSATION_H