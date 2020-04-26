#include <QDateTime>
#include <QDebug>
#include "PanoLog.h"

PanoLog::PanoLog(QObject* parent, QString name)
    : m_parent(parent)
	, QObject(parent)
    , m_fileName(name)
{
    m_level = PANO_LOG_LEVEL::WARNING;
    m_isStdout = false;
	
}

PanoLog::~PanoLog()
{
    m_logFile->close();
    delete m_logFile;
}

bool PanoLog::initialize(PANO_LOG_LEVEL level, QString name)
{
    bool ret = false;

    if (!name.isEmpty())    m_fileName = name;

    if (m_fileName.isEmpty())   return false;

    m_level = level;

	if (m_parent) {
		connect(this, SIGNAL(notify(int, QString, QString)), m_parent, SLOT(onNotify(int, QString, QString)));
	}

    m_logFile = new QFile(m_fileName);
    ret = m_logFile->open(QIODevice::ReadWrite | QIODevice::Append);

    return ret;
}

void PanoLog::critical(QString msg, bool isNotify)
{
    output(QString("[Critical] %1").arg(msg));

	if (isNotify)
		emit notify(PANO_LOG_LEVEL::CRITICAL, "Error", msg);
}

void PanoLog::debug(QString msg, bool isNotify)
{
    if (m_level > PANO_LOG_LEVEL::DEBUG)    return;

    output(QString("[Debug   ] %1").arg(msg));
}

void PanoLog::info(QString msg, bool isNotify)
{
    if (m_level > PANO_LOG_LEVEL::INFO)    return;

    output(QString("[Info    ] %1").arg(msg));

	if (isNotify)
		emit notify(PANO_LOG_LEVEL::INFO, "Information", msg);
}

void PanoLog::warning(QString msg, bool isNotify)
{
    if (m_level > PANO_LOG_LEVEL::WARNING)  return;

    output(QString("[Warning ] %1").arg(msg));

	if (isNotify)
		emit notify(PANO_LOG_LEVEL::WARNING, "Warning", msg);
}

int PanoLog::output(QString msg)
{
    QString outMsg = QString("[%1] %2").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss:zzz")).arg(msg);

	if (m_isStdout)
		qDebug() << outMsg;

	outMsg += "\r\n";

    int ret = m_logFile->write(outMsg.toLocal8Bit().data());
    
    return ret;
}

