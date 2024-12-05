#ifndef MODELLOGDATATRANSFER_H
#define MODELLOGDATATRANSFER_H

#include <QAbstractListModel>
#include <QList>
#include <QTime>

class ModelLogDataTransfer : public QAbstractListModel
{
public:

    enum Roles {
          TimeRole = Qt::UserRole + 1,
          TextRole,
          RowNum
      };

    ModelLogDataTransfer();
    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;
    Q_INVOKABLE void clearModel();
    Qt::ItemFlags flags(const QModelIndex &index) const;
public slots:
    void addItem(const QString &text);

private:
    QList<QString> m_list;

};

struct ElementModelList{
    QString str;
    QVariant otherParam;
};

class ModelList : public QAbstractListModel
{
public:

    enum Roles {
          TextRole =  Qt::UserRole + 1,
          OtherParam
      };

    ModelList();
    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;
    Q_INVOKABLE void clearModel();
    void addItem(const ElementModelList &text);
    void addItem(const QString &text);
    void addItems(const QList<ElementModelList> &text);

    const  QList<ElementModelList> &list() const;
    void setList(const QList<ElementModelList> &newList);

    int index(int role, QVariant value);
private:

    QList<ElementModelList> m_list;

};

#endif // MODELLOGDATATRANSFER_H
