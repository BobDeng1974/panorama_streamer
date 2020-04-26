#ifndef DEFINE_H
#define DEFINE_H
#include <QDebug>
#include <QTime>
#include <qfile.h>
#include "PanoLog.h"

#define THREAD_TYPE_ID				1000
#define THREAD_TYPE_MAIN			THREAD_TYPE_ID + 0
#define THREAD_TYPE_STITCHER		THREAD_TYPE_ID + 1
#define THREAD_TYPE_CAPTURE			THREAD_TYPE_ID + 2
#define THREAD_TYPE_STREAM			THREAD_TYPE_ID + 3
#define THREAD_TYPE_AUDIO			THREAD_TYPE_ID + 4
#define THREAD_TYPE_OCULUS			THREAD_TYPE_ID + 5

#define CUR_TIME							QTime::currentTime().toString("mm:ss.zzz")
#define CUR_TIME_H							QTime::currentTime().toString("hh.mm.ss")
#define ARGN(num)							arg(QString::number(num))

#define PANO_CONN(sender, signal, receiver, slot)	connect(sender, SIGNAL(signal), receiver, SLOT(slot))

extern PanoLog* g_logger;

//#define PANO_LOG(msg)			qDebug() << QString("[%1] %2").arg(this->m_Name).arg(msg)
#define PANO_LOG(msg)			g_logger->info(QString("[%1] %2").arg(this->m_Name).arg(msg))
#define PANO_N_LOG(msg)			g_logger->info(msg, true)
#define PANO_DEVICE_LOG(msg)	g_logger->info(QString("[%1] [%2] %3").arg(this->m_Name).arg(QString::number(m_deviceNumber)).arg(msg))
#define PANO_DEVICE_N_LOG(msg)	g_logger->info(msg, true)
#define PANO_WARN(msg)			g_logger->warning(QString("[%1] %2").arg(this->m_Name).arg(msg))
#define PANO_N_WARN(msg)		g_logger->warning(msg, true)
#define PANO_DEVICE_WARN(msg)	g_logger->warning(QString("[%1] [%2] %3").arg(this->m_Name).arg(QString::number(m_deviceNumber)).arg(msg))
#define PANO_DEVICE_N_WARN(msg)	g_logger->warning(msg, true)
#define PANO_ERROR(msg)			g_logger->critical(QString("[%1] %2").arg(this->m_Name).arg(msg))
#define PANO_N_ERROR(msg)		g_logger->critical(msg, true)
#define PANO_DEVICE_ERROR(msg)	g_logger->critical(QString("[%1] [%2] %3").arg(this->m_Name).arg(QString::number(m_deviceNumber)).arg(msg))
#define PANO_DEVICE_N_ERROR(msg)	g_logger->critical(msg, true)
#define PANO_DLOG(msg)			g_logger->debug(QString("[%1] %2").arg(this->m_Name).arg(msg))
#define PANO_N_DLOG(msg)		g_logger->debug(msg, true)
#define PANO_DEVICE_DLOG(msg)	g_logger->debug(QString("[%1] [%2] %3").arg(this->m_Name).arg(QString::number(m_deviceNumber)).arg(msg))
#define PANO_DEVICE_N_DLOG(msg)	g_logger->debug(msg, true)

#define PANO_TIME_LOG(msg)			g_logger->info( QString("[%1] %2 (%3)").arg(this->m_Name).arg(msg).arg(CUR_TIME))
#define PANO_DEVICE_TIME_LOG(msg)	g_logger->info(QString("[%1] [%2] %3 (%4)").arg(this->m_Name).arg(QString::number(m_deviceNumber)).arg(msg).arg(CUR_TIME))

#if 0
//#define PANO_DEBUG
#ifdef _DEBUG
#define PANO_DLOG(msg)			g_logger->debug(QString("[%1] %2").arg(this->m_Name).arg(msg))
#define PANO_DEVICE_DLOG(msg)	g_logger->debug(QString("[%1] [%2] %3").arg(this->m_Name).arg(QString::number(m_deviceNumber)).arg(msg))
#else
#define PANO_DLOG(msg)			{}
#define PANO_DEVICE_DLOG(msg)	{}
#define PANO_TIME_DLOG(msg)		{}
#define PANO_DEVICE_TIME_DLOG(msg)		{}
#endif
#endif

#define ERROR_MSG(msg)						QMessageBox::critical(this, tr("Error"), msg)
#define WARNING_MSG(msg)					QMessageBox::warning(this, tr("Warning"), msg)
//#define LOG(msg)							qDebug() << msg//.toStdString().c_str())
#define Q_LOG(msg)							T_LOG(msg.toStdString().c_str())
#define FREE_PTR(ptr)						{ if (ptr) { delete ptr; ptr = NULL; } }
#define FREE_MEM(ptr)						{ if (ptr) { free(ptr); ptr = NULL; } }
#define FREE_AV_MEM(ptr)						{ if (*ptr) { av_frame_free(ptr); *ptr = NULL; } }

typedef unsigned char	byte;

#ifdef USE_SHADER_CODE
#define ADD_SHADER_FROM_CODE(program, type, res)	{ \
														QFile shaderFile(QString(":/Shaders/") + QString(res) + QString(".") + QString(type)); \
														shaderFile.open(QIODevice::ReadOnly); \
														QString shaderCode(shaderFile.readAll()); \
														program->addShaderFromSourceCode(QString(type)==QString("frag")?QOpenGLShader::Fragment:QOpenGLShader::Vertex, shaderCode); \
													}
#else
#define ADD_SHADER_FROM_CODE(program, type, res)
#endif

#endif // DEFINE_H