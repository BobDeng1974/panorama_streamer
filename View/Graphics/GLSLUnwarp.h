#ifndef GLSLUNWARP_H
#define GLSLUNWARP_H

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>

#include "D360Parser.h"
#include "common.h"

struct FisheyeCamera
{
    float m_cameraNumber;
    float m_imageWidth;
    float m_imageHeight;
    float m_focal;
    float m_radius;
    float m_fisheyeRadius;
    float m_cx;
    float m_cy;
    float m_offsetx;
    float m_offsety;

    float m_k1;
    float m_k2;
    float m_k3;
    float m_vx;
    float m_vy;
    float m_vz;
    float m_maxcos;

    float m_pitch;
    float m_roll;
    float m_yaw;
};

// undistorts fisheye-lens camera image to panorama image coordinate and render it to FrameBuffer
class GLSLUnwarp : public QObject
{
    Q_OBJECT
public:
    explicit GLSLUnwarp(QObject *parent = 0, bool isYUV = true);
    ~GLSLUnwarp();

    void setGL(QOpenGLFunctions* gl);
	void initialize(int id, int xres, int yres, int panoWidth, int panoHeight);
	void render(unsigned int rgbTextureId, int camID);
	void updateCameraParams();

    const GLuint getTargetTexture() { return m_fboTextureId; }
	const GLuint getTargetFrameBuffer() { return m_fboId; }

	void setCameraInput(CameraInput camIn) { camInput = camIn; }

	int inputWidth;
	int inputHeight;

    int panoramaWidth;
    int panoramaHeight;
	//const int panoramaWidth = 512;
	//const int panoramaHeight = 512;

    // framebuffer index
    GLuint m_fboId;

    // framebuffer-attached color texture
    GLuint m_fboTextureId;

    // undistort GLSL program
    QOpenGLShaderProgram *m_program;

    GLuint m_vertexAttr;
    GLuint m_texCoordAttr;
    int m_fisheyeLensRadiusRatio1Unif;
    int m_fisheyeLensRadiusRatio2Unif;
    int m_blendCurveStartUnif;

    // gl functions
    QOpenGLFunctions* m_gl;

	bool m_initialized;
	int camID;

	GLuint cpUnif;
	GLuint xrad1Unif;
	GLuint xrad2Unif;
	GLuint yrad1Unif;
	GLuint yrad2Unif;

private:
	CameraInput camInput;
};

#endif // GLSLUNWARP_H
