#include "GLSLColorCvt.h"


GLSLColorCvt_2RGB::GLSLColorCvt_2RGB(QObject *parent) : QObject(parent)
, m_fboTextureId(0)
, m_program(0)
, m_initialized(false)
{
}

GLSLColorCvt_2RGB::~GLSLColorCvt_2RGB()
{
	if (m_initialized)
	{
		delete m_program;
		m_program = NULL;
		m_gl->glDeleteTextures(1, &m_srcTextureId);
		m_gl->glDeleteFramebuffers(1, &m_fboId);
		m_gl->glDeleteTextures(1, &m_fboTextureId);

		for (int i = 0; i < m_pbos.size(); i++)
			delete m_pbos[i];
		m_pbos.clear();
	}
}

void GLSLColorCvt_2RGB::setGL(QOpenGLFunctions* gl)
{
	m_gl = gl;
}

void GLSLColorCvt_2RGB::initialize(int imgWidth, int imgHeight)
{
	imageWidth = 0;
	imageHeight = 0;
	imageBytesPerLine = 0;

	// create fbo shader
	m_program = new QOpenGLShaderProgram();
	addShaderFile();
	m_program->link();

	m_program->bind();

	m_vertexAttr = m_program->attributeLocation("vertex");
	m_texCoordAttr = m_program->attributeLocation("texCoord");
	
	QMatrix4x4 matrix;
	matrix.ortho(-imgWidth / 2, imgWidth / 2, -imgHeight / 2, imgHeight / 2, -10.0f, 10.0f);
	matrix.translate(0, 0, -2);
	m_program->setUniformValue("matrix", matrix);
	m_program->setUniformValue("texture", 0);
	m_program->setUniformValue("textureU", 1);
	m_program->setUniformValue("textureV", 2);

	bytesPerLineUnif = m_program->uniformLocation("bytesPerLine");
	imageWidthUnif = m_program->uniformLocation("width");
	imageHeightUnif = m_program->uniformLocation("height");


	//addPBO();

	m_pboIndex = 0;

	m_program->release();

	m_initialized = true;

	renderEmptyFrame();
}


void GLSLColorCvt_2RGB::render(ImageBufferData& img)
{
	m_program->bind();

	// Simple glTexImage2D, with PBO
	//const uchar * bits = img.constBits();
	int width = img.mImageY.width;
	int height = img.mImageY.height;

	bool blTextureFormatChanged = false;
	if (imageWidth != width || imageHeight != height)
	{
		imageWidth = width;
		imageHeight = height;
		imageBytesPerLine = img.mImageY.stride;

		if (isColorConversionNeeded())
		{
			m_program->setUniformValue(bytesPerLineUnif, imageBytesPerLine);
			m_program->setUniformValue(imageWidthUnif, imageWidth);
			m_program->setUniformValue(imageHeightUnif, imageHeight);
		}

		createSourceTexture();

		// frame buffer
		m_gl->glGenTextures(1, &m_fboTextureId);
		m_gl->glBindTexture(GL_TEXTURE_2D, m_fboTextureId);
		m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, imageWidth, imageHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		// load textures and create framebuffers
		m_gl->glGenFramebuffers(1, &m_fboId);
		m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
		m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextureId, 0);
		m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

		addPBO();

		blTextureFormatChanged = true;
	}
	
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

	m_gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	m_gl->glClear(GL_COLOR_BUFFER_BIT);

	m_gl->glViewport(0, 0, imageWidth, imageHeight);

	const unsigned char * buffers[3] = { img.mImageY.buffer, img.mImageU.buffer, img.mImageV.buffer };
	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_srcTextureId);

	m_pboIndex = (m_pboIndex + 1) % m_pboCount;
	for (int i = 0; i < 3; i++)
	{	// host to device buffer uploading.
		setSourceTextureBuffer(i, buffers[i], blTextureFormatChanged);
	}
	if (blTextureFormatChanged)
	{
		m_pboIndex = (m_pboIndex + 1) % m_pboCount;
		for (int i = 0; i < 3; i++)
		{	// host to device buffer uploading.
			setSourceTextureBuffer(i, buffers[i], false);
		}
	}

	GLfloat vertices[] = {
		-imageWidth / 2, -imageHeight / 2,
		-imageWidth / 2, imageHeight / 2,
		imageWidth / 2, imageHeight / 2,
		imageWidth / 2, -imageHeight / 2
	};

	GLfloat texCoords[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
	};

	int vertexAttr = m_vertexAttr;
	int texCoordAttr = m_texCoordAttr;

	m_gl->glVertexAttribPointer(vertexAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	m_gl->glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

	m_gl->glEnableVertexAttribArray(vertexAttr);
	m_gl->glEnableVertexAttribArray(texCoordAttr);

	m_gl->glDrawArrays(GL_QUADS, 0, 4);

	m_gl->glDisableVertexAttribArray(texCoordAttr);
	m_gl->glDisableVertexAttribArray(vertexAttr);

	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_program->release();
}

