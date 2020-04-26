
#include <iostream>
#include <sstream>

#include <QSettings>
#include <QDebug>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QFile>
#include <QDir>
#include <qlocale.h>
#include <qcoreapplication.h>
#include "D360Parser.h"
#include "ImageHandler.h"
#include "ptsparser.h"
#include "define.h"

#define INI_OPT

////////////
QString getPanoramaStereoTypeStringFromType(CameraInput::PanoramaStereoType panoramaStereoType)
{
	if (panoramaStereoType == CameraInput::Panorama_LeftEye)
	{
		return "left";
	}
	else if (panoramaStereoType == CameraInput::Panorama_RightEye)
	{
		return "right";
	}
	else if (panoramaStereoType == CameraInput::Panorama_BothEye)
	{
		return "both";
	}
	else
	{
		return "none";
	}
}

CameraInput::PanoramaStereoType getPanoramaStereoTypeFromString(QString panoramaStereoTypeString)
{
	if (panoramaStereoTypeString == "left" || panoramaStereoTypeString == "Left")
		return CameraInput::PanoramaStereoType::Panorama_LeftEye;
	else if (panoramaStereoTypeString == "right" || panoramaStereoTypeString == "Right")
		return CameraInput::PanoramaStereoType::Panorama_RightEye;
	else if (panoramaStereoTypeString == "both" || panoramaStereoTypeString == "Both")
		return CameraInput::PanoramaStereoType::Panorama_BothEye;
	else
		return CameraInput::PanoramaStereoType::Panorama_Mono;
}

QString getCaptureModeStringFromType(D360::Capture::CaptureDomain capType)
{
	if (capType == D360::Capture::CAPTURE_DSHOW)
		return "dshow";
	else if (capType == D360::Capture::CAPTURE_XIMEA)
		return "ximea";
	else if (capType == D360::Capture::CAPTURE_VIDEO)
		return "video";
	else if (capType == D360::Capture::CAPTURE_FILE)
		return "sequence";
	return "video";
}

D360::Capture::CaptureDomain getCaptureModeFromString(QString capTypeString)
{
	if (capTypeString == "dshow")
		return D360::Capture::CAPTURE_DSHOW;
	else if (capTypeString == "ximea")
		return D360::Capture::CAPTURE_XIMEA;
	else if (capTypeString == "video")
		return D360::Capture::CAPTURE_VIDEO;
	else // if (capTypeString == "sequence")
		return D360::Capture::CAPTURE_FILE;
	return D360::Capture::CAPTURE_VIDEO;
}

QString getAudioChannelTypeStringFromType(CameraInput::InputAudioChannelType audioType)
{
	if (audioType == CameraInput::InputAudioChannelType::MixedChannel)
	{
		return "mixed";
	}
	else if (audioType == CameraInput::InputAudioChannelType::LeftChannel)
	{
		return "left";
	}
	else if (audioType == CameraInput::InputAudioChannelType::RightChannel)
	{
		return "right";
	}
	else
	{
		return "none";
	}
}

CameraInput::InputAudioChannelType getAudioChannelTypeFromString(QString audioTypeString)
{
	if (audioTypeString == "Mixed" || audioTypeString == "mixed")
	{
		return CameraInput::InputAudioChannelType::MixedChannel;
	}
	else if (audioTypeString == "Left" || audioTypeString == "left")
	{
		return CameraInput::InputAudioChannelType::LeftChannel;
	}
	else if (audioTypeString == "Right" || audioTypeString == "right")
	{
		return CameraInput::InputAudioChannelType::RightChannel;
	}
	else
	{
		return CameraInput::InputAudioChannelType::NoAudio;
	}
}

QString getBlendingModeStringFromType(GlobalAnimSettings::BlendingMode blendingMode)
{
	if (blendingMode == GlobalAnimSettings::Feathering)
	{
		return "feathering";
	}
	else if (blendingMode == GlobalAnimSettings::MultiBandBlending)
	{
		return "multiband";
	}
	return "multiband";
}

GlobalAnimSettings::BlendingMode getBlendingModeTypeFromString(QString blendingModeString)
{
	if (blendingModeString == "feathering")
	{
		return GlobalAnimSettings::Feathering;
	}
	else if (blendingModeString == "multiband")
	{
		return GlobalAnimSettings::MultiBandBlending;
	}
	
	return GlobalAnimSettings::MultiBandBlending;
}

QString getVideoCodecStringFromType(int videoCodec)
{
	if (videoCodec == AV_CODEC_ID_H264)
	{
		return "H.264";
	}
	else if (videoCodec == AV_CODEC_ID_H265)
	{
		return "H.265";
	}
	return "H.264";
}

int getVideoCodecTypeFromString(QString videoCodecString)
{
	if (videoCodecString.toUpper() == "H.264")
		return (int)AV_CODEC_ID_H264;
	else if (videoCodecString.toUpper() == "H.265")
		return (int)AV_CODEC_ID_H265;
	
	return (int)AV_CODEC_ID_H264;
}

QString getAudioCodecStringFromType(int audioCodec)
{
	if (audioCodec == AV_CODEC_ID_AAC)
	{
		return "AAC";
	}
	return "AAC";
}

int getAudioCodecTypeFromString(QString audioCodecString)
{
	if (audioCodecString.toUpper() == "AAC")
		return (int)AV_CODEC_ID_AAC;
	
	return (int)AV_CODEC_ID_AAC;
}

////////////

GlobalAnimSettings& GlobalAnimSettings::operator = (const GlobalAnimSettings& other)
{
	try
	{
		m_cameraCount = other.m_cameraCount;

		m_startFrame = other.m_startFrame;
		m_endFrame = other.m_endFrame;
		m_fileVersion = other.m_fileVersion;
		m_fps = other.m_fps;
		m_fYaw = other.m_fYaw;
		m_fPitch = other.m_fPitch;
		m_fRoll = other.m_fRoll;
		m_fInteractYaw = other.m_fInteractYaw;
		m_fInteractPitch = other.m_fInteractPitch;
		m_fInteractRoll = other.m_fInteractRoll;


		m_leftIndices = other.m_leftIndices;
		m_rightIndices = other.m_rightIndices;

		m_xres = other.m_xres;
		m_yres = other.m_yres;
		m_panoXRes = other.m_panoXRes;
		m_panoYRes = other.m_panoYRes;
		m_audioLag = other.m_audioLag;
		m_sampleFmt = other.m_sampleFmt;
		m_sampleRate = other.m_sampleRate;

		m_playbackfps = other.m_playbackfps;

		m_captureType = other.m_captureType;

		m_stitch = other.m_stitch;
		m_oculus = other.m_oculus;

		m_ui = other.m_ui;

		m_snapshotDir = other.m_snapshotDir;

		m_audioDeviceName = other.m_audioDeviceName;

		m_cameraCalibFile = other.m_cameraCalibFile;
		m_blendingMode = other.m_blendingMode;
		m_multiBandLevel = other.m_multiBandLevel;

		m_offlineVideo = other.m_offlineVideo;
		m_videoCodec = other.m_videoCodec;
		m_audioCodec = other.m_audioCodec;
		m_wowzaServer = other.m_wowzaServer;

		m_cameraSettings = other.m_cameraSettings;

		m_lensType = other.m_lensType;
		m_fov = other.m_fov;
		m_crf = other.m_crf;

		m_isAdv = other.m_isAdv;
		m_advFile = other.m_advFile;
		m_isVideoAdv = other.m_isVideoAdv;
	}
	catch (...)
	{

	}


	return *this;
}

