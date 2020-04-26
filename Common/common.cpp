
#include <math.h>
#include "common.h"

int g_panoramaWidth = 0;
int g_panoramaHeight = 0;

float ev2gain(float ev)
{
	// experimental exposure value to gain conversion
	return 2.5778*exp(-0.315*(3 - ev));
}

float gain2ev(float gain)
{
	// gain conversion to experimental exposure value
	return log(gain / 2.5778) / 0.315 + 3;
}

void pano2window(double panoW, double panoH, double windowW, double windowH, double px, double py, double& wx, double& wy, bool isStereo, bool atRightEye)
{
	if (isStereo)
		panoH *= 2;

	double curW = windowW;
	double curH = windowH;

	if (panoH / panoW > windowH / windowW)
	{
		curH = windowH;
		curW = curH * (panoW / panoH);
	}
	else
	{
		curW = windowW;
		curH = curW * panoH / panoW;
	}
	double xOrg = (windowW - curW) / 2;
	double yOrg = (windowH - curH) / 2;

	wx = (curW / panoW) * px + xOrg;
	wy = (curH / panoH) * py + yOrg;
	if (isStereo && atRightEye)
		wy += curH / 2;
}

void window2pano_1x1(double panoW, double panoH, double windowW, double windowH, double& px, double& py, double wx, double wy, bool isStereo, bool atRightEye)
{
	if (isStereo)
		panoH *= 2;

	double curW = windowW;
	double curH = windowH;

	if (panoH / panoW > windowH / windowW)
	{
		curH = windowH;
		curW = curH * (panoW / panoH);
	}
	else
	{
		curW = windowW;
		curH = curW * panoH / panoW;
	}
	double xOrg = (windowW - curW) / 2;
	double yOrg = (windowH - curH) / 2;

	px = (wx - xOrg) / curW;
	py = (wy - yOrg) / curH;
	if (isStereo)
	{
		py *= 2; // py = (wy - yOrg) / (curH/2);
		if (atRightEye)
			py += 0.5;
	}
}

QMatrix3x3 getViewMatrix(float yaw, float pitch, float roll)
{
	float A = pitch;
	float B = yaw;
	float C = roll;

	A = A * M_PI / 180.0f;
	B = B * M_PI / 180.0f;
	C = C * M_PI / 180.0f;
	QMatrix3x3 m;
	m.setToIdentity();

	QMatrix3x3 pitchMat;
	QMatrix3x3 yawMat;
	QMatrix3x3 rollMat;
	pitchMat.setToIdentity();
	yawMat.setToIdentity();
	rollMat.setToIdentity();
	float* rollData = rollMat.data();

	rollData[0] = cos(C);	rollData[1] = sin(C);	rollData[2] = 0;
	rollData[3] = -sin(C);	rollData[4] = cos(C);	rollData[5] = 0.0;
	rollData[6] = 0.0;		rollData[7] = 0.0;		rollData[8] = 1.0;

	float* yawData = yawMat.data();
	yawData[0] = cos(B);	yawData[1] = 0.0;	yawData[2] = sin(B);
	yawData[3] = 0;			yawData[4] = 1.0;	yawData[5] = 0.0;
	yawData[6] = -sin(B);	yawData[7] = 0;		yawData[8] = cos(B);

	float* pitchData = pitchMat.data();
	pitchData[0] = 1; pitchData[1] = 0;			pitchData[2] = 0;
	pitchData[3] = 0; pitchData[4] = cos(A);	pitchData[5] = sin(A);
	pitchData[6] = 0; pitchData[7] = -sin(A);	pitchData[8] = cos(A);

	m = rollMat*pitchMat*yawMat;
	return m;
}

void cartesianToXYn(vec3 cartesian, int *XYn)
{
	// cartesianToSpherical
	float theta;
	float phi = asin(cartesian.y);
	float cosphi = pow(1.0f - (cartesian.y*cartesian.y), 0.5f);
	if (cosphi == 0.0f)
	{
		theta = 0.0f;
	}
	else
	{
		theta = atan2(cartesian.x / cosphi, cartesian.z / cosphi);
	}

	// sphericalToXYn
	XYn[0] = g_panoramaWidth * (theta / M_PI + 1) / 2;
	XYn[1] = g_panoramaHeight * (phi + M_PI_2) / M_PI;
}