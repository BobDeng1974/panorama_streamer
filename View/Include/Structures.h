/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* Structures.h                                                         */
/*                                                                      */
/* Nick D'Ademo <nickdademo@gmail.com>                                  */
/*                                                                      */
/* Copyright (c) 2012-2013 Nick D'Ademo                                 */
/*                                                                      */
/* Permission is hereby granted, free of charge, to any person          */
/* obtaining a copy of this software and associated documentation       */
/* files (the "Software"), to deal in the Software without restriction, */
/* including without limitation the rights to use, copy, modify, merge, */
/* publish, distribute, sublicense, and/or sell copies of the Software, */
/* and to permit persons to whom the Software is furnished to do so,    */
/* subject to the following conditions:                                 */
/*                                                                      */
/* The above copyright notice and this permission notice shall be       */
/* included in all copies or substantial portions of the Software.      */
/*                                                                      */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,      */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF   */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                */
/* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS  */
/* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN   */
/* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN    */
/* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE     */
/* SOFTWARE.                                                            */
/*                                                                      */
/************************************************************************/

#ifndef STRUCTURES_H
#define STRUCTURES_H

// Qt
#include <QtCore/QRect>
#include <QTime>
#include <qlocale.h>

struct ImageProcessingSettings{
    int smoothType;
    int smoothParam1;
    int smoothParam2;
    double smoothParam3;
    double smoothParam4;
    int dilateNumberOfIterations;
    int erodeNumberOfIterations;
    int flipCode;
    double cannyThreshold1;
    double cannyThreshold2;
    int cannyApertureSize;
    bool cannyL2gradient;
};

struct ImageProcessingFlags{
    bool grayscaleOn;
    bool smoothOn;
    bool dilateOn;
    bool erodeOn;
    bool flipOn;
    bool cannyOn;
};

struct MouseData{
    QRect selectionBox;
    bool leftButtonRelease;
    bool rightButtonRelease;
};

struct ThreadStatisticsData{
    float averageFPS;
    int nFramesProcessed;
	int nAudioFrames;
	int instantFPS;
	int elapesedTime;

	QString toString(int fps)
	{
		int frames = nFramesProcessed - 1;
		if (frames < 0) frames = 0;
		int seconds = frames / fps;
		int minutes = seconds / 60;
		int hours = minutes / 60;
		
		frames = frames % fps;
		seconds = seconds % 60;
		minutes = minutes % 60;
		
		QTime time(hours, minutes, seconds);
		QLocale conv;
		QString strFF = conv.toString(frames);
		switch (strFF.length())
		{
		case 1:
			strFF = "0" + strFF;
			break;
		case 2:
			strFF = strFF;
			break;
		default:
			break;
		}
		QString strElapsedTime = time.toString() + "." + strFF;
		return strElapsedTime;
	}
};

struct CameraParameters
{
	int   m_cameraNumber;
	int   m_lensType; // 0:normal, 1:fisheye
	float m_yaw;
	float m_pitch;
	float m_roll;
	float m_fov;
	float m_k1;
	float m_k2;
	float m_k3;
	float m_offset_x;
	float m_offset_y;
	float m_expOffset;
	float m_xrad1; // feathering crop (0.0:none~1.0:all)
	float m_xrad2;
	float m_yrad1;
	float m_yrad2;

	/*float m_focal35mm; // not in use, since it's initial focal estimate
	float m_fisheyeCoffX; 
	float m_fisheyeCoffY;
	float m_yaw;
	float m_pitch;
	float m_roll;
	float m_f;
	float m_k1;
	float m_k2;
	float m_k3;
	float m_offset_x;
	float m_offset_y;
	float m_fisheye_radius;
	float m_ccdwidthinMM;
	float m_ccdheightinMM;*/

	CameraParameters()
	{
		//m_ccdwidthinMM  = 11.27; // Based on Ximea xiQ USB Vision Camera - other cameras might vary
		//m_ccdheightinMM = 11.27;
		m_lensType = 1;		// Default is fisheye lens
		m_fov = 240;
		m_expOffset = 0.0f;
		m_xrad1 = m_xrad2 = 0.9f;
		m_yrad1 = m_yrad2 = 0.9f;
		m_yaw = 0.0f;
		m_pitch = 0.0f;
		m_roll = 0.0f;
		m_k1 = m_k2 = m_k3 = 0;
		m_offset_x = m_offset_y = 0;
	}
};


struct CameraInput
{
	CameraInput()
	{
		audioType = NoAudio;
		exposure = 0.0f;
		xres = 1280;
		yres = 960;
		fps  = -1.0f;
		playbackfps = 30.0f;
		fileExt = "mp4";

		stereoType = PanoramaStereoType::Panorama_Mono;
	}

	QString getStereoTypeText() {
		QString typeText = "";
		switch (stereoType)
		{
		case PanoramaStereoType::Panorama_LeftEye:
			typeText = "Left";
			break;
		case PanoramaStereoType::Panorama_RightEye:
			typeText = "Right";
			break;
		case PanoramaStereoType::Panorama_BothEye:
			typeText = "Both";
			break;
		case PanoramaStereoType::Panorama_Mono:
		default:
			typeText = "Mono";
			break;
		}
		return typeText;
	}

	enum PanoramaStereoType
	{
		Panorama_Mono,
		Panorama_LeftEye,
		Panorama_RightEye,
		Panorama_BothEye
	};

	enum InputAudioChannelType
	{
		MixedChannel,
		LeftChannel,
		RightChannel,
		NoAudio
	};

	bool isExistAudio() { return audioName.isEmpty() || audioName.trimmed() == "" ? false : true; }
	
	QString name;
	PanoramaStereoType stereoType;
	QString audioName;
	InputAudioChannelType audioType;
	QString fileDir;			// Input file Dir
	QString filePrefix;			// Input file Prefix
	QString fileExt;			// Input file ext

	float		exposure;			// Capture camera exposure setting
	int			xres;				// Capture X res
	int			yres;				// Capture Y res
	float		fps;				// Capture fps
	float       playbackfps;        // Playback fps

	bool		valid;

	CameraParameters m_cameraParams; // Camera information from the files
};

typedef unsigned char byte;
typedef struct PanoBuffer {
	byte* bufferPtr;
	uint size;
} PANO_BUFFER, *PANO_BUFFER_PTR;
#endif // STRUCTURES_H