void GLSLColorCvt_2RGB::renderEmptyFrame()
{
	/*ImageBufferData img;
	img.mImageV.width = this->imageWidth;
	img.mImageV.height = this->imageHeight;
	img.mImageY.stride = this->imageWidth * 3;

	img.mImageY.buffer = new unsigned char[img.mImageV.height * img.mImageY.stride];
	render(img, false, 0.0f);
	delete[] img.mImageY.buffer;*/
}

//////////////

GLSLColorCvt_YUV2RGB::GLSLColorCvt_YUV2RGB(QObject *parent) : GLSLColorCvt_2RGB(parent)
{
}

GLSLColorCvt_YUV2RGB::~GLSLColorCvt_YUV2RGB()
{

}

void GLSLColorCvt_YUV2RGB::createSourceTexture()
{
	// Source texture
	m_gl->glGenTextures(1, &m_srcTextureId);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_srcTextureId);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, imageBytesPerLine, imageHeight * 3 / 2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
}

void GLSLColorCvt_YUV2RGB::addShaderFile()
{
#ifdef USE_SHADER_CODE
	ADD_SHADER_FROM_CODE(m_program, "vert", "stitcher");
	ADD_SHADER_FROM_CODE(m_program, "frag", "yuv4202rgb");
#else
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "./Shaders/stitcher.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "./Shaders/yuv4202rgb.frag");
#endif
}

void GLSLColorCvt_YUV2RGB::addPBO()
{
	for (int i = 0; i < m_pboCount; i++)
	{
		QOpenGLBuffer* pbo = new QOpenGLBuffer(QOpenGLBuffer::PixelUnpackBuffer);
		pbo->create();
		pbo->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);
		pbo->bind();
		pbo->allocate(imageBytesPerLine * imageHeight * 3 / 2);
		pbo->release();
		m_pbos.push_back(pbo);
	}
}

void GLSLColorCvt_YUV2RGB::setSourceTextureBuffer(int index, const uchar* bits, bool textureChanged)
{
	buffer_ptrs[index] = bits;

	if (index == 2)
	{
		int widths[3] = { imageBytesPerLine, imageBytesPerLine / 2, imageBytesPerLine / 2 };
		int heights[3] = { imageHeight, imageHeight / 2, imageHeight / 2 };
#if 1
		QOpenGLBuffer * pboCurrent = m_pbos[m_pboIndex];
		QOpenGLBuffer * pboNext = m_pbos[(m_pboIndex + 1) % m_pboCount];

		pboCurrent->bind();
		if (textureChanged)
			m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, imageBytesPerLine, imageHeight * 3 / 2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
		else
			m_gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageBytesPerLine, imageHeight * 3 / 2, GL_RED, GL_UNSIGNED_BYTE, 0);
		pboCurrent->release();

		if (imageBytesPerLine > 0 && imageHeight > 0)
		{
			pboNext->bind();
			GLubyte* ptr = (GLubyte*)pboNext->map(QOpenGLBuffer::WriteOnly);
			if (ptr)
			{
				int offset = 0;
				for (int i = 0; i < 3; i++)
				{
					int len = widths[i] * heights[i];
					memcpy(ptr + offset, buffer_ptrs[i], len);
					offset += len;
				}
				pboNext->unmap();
			}
			pboNext->release();
		}
#else
		m_gl->glActiveTexture(GL_TEXTURE0);
		m_gl->glBindTexture(GL_TEXTURE_2D, m_srcTextureId);
		if (blTextureFormatChanged)
			m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height * 3 / 2, 0, GL_RED, GL_UNSIGNED_BYTE, bufferPtrs[0]);
		else
			m_gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height * 3 / 2, GL_RED, GL_UNSIGNED_BYTE, bufferPtrs[0]);
#endif
	}
}

bool GLSLColorCvt_YUV2RGB::isColorConversionNeeded()
{
	return true;
}

//////////////

GLSLColorCvt_YUV4222RGB::GLSLColorCvt_YUV4222RGB(QObject *parent) : GLSLColorCvt_2RGB(parent)
{
}

GLSLColorCvt_YUV4222RGB::~GLSLColorCvt_YUV4222RGB()
{

}

