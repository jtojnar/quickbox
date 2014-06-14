#include "mainwindow.h"

#include <qf/qmlwidgets/frame.h>
#include <qf/qmlwidgets/label.h>

#include <qf/core/log.h>
#include <qf/core/logdevice.h>

#include <QApplication>
#include <QPushButton>
#include <QLayout>
#include <QtQml>

int main(int argc, char *argv[])
{
	QScopedPointer<qf::core::LogDevice> log_device(qf::core::FileLogDevice::install(argc, argv));
	log_device->setPrettyDomain(true);

	qfError() << "QFLog(ERROR) test OK.";
	qfWarning() << "QFLog(WARNING) test OK.";
	qfInfo() << "QFLog(INFO) test OK.";
	qfDebug() << "QFLog(DEBUG) test OK.";

	qmlRegisterType<Label>("mywidgets", 1, 0, "Label");
	qmlRegisterType<QPushButton>("mywidgets", 1, 0, "Button");
	qmlRegisterType<Frame>("mywidgets", 1, 0, "Frame");
	qmlRegisterType<QWidget>("mywidgets", 1, 0, "Widget");
	qmlRegisterType<QVBoxLayout>("mywidgets", 1, 0, "VBoxLayout");

	qDebug() << "creating application instance";
	//qFatal("ASSERT");

	QApplication a(argc, argv);

	QQmlEngine engine;
	QQmlComponent component(&engine, QUrl::fromLocalFile("main.qml"));
	if(!component.isReady()) {
		qfError() << component.errorString();
	}
	else {
		QWidget *root = qobject_cast<QWidget*>(component.create());
		if(root)
			root->show();

		//MainWindow w;
		//w.show();

		return a.exec();
	}
}