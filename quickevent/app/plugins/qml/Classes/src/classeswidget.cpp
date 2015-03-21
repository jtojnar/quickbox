#include "classeswidget.h"
#include "ui_classeswidget.h"

#include "classesplugin.h"
#include "coursedef.h"

#include <EventPlugin/eventplugin.h>

#include <qf/core/string.h>
#include <qf/core/utils.h>
#include <qf/core/collator.h>

#include <qf/core/model/sqltablemodel.h>
#include <qf/core/sql/querybuilder.h>

#include <qf/qmlwidgets/action.h>
#include <qf/qmlwidgets/menubar.h>
#include <qf/qmlwidgets/framework/mainwindow.h>
#include <qf/qmlwidgets/dialogs/filedialog.h>
#include <qf/qmlwidgets/dialogs/messagebox.h>

namespace qfc = qf::core;
namespace qfw = qf::qmlwidgets;
namespace qfd = qf::qmlwidgets::dialogs;
namespace qfm = qf::core::model;
namespace qfs = qf::core::sql;

ClassesWidget::ClassesWidget(QWidget *parent) :
	Super(parent),
	ui(new Ui::ClassesWidget)
{
	ui->setupUi(this);
	{
		ui->tblClassesTB->setTableView(ui->tblClasses);
		qfm::SqlTableModel *m = new qfm::SqlTableModel(this);
		m->addColumn("id").setReadOnly(true);
		m->addColumn("classes.name", tr("Class"));
		m->addColumn("classdefs.mapCount", tr("Maps"));
		m->addColumn("courses.name", tr("Course"));
		m->addColumn("courses.length", tr("Length"));
		m->addColumn("courses.climb", tr("Climb"));
		ui->tblClasses->setTableModel(m);
		m_classesModel = m;
	}
}

ClassesWidget::~ClassesWidget()
{
	delete ui;
}

void ClassesWidget::settleDownInPartWidget(ThisPartWidget *part_widget)
{
	connect(part_widget, SIGNAL(resetPartRequest()), this, SLOT(reset()));
	connect(part_widget, SIGNAL(reloadPartRequest()), this, SLOT(reload()));

	qfw::Action *a_import = part_widget->menuBar()->actionForPath("import", true);
	a_import->setText("&Import");

	qfw::Action *a_ocad = new qfw::Action("OCad", this);
	connect(a_ocad, SIGNAL(triggered()), this, SLOT(import_ocad()));

	a_import->addActionInto(a_ocad);
}

void ClassesWidget::reset()
{
	reload();
}

void ClassesWidget::reload()
{
	qf::qmlwidgets::framework::MainWindow *fwk = qf::qmlwidgets::framework::MainWindow::frameWork();
	EventPlugin *event_plugin = qobject_cast<EventPlugin *>(fwk->plugin("Event"));
	{
		qfs::QueryBuilder qb;
		qb.select2("classes", "*")
				.select2("classdefs", "id, mapCount")
				.select2("courses", "name, length, climb")
				.from("classes")
				.joinRestricted("classes.id", "classdefs.classId", "classdefs.stageId=" QF_IARG(event_plugin->currentStage()))
				.join("classdefs.courseId", "courses.id")
				.orderBy("classes.name");//.limit(10);
		/*
		int class_id = m_cbxClasses->currentData().toInt();
		if(class_id > 0) {
			qb.where("competitors.classId=" + QString::number(class_id));
		}
		*/
		m_classesModel->setQueryBuilder(qb);
		m_classesModel->reload();
	}

}

static QString normalize_course_name(const QString &name)
{
	QString ret = qf::core::Collator::toAscii7(name, false);
	ret.replace(' ', QString());
	ret.replace(',', '+');
	return ret;
}

void ClassesWidget::import_ocad()
{
	QString fn = qfd::FileDialog::getOpenFileName(this, tr("Open file"));
	if(fn.isEmpty())
		return;
	QFile f(fn);
	if(f.open(QFile::ReadOnly)) {
		QStringList lines;
		while (true) {
			QByteArray ba = f.readLine();
			if(ba.isEmpty())
				break;
			lines << QString::fromUtf8(ba).trimmed();
		}
		try {
			//QSet<QString> parsed_classes;
			//QSet<QString> parsed_courses;
			QMap<QString, CourseDef> defined_courses;
			for(QString line : lines) {
				// [classname];coursename;0;lenght_km;climb;S1;dist_1;code_1[;dist_n;code_n];dist_finish;F1
				if(line.isEmpty())
					continue;
				QString course_name = normalize_course_name(line.section(';', 1, 1));
				qfc::String class_name = line.section(';', 0, 0);
				if(class_name.isEmpty())
					class_name = course_name;
				if(defined_courses.contains(course_name)) {
					CourseDef cd = defined_courses.value(course_name);
					QStringList classes = cd.classes() << class_name;
					defined_courses[course_name].setClasses(classes);
					continue;
				}
				/*
				QStringList classes = classes_str.splitAndTrim(',');
				for(auto c : classes) {
					if(parsed_classes.contains(c))
						QF_EXCEPTION("Duplicate definition for class:" + c);
					else
						parsed_classes << c;
				}
				cd.setClasses(classes);
				if(course.isEmpty())
					course = cd.classes().value(0);
				{
					if(parsed_courses.contains(course))
						QF_EXCEPTION("Duplicate definition for course:" + course);
					else
						parsed_courses << course;
				}
				*/
				CourseDef &cd = defined_courses[course_name];
				cd.setCourse(course_name);
				cd.setClasses(QStringList() << class_name);
				{
					QString s = line.section(';', 3, 3).trimmed();
					s.replace('.', QString()).replace(',', QString());
					cd.setLenght(s.toInt());
				}
				{
					QString s = line.section(';', 4, 4).trimmed();
					s.replace('.', QString()).replace(',', QString());
					cd.setClimb(s.toInt());
				}
				{
					qfc::String s = line.section(';', 6);
					QVariantList codes;
					QStringList sl = s.splitAndTrim(';');
					for (int i = 0; i < sl.count()-2; i+=2) {
						codes << sl[i+1].toInt();
					}
					cd.setCodes(codes);
				}
			}
			QVariantList courses;
			for(auto cd : defined_courses.values())
				courses << cd;
			{
				QJsonDocument doc = QJsonDocument::fromVariant(courses);
				qfDebug() << doc.toJson();
			}

			qf::qmlwidgets::framework::MainWindow *fwk = qf::qmlwidgets::framework::MainWindow::frameWork();
			EventPlugin *event_plugin = qobject_cast<EventPlugin *>(fwk->plugin("Event"));
			ClassesPlugin *classes_plugin = qobject_cast<ClassesPlugin *>(fwk->plugin("Classes"));
			if(event_plugin && classes_plugin) {
				QString msg = tr("Delete all courses definitions for stage %1?").arg(event_plugin->currentStage());
				if(qfd::MessageBox::askYesNo(fwk, msg, false)) {
					classes_plugin->createCourses(event_plugin->currentStage(), courses);
					reload();
				}
			}
		}
		catch (const qf::core::Exception &e) {
			qf::qmlwidgets::framework::MainWindow *fwk = qf::qmlwidgets::framework::MainWindow::frameWork();
			qf::qmlwidgets::dialogs::MessageBox::showException(fwk, e);
		}
	}
}