void GLSLColorCvt_YUV4222RGB::createSourceTexture()
{
	// Source texture
	m_gl->glGenTextures(1, &m_srcTextureId);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_srcTextureId);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageBytesPerLine / 4, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	//m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, imageBytesPerLine / 2, imageHeight, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
}

void GLSLColorCvt_YUV4222RGB::addShaderFile()
{
#ifdef USE_SHADER_CODE
	ADD_SHADER_FROM_CODE(m_program, "vert", "stitcher");
	ADD_SHADER_FROM_CODE(m_program, "frag", "yuv4222rgb");
#else
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "./Shaders/stitcher.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "./Shaders/yuv4222rgb.frag");
#endif
}

void GLSLColorCvt_YUV4222RGB::addPBO()
{
	for (int j = 0; j < m_pboCount; j++)
	{
		QOpenGLBuffer* pbo = new QOpenGLBuffer(QOpenGLBuffer::PixelUnpackBuffer);
		pbo->create();
		pbo->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);
		pbo->bind();
		pbo->allocate(imageBytesPerLine * imageHeight);
		pbo->release();
		m_pbos.push_back(pbo);
	}
}

void GLSLColorCvt_YUV4222RGB::setSourceTextureBuffer(int index, const uchar* bits, bool textureChanged)
{
	if (index != 0)
		return;

#if 1
	QOpenGLBuffer * pboCurrent = m_pbos[m_pboIndex];
	QOpenGLBuffer * pboNext = m_pbos[(m_pboIndex + 1) % m_pboCount];

	int widthPixels = imageBytesPerLine / 4;

	pboCurrent->bind();
	if (textureChanged)
		m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthPixels, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	else
		m_gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, widthPixels, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	pboCurrent->release();

	if (imageBytesPerLine > 0 && imageHeight > 0)
	{
		pboNext->bind();
		GLubyte* ptr = (GLubyte*)pboNext->map(QOpenGLBuffer::WriteOnly);
		if (ptr)
		{
			memcpy(ptr, bits, imageBytesPerLine * imageHeight);
			pboNext->unmap();
		}
		pboNext->release();
	}
#else
	if (blTextureFormatChanged)
		m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height * 3 / 2, 0, GL_RED, GL_UNSIGNED_BYTE, bufferPtrs[0]);
	else
		m_gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height * 3 / 2, GL_RED, GL_UNSIGNED_BYTE, bufferPtrs[0]);
#endif
}

bool GLSLColorCvt_YUV4222RGB::isColorConversionNeeded()
{
	return true;
}

//////////////
GLSLColorCvt_RGB2RGB::GLSLColorCvt_RGB2RGB(QObject *parent) : GLSLColorCvt_2RGB(parent)
{
}

GLSLColorCvt_RGB2RGB::~GLSLColorCvt_RGB2RGB()
{

}

void GLSLColorCvt_RGB2RGB::createSourceTexture()
{
	// Source texture
	m_gl->glGenTextures(1, &m_srcTextureId);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_srcTextureId);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}

void GLSLColorCvt_RGB2RGB::addShaderFile()
{
#ifdef USE_SHADER_CODE
	ADD_SHADER_FROM_CODE(m_program, "vert", "stitcher");
	ADD_SHADER_FROM_CODE(m_program, "frag", "rgb2rgb");
#else
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "./Shaders/stitcher.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "./Shaders/rgb2rgb.frag");
#endif
}

void GLSLColorCvt_RGB2RGB::addPBO()
{
	for (int i = 0; i < m_pboCount; i++)
	{
		QOpenGLBuffer* pbo = new QOpenGLBuffer(QOpenGLBuffer::PixelUnpackBuffer);
		pbo->create();
		pbo->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);
		pbo->bind();
		pbo->allocate(imageWidth * imageHeight * 3);
		pbo->release();
		m_pbos.push_back(pbo);
	}
}

void GLSLColorCvt_RGB2RGB::setSourceTextureBuffer(int index, const uchar* bits, bool textureChanged)
{
	if (index != 0)
		return;
#if 1
	QOpenGLBuffer * pboCurrent = m_pbos[m_pboIndex];
	QOpenGLBuffer * pboNext = m_pbos[(m_pboIndex + 1) % m_pboCount];

	pboCurrent->bind();
	if (textureChanged)
		m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	else
		m_gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageWidth, imageHeight, GL_RGB, GL_UNSIGNED_BYTE, 0);
	pboCurrent->release();

	if (imageWidth > 0 && imageHeight > 0)
	{
		pboNext->bind();
		GLubyte* ptr = (GLubyte*)pboNext->map(QOpenGLBuffer::WriteOnly);
		if (ptr)
		{
			memcpy(ptr, bits, imageHeight * imageBytesPerLine);
			pboNext->unmap();
		}
		pboNext->release();
	}