D360Parser::D360Parser(QObject* parent)
	: QObject(parent)
{
}


D360Parser::~D360Parser()
{
}

void D360Parser::clear()
{
	m_globalAnimSettings = GlobalAnimSettings();
}

//
// The INI file contains descriptions of the events and components, first parse this 
// and make the appropriate database entries needed as parameters and 
// for the config
//
#define GET_CAM_PARAM(settings, name, param)	{ \
				if (childKey == name) { \
					float tempVal = settings.value(name).toFloat(); \
					param = tempVal; \
				} \
			}

bool D360Parser::parseINI(QString iniFileName)
{
	if (!QFile::exists(iniFileName))
		return false;
	int numCameras = 0;
	m_globalAnimSettings.setDefault();

	QSettings settings(iniFileName, QSettings::IniFormat);	
	{
		settings.beginGroup("D360");
		const QStringList childKeys = settings.childKeys();
		foreach(const QString &childKey, childKeys)
		{
			if (childKey == "version")
			{
				m_globalAnimSettings.m_fileVersion = settings.value(childKey).toFloat();
			}
		}
		settings.endGroup();
	}
	if (m_globalAnimSettings.m_fileVersion < 2.0f)
	{
		return false;
	}
	{
		settings.beginGroup("CameraCapture");
		const QStringList childKeys = settings.childKeys();

		foreach(const QString &childKey, childKeys)
		{
			if (childKey == "Cnt")
			{
				numCameras = settings.value(childKey).toInt();
				m_globalAnimSettings.m_cameraCount = numCameras;
				//qDebug() << numCameras;
			}
			if (childKey == "xres")
			{
				int xres = settings.value(childKey).toInt();
				m_globalAnimSettings.m_xres = xres;
			}
			if (childKey == "yres")
			{
				int yres = settings.value(childKey).toInt();
				m_globalAnimSettings.m_yres = yres;
			}

			if (childKey == "fps")
			{
				float fps = settings.value(childKey).toFloat();
				m_globalAnimSettings.m_fps = fps;
			}
			if (childKey == "playbackfps")
			{
				float fps = settings.value(childKey).toFloat();
				m_globalAnimSettings.m_playbackfps = fps;
			}
			if (childKey == "captureType")
			{
				QString capType = settings.value(childKey).toString();
				m_globalAnimSettings.m_captureType = getCaptureModeFromString(capType);
			}

			if (childKey == "snapshotDir")
			{
				m_globalAnimSettings.m_snapshotDir = settings.value(childKey).toString();
			}

			if (childKey == "yaw")
			{
				float yaw = settings.value(childKey).toFloat();
				m_globalAnimSettings.m_fYaw = yaw;
			}
			if (childKey == "pitch")
			{
				float pitch = settings.value(childKey).toFloat();
				m_globalAnimSettings.m_fPitch = pitch;
			}
			if (childKey == "roll")
			{
				float roll = settings.value(childKey).toFloat();
				m_globalAnimSettings.m_fRoll = roll;
			}
			if (childKey == "interact_yaw")
			{
				float yaw = settings.value(childKey).toFloat();
				m_globalAnimSettings.m_fInteractYaw = yaw;
			}
			if (childKey == "interact_pitch")
			{
				float pitch = settings.value(childKey).toFloat();
				m_globalAnimSettings.m_fInteractPitch = pitch;
			}
			if (childKey == "interact_roll")
			{
				float roll = settings.value(childKey).toFloat();
				m_globalAnimSettings.m_fInteractRoll = roll;
			}
			if (childKey == "ui")
			{
				bool ui = settings.value(childKey).toBool();
				m_globalAnimSettings.m_ui = ui;
			}
		}
		settings.endGroup();
	}

	GlobalAnimSettings::CameraSettingsList& cameraSettings = m_globalAnimSettings.cameraSettingsList();
	cameraSettings.resize(numCameras);
	// StitchSettings
	{
		settings.beginGroup("StitchSettings");
		const QStringList childKeys = settings.childKeys();

		foreach(const QString &childKey, childKeys)
		{
			if (childKey == "cameraCalibFile")
			{
				QString calibFile = settings.value(childKey).toString(); // .toUtf8().constData();
				m_globalAnimSettings.m_cameraCalibFile = calibFile;

				parseCameraCalibrationFile(m_globalAnimSettings.m_cameraCalibFile, cameraSettings);
			}
			if (childKey == "blendingMode")
			{
				QString blendingMode = settings.value(childKey).toString();;
				m_globalAnimSettings.m_blendingMode = getBlendingModeTypeFromString(blendingMode);
			}
			if (childKey == "multiBandLevel")
			{
				int blendLevel = settings.value(childKey).toInt();
				m_globalAnimSettings.m_multiBandLevel = blendLevel;
			}
			if (childKey == "panoXRes")
			{
				int panoXRes = settings.value(childKey).toInt();
				m_globalAnimSettings.m_panoXRes = panoXRes;
			}
			if (childKey == "panoYRes")
			{
				int panoYRes = settings.value(childKey).toInt();
				m_globalAnimSettings.m_panoYRes = panoYRes;
			}
		}
		settings.endGroup();
	}

	{
		for (unsigned i = 0; i < numCameras; ++i)
		{
			std::stringstream camName;
			camName << "Camera" << i + 1;
			//std::cout << "Camera Name : " << camName.str().c_str() << std::endl;

			settings.beginGroup(camName.str().c_str());
			const QStringList childKeys = settings.childKeys();
			foreach(const QString &childKey, childKeys)
			{
				if (childKey == "name")
				{
					//std::string camName = settings.value( childKey ).toString().toStdString();
					QString camName = settings.value(childKey).toString();
					cameraSettings[i].name = camName;
				}
				if (childKey == "exposure")
				{
					float exposure = settings.value(childKey).toFloat();
					cameraSettings[i].exposure = exposure;
				}
				if (childKey == "fileDir")
				{
					//std::string dir = settings.value( childKey ).toString().toStdString();
					QString dir = settings.value(childKey).toString();
					cameraSettings[i].fileDir = dir;
				}
				if (childKey == "filePrefix")
				{
					//std::string prefix = settings.value( childKey ).toString().toStdString();
					QString prefix = settings.value(childKey).toString();
					cameraSettings[i].filePrefix = prefix;
				}
				if (childKey == "fileExt")
				{
					//std::string ext = settings.value( childKey ).toString().toStdString();
					QString ext = settings.value(childKey).toString();
					cameraSettings[i].fileExt = ext;
				}

				if (childKey == "xres")
				{
					int xres = settings.value(childKey).toInt();
					cameraSettings[i].xres = xres;
				}
				if (childKey == "yres")
				{
					int yres = settings.value(childKey).toInt();
					cameraSettings[i].yres = yres;
				}
				if (childKey == "fps")
				{
					float fps = settings.value(childKey).toFloat();
					cameraSettings[i].fps = fps;
				}
				if (childKey == "stereoType")
				{
					//std::string stereoType = settings.value( childKey ).toString().toStdString();
					QString stereoType = settings.value(childKey).toString();
					cameraSettings[i].stereoType = getPanoramaStereoTypeFromString(stereoType);
				}
				if (childKey == "audioName")
				{
					QString name = settings.value(childKey).toString();
					cameraSettings[i].audioName = name;
				}
				if (childKey == "audioType")
				{
					QString type = settings.value(childKey).toString();
					cameraSettings[i].audioType = getAudioChannelTypeFromString(type);
				}
				if (childKey == "expOffset")
				{
					float expOffset = settings.value(childKey).toFloat();
					cameraSettings[i].exposure = expOffset;
				}
				if (childKey == "xrad1")
				{
					float xrad1 = settings.value(childKey).toFloat();
					cameraSettings[i].m_cameraParams.m_xrad1 = xrad1;
				}
				if (childKey == "xrad2")
				{
					float xrad2 = settings.value(childKey).toFloat();
					cameraSettings[i].m_cameraParams.m_xrad2 = xrad2;
				}
				if (childKey == "yrad1")
				{
					float yrad1 = settings.value(childKey).toFloat();
					cameraSettings[i].m_cameraParams.m_yrad1 = yrad1;
				}
				if (childKey == "yrad2")
				{
					float yrad2 = settings.value(childKey).toFloat();
					cameraSettings[i].m_cameraParams.m_yrad2 = yrad2;
				}
				
				// Read calibration params
				GET_CAM_PARAM(settings, "lensType", cameraSettings[i].m_cameraParams.m_lensType);
				GET_CAM_PARAM(settings, "yaw", cameraSettings[i].m_cameraParams.m_yaw);
				GET_CAM_PARAM(settings, "pitch", cameraSettings[i].m_cameraParams.m_pitch);
				GET_CAM_PARAM(settings, "roll", cameraSettings[i].m_cameraParams.m_roll);
				GET_CAM_PARAM(settings, "fov", cameraSettings[i].m_cameraParams.m_fov);
				GET_CAM_PARAM(settings, "k1", cameraSettings[i].m_cameraParams.m_k1);
				GET_CAM_PARAM(settings, "k2", cameraSettings[i].m_cameraParams.m_k2);
				GET_CAM_PARAM(settings, "k3", cameraSettings[i].m_cameraParams.m_k3);
				GET_CAM_PARAM(settings, "offset_x", cameraSettings[i].m_cameraParams.m_offset_x);
				GET_CAM_PARAM(settings, "offset_y", cameraSettings[i].m_cameraParams.m_offset_y);
			}
			settings.endGroup();
			/*
			qDebug() << m_cameraSettings[i].name.c_str() << " " << m_cameraSettings[i].exposure << " "
			<< m_cameraSettings[i].fileDir.c_str() << " " << m_cameraSettings[i].filePrefix.c_str() << " " << m_cameraSettings[i].fileExt.c_str() ;
			*/
		}
		for (unsigned i = 0; i < numCameras; ++i)
		{
			std::stringstream camName;

			if (cameraSettings[i].xres < 0)
			{
				cameraSettings[i].xres = m_globalAnimSettings.m_xres;
			}
			if (cameraSettings[i].yres < 0)
				cameraSettings[i].yres = m_globalAnimSettings.m_yres;
			//if (cameraSettings[i].exposure < 0)
			//{
			//	cameraSettings[i].exposure = m_globalAnimSettings.m_exp;
			//}
			//if (cameraSettings[i].gain < 0)
			//{
			//	cameraSettings[i].gain = m_globalAnimSettings.m_gain;
			//}
			if (cameraSettings[i].fps < 0)
				cameraSettings[i].fps = m_globalAnimSettings.m_fps;

			if (cameraSettings[i].playbackfps < 0)
				cameraSettings[i].fps = m_globalAnimSettings.m_fps;

			if (cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_LeftEye || cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_BothEye)
			{
				m_globalAnimSettings.getLeftIndices().push_back(i);
			}
			if (cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_RightEye || cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_BothEye)
			{
				m_globalAnimSettings.getRightIndices().push_back(i);
			}
		}
	}


	// Audio
	{
		settings.beginGroup("Audio");
		const QStringList childKeys = settings.childKeys();

		foreach(const QString &childKey, childKeys)
		{
			if (childKey == "deviceName")
			{
				QString deviceName = settings.value(childKey).toString();
				m_globalAnimSettings.m_audioDeviceName = deviceName;
			}
			else if (childKey == "audioLag")
			{
				int audioLag = settings.value(childKey).toInt();
				m_globalAnimSettings.m_audioLag = audioLag;
			}
			else if (childKey == "sampleFmt")
			{
				int sampleFmt = settings.value(childKey).toInt();
				m_globalAnimSettings.m_sampleFmt = sampleFmt;
			}
			else if (childKey == "sampleRate")
			{
				int sampleRate = settings.value(childKey).toInt();
				m_globalAnimSettings.m_sampleRate = sampleRate;
			}
		}
		settings.endGroup();

		//qDebug() << "Audio device: " << m_globalAnimSettings.m_audioDeviceName << " , Lag:" << m_globalAnimSettings.m_audioLag;
	}

	// OculusRift
	{
		settings.beginGroup("OculusRift");
		const QStringList childKeys = settings.childKeys();

		foreach(const QString &childKey, childKeys)
		{
			if (childKey == "DK2")
			{
				QString dk2Flag = settings.value(childKey).toString();
				if (dk2Flag == "On")
					m_globalAnimSettings.m_oculus = true;
			}
		}
		settings.endGroup();
	}

	// Streaming
	{
		settings.beginGroup("Streaming");
		const QStringList childKeys = settings.childKeys();

		foreach(const QString &childKey, childKeys)
		{
			if (childKey == "oculusRift")
			{
				QString dk2Flag = settings.value(childKey).toString();
				if (dk2Flag == "On")
					m_globalAnimSettings.m_oculus = true;
				else
					m_globalAnimSettings.m_oculus = false;
			}
			if (childKey == "offlineVideo")
			{
				QString videoFilename = settings.value(childKey).toString();
				m_globalAnimSettings.m_offlineVideo = videoFilename;
			}
			if (childKey == "server")
			{
				QString serverAddress = settings.value(childKey).toString();
				m_globalAnimSettings.m_wowzaServer = serverAddress;
			}
			if (childKey == "videoCodec")
			{
				QString videoCodec = settings.value(childKey).toString();
				m_globalAnimSettings.m_videoCodec = getVideoCodecTypeFromString(videoCodec);
			}
			if (childKey == "audioCodec")
			{
				QString audioCodec = settings.value(childKey).toString();
				m_globalAnimSettings.m_audioCodec = getAudioChannelTypeFromString(audioCodec);
			}
			if (childKey == "gpu")
			{
				bool gpu = settings.value(childKey).toBool();
				m_globalAnimSettings.m_isGPU = gpu;
			}
			if (childKey == "quality")
			{
				int crf = settings.value(childKey).toInt();
				m_globalAnimSettings.m_crf = crf;
			}
		}
		settings.endGroup();
	}

	// Timeline
	{
		settings.beginGroup("Timeline");
		const QStringList childKeys = settings.childKeys();

		if (m_globalAnimSettings.m_captureType == D360::Capture::CAPTURE_FILE)
		{
			// Use timeline for sequence mode only.
			foreach(const QString &childKey, childKeys)
			{
				if (childKey == "Start")
					m_globalAnimSettings.m_startFrame = settings.value(childKey).toInt();
				else if (childKey == "End")
					m_globalAnimSettings.m_endFrame = settings.value(childKey).toInt();
			}
		}
		settings.endGroup();
	}

	if (numCameras > 0)		// Set global lens type
		m_globalAnimSettings.m_lensType = cameraSettings[0].m_cameraParams.m_lensType;

	return true;
}

