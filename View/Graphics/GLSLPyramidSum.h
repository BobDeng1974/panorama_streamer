#ifndef GLSLPYRAMIDSUM_H
#define GLSLPYRAMIDSUM_H

#include "GLSLProgram.h"

/// <summary>
/// The opengl shader that sum up all colors for all level
/// </summary>
class GLSLMergeTexture : public GLSLProgram
{
	Q_OBJECT
public:
	explicit GLSLMergeTexture(QObject *parent = 0);
	~GLSLMergeTexture();

	void initialize(int panoWidth, int panoHeight);
	void render(GLuint texture, GLuint texture2);
	void clear();

	virtual const int getWidth();
	virtual const int getHeight();

private:
	int panoramaWidth;
	int panoramaHeight;
};

/// <summary>
/// The opengl shader that sum up all colors for all level
/// </summary>
class GLSLPyramidSum : public GLSLProgram
{
	Q_OBJECT
public:
	explicit GLSLPyramidSum(QObject *parent = 0);
	~GLSLPyramidSum();

	void initialize(int panoWidth, int panoHeight);
	void clear();
	void render(GLuint texture);

	virtual int getTargetTexture();

	virtual const int getWidth();
	virtual const int getHeight();

private:
	int panoramaWidth;
	int panoramaHeight;

	GLSLMergeTexture *m_merge1, *m_merge2;
	GLuint lastTexture;
	int m_currentMerge;
};

#endif // GLSLPYRAMIDSUM_H