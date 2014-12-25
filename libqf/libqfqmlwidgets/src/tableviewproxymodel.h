#ifndef QF_QMLWIDGETS_TABLEVIEWPROXYMODEL_H
#define QF_QMLWIDGETS_TABLEVIEWPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace qf {
namespace qmlwidgets {

class TableViewProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
private:
	typedef QSortFilterProxyModel Super;
public:
	TableViewProxyModel(QObject *parent = nullptr);
	~TableViewProxyModel() Q_DECL_OVERRIDE;
public:
	Q_SLOT void setRowFilterString(const QString &s);
protected:
	QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const Q_DECL_OVERRIDE;
	bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const Q_DECL_OVERRIDE;
private:
	//static QTextCodec* tcASCII7();

	bool dataMatchFilter(const QVariant &d) const;
private:
	QByteArray m_rowFilterString;
};

}}

#endif // QF_QMLWIDGETS_TABLEVIEWPROXYMODEL_H