#define SET_CAM_PARAM(settings, name, value)	{ \
						if (value != 0.0f) { \
							settings.setValue(QLatin1String(name), QString::number(value)); \
						} \
					}


void D360Parser::saveINI(QString  iniFileName)
{	
	QLocale conv;
	if (m_globalAnimSettings.m_cameraCount == 0)
		return;
	if (QFile(iniFileName).exists())
		QFile(iniFileName).remove();
	QSettings settings(iniFileName, QSettings::IniFormat);
	
	{
		// [D360]
		settings.beginGroup("D360");
		if (m_globalAnimSettings.m_fileVersion >= 0)
		{
			settings.setValue(QLatin1String("version"), conv.toString(m_globalAnimSettings.m_fileVersion));
		}
		settings.endGroup();

		// [CameraCapture]
		settings.beginGroup("CameraCapture");
		settings.setValue(QLatin1String("Cnt"), m_globalAnimSettings.m_cameraCount);
		settings.setValue(QLatin1String("fps"), conv.toString(m_globalAnimSettings.m_fps));
		settings.setValue(QLatin1String("xres"), m_globalAnimSettings.m_xres);
		settings.setValue(QLatin1String("yres"), m_globalAnimSettings.m_yres);
#ifndef INI_OPT
		if (m_globalAnimSettings.m_playbackfps >= 0)
		{
			settings.setValue(QLatin1String("playbackfps"), conv.toString(m_globalAnimSettings.m_playbackfps));
			//qDebug() << "playbackfps of the iniFile = " << conv.toString(m_globalAnimSettings.m_playbackfps);
		}
#endif
		
		settings.setValue(QLatin1String("captureType"), getCaptureModeStringFromType((D360::Capture::CaptureDomain)m_globalAnimSettings.m_captureType));

		if (m_globalAnimSettings.m_snapshotDir != "")
		{
			settings.setValue(QLatin1String("snapshotDir"), m_globalAnimSettings.m_snapshotDir);
		}
		
		settings.setValue(QLatin1String("yaw"), conv.toString(m_globalAnimSettings.m_fYaw));
		settings.setValue(QLatin1String("pitch"), conv.toString(m_globalAnimSettings.m_fPitch));
		settings.setValue(QLatin1String("roll"), conv.toString(m_globalAnimSettings.m_fRoll));
		settings.setValue(QLatin1String("interact_yaw"), conv.toString(m_globalAnimSettings.m_fInteractYaw));
		settings.setValue(QLatin1String("interact_pitch"), conv.toString(m_globalAnimSettings.m_fInteractPitch));
		settings.setValue(QLatin1String("interact_roll"), conv.toString(m_globalAnimSettings.m_fInteractRoll));

#ifndef INI_OPT
		if (m_globalAnimSettings.m_ui)
		{
			settings.setValue(QLatin1String("ui"), m_globalAnimSettings.m_ui);
		}
#endif
		
		settings.endGroup();
	}

	// [StitchSettings]
	{
		settings.beginGroup("StitchSettings");
		if (m_globalAnimSettings.m_cameraCalibFile != "")
		{
			settings.setValue(QLatin1String("cameraCalibFile"), m_globalAnimSettings.m_cameraCalibFile);
		}
		settings.setValue(QLatin1String("blendingMode"), getBlendingModeStringFromType(m_globalAnimSettings.m_blendingMode));
		settings.setValue(QLatin1String("multiBandLevel"), m_globalAnimSettings.m_multiBandLevel);
		settings.setValue(QLatin1String("panoXRes"), m_globalAnimSettings.m_panoXRes);
		settings.setValue(QLatin1String("panoYRes"), m_globalAnimSettings.m_panoYRes);
		settings.endGroup();
	}

	// [Camera#n]
	{
		GlobalAnimSettings::CameraSettingsList& cameraSettings = m_globalAnimSettings.cameraSettingsList();
		cameraSettings.resize(m_globalAnimSettings.m_cameraCount);
		//parsePAC(m_globalAnimSettings.m_cameraCalibFile, cameraSettings);

		for (unsigned i = 0; i < m_globalAnimSettings.m_cameraCount; ++i)
		{
			std::stringstream camName;
			camName << "Camera" << i + 1;
			std::cout << "Camera Name : " << camName.str().c_str() << std::endl;

			settings.beginGroup(camName.str().c_str());
			if (cameraSettings[i].name != "")
			{
				settings.setValue(QLatin1String("name"), cameraSettings[i].name);
			}
			if (cameraSettings[i].fileDir != "")
			{
				settings.setValue(QLatin1String("fileDir"), cameraSettings[i].fileDir);
			}
			if (cameraSettings[i].filePrefix != "")
			{
				settings.setValue(QLatin1String("filePrefix"), cameraSettings[i].filePrefix);
			}
			if (cameraSettings[i].fileExt != "")
			{
				settings.setValue(QLatin1String("fileExt"), cameraSettings[i].fileExt);
			}
			/*if (cameraSettings[i].xres)
			{
				settings.setValue(QLatin1String("xres"), cameraSettings[i].xres);
			}
			if (cameraSettings[i].yres)
			{
				settings.setValue(QLatin1String("yres"), cameraSettings[i].yres);
			}
			if (cameraSettings[i].fps >= 0)
			{
				settings.setValue(QLatin1String("fps"), conv.toString(cameraSettings[i].fps));
			}*/

			settings.setValue(QLatin1String("stereoType"), getPanoramaStereoTypeStringFromType(cameraSettings[i].stereoType));

			if (cameraSettings[i].audioName != "")
			{
				settings.setValue(QLatin1String("audioName"), cameraSettings[i].audioName);
			}
			settings.setValue(QLatin1String("audioType"), getAudioChannelTypeStringFromType(cameraSettings[i].audioType));
			if (cameraSettings[i].exposure != 0.0f)
			{
				settings.setValue(QLatin1String("expOffset"), QString::number(cameraSettings[i].exposure));
			}
			if (cameraSettings[i].exposure >= 0)
			{
				settings.setValue(QLatin1String("exposure"), conv.toString(cameraSettings[i].exposure));
			}
			settings.setValue(QLatin1String("xrad1"), QString::number(cameraSettings[i].m_cameraParams.m_xrad1));
			settings.setValue(QLatin1String("xrad2"), QString::number(cameraSettings[i].m_cameraParams.m_xrad2));
			settings.setValue(QLatin1String("yrad1"), QString::number(cameraSettings[i].m_cameraParams.m_yrad1));
			settings.setValue(QLatin1String("yrad2"), QString::number(cameraSettings[i].m_cameraParams.m_yrad2));

			// Save calibration params
			SET_CAM_PARAM(settings, "lensType", cameraSettings[i].m_cameraParams.m_lensType);
			SET_CAM_PARAM(settings, "yaw", cameraSettings[i].m_cameraParams.m_yaw);
			SET_CAM_PARAM(settings, "pitch", cameraSettings[i].m_cameraParams.m_pitch);
			SET_CAM_PARAM(settings, "roll", cameraSettings[i].m_cameraParams.m_roll);
			SET_CAM_PARAM(settings, "fov", cameraSettings[i].m_cameraParams.m_fov);
			SET_CAM_PARAM(settings, "k1", cameraSettings[i].m_cameraParams.m_k1);
			SET_CAM_PARAM(settings, "k2", cameraSettings[i].m_cameraParams.m_k2);
			SET_CAM_PARAM(settings, "k3", cameraSettings[i].m_cameraParams.m_k3);
			SET_CAM_PARAM(settings, "offset_x", cameraSettings[i].m_cameraParams.m_offset_x);
			SET_CAM_PARAM(settings, "offset_y", cameraSettings[i].m_cameraParams.m_offset_y);

			settings.endGroup();
		}
	}

	// [Audio]
	{
		settings.beginGroup("Audio");
		if (m_globalAnimSettings.m_audioDeviceName != "")
		{
			settings.setValue(QLatin1String("deviceName"), m_globalAnimSettings.m_audioDeviceName);
		}
		if (m_globalAnimSettings.m_audioLag != 0)
		{
			settings.setValue(QLatin1String("audioLag"), m_globalAnimSettings.m_audioLag);
		}
		settings.setValue(QLatin1String("sampleFmt"), m_globalAnimSettings.m_sampleFmt);
		settings.setValue(QLatin1String("sampleRate"), m_globalAnimSettings.m_sampleRate);
		settings.endGroup();
	}

	// [Streaming]
	{
		settings.beginGroup("Streaming");
		if (m_globalAnimSettings.m_oculus == true)
		{
			settings.setValue(QLatin1String("OculusRift"), "On");
		}
		else
		{
			settings.setValue(QLatin1String("OculusRift"), "Off");
		}
		if (m_globalAnimSettings.m_offlineVideo != "")
		{
			settings.setValue(QLatin1String("offlineVideo"), m_globalAnimSettings.m_offlineVideo);
		}
		if (m_globalAnimSettings.m_wowzaServer != "")
		{
			settings.setValue(QLatin1String("server"), m_globalAnimSettings.m_wowzaServer);
		}
		settings.setValue(QLatin1String("videoCodec"), getVideoCodecStringFromType(m_globalAnimSettings.m_videoCodec));
		settings.setValue(QLatin1String("audioCodec"), getAudioCodecStringFromType(m_globalAnimSettings.m_audioCodec));
		settings.setValue(QLatin1String("quality"), m_globalAnimSettings.m_crf);
		settings.endGroup();
	}

	// Timeline
	{
		settings.beginGroup("Timeline");
		settings.setValue(QLatin1String("Start"), m_globalAnimSettings.m_startFrame);
		settings.setValue(QLatin1String("End"), m_globalAnimSettings.m_endFrame);
		settings.endGroup();
	}
}

