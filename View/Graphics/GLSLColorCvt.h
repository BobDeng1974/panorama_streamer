#ifndef GLSLCOLORCVT_H
#define GLSLCOLORCVT_H

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLFunctions_2_0>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>

#include "D360Parser.h"
#include "SharedImageBuffer.h"
#include "define.h"

class GLSLColorCvt_2RGB : public QObject
{
	Q_OBJECT
public:
	GLSLColorCvt_2RGB(QObject *parent = 0);
	~GLSLColorCvt_2RGB();

	virtual void setGL(QOpenGLFunctions* gl);
	void initialize(int imgWidth, int imgHeight);
	virtual void render(ImageBufferData& img);

	GLuint getTargetTexture() const { return m_fboTextureId; }
protected:
	virtual void createSourceTexture() = 0;
	virtual void addShaderFile() = 0;
	virtual void addPBO() = 0;
	virtual void setSourceTextureBuffer(int index, const uchar* buf, bool textureChanged) = 0;
	virtual bool isColorConversionNeeded() = 0;
	void renderEmptyFrame();
public:
	int ImageWidth() const { return imageWidth; }
	int ImageHeight() const { return imageHeight; }
protected:
	int imageWidth;
	int imageHeight;
	int imageBytesPerLine;

	GLuint m_srcTextureId;
	// framebuffer index and color texture
	GLuint m_fboId;
	GLuint m_fboTextureId;

	// PBO
	const int m_pboCount = 2;
	std::vector<QOpenGLBuffer*> m_pbos;
	int m_pboIndex;

	// undistort GLSL program
	QOpenGLShaderProgram *m_program;

	GLuint m_vertexAttr;
	GLuint m_texCoordAttr;

	// gl functions
	QOpenGLFunctions* m_gl;

	bool m_initialized;

	GLuint bytesPerLineUnif;
	GLuint imageWidthUnif;
	GLuint imageHeightUnif;
};

// convert yuv420 buffer to rgb buffer. This is common case for importing video files.
class GLSLColorCvt_YUV2RGB : public GLSLColorCvt_2RGB
{
    Q_OBJECT
public:
	explicit GLSLColorCvt_YUV2RGB(QObject *parent = 0);
	~GLSLColorCvt_YUV2RGB();

protected:
	virtual void createSourceTexture();
	virtual void addShaderFile();
	virtual void addPBO();
	virtual void setSourceTextureBuffer(int index, const uchar* bits, bool textureChanged);
	virtual bool isColorConversionNeeded();

private:
	const uchar* buffer_ptrs[3];
};

// convert yuv422 buffer to rgb buffer. This is usual 
class GLSLColorCvt_YUV4222RGB : public GLSLColorCvt_2RGB
{
	Q_OBJECT
public:
	explicit GLSLColorCvt_YUV4222RGB(QObject *parent = 0);
	~GLSLColorCvt_YUV4222RGB();

protected:
	virtual void createSourceTexture();
	virtual void addShaderFile();
	virtual void addPBO();
	virtual void setSourceTextureBuffer(int index, const uchar* bits, bool textureChanged);
	virtual bool isColorConversionNeeded();
};

class GLSLColorCvt_RGB2RGB : public GLSLColorCvt_2RGB
{
	Q_OBJECT
public:
	explicit GLSLColorCvt_RGB2RGB(QObject *parent = 0);
	~GLSLColorCvt_RGB2RGB();

protected:
	virtual void createSourceTexture();
	virtual void addShaderFile();
	virtual void addPBO();
	virtual void setSourceTextureBuffer(int index, const uchar* bits, bool textureChanged);
	virtual bool isColorConversionNeeded();
};

// convert rgb buffer to yuv420 so that we can directly broadcast it to the wowza server
class GLSLColorCvt_RGB2YUV : public QObject
{
	Q_OBJECT
public:
	enum GBUFFER_TEXTURE_TYPE {
		GBUFFER_TEXTURE_TYPE_Y,
		GBUFFER_TEXTURE_TYPE_U,
		GBUFFER_TEXTURE_TYPE_V,
		GBUFFER_NUM_TEXTURES
	};

	explicit GLSLColorCvt_RGB2YUV(QObject *parent = 0);
	~GLSLColorCvt_RGB2YUV();

	void setGL(QOpenGLFunctions* gl, QOpenGLFunctions_2_0* functions_2_0);
	void initialize(int panoWidth, int panoHeight);
	void render(GLuint rgbTextureId);

	int getFBOId() { return m_fboId; }
	void getYUVBuffer(unsigned char* yuvBuffer);

private:

	int panoramaWidth;
	int panoramaHeight;

	// framebuffer index and color texture
	GLuint m_fboId;
	//GLuint m_fboTextureId;
	GLuint m_textures[GBUFFER_NUM_TEXTURES];

	// undistort GLSL program
	QOpenGLShaderProgram *m_program;

	GLuint m_vertexAttr;
	GLuint m_texCoordAttr;

	// gl functions
	QOpenGLFunctions* m_gl;
	QOpenGLFunctions_2_0* m_functions_2_0;

	unsigned char * m_buffer;

	const int m_pboCount = 6;
	std::vector<QOpenGLBuffer*> m_pbos;
	int m_pboIndex;

	bool m_initialized;
};

class UniColorCvt
{
private:
	GLSLColorCvt_2RGB* mConverter;
	int mFormat;
public:
	UniColorCvt();
	~UniColorCvt();
	void Free();
	// render
	void DynRender(ImageBufferData& img, QOpenGLFunctions* gl = NULL);
	GLuint getTargetTexture() const;
};

#endif // GLSLCOLORCVT_H