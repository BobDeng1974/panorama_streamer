#ifndef GLSLPANORAMA_PLACEMENT_H
#define GLSLPANORAMA_PLACEMENT_H

#include "GLSLProgram.h"

/// <summary>
/// The opengl shader that blends all color views with blend map
/// </summary>
class GLSLPanoramaPlacement : public GLSLProgram
{
	Q_OBJECT
public:
	explicit GLSLPanoramaPlacement(QObject *parent = 0);
	~GLSLPanoramaPlacement();

	void initialize(int panoWidth, int panoHeight);
	void render(GLuint originalTextures, float yaw, float pitch, float roll);

	virtual const int getWidth();
	virtual const int getHeight();

private:
	int panoramaWidth;
	int panoramaHeight;

	GLuint placeMatUnif;
};

#endif // GLSLPANORAMA_PLACEMENT_H