int D360Parser::parseCameraCalibrationFile(QString fileName, GlobalAnimSettings::CameraSettingsList& cameraSettings)
{
	if (fileName.right(3).toLower() == "pac")
		return parsePAC(fileName, cameraSettings);
	else if (fileName.right(3).toLower() == "pts")
		return parsePTS(fileName, cameraSettings);
	return 0;
}

int D360Parser::parsePTS(QString fileName, GlobalAnimSettings::CameraSettingsList& cameraSettings)
{
	PTSParser ptsParser;
	QMap<QString, PTSVAL> paramMap = ptsParser.parsePTS(fileName);

	if (paramMap.size() == 0) {
		PANO_N_WARN("PTS file parsing failed! (Can be unsupported file.)");
		return 0;
	}

	PANO_N_WARN("PTS file parsing failed! (Can be unsupported file.)");

	// Initialize PAC and PTS map
	QMap<QString, QString>  pacPTS;
	pacPTS["lensType"]	= "f";
	pacPTS["yaw"]		= "y";
	pacPTS["pitch"]		= "p";
	pacPTS["roll"]		= "r";
	pacPTS["fov"]		= "v";
	pacPTS["k1"]		= "a";
	pacPTS["k2"]		= "b";
	pacPTS["k3"]		= "c";
	pacPTS["offset_x"]	= "d";
	pacPTS["offset_y"]	= "e";
	pacPTS["expOffset"] = "exposureparams";

	int devIndex = 0;
	while (paramMap.contains(QString("%1%2").arg(IMAGE_KEY).arg(QString::number(devIndex)))) {
		PTSVAL entry = paramMap[QString("%1%2").arg(IMAGE_KEY).arg(QString::number(devIndex))];
		//qDebug() << endl << entry.value;
		QMap<QString, float> params = entry.camParams;
		QMap<QString, float> shareParams = paramMap[SHARE_KEY].camParams;
		QMapIterator<QString, QString> i(pacPTS);
		while (i.hasNext()) {
			i.next();
			QString key = i.value();
			float value = 0;
			if (shareParams.contains(key)) {
				value = shareParams[key];
			}
			if ((shareParams.contains(key) && value == 0) && params.contains(key)) {
				value = params[key];
			}
			if (!shareParams.contains(key) && !params.contains(key)) {
				if (key == EXPOSURE_VAL && paramMap.contains(EXPOSURE_KEY + QString::number(devIndex))) {
					QString expOffsetStr = paramMap[EXPOSURE_KEY + QString::number(devIndex)].value.at(0);
					value = expOffsetStr.toFloat();
				}
			}
			if (i.key() == "lensType")
				cameraSettings[devIndex].m_cameraParams.m_lensType = (value == 3 ? 1 : value);
			else if (i.key() == "yaw")
				cameraSettings[devIndex].m_cameraParams.m_yaw = value;
			else if (i.key() == "pitch")
				cameraSettings[devIndex].m_cameraParams.m_pitch = value;
			else if (i.key() == "roll")
				cameraSettings[devIndex].m_cameraParams.m_roll = value;
			else if (i.key() == "fov")
				cameraSettings[devIndex].m_cameraParams.m_fov = value;
			else if (i.key() == "k1")
				cameraSettings[devIndex].m_cameraParams.m_k1 = value;
			else if (i.key() == "k2")
				cameraSettings[devIndex].m_cameraParams.m_k2 = value;
			else if (i.key() == "k3")
				cameraSettings[devIndex].m_cameraParams.m_k3 = value;
			else if (i.key() == "offset_x")
				cameraSettings[devIndex].m_cameraParams.m_offset_x = value;
			else if (i.key() == "offset_y")
				cameraSettings[devIndex].m_cameraParams.m_offset_y = value;
			else if (i.key() == "expOffset")
			{
				cameraSettings[devIndex].m_cameraParams.m_expOffset = value;
				cameraSettings[devIndex].exposure = value;
			}
			//qDebug() << QString("%1 (%2) = %3").arg(i.key()).arg(key).arg(value);
		}
		devIndex++;
		if (devIndex >= cameraSettings.size())
			break;
	}

	return 0;
}

