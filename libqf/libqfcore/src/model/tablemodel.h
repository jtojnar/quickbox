#ifndef QF_CORE_MODEL_TABLEMODEL_H
#define QF_CORE_MODEL_TABLEMODEL_H

#include "../core/coreglobal.h"
#include "../utils/table.h"
#include "../core/utils.h"

#include <QAbstractTableModel>

namespace qf {
namespace core {

namespace utils {
class TreeTable;
}

namespace model {

class QFCORE_DECL_EXPORT TableModel : public QAbstractTableModel
{
	Q_OBJECT
	Q_PROPERTY(bool nullReportedAsString READ isNullReportedAsString WRITE setNullReportedAsString NOTIFY nullReportedAsStringChanged)
private:
	typedef QAbstractTableModel Super;
public:
	enum ItemDataRole {FieldNameRole = Qt::UserRole+1,
				FieldTypeRole, //FieldIsNullableRole,
				RawValueRole, ValueIsNullRole, FirstUnusedRole };
	enum RecordEditMode {ModeView, ModeEdit, ModeInsert, ModeCopy, ModeDelete};
public:
	explicit TableModel(QObject *parent = 0);
public:
	class QFCORE_DECL_EXPORT ColumnDefinition
	{
	public:
		private:
		class SharedDummyHelper {};
			class Data : public QSharedData
			{
				public:
					QString fieldName; //!< ID to pair ColumnDefinitions with fields
					int fieldIndex;
					QString caption;
					QString toolTip;
					//int initialSize; //!< initial width of column
					bool readOnly;
					Qt::Alignment alignment;
					//QPointer<QFDlgDataTable> chooser;
					QString format; //!< format for date, time, ... types nebo enumz/group_name[/'ruzny place holders']
					QVariant::Type castType;

					Data(const QString &fldname = QString()) : fieldName(fldname), fieldIndex(-1), readOnly(false), castType(QVariant::Invalid) {}
			};
		private:
			QSharedDataPointer<Data> d;
			ColumnDefinition(SharedDummyHelper) {
				d = new Data();
			}
		public:
			static const ColumnDefinition& sharedNull();
			bool isNull() const {return d == sharedNull().d;}
		public:
			ColumnDefinition() {
				*this = sharedNull();
			}
			ColumnDefinition(const QString &fldname) {
				d = new Data(fldname);
			}

			QString fieldName() const {return d->fieldName;}
			ColumnDefinition& setFieldName(const QString &s) {d->fieldName = s; return *this;}
			int fieldIndex() const {return d->fieldIndex;}
			ColumnDefinition& setFieldIndex(int i) {d->fieldIndex = i; return *this;}
			QString caption() const {return d->caption.isEmpty()? fieldName(): d->caption;}
			ColumnDefinition& setCaption(const QString &s) {d->caption = s; return *this;}
			QString toolTip() const {return d->toolTip;}
			ColumnDefinition& setToolTip(const QString &s) {d->toolTip = s; return *this;}
			//int initialSize() const {return d->initialSize;}
			//ColumnDefinition& setInitialSize(int i) {d->initialSize = i; return *this;}
			bool isReadOnly() const {return d->readOnly;}
			ColumnDefinition& setReadOnly(bool b = true) {d->readOnly = b; return *this;}
			Qt::Alignment alignment() const {return d->alignment;}
			ColumnDefinition& setAlignment(const Qt::Alignment &al) {d->alignment = al; return *this;}
			QString format() const {return d->format;}
			/// for double see QString::number(...)
			/// for QTime see QTime::toString(...)
			/// for QDate see QDate::toString(...)
			ColumnDefinition& setFormat(const QString &s) {d->format = s; return *this;}

			ColumnDefinition& setCastType(QVariant::Type t) {d->castType = t; return *this;}
			QVariant::Type castType() const {return d->castType;}
	};
	typedef QList<ColumnDefinition> ColumnList;

public:
	void clearColumns();
	ColumnDefinition& addColumn(const QString &field_name, const QString &caption = QString()) {
		return insertColumn(m_columns.count(), field_name, caption);
	}
	ColumnDefinition& insertColumn(int before_ix, const QString &field_name, const QString &_caption = QString());
	ColumnDefinition& insertColumn(int before_ix, const ColumnDefinition &cd);
	ColumnDefinition removeColumn(int ix);

	int appendRow() {return insertRows(rowCount(), 1);}
public:
	const qf::core::utils::Table& table() {return m_table;}

	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	bool setData(const QModelIndex &index, const QVariant & value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) Q_DECL_OVERRIDE;

	Q_SLOT virtual bool reload();
	Q_SLOT virtual bool postRow(int row_no, bool throw_exc);
	Q_SLOT virtual void revertRow(int row_no);

	bool insertRows(int row_ix, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	bool removeRows(int row_ix, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;

	bool isNullReportedAsString() const { return m_nullReportedAsString; }
	void setNullReportedAsString(bool arg);
	Q_SIGNAL void nullReportedAsStringChanged(bool arg);

	Q_SIGNAL void columnsAutoGenerated();
	Q_SIGNAL void reloaded();

	Q_INVOKABLE virtual bool setValue(int row, int column, const QVariant &val);
	Q_INVOKABLE bool setValue(int row_ix, const QString& col_name, const QVariant &val);
	Q_INVOKABLE virtual QVariant value(int row_ix, int column_ix) const;
	Q_INVOKABLE QVariant value(int row_ix, const QString& col_name) const;

	qf::core::utils::TableRow tableRow(int row_no);
	//Q_INVOKABLE QVariant origValue(int row, const QString& col_name) const;
	//Q_INVOKABLE bool isDirty(int row, const QString& col_name) const;

	class QFCORE_DECL_EXPORT TreeTableExportOptions : public QVariantMap
	{
		QF_OPTION_FIELD2_RW(bool, is, set, ExportRawValues, true)
	public:
		TreeTableExportOptions(const QVariantMap &m = QVariantMap()) : QVariantMap(m) {}
	};
	qf::core::utils::TreeTable toTreeTable(const QVariantList& exported_columns, const QString& table_name, const TreeTableExportOptions &opts = TreeTableExportOptions()) const;
	Q_INVOKABLE QVariant toTreeTableData(const QVariantList& exported_columns = QVariantList(), const QString& table_name = QString()) const;

protected:
	void createColumnsFromTableFields();
	void fillColumnIndexes();
	QVariant::Type columnType(int column_index) const;
	int columnIndex(const QString &column_name) const;
	int tableFieldIndex(int column_index) const;
	qf::core::utils::Table::Field tableField(int column_index) const;
	/// @returns: index of inserted line or -1
	virtual bool insertOneRow(int before_row);
	virtual bool removeOneRow(int row_ix, bool throw_exc = false);
protected:
	qf::core::utils::Table m_table;
	ColumnList m_columns;
	bool m_autoColumns;
	bool m_nullReportedAsString;
	static QString m_defaultTimeFormat;
	static QString m_defaultDateFormat;
	static QString m_defaultDateTimeFormat;
};

}}}

#endif // QF_CORE_MODEL_TABLEMODEL_H
