#pragma once
#include <qvector.h>
#include <3DMath.h>
#include <qgenericmatrix.h>

#define  PO_PI			(3.1415927)
#define PO_TOW_PI		(PO_PI * 2)
#define PO_HALF_PI		(PO_PI / 2)

float ev2gain(float ev);
float gain2ev(float gain);
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2      1.57079632679489661923
#endif
#ifndef M_PI_4
#define M_PI_4      0.785398163397448309616
#endif

#define ROUND(v) ((int)(v>=0 ? (v+0.5f):(v-0.5f)))

extern int g_panoramaWidth, g_panoramaHeight;

void pano2window(double panoW, double panoH, double windowW, double windowH, double px, double py, double& wx, double& wy, bool isStereo, bool atRightEye);
void window2pano_1x1(double panoW, double panoH, double windowW, double windowH, double& px, double& py, double wx, double wy, bool isStereo, bool atRightEye);

QMatrix3x3 getViewMatrix(float yaw, float pitch, float roll);
void cartesianToXYn(vec3 cartesian, int *XYn);