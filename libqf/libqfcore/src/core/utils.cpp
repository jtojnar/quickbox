#include "utils.h"
#include "log.h"

#include <QString>
#include <QDate>

using namespace qf::core;

void qf::core::Utils::parseFieldName(const QString &full_field_name, QString *pfield_name, QString *ptable_name, QString *pdb_name)
{
	QString s = full_field_name;
	QString field_name, table_name, db_name;
	field_name = s;

	int ix = s.lastIndexOf('.');
	if(ix >= 0) {
		field_name = s.mid(ix+1);
		s = s.mid(0, ix);
		table_name = s;

		ix = s.lastIndexOf('.');
		if(ix >= 0) {
			table_name = s.mid(ix+1);
			s = s.mid(0, ix);
			db_name = s;
		}
	}

	if(pfield_name) *pfield_name = field_name;
	if(ptable_name) *ptable_name = table_name;
	if(pdb_name) *pdb_name = db_name;
}

bool Utils::fieldNameEndsWith(const QString &field_name1, const QString &field_name2)
{
	/// psql (podle SQL92) predelava vsechny nazvy sloupcu, pokud nejsou v "" do lowercase, ale mixedcase se lip cte, tak at se to sparuje.
	int l1 = field_name1.length();
	int l2 = field_name2.length();
	if(l2 > l1)
		return false;
	if(field_name1.endsWith(field_name2, Qt::CaseInsensitive)) {
		if(l1 == l2)
			return true; /// same length, must be same
		if(field_name1[l1 - l2 - 1] == '.')
			return true; /// dot '.' is on position l1 - l2 - 1
	}
	return false;
}

bool Utils::fieldNameCmp(const QString &fld_name1, const QString &fld_name2)
{
	if(fld_name1.isEmpty() || fld_name2.isEmpty()) return false;
	if(fieldNameEndsWith(fld_name1, fld_name2)) return true;
	if(fieldNameEndsWith(fld_name2, fld_name1)) return true;
	return false;
}

QVariant Utils::retypeVariant(const QVariant &val, QVariant::Type type)
{
	if(val.type() == type)
		return val;
	if(!val.isValid())
		return val;
	if(val.isNull())
		return QVariant(type);
	switch(type) {
	case QVariant::Bool:
		return QVariant(val.toBool());
	case QVariant::LongLong:
		if(val.type() == QVariant::Double) return val.toLongLong();
		else if(val.type() == QVariant::Bool) return val.toBool()? 1: 0;
		return QVariant(val.toLongLong());
	case QVariant::ULongLong:
		if(val.type() == QVariant::Double) return val.toULongLong();
		else if(val.type() == QVariant::Bool) return val.toBool()? 1: 0;
		return QVariant(val.toULongLong());
	case QVariant::Int:
		if(val.type() == QVariant::Double) return val.toInt();
		else if(val.type() == QVariant::Bool) return val.toBool()? 1: 0;
		return QVariant(val.toInt());
	case QVariant::UInt:
		if(val.type() == QVariant::Double) return val.toUInt();
		else if(val.type() == QVariant::Bool) return val.toBool()? 1: 0;
		return QVariant(val.toUInt());
	case QVariant::Double:
		if(val.type() == QVariant::Bool) return val.toBool()? 1: 0;
		return QVariant(val.toDouble());
	case QVariant::Date: {
		QString str_val = val.toString();
		if(str_val.isEmpty()) {
			return QVariant(QDate());
		}
		else	{
			if(val.type() == QVariant::DateTime)
				return val.toDate();
			return QDate::fromString(val.toString());
			//if(QLocale().name() == "cs_CZ") return QVariant(QDate::fromString(str_val, "d.M.yyyy") );
			//return QVariant(QDate::fromString(str_val, Qt::ISODate) );
		}
	}
	case QVariant::Time: {
		QString str_val = val.toString();
		if (str_val.isEmpty()) {
			return QVariant(QTime());
		}
		else {
			if(val.type() == QVariant::DateTime) return val.toTime();
			return QVariant(QTime::fromString(str_val, Qt::ISODate));
		}
	}
	case QVariant::DateTime: {
		QString str_val = val.toString();
		if (str_val.isEmpty()) {
			return QVariant(QDateTime());
		}
		if (str_val.length() == 14) {
			/// TIMESTAMPS with format yyyyMMddhhmmss
			str_val.insert(4, QLatin1Char('-')).insert(7, QLatin1Char('-')).insert(10, QLatin1Char('T')).insert(13, QLatin1Char(':')).insert(16, QLatin1Char(':'));
		}
		return QVariant(QDateTime::fromString(str_val, Qt::ISODate));
	}
	case QVariant::String:
	default: {
		QString str_val = val.toString();
		return QVariant(str_val);
	}
	}
	qfWarning() << "retypeVariant(): unknown data type" << QVariant::typeToName(type);
	return QVariant();
}