#ifndef GLSLFINALPANORAMA_H
#define GLSLFINALPANORAMA_H

#include "GLSLProgram.h"
#include <QOpenGLBuffer>

/// <summary>
/// The final opengl shader that shows mono and stereo panorama in one view.
/// It can just show one panorama with the resolution of width by height,
/// or can show stereo panorama by top-bottom mode with the resolution of width by height*2.
/// </summary>
class GLSLFinalPanorama : public GLSLProgram
{
	Q_OBJECT
public:
	explicit GLSLFinalPanorama(QObject *parent = 0);
	~GLSLFinalPanorama();

	void initialize(int panoWidth, int panoHeight, bool isStereo);
	void render(GLuint fbos[]);

	void getRGBBuffer(unsigned char* rgbBuffer);

	const int getWidth();
	const int getHeight();

	int getTargetTexture() { return m_fboTextureIds[1 - m_workingTexture]; }

private:
	int panoramaViewWidth;
	int panoramaViewHeight;
	
	GLuint m_fboIds[2];
	GLuint m_fboTextureIds[2];
	int m_workingTexture;

	// gl functions
	QOpenGLBuffer* m_pbos[2];
	int m_pboIndex;

	bool m_stereo;
};

#endif // GLSLFINALPANORAMA_H