#else
	if (textureChanged)
		m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, bits);
	else
		m_gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageWidth, imageHeight, GL_RGB, GL_UNSIGNED_BYTE, bits);
#endif
}

bool GLSLColorCvt_RGB2RGB::isColorConversionNeeded()
{
	return false;
}

//////////////

GLSLColorCvt_RGB2YUV::GLSLColorCvt_RGB2YUV(QObject *parent) : QObject(parent)
, m_program(0)
, m_initialized(false)
, m_buffer(0)
{
	memset(m_textures, 0, sizeof(m_textures));
}

GLSLColorCvt_RGB2YUV::~GLSLColorCvt_RGB2YUV()
{
	if (m_initialized)
	{
		delete m_program;
		m_program = NULL;
		m_gl->glDeleteFramebuffers(1, &m_fboId);
		if (m_textures[0] != 0)
		{
			m_gl->glDeleteTextures(GBUFFER_NUM_TEXTURES, m_textures);
		}
		//m_gl->glDeleteTextures(1, &m_fboTextureId);

		for (int i = 0; i < m_pbos.size(); i++)
			delete m_pbos[i];
		m_pbos.clear();

		delete[] m_buffer;
		m_buffer = 0;
	}
}

void GLSLColorCvt_RGB2YUV::setGL(QOpenGLFunctions* gl, QOpenGLFunctions_2_0* functions_2_0)
{
	m_gl = gl;
	m_functions_2_0 = functions_2_0;
}

void GLSLColorCvt_RGB2YUV::initialize(int panoWidth, int panoHeight)
{
	panoramaWidth = panoWidth;
	panoramaHeight = panoHeight;

	// load textures and create framebuffers
	m_gl->glGenFramebuffers(1, &m_fboId);
	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

	// frame buffer
	/*m_gl->glGenTextures(1, &m_fboTextureId);
	m_gl->glBindTexture(GL_TEXTURE_2D, m_fboTextureId);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, panoramaWidth, panoramaHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
	m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextureId, 0);*/

	m_gl->glGenTextures(GBUFFER_NUM_TEXTURES, m_textures);
	int widths[3] = { panoramaWidth, panoramaWidth, panoramaWidth };
	int heights[3] = { panoramaHeight, panoramaHeight, panoramaHeight };
	for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; i++) {
		m_gl->glBindTexture(GL_TEXTURE_2D, m_textures[i]);
		m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, widths[i], heights[i], 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_textures[i], 0);
	}

	GLenum status = m_gl->glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		//
	}


	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// create fbo shader
	m_program = new QOpenGLShaderProgram();
#ifdef USE_SHADER_CODE
	ADD_SHADER_FROM_CODE(m_program, "vert", "stitcher");
	ADD_SHADER_FROM_CODE(m_program, "frag", "rgb2yuv420");
#else
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "./Shaders/stitcher.vert");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "./Shaders/rgb2yuv420.frag");
#endif
	m_program->link();

	m_program->bind();

	m_vertexAttr = m_program->attributeLocation("vertex");
	m_texCoordAttr = m_program->attributeLocation("texCoord");

	QMatrix4x4 matrix;
	matrix.ortho(-panoramaWidth / 2, panoramaWidth / 2, -panoramaHeight / 2, panoramaHeight / 2, -10.0f, 10.0f);
	matrix.translate(0, 0, -2);
	m_program->setUniformValue("matrix", matrix);
	m_program->setUniformValue("texture", 0);


	// PBO for glReadPixels
	for (int i = 0; i < m_pboCount; i++)
	{
		QOpenGLBuffer* pbo = new QOpenGLBuffer(QOpenGLBuffer::PixelPackBuffer);
		pbo->create();
		pbo->setUsagePattern(QOpenGLBuffer::UsagePattern::StreamRead);
		pbo->bind();
		pbo->allocate(panoWidth * panoHeight * 1);
		pbo->release();
		m_pbos.push_back(pbo);
	}
	m_pboIndex = 0;

	m_program->release();

	m_buffer = new unsigned char[panoramaWidth * panoramaHeight * 4];

	m_initialized = true;
}

void GLSLColorCvt_RGB2YUV::render(GLuint rgbTextureId)
{
	m_program->bind();

	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2 };
	//GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	m_functions_2_0->glDrawBuffers(sizeof(DrawBuffers) / sizeof(DrawBuffers[0]), DrawBuffers);

	m_gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	m_gl->glClear(GL_COLOR_BUFFER_BIT);

	m_gl->glViewport(0, 0, panoramaWidth, panoramaHeight);
	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindTexture(GL_TEXTURE_2D, rgbTextureId);


	GLfloat vertices[] = {
		-panoramaWidth / 2, -panoramaHeight / 2,
		-panoramaWidth / 2, panoramaHeight / 2,
		panoramaWidth / 2, panoramaHeight / 2,
		panoramaWidth / 2, -panoramaHeight / 2
	};

	GLfloat texCoords[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
	};

	int vertexAttr = m_vertexAttr;
	int texCoordAttr = m_texCoordAttr;

	m_gl->glVertexAttribPointer(vertexAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	m_gl->glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

	m_gl->glEnableVertexAttribArray(vertexAttr);
	m_gl->glEnableVertexAttribArray(texCoordAttr);

	m_gl->glDrawArrays(GL_QUADS, 0, 4);

	m_gl->glDisableVertexAttribArray(texCoordAttr);
	m_gl->glDisableVertexAttribArray(vertexAttr);

	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_program->release();
}



void GLSLColorCvt_RGB2YUV::getYUVBuffer(unsigned char* yuvBuffer)
{
	m_program->bind();

	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

	m_pboIndex = (m_pboIndex + 1) % 2;
	for (int i = 0; i < 3; i++)
	{
		QOpenGLBuffer * pboCurrent = m_pbos[i * 2 + m_pboIndex];
		QOpenGLBuffer * pboNext = m_pbos[i * 2 + (m_pboIndex + 1) % 2];

		m_functions_2_0->glReadBuffer(GL_COLOR_ATTACHMENT0 + i);

		pboCurrent->bind();
		m_functions_2_0->glReadPixels(0, 0, panoramaWidth, panoramaHeight, GL_RED, GL_UNSIGNED_BYTE, 0);
		pboCurrent->release();

		pboNext->bind();
		GLubyte* ptr = (GLubyte*)pboNext->map(QOpenGLBuffer::ReadOnly);
		if (ptr)
		{
			memcpy(yuvBuffer + panoramaWidth * panoramaHeight * i, ptr, panoramaWidth * panoramaHeight);
			pboNext->unmap();
		}
		pboNext->release();
	}

	/*m_pboIndex = (m_pboIndex + 1) % m_pboCount;
	QOpenGLBuffer * pboCurrent = m_pbos[m_pboIndex];
	QOpenGLBuffer * pboNext = m_pbos[(m_pboIndex + 1) % m_pboCount];

	pboCurrent->bind();
	m_functions_2_0->glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, 0);
	pboCurrent->release();

	pboNext->bind();
	GLubyte* ptr = (GLubyte*)pboNext->map(QOpenGLBuffer::ReadOnly);
	if (ptr)
	{
	memcpy(yuvBuffer, ptr, panoramaWidth * panoramaHeight * 3);
	pboNext->unmap();
	}
	pboNext->release();*/

	m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_program->release();
}

///
UniColorCvt::UniColorCvt()
{
	mConverter = NULL;
	mFormat = -1;
}

UniColorCvt::~UniColorCvt()
{
	Free();
}

void UniColorCvt::Free()
{
	if (mConverter != NULL)
	{
		delete mConverter;
		mConverter = NULL;
		mFormat = -1;
	}
}

void UniColorCvt::DynRender(ImageBufferData& img, QOpenGLFunctions* gl)
{
	bool initialized = false;
	if (mFormat == img.mFormat
		&& mConverter != NULL
		&& mConverter->ImageWidth() == img.mImageY.width
		&& mConverter->ImageHeight() == img.mImageY.height)
	{
		initialized = true;
	}

	if (!initialized)
	{	// reset
		Free();

		mFormat = img.mFormat;
		switch (img.mFormat)
		{
		case ImageBufferData::YUV420:
			mConverter = new GLSLColorCvt_YUV2RGB();
			break;
		case ImageBufferData::RGB888:
			mConverter = new GLSLColorCvt_RGB2RGB();
			break;
		}
		if (gl == NULL)
			gl = QOpenGLContext::currentContext()->functions();
		mConverter->setGL(gl);
		mConverter->initialize(img.mImageY.width, img.mImageY.height);
	}
	mConverter->render(img);
}

GLuint UniColorCvt::getTargetTexture() const
{
	if (mConverter != NULL)
		return mConverter->getTargetTexture();
	return 0;
}