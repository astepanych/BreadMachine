#include "modellogdatatransfer.h"

ModelLogDataTransfer::ModelLogDataTransfer()
{
}

int ModelLogDataTransfer::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
           return 0;
    }
    return 3;
}

int ModelLogDataTransfer::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
           return 0;
    }
    return m_list.length();
}

QVariant ModelLogDataTransfer::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
           return QVariant();
       }

       switch (role) {
       case TextRole:
           return QVariant(QString("%1").arg(m_list.at(index.row())));
       default:
           return QVariant();
       }
}

QHash<int, QByteArray> ModelLogDataTransfer::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractTableModel::roleNames();
    roles[TextRole] = "textModel";
    return roles;
}

void ModelLogDataTransfer::clearModel()
{
    beginResetModel();
    m_list.clear();
    endResetModel();
}

Qt::ItemFlags ModelLogDataTransfer::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
           return Qt::NoItemFlags;
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void ModelLogDataTransfer::addItem(const QString &text)
{
    beginResetModel();
    m_list.insert(0,QString("%1: %2").arg(QTime::currentTime().toString("hh:mm:ss.zzz")).arg(text));
    endResetModel();
}

ModelList::ModelList()
{

}

int ModelList::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
           return 0;
    }
    return 1;
}

int ModelList::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
           return 0;
    }
    return m_list.length();
}

#include <qdebug.h>
QVariant ModelList::data(const QModelIndex &index, int role) const
{
    qDebug()<<__PRETTY_FUNCTION__;
    if (!index.isValid()) {
           return QVariant();
       }

       switch (role) {
       case TextRole:
           return QVariant(QString("%1").arg(m_list.at(index.row()).str));
       case OtherParam:
           return m_list.at(index.row()).otherParam;

       default:
           return QVariant();
       }
}

int ModelList::index(int role, QVariant value) {
    if(m_list.length() == 0)
        return -1;

    for(int i = 0; i < m_list.length(); i++) {
        switch (role) {
        case TextRole:
                if(m_list.at(i).str == value) {
                        return i;
                }
                break;
        case OtherParam:
            if(m_list.at(i).otherParam == value) {
                    return i;
            }
                break;
        default:
            return -1;
        }
    }
    return -1;
}

QHash<int, QByteArray> ModelList::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[TextRole] = "textModel";

    qDebug()<<roles;
    return roles;
}

void ModelList::clearModel()
{
    beginResetModel();
    m_list.clear();
    endResetModel();
}

void ModelList::addItem(const ElementModelList &text)
{
    beginResetModel();
    m_list.append(text);
    endResetModel();
}

void ModelList::addItem(const QString &text)
{

    beginResetModel();
    m_list.append({text,QVariant()});
    endResetModel();
}

void ModelList::addItems(const QList<ElementModelList> &text)
{
    beginResetModel();
    m_list.append(text);
    endResetModel();
}

const QList<ElementModelList> &ModelList::list() const
{
    return m_list;
}

void ModelList::setList(const  QList<ElementModelList> &newList)
{
    beginResetModel();
    m_list = newList;
    endResetModel();
}