int D360Parser::parsePAC(QString fileName, GlobalAnimSettings::CameraSettingsList& cameraSettings)
{
	//
	// If camera calibration exists parse it and update the appropriate data structures
	// 
	QFile qfCalibFile(fileName);
	bool calibExists = qfCalibFile.exists();
	if (!calibExists)
	{
		qDebug() << "parsePAC: Camera calibration file of " << fileName << " is not found.";
		return 0;
	}

	// 
	// Only Read the following section in the XML file which is appropriate for camera
	// 
	/*
	<?xml version="1.0" encoding="UTF-8"?>
	<PanoOne>
	<images>
	<image>
	<camera model="1" yaw="141.106" pitch="0.1701" roll="90.2108" fov="237.573" k1="-0.2328" k2="0.564" k3="-0.4256" offset_x="-3.612" offset_y="-81.0957" />
	</image>
	<image>
	<camera model="1" yaw="57.5656" pitch="-9.799" roll="94.965" fov="240.25" k1="0.3084" k2="-0.4796" k3="0.1402" offset_x="75.1545" offset_y="-102.707" />
	</image>
	<image>
	<camera model="1" yaw="-37.1667" pitch="7.9932" roll="91.1348" fov="238.124" k1="0.2748" k2="-0.8921" k3="0.8506" offset_x="-38.1781" offset_y="-44.8615" />
	</image>
	<image>
	<camera model="1" yaw="-120.032" pitch="-5.1979" roll="83.5218" fov="235.01" k1="0.1559" k2="-0.4148" k3="0.3139" offset_x="18.5395" offset_y="-87.075" />
	</image>
	</images>
	</PanoOne>
	*/

	if (!qfCalibFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return 0;
	}
	//
	// QXmlStreamReader takes any QIODevice. 
	// 
	QXmlStreamReader xml(&qfCalibFile);
	QList< QMap< QString, QString> > images;

	//
	// We'll parse the XML until we reach end of it.
	// 

	int camIndex = -1;
	std::cout << "Reading PanoOne Camera XML(PAC) file " << std::endl;
	int numCamSettings = 0;
	while (!xml.atEnd() && !xml.hasError())
	{
		// Read next element
		//
		QXmlStreamReader::TokenType token = xml.readNext();

		//qDebug() << " string " << xml.name().toString() << " Token " << token ;

		//
		// If token is just StartDocument, we'll go to next
		//
		if (token == QXmlStreamReader::StartDocument)
		{
			continue;
		}
		//
		// If token is StartElement, we'll see if we can read it.
		//
		if (token == QXmlStreamReader::StartElement)
		{
			if (xml.name().toString() == "PanoOne")
			{
				qDebug() << "Got PanoOne";
				continue;
			}

			// If it's named images, we'll continue
			if (xml.name().toString() == "images")
			{
				if (token == QXmlStreamReader::StartElement)
				{
					//cameraSettings.resize( numCamSettings );
					while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name().toString() == "images"))
					{
						QXmlStreamReader::TokenType imagetoken = xml.readNext();
						// If its image - lets dig the information
						if (xml.name().toString() == "image" && xml.tokenType() == QXmlStreamReader::StartElement)
						{
							// Next element 
							// xml.readNext();
							// 
							// We're going to loop over the things because the order might change.
							// We'll continue the loop until we hit an EndElement named image.
							// 

							camIndex++;
							if (cameraSettings.size() <= camIndex) break;
							continue;
						}

						if (xml.name().toString() == "camera")
						{
							QXmlStreamAttributes attributes = xml.attributes();
							if (attributes.hasAttribute("lensType"))
							{
								int lensType = attributes.value("lensType").toString().toInt();

								cameraSettings[camIndex].m_cameraParams.m_lensType = lensType;
							}
							if (attributes.hasAttribute("yaw"))
							{
								//
								// Lets get that information and add it
								//
								float yaw = attributes.value("yaw").toString().toFloat();


								cameraSettings[camIndex].m_cameraParams.m_yaw = yaw;
							}
							if (attributes.hasAttribute("pitch"))
							{
								//
								// Lets get that information and add it
								//
								float pitch = attributes.value("pitch").toString().toFloat();
								cameraSettings[camIndex].m_cameraParams.m_pitch = pitch;
							}
							if (attributes.hasAttribute("roll"))
							{
								//
								// Lets get that information and add it
								//
								float roll = attributes.value("roll").toString().toFloat();
								cameraSettings[camIndex].m_cameraParams.m_roll = roll;
							}
							if (attributes.hasAttribute("fov"))
							{
								//
								// Lets get that information and add it
								//
								float fov = attributes.value("fov").toString().toUtf8().toFloat();
								cameraSettings[camIndex].m_cameraParams.m_fov = fov;
							}
							if (attributes.hasAttribute("k1"))
							{
								//
								// Lets get that information and add it
								//
								float k1 = attributes.value("k1").toString().toFloat();
								cameraSettings[camIndex].m_cameraParams.m_k1 = k1;
							}
							if (attributes.hasAttribute("k2"))
							{
								//
								// Lets get that information and add it
								//
								float k2 = attributes.value("k2").toString().toFloat();
								cameraSettings[camIndex].m_cameraParams.m_k2 = k2;

							}
							if (attributes.hasAttribute("k3"))
							{
								//
								// Lets get that information and add it
								//

								float k3 = attributes.value("k3").toString().toFloat();
								cameraSettings[camIndex].m_cameraParams.m_k3 = k3;

							}
							if (attributes.hasAttribute("offset_x"))
							{
								//
								// Lets get that information and add it
								//
								float offset_x = attributes.value("offset_x").toString().toFloat();
								cameraSettings[camIndex].m_cameraParams.m_offset_x = offset_x;
							}
							if (attributes.hasAttribute("offset_y"))
							{
								//
								// Lets get that information and add it
								//
								float offset_y = attributes.value("offset_y").toString().toFloat();
								cameraSettings[camIndex].m_cameraParams.m_offset_y = offset_y;
							}
							if (attributes.hasAttribute("expOffset"))
							{
								//
								// Lets get that information and add it
								//
								float expOffset = attributes.value("expOffset").toString().toFloat();
								cameraSettings[camIndex].m_cameraParams.m_expOffset = expOffset;
								cameraSettings[camIndex].exposure = expOffset;
							}
							if (attributes.hasAttribute("xrad1"))
							{
								//
								float xrad1 = attributes.value("xrad1").toString().toFloat();
								cameraSettings[camIndex].m_cameraParams.m_xrad1 = xrad1;
							}
							if (attributes.hasAttribute("xrad2"))
							{
								//
								float xrad2 = attributes.value("xrad2").toString().toFloat();
								cameraSettings[camIndex].m_cameraParams.m_xrad2 = xrad2;
							}
							if (attributes.hasAttribute("yrad1"))
							{
								//
								float yrad1 = attributes.value("yrad1").toString().toFloat();
								cameraSettings[camIndex].m_cameraParams.m_yrad1 = yrad1;
							}
							if (attributes.hasAttribute("yrad2"))
							{
								//
								float yrad2 = attributes.value("yrad2").toString().toFloat();
								cameraSettings[camIndex].m_cameraParams.m_yrad2 = yrad2;
							}
						}
					}
				}
			}
		}
	}
	// Error handling. 
	if (xml.hasError())
	{
		return 0;
	}
	//
	// Removes any device() or data from the reader
	// and resets its internal state to the initial state. 
	// 
	xml.clear();

	return 1;
}

void D360Parser::initTemplateImage(QMap<int, QString> imagePathList)
{
	GlobalAnimSettings::CameraSettingsList& cameraSettings = m_tempGlobalAnimSettings.cameraSettingsList();
	cameraSettings.resize(imagePathList.size());
	m_tempGlobalAnimSettings.m_cameraCount = imagePathList.size();
	m_tempGlobalAnimSettings.getLeftIndices().clear();
	m_tempGlobalAnimSettings.getRightIndices().clear();

	QMapIterator<int, QString> iter(imagePathList);
	int i = 0;
	while (iter.hasNext()) {
		iter.next();
		int key = iter.key();
		int stereoType = m_tempStereoList[key];
		int audioType = m_tempAudioList.contains(key) ? m_tempAudioList[key] : 3;	// if no exist, default NONE(3)

		cameraSettings[i].stereoType = (CameraInput::PanoramaStereoType)stereoType;
		if (cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_LeftEye || 
			cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_BothEye)
		{
			m_tempGlobalAnimSettings.getLeftIndices().push_back(i);
		}
		if (cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_RightEye || 
			cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_BothEye)
		{
			m_tempGlobalAnimSettings.getRightIndices().push_back(i);
		}

		cameraSettings[i].audioType = (CameraInput::InputAudioChannelType)audioType;

		// ex: D:/Projects/Panorama/Source/examples/frame_seq/office-01
		QStringList tokens = iter.value().split("/");
		cameraSettings[i].name = tokens.last();
		cameraSettings[i].filePrefix = m_tempPrefixList.contains(key) ? m_tempPrefixList[key] : tokens.last();
		//tokens.removeLast();
		cameraSettings[i].fileDir = tokens.join("/");
		//cameraSettings[i].debayerFileExt = "jpg";
		cameraSettings[i].fileExt = m_tempExtList.contains(key) ? m_tempExtList[key] : m_tempGlobalAnimSettings.getFileExt();

		cameraSettings[i].xres = m_tempGlobalAnimSettings.m_xres;
		cameraSettings[i].yres = m_tempGlobalAnimSettings.m_yres;
		cameraSettings[i].fps = m_tempGlobalAnimSettings.m_fps;

		i++;
	}

	parseCameraCalibrationFile(m_tempGlobalAnimSettings.m_cameraCalibFile, cameraSettings);
	m_globalAnimSettings = m_tempGlobalAnimSettings;
	m_globalAnimSettings.m_captureType = D360::Capture::CAPTURE_FILE;
}

void D360Parser::initTemplateVideo(QMap<int, QString> videoPathList)
{
	GlobalAnimSettings::CameraSettingsList& cameraSettings = m_tempGlobalAnimSettings.cameraSettingsList();
	cameraSettings.resize(videoPathList.size());
	m_tempGlobalAnimSettings.m_cameraCount = videoPathList.size();
	m_tempGlobalAnimSettings.getLeftIndices().clear();
	m_tempGlobalAnimSettings.getRightIndices().clear();

	QMapIterator<int, QString> iter(videoPathList);
	int i = 0;
	while (iter.hasNext()) {
		iter.next();
		int key = iter.key();
		int stereoType = m_tempStereoList[key];
		int audioType = m_tempAudioList.contains(key) ? m_tempAudioList[key] : 3;	// if no exist, default NONE(3)

		cameraSettings[i].stereoType = (CameraInput::PanoramaStereoType)stereoType;
		if (cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_LeftEye ||
			cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_BothEye)
		{
			m_tempGlobalAnimSettings.getLeftIndices().push_back(i);
		}
		if (cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_RightEye ||
			cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_BothEye)
		{
			m_tempGlobalAnimSettings.getRightIndices().push_back(i);
		}

		cameraSettings[i].audioType = (CameraInput::InputAudioChannelType)audioType;
		cameraSettings[i].name = iter.value();

		cameraSettings[i].xres = m_tempGlobalAnimSettings.m_xres;
		cameraSettings[i].yres = m_tempGlobalAnimSettings.m_yres;
		cameraSettings[i].fps = m_tempGlobalAnimSettings.m_fps;
		
		i++;
	}

	parseCameraCalibrationFile(m_tempGlobalAnimSettings.m_cameraCalibFile, cameraSettings);
	m_globalAnimSettings = m_tempGlobalAnimSettings;
	m_globalAnimSettings.m_captureType = D360::Capture::CAPTURE_VIDEO;
}

void D360Parser::initTemplateCamera(QMap<int, QString> cameraNameList, QMap<int, QString> audioNameList)
{
	GlobalAnimSettings::CameraSettingsList& cameraSettings = m_tempGlobalAnimSettings.cameraSettingsList();
	cameraSettings.resize(cameraNameList.size());
	m_tempGlobalAnimSettings.m_cameraCount = cameraNameList.size();
	m_tempGlobalAnimSettings.getLeftIndices().clear();
	m_tempGlobalAnimSettings.getRightIndices().clear();

	QMapIterator<int, QString> iter(cameraNameList);
	int i = 0;
	while (iter.hasNext()) {
		iter.next();
		int key = iter.key();
		int stereoType = m_tempStereoList[key];
#if 0
		int audioType = m_tempAudioList.contains(key) ? m_tempAudioList[key] : 3;	// if no exist, default NONE(3)
#else
		int audioType = 3;
		int j = 0;
		foreach(const int type, m_tempAudioList) {
			if (j++ == i)		audioType = type;
		}
#endif

		cameraSettings[i].stereoType = (CameraInput::PanoramaStereoType)stereoType;
		if (cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_LeftEye ||
			cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_BothEye)
		{
			m_tempGlobalAnimSettings.getLeftIndices().push_back(i);
		}
		if (cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_RightEye ||
			cameraSettings[i].stereoType == CameraInput::PanoramaStereoType::Panorama_BothEye)
		{
			m_tempGlobalAnimSettings.getRightIndices().push_back(i);
		}

		cameraSettings[i].audioType = (CameraInput::InputAudioChannelType)audioType;
		cameraSettings[i].name = iter.value();
#if 0
		if (audioNameList.contains(i)) {
			cameraSettings[i].audioName = audioNameList[i];
		}
#else
		j = 0;
		foreach(const QString name, audioNameList) {
			if (j++ == i)		cameraSettings[i].audioName = name;
		}
#endif

		cameraSettings[i].xres = m_tempGlobalAnimSettings.m_xres;
		cameraSettings[i].yres = m_tempGlobalAnimSettings.m_yres;
		cameraSettings[i].fps = m_tempGlobalAnimSettings.m_fps;
#if 0
		// Temp code
		cameraSettings[i].xres = 640;
		cameraSettings[i].yres = 480;
		cameraSettings[i].fps = 30;
#endif
		i++;
	}

	parseCameraCalibrationFile(m_tempGlobalAnimSettings.m_cameraCalibFile, cameraSettings);
	m_globalAnimSettings = m_tempGlobalAnimSettings;
	m_globalAnimSettings.m_captureType = D360::Capture::CAPTURE_DSHOW;
	m_globalAnimSettings.m_sampleFmt = AV_SAMPLE_FMT_S16;	// Important. No change!
#if 0
	// Temp code
	m_globalAnimSettings.m_xres = 640;
	m_globalAnimSettings.m_yres = 480;
	m_globalAnimSettings.m_panoXRes = 1440;
	m_globalAnimSettings.m_panoYRes = 720;
	m_globalAnimSettings.m_fps = 30;
#endif
}

float D360Parser::getYaw(int deviceNum)
{
	
	return m_globalAnimSettings.cameraSettingsList()[deviceNum].m_cameraParams.m_yaw;

}

void D360Parser::setYaw(float yaw, int deviceNum)
{
	m_globalAnimSettings.cameraSettingsList()[deviceNum].m_cameraParams.m_yaw = yaw;
}
float D360Parser::getPitch(int deviceNum)
{
	return m_globalAnimSettings.cameraSettingsList()[deviceNum].m_cameraParams.m_pitch;

}

void D360Parser::setPitch(float pitch, int deviceNum)
{
	m_globalAnimSettings.cameraSettingsList()[deviceNum].m_cameraParams.m_pitch = pitch;
}

float D360Parser::getRoll(int deviceNum)
{
	return m_globalAnimSettings.cameraSettingsList()[deviceNum].m_cameraParams.m_roll;
}

void D360Parser::setRoll(float roll, int deviceNum)
{
	m_globalAnimSettings.cameraSettingsList()[deviceNum].m_cameraParams.m_roll = roll;
}

float D360Parser::getLeft(int iDeviceNum)
{
	return (1.0f - m_globalAnimSettings.cameraSettingsList()[iDeviceNum].m_cameraParams.m_xrad1) * m_globalAnimSettings.m_xres / 2;
}
void D360Parser::setLeft(float fLeft, int iDeviceNum)
{
	m_globalAnimSettings.cameraSettingsList()[iDeviceNum].m_cameraParams.m_xrad1 = 1.0f - fLeft / (m_globalAnimSettings.m_xres / 2);
}
float D360Parser::getRight(int iDeviceNum)
{
	return (1.0f + m_globalAnimSettings.cameraSettingsList()[iDeviceNum].m_cameraParams.m_xrad2) * m_globalAnimSettings.m_xres / 2;
}
void D360Parser::setRight(float fRight, int iDeviceNum)
{
	m_globalAnimSettings.cameraSettingsList()[iDeviceNum].m_cameraParams.m_xrad2 = fRight / (m_globalAnimSettings.m_xres / 2) - 1.0f;
}
float D360Parser::getTop(int iDeviceNum)
{
	return (1.0f - m_globalAnimSettings.cameraSettingsList()[iDeviceNum].m_cameraParams.m_yrad1) * m_globalAnimSettings.m_yres / 2;
}
void D360Parser::setTop(float fTop, int iDeviceNum)
{
	m_globalAnimSettings.cameraSettingsList()[iDeviceNum].m_cameraParams.m_yrad1 = 1.0f - fTop / (m_globalAnimSettings.m_yres / 2);
}
float D360Parser::getBottom(int iDeviceNum)
{
	return (1.0f + m_globalAnimSettings.cameraSettingsList()[iDeviceNum].m_cameraParams.m_yrad2) * m_globalAnimSettings.m_yres / 2;
}
void D360Parser::setBottom(float fBottom, int iDeviceNum)
{
	m_globalAnimSettings.cameraSettingsList()[iDeviceNum].m_cameraParams.m_yrad2 = fBottom / (m_globalAnimSettings.m_yres / 2) - 1.0f;
}

int D360Parser::getStereoType(int iDeviceNum)
{
	return m_globalAnimSettings.cameraSettingsList()[iDeviceNum].stereoType;
}

void D360Parser::setStereoType(int iStereoType, int iDeviceNum)
{
	//m_globalAnimSettings.cameraSettingsList()[iDeviceNum].stereoType = iStereoType;
	switch (iStereoType)
	{
	case 0: m_globalAnimSettings.cameraSettingsList()[iDeviceNum].stereoType = CameraInput::PanoramaStereoType::Panorama_Mono; break;
	case 1: m_globalAnimSettings.cameraSettingsList()[iDeviceNum].stereoType = CameraInput::PanoramaStereoType::Panorama_LeftEye; break;
	case 2: m_globalAnimSettings.cameraSettingsList()[iDeviceNum].stereoType = CameraInput::PanoramaStereoType::Panorama_RightEye; break;
	case 3: m_globalAnimSettings.cameraSettingsList()[iDeviceNum].stereoType = CameraInput::PanoramaStereoType::Panorama_BothEye; break;
	default:
		break;
	}
}

void D360Parser::setTempCameraSettings()
{
	m_tempGlobalAnimSettings = m_globalAnimSettings;
}

void D360Parser::resetCameraSettings()
{
	m_globalAnimSettings = m_tempGlobalAnimSettings;
}

void D360Parser::setAudioType(int iAudioType, int iDeviceNum)
{
	m_globalAnimSettings.cameraSettingsList()[iDeviceNum].audioType = (CameraInput::InputAudioChannelType)iAudioType;
}

void D360Parser::setTempStereoType(int iIndex, int iStereType)
{
	if (iStereType >= 0) {
		m_tempStereoList[iIndex] = iStereType;
	}
	else {
		m_tempStereoList.remove(iIndex);
	}
}

int D360Parser::getTempStereoType(int iDeviceNum)
{
	if (!m_tempStereoList.contains(iDeviceNum))
		return 0;		// If not exist iDeviceNum camera's stereo type, return default NONE type (0).

	return m_tempStereoList[iDeviceNum];
}

void D360Parser::setTempImagePath(int iDeviceNum, QString fileDir)
{
	m_tempPathList[iDeviceNum] = fileDir;
}

QString D360Parser::getTempImagePath(int iDeviceNum)
{
	if (!m_tempPathList.contains(iDeviceNum))
		return "";
	return m_tempPathList[iDeviceNum];
	
}

void D360Parser::setTempImagePrefix(int iDeviceNum, QString filePrefix)
{
	m_tempPrefixList[iDeviceNum] = filePrefix;
}

QString D360Parser::getTempImagePrefix(int iDeviceNum)
{
	if (!m_tempPrefixList.contains(iDeviceNum))
		return "";
	return m_tempPrefixList[iDeviceNum];

}

void D360Parser::setTempImageExt(int iDeviceNum, QString fileExt)
{
	m_tempExtList[iDeviceNum] = fileExt;
}

QString D360Parser::getTempImageExt(int iDeviceNum)
{
    if (!m_tempExtList.contains(iDeviceNum))
		return "";
	return m_tempExtList[iDeviceNum];

}

void D360Parser::setTempAudioSettings(int iIndex, int iAudioType)
{
	if (iAudioType > -1) {
		m_tempAudioList[iIndex] = iAudioType;
	}
	else {
		m_tempAudioList.remove(iIndex);
	}
}

int D360Parser::getTempAudioSettings(int iDeviceNum)
{
	if (!m_tempAudioList.contains(iDeviceNum))
		return 1;	// Default is Left mode.

	return m_tempAudioList[iDeviceNum];
}