#ifndef GLSLBOUNDARY_H
#define GLSLBOUNDARY_H

#include "GLSLProgram.h"

/// <summary>
/// The opengl shader that calculates the boundary area for each view.
/// </summary>
class GLSLBoundary : public GLSLProgram
{
	Q_OBJECT
public:
	explicit GLSLBoundary(QObject *parent = 0);
	~GLSLBoundary();

	void initialize(int panoWidth, int panoHeight, int viewCount, int viewIdx);
	void render(GLuint fbos[]);

	virtual const int getWidth();
	virtual const int getHeight();

private:
	int panoramaWidth;
	int panoramaHeight;

	int m_viewCount;
	int m_viewIdx;
};

#endif // GLSLBOUNDARY_H