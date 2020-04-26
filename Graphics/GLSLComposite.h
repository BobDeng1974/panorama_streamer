#ifndef GLSLCOMPOSITE_H
#define GLSLCOMPOSITE_H

#include <QObject>

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLFunctions_2_0>
#include <QOpenGLTexture>

#include "GLSLFeathering.h"
#include "GLSLMultibandBlending.h"
#include "common.h"
#include "D360Parser.h"

typedef struct {
	int intersectPixelCnt;
	int meanSrc1Intensity;
	int meanSrc2Intensity;
} INTERSECT_DATA;

class GLSLComposite : public QObject
{
    Q_OBJECT
public:
    explicit GLSLComposite(QObject *parent = 0);
    ~GLSLComposite();

    void setGL(QOpenGLFunctions* gl, QOpenGLFunctions_2_0* functions_2_0);
	void initialize(int nViewCount, int panoWidth, int panoHeight);
	void render(GLuint textures[]);

	int getBlendTexture();
	int getTargetTexture(bool useBill);
	int getPanoramaWidth() { return m_panoramaWidth; }
	int getPanoramaHeight() { return m_panoramaHeight; }

	std::vector<float> getExposureData(GLuint fbos[], int viewCnt);

	void setBlendingMode(GlobalAnimSettings::BlendingMode mode, int multiBandLevel = 0)
	{
		m_blendingMode = mode;
		if (mode == GlobalAnimSettings::MultiBandBlending)
			m_multiBandLevel = multiBandLevel;
	}
	void setSeamIndex(int index) { m_showSeamIdx = index; }
	void mixBill(GLuint bill, vec2 billPos[]);
private:
	void multibandBlending(GLuint fboTextures[], int bandLevel);
	INTERSECT_DATA getInterSectData(GLuint src1, GLuint src2);

	void getCameraRegionImage();
	void getSeamMaskImage();
	void getConfirmImage(GLuint fboTextureID);
	void saveTexture(GLuint fboID, QString strSaveName);

private:
	int m_panoramaWidth, m_panoramaHeight, m_viewCount;	

	QOpenGLFunctions* m_gl;
	QOpenGLFunctions_2_0* m_functions_2_0;

	QOpenGLShaderProgram *m_maskProgram, *m_seamProgram, *m_regionProgram;
	GLuint m_fboMaskId, m_fboTextureMaskId, m_fboSeamId, m_fboTextureSeamId, m_fboRegionId, m_fboTextureRegionId;

	GLuint m_vertexAttr, m_texCoordAttr;

	bool m_initialized;

	GLSLFeathering* m_feathering;
	GLSLMultibandBlending* m_multibandBlending;

	GlobalAnimSettings::BlendingMode m_blendingMode;
	int m_multiBandLevel;
	int m_showSeamIdx;

	// adv bill
	QOpenGLShaderProgram *m_billProgram;
	GLuint m_fboBillId, m_fboTextureBillId;
};

#endif // GLSLCOMPOSITE_H