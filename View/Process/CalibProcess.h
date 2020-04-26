#ifndef CALIBPROCESS_H
#define CALIBPROCESS_H

#include <QProcess>
#include <QMap>

#define CALIB_CMD			QString("PanoramaTools.bat")
#define CALIB_WORK_PATH		QString("./PanoramaTools/")
#define CALIB_RESULT_FILE   "complete.pto"
#define START_SYM           "#"
#define VALUE_SYM           "-"
#define VALUE_EXP           "#hugin_"
#define COMMENT_EXP         "# "
#define ROOT_BLOCK          "Root"
#define IMAGE_VAL           "image lines"
#define CP_VAL              "control points"
#define OPT_VAL             "optimized"
#define CAMERA_EXP          "i "
#define CP_EXP              "c "
#define OPT_EXP             "v "

typedef enum {
    NONE_VAL = -1,
    BOOL_VAL,
    NUMBER_VAL,
    STRING_VAL,
    STRUCTURE_VAL,
} TYPEOFVAL;

typedef struct {
    TYPEOFVAL type;
    QStringList value;
    int nbVal;
    QMap<QString, float> camParams;
} PTOVAL;

typedef struct {
	int point1;
	float x1;
	float y1;
	int point2;
	float x2;
	float y2;
} CPOINT;

class CalibProcess : public QObject
{
    Q_OBJECT
public:
    CalibProcess(QString command = "", QObject * parent = 0);
    void setPath(QString path) {
        if (!path.isNull() && !path.isEmpty())
        m_Process.setWorkingDirectory(path);
    }
	bool calibrate(QString command = "", bool isFinal = false);
	void setCommand(QString command);
	QString getCommand();
    void addArg(QString arg);
    QMap<QString, PTOVAL> parsePTO(QString filename = "");
	QMap<QString, PTOVAL> getCalibParams() { return m_ptoConfig; }
	bool isError() { return m_isError; }
	bool isFinished() { return m_isFinished; }
	void initialize();

private:
    QString m_Name;
    QProcess m_Process;
    QString m_Command;
    QString m_Output;
    QMap<QString, PTOVAL>   m_ptoConfig;
    QMap<QString, QString>  m_pacPTS;
	bool m_isError;
	bool m_isFinished;
	bool m_isFinal;

    int processLine(QString line);

public slots:
    void readOutput();
    void finishedCalib();
	void errorProcess(QProcess::ProcessError error);

signals:
	void finishedSignal(int);
};

#endif // CALIBPROCESS_H
