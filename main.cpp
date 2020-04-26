#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include "MCQmlCameraView.h"
#include "QmlInteractiveView.h"
#include "D360Parser.h"
#include "QmlMainWindow.h"
#include "QmlRecentDialog.h"

QThread* g_mainThread;

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
    QGuiApplication app(argc, argv);
	g_mainThread = app.thread();

	if (qgetenv("QT_QUICK_CONTROLS_STYLE").isEmpty()) {
           qputenv("QT_QUICK_CONTROLS_STYLE", "Flat");
    }
	
    QQmlApplicationEngine engine;
	qmlRegisterType<MCQmlCameraView>("MCQmlCameraView", 1, 0, "MCQmlCameraView");
	qmlRegisterType<QmlInteractiveView>("QmlInteractiveView", 1, 0, "QmlInteractiveView");
	qmlRegisterType<QmlMainWindow>("QmlMainWindow", 1, 0, "QmlMainWindow");
	qmlRegisterType<QmlRecentDialog>("QmlRecentDialog", 1, 0, "QmlRecentDialog");
    engine.load(QUrl("qrc:/main.qml"));

	return app.exec